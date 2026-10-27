#include <basewidgets/Core/NativeStyle.h>
#include <basewidgets/Core/ButtonAppearance.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDynamicPropertyChangeEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTabBar>
#include <QSlider>
#include <cmath>

namespace basewidgets {
namespace {
ButtonAppearance appearance(const QWidget* widget)
{
    return widget ? static_cast<ButtonAppearance>(widget->property("bwAppearance").toInt())
                  : ButtonAppearance::Neutral;
}
bool selected(const QStyleOption* option, const QWidget* widget)
{
    return (option->state & QStyle::State_On) || (widget && widget->property("active").toBool());
}
bool accented(const QStyleOption* option, const QWidget* widget)
{
    const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option);
    return appearance(widget)==ButtonAppearance::Primary ||
        (button && (button->features & QStyleOptionButton::DefaultButton)) ||
        (widget && (widget->property("bwPrimary").toBool() || widget->property("primary").toBool()));
}
bool filled(const QStyleOption* option, const QWidget* widget)
{
    return accented(option,widget) || appearance(widget)==ButtonAppearance::Danger;
}
// Choose a white-on-color surface with a minimum 4.5:1 contrast ratio.
// Theme primaryColor stays unchanged for sliders/indicators; button fills are adjusted locally.
QColor buttonFill(const QStyleOption* option, const QWidget* widget)
{
    QColor color = appearance(widget)==ButtonAppearance::Danger
        ? ThemeTokens::instance()->danger() : option->palette.color(QPalette::Highlight);
    auto linear = [](double value) { return value<=.04045 ? value/12.92 : std::pow((value+.055)/1.055,2.4); };
    for (int i=0;i<20;++i) {
        const double luminance = .2126*linear(color.redF())+.7152*linear(color.greenF())+.0722*linear(color.blueF());
        if (1.05/(luminance+.05)>=4.5) break;
        color=color.darker(108);
    }
    return color;
}
QColor edge(const QStyleOption* option)
{
    return option->palette.color((option->state & QStyle::State_Enabled) &&
        (option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver))
        ? QPalette::Highlight : QPalette::Mid);
}
void rounded(QPainter* p, const QRectF& rect, const QColor& fill, const QColor& border,
             qreal radius = 4)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setBrush(fill);
    p->setPen(border.alpha() ? QPen(border, 1) : QPen(Qt::NoPen));
    p->drawRoundedRect(rect.adjusted(.5, .5, -.5, -.5), radius, radius);
    p->restore();
}
void arrow(QPainter* p, const QRect& rect, QStyle::PrimitiveElement direction, const QColor& color)
{
    const QPointF c = QRectF(rect).center();
    QPolygonF points;
    if (direction == QStyle::PE_IndicatorArrowUp)
        points << c + QPointF(-3, 2) << c + QPointF(0, -1) << c + QPointF(3, 2);
    else if (direction == QStyle::PE_IndicatorArrowDown)
        points << c + QPointF(-3, -2) << c + QPointF(0, 1) << c + QPointF(3, -2);
    else if (direction == QStyle::PE_IndicatorArrowLeft)
        points << c + QPointF(2, -3) << c + QPointF(-1, 0) << c + QPointF(2, 3);
    else
        points << c + QPointF(-2, -3) << c + QPointF(1, 0) << c + QPointF(-2, 3);
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->drawPolyline(points);
    p->restore();
}
}

NativeStyle::NativeStyle() : QProxyStyle(QStringLiteral("Fusion"))
{
    setObjectName(QStringLiteral("BaseWidgetsNative"));
}

void NativeStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<QAbstractButton*>(widget) || qobject_cast<QAbstractSlider*>(widget)
        || qobject_cast<QComboBox*>(widget) || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QLineEdit*>(widget) || qobject_cast<QTabBar*>(widget)) {
        if (!widget->testAttribute(Qt::WA_Hover)) {
            widget->setProperty("_bwNativeHover", true);
            widget->setAttribute(Qt::WA_Hover);
        }
    }
    widget->installEventFilter(this);
}

void NativeStyle::unpolish(QWidget* widget)
{
    widget->removeEventFilter(this);
    if (widget->property("_bwNativeHover").toBool()) {
        widget->setAttribute(Qt::WA_Hover, false);
        widget->setProperty("_bwNativeHover", QVariant());
    }
    QProxyStyle::unpolish(widget);
}

bool NativeStyle::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::DynamicPropertyChange) {
        const QByteArray name = static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName();
        if (name == "bwPrimary" || name == "primary" || name == "active" || name == "bwAppearance" || name == "bwWindowControl")
            if (auto* widget = qobject_cast<QWidget*>(object)) widget->update();
    }
    return QProxyStyle::eventFilter(object, event);
}

int NativeStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric) {
    case PM_DefaultFrameWidth: return 1;
    case PM_ButtonMargin: return 12;
    case PM_IndicatorWidth: case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth: case PM_ExclusiveIndicatorHeight: return 16;
    case PM_CheckBoxLabelSpacing: case PM_RadioButtonLabelSpacing: return 6;
    case PM_SliderLength: case PM_SliderControlThickness: return 22;
    case PM_SliderThickness: return 26;
    case PM_ScrollBarExtent: return 14;
    case PM_ScrollBarSliderMin: return 24;
    case PM_TabBarTabHSpace: return 24;
    case PM_TabBarTabVSpace: return 12;
    default: return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

int NativeStyle::styleHint(StyleHint hint, const QStyleOption* option,
                           const QWidget* widget, QStyleHintReturn* returnData) const
{
    if (hint == SH_DialogButtonBox_ButtonsHaveIcons) return 0;
    return QProxyStyle::styleHint(hint,option,widget,returnData);
}

QSize NativeStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                  const QSize& size, const QWidget* widget) const
{
    QSize result = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == CT_PushButton || type == CT_ToolButton || type == CT_LineEdit
        || type == CT_ComboBox || type == CT_SpinBox)
        result.setHeight(qMax(result.height(), option->fontMetrics.height() + 12));
    if (type == CT_ToolButton) {
        const auto* tool=qstyleoption_cast<const QStyleOptionToolButton*>(option);
        // Reserve a full-height trailing arrow slot, including large fonts and icon-only buttons.
        if (tool && (tool->features & QStyleOptionToolButton::HasMenu))
            result.rwidth() += qMax(20,option->fontMetrics.height()+4);
    }
    return result;
}

QRect NativeStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                                 SubControl subControl, const QWidget* widget) const
{
    if (control == CC_ToolButton) {
        const auto* tool=qstyleoption_cast<const QStyleOptionToolButton*>(option);
        if (tool && (tool->features & QStyleOptionToolButton::MenuButtonPopup)) {
            const int width=qMin(option->rect.width(),qMax(20,option->fontMetrics.height()+4));
            QRect logical=option->rect;
            if (subControl==SC_ToolButtonMenu) logical.setLeft(logical.right()-width+1);
            else if (subControl==SC_ToolButton) logical.setRight(logical.right()-width);
            else return QRect();
            return visualRect(option->direction,option->rect,logical);
        }
    }
    const QRect original = QProxyStyle::subControlRect(control, option, subControl, widget);
    const auto* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
    if (control != CC_Slider || !slider) return original;
    const bool horizontal = slider->orientation == Qt::Horizontal;
    QRect handle = QProxyStyle::subControlRect(control, option, SC_SliderHandle, widget);
    if (horizontal) {
        handle.setHeight(qMin(22, option->rect.height()));
        handle.moveCenter(QPoint(handle.center().x(), option->rect.center().y()));
    } else {
        handle.setWidth(qMin(22, option->rect.width()));
        handle.moveCenter(QPoint(option->rect.center().x(), handle.center().y()));
    }
    if (subControl == SC_SliderHandle) return handle;
    if (subControl == SC_SliderGroove) {
        const QRect r = option->rect;
        return horizontal ? QRect(r.left() + 11, handle.center().y() - 2, qMax(0, r.width() - 22), 4)
                          : QRect(handle.center().x() - 2, r.top() + 11, 4, qMax(0, r.height() - 22));
    }
    return original;
}

QStyle::SubControl NativeStyle::hitTestComplexControl(ComplexControl control,
    const QStyleOptionComplex* option, const QPoint& point, const QWidget* widget) const
{
    if (control == CC_ToolButton) {
        const auto* tool=qstyleoption_cast<const QStyleOptionToolButton*>(option);
        if (tool && (tool->features & QStyleOptionToolButton::MenuButtonPopup)) {
            if (subControlRect(control,option,SC_ToolButtonMenu,widget).contains(point)) return SC_ToolButtonMenu;
            if (subControlRect(control,option,SC_ToolButton,widget).contains(point)) return SC_ToolButton;
            return SC_None;
        }
    }
    if (control == CC_Slider) {
        if (subControlRect(control, option, SC_SliderHandle, widget).contains(point)) return SC_SliderHandle;
        if (subControlRect(control, option, SC_SliderGroove, widget).contains(point)) return SC_SliderGroove;
        return SC_None;
    }
    return QProxyStyle::hitTestComplexControl(control, option, point, widget);
}

void NativeStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                QPainter* p, const QWidget* widget) const
{
    const QPalette& pal = option->palette;
    const bool enabled = option->state & State_Enabled;
    const bool hover = enabled && (option->state & State_MouseOver);
    const bool down = enabled && (option->state & State_Sunken);
    switch (element) {
    case PE_PanelButtonCommand: case PE_PanelButtonTool: {
        const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option);
        const bool strong = filled(option,widget);
        const bool checked = selected(option,widget);
        const bool flat = appearance(widget)==ButtonAppearance::Subtle ||
            (element==PE_PanelButtonTool && (option->state & State_AutoRaise)) ||
            (button && (button->features & QStyleOptionButton::Flat));
        QColor fill = flat ? QColor(Qt::transparent) : pal.color(QPalette::Button);
        if (enabled && strong) fill=buttonFill(option,widget);
        else if (enabled && checked) {
            // Selected controls use a quiet tinted surface; primary actions keep the strongest weight.
            fill=pal.color(QPalette::Highlight); fill.setAlpha(45);
        }
        if (enabled && down) fill = strong ? buttonFill(option,widget).darker(115) : pal.color(QPalette::Dark);
        else if (hover) fill = strong ? buttonFill(option,widget).darker(105) : pal.color(QPalette::Light);
        QColor border(Qt::transparent);
        if (appearance(widget)==ButtonAppearance::Outline) border=pal.color(QPalette::Mid);
        if (checked) border=pal.color(QPalette::Highlight);
        // Mouse clicks do not leave every button outlined. Keyboard focus must remain visible.
        if (enabled && (option->state & State_HasFocus) && (option->state & State_KeyboardFocusChange))
            border=pal.color(strong ? QPalette::HighlightedText : QPalette::Highlight);
        rounded(p, option->rect, fill, border);
        return;
    }
    case PE_IndicatorCheckBox: case PE_IndicatorRadioButton: case PE_IndicatorItemViewItemCheck: {
        const bool radio = element == PE_IndicatorRadioButton;
        const bool checked = option->state & State_On;
        const bool partial = option->state & State_NoChange;
        const QRectF r = QRectF(option->rect).adjusted(1, 1, -1, -1);
        const QColor fill = enabled && (checked || partial) ? pal.color(QPalette::Highlight)
                                                           : pal.color(QPalette::Button);
        rounded(p, r, fill, enabled && (checked || partial) ? fill : edge(option), radio ? r.height()/2 : 3);
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(enabled ? pal.color(QPalette::HighlightedText) : pal.color(QPalette::Text),
                       1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (partial && !radio) {
            p->drawLine(QPointF(r.left()+3, r.center().y()), QPointF(r.right()-3, r.center().y()));
        } else if (checked && radio) {
            p->setBrush(p->pen().color()); p->setPen(Qt::NoPen);
            p->drawEllipse(r.center(), 2.5, 2.5);
        } else if (checked) {
            QPolygonF tick;
            tick << QPointF(r.left()+3, r.center().y()) << QPointF(r.center().x()-1, r.bottom()-3)
                 << QPointF(r.right()-2, r.top()+3);
            p->drawPolyline(tick);
        }
        p->restore();
        return;
    }
    case PE_FrameFocusRect:
        if (enabled && (option->state & State_KeyboardFocusChange))
            rounded(p, option->rect, Qt::transparent, pal.color(QPalette::Highlight), 3);
        return;
    case PE_PanelLineEdit: {
        const auto* frame = qstyleoption_cast<const QStyleOptionFrame*>(option);
        rounded(p, option->rect, pal.color(QPalette::Base),
                frame && frame->lineWidth == 0 ? QColor(Qt::transparent) : edge(option));
        return;
    }
    case PE_FrameLineEdit: case PE_Frame: case PE_FrameGroupBox: case PE_FrameTabWidget:
        rounded(p, option->rect, Qt::transparent, edge(option));
        return;
    case PE_IndicatorArrowUp: case PE_IndicatorArrowDown:
    case PE_IndicatorArrowLeft: case PE_IndicatorArrowRight:
        arrow(p, option->rect, element, pal.color(QPalette::ButtonText));
        return;
    default:
        QProxyStyle::drawPrimitive(element, option, p, widget);
    }
}

void NativeStyle::drawControl(ControlElement element, const QStyleOption* option,
                              QPainter* p, const QWidget* widget) const
{
    // Remove hover from both shape and label paths. Selection and keyboard focus remain intact.
    QStyleOptionTab quietTab;
    if (element==CE_TabBarTab || element==CE_TabBarTabShape || element==CE_TabBarTabLabel) {
        if (const auto* tab=qstyleoption_cast<const QStyleOptionTab*>(option)) {
            if (!widget || !widget->property("bwTabHoverEnabled").toBool()) {
                quietTab=*tab;
                quietTab.state &= ~State_MouseOver;
                option=&quietTab;
            }
        }
    }
    const QPalette& pal = option->palette;
    if (element == CE_PushButtonBevel) {
        drawPrimitive(PE_PanelButtonCommand, option, p, widget);
        const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (button && (button->features & QStyleOptionButton::HasMenu)) {
            const int width = pixelMetric(PM_MenuButtonIndicator, option, widget);
            QRect r(option->rect.right()-width, option->rect.top(), width, option->rect.height());
            arrow(p, visualRect(option->direction, option->rect, r), PE_IndicatorArrowDown,
                  filled(option, widget) ? pal.color(QPalette::HighlightedText) : pal.color(QPalette::ButtonText));
        }
        return;
    }
    if (element == CE_PushButtonLabel) {
        if (const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            QStyleOptionButton copy(*button);
            if ((option->state & State_Enabled) && filled(option, widget))
                copy.palette.setColor(QPalette::ButtonText, pal.color(QPalette::HighlightedText));
            QProxyStyle::drawControl(element, &copy, p, widget);
            return;
        }
    }
    if (element == CE_ToolButtonLabel) {
        if (const auto* button = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
            QStyleOptionToolButton copy(*button);
            if ((option->state & State_Enabled) && filled(option, widget))
                copy.palette.setColor(QPalette::ButtonText, pal.color(QPalette::HighlightedText));
            QProxyStyle::drawControl(element, &copy, p, widget);
            return;
        }
    }
    if (element == CE_TabBarTabShape) {
        if (const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
            if (option->state & (State_Selected | State_MouseOver))
                rounded(p, option->rect, pal.color(QPalette::Button), Qt::transparent);
            if (option->state & State_Selected) {
                QRect r = option->rect;
                switch (tab->shape) {
                case QTabBar::RoundedSouth: case QTabBar::TriangularSouth: r.setHeight(2); break;
                case QTabBar::RoundedWest: case QTabBar::TriangularWest: r.setLeft(r.right()-1); break;
                case QTabBar::RoundedEast: case QTabBar::TriangularEast: r.setWidth(2); break;
                default: r.setTop(r.bottom()-1); break;
                }
                p->fillRect(r, pal.highlight());
            }
            return;
        }
    }
    if (element == CE_ProgressBarGroove) {
        rounded(p, option->rect, pal.color(QPalette::AlternateBase), Qt::transparent, 3);
        return;
    }
    if (element == CE_ProgressBarContents) {
        if (const auto* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
            // Fusion owns the indeterminate animation, including timer lifecycle.
            if (bar->minimum == 0 && bar->maximum == 0) {
                QProxyStyle::drawControl(element, option, p, widget);
                return;
            }
            if (bar->progress < bar->minimum) return; // reset state
            const double range = double(bar->maximum) - double(bar->minimum);
            const double fraction = range > 0 ? qBound(0., (double(bar->progress)-bar->minimum)/range, 1.) : 1.;
            QRectF fill(option->rect);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const bool horizontal = bar->state & State_Horizontal;
#else
            const bool horizontal = bar->orientation == Qt::Horizontal;
#endif
            const bool reversed = horizontal ? ((bar->direction == Qt::RightToLeft) != bar->invertedAppearance)
                                             : !bar->invertedAppearance;
            if (horizontal) {
                fill.setWidth(fill.width()*fraction);
                if (reversed) fill.moveRight(option->rect.right()+1);
            } else {
                fill.setHeight(fill.height()*fraction);
                if (reversed) fill.moveBottom(option->rect.bottom()+1);
            }
            if (fill.width() > 0 && fill.height() > 0)
                rounded(p, fill, pal.color(QPalette::Highlight), Qt::transparent, 3);
            return;
        }
    }
    if (element == CE_ScrollBarAddPage || element == CE_ScrollBarSubPage) {
        p->fillRect(option->rect, pal.window());
        return;
    }
    if (element == CE_ScrollBarSlider) {
        const bool hover = (option->state & State_Enabled) && (option->state & (State_MouseOver | State_Sunken));
        rounded(p, option->rect.adjusted(3, 3, -3, -3),
                pal.color(hover ? QPalette::Highlight : QPalette::Mid), Qt::transparent, 4);
        return;
    }
    QProxyStyle::drawControl(element, option, p, widget);
}

void NativeStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                     QPainter* p, const QWidget* widget) const
{
    const QPalette& pal = option->palette;
    if (control == CC_Slider) {
        const auto* slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
        if (!slider) return;
        const QRect groove = subControlRect(control, option, SC_SliderGroove, widget);
        const QRect handle = subControlRect(control, option, SC_SliderHandle, widget);
        if ((slider->subControls & SC_SliderTickmarks) && slider->tickPosition != QSlider::NoTicks) {
            const bool horizontal = slider->orientation == Qt::Horizontal;
            const qint64 range = qint64(slider->maximum) - slider->minimum;
            const int span = horizontal ? option->rect.width() : option->rect.height();
            // Bound work for very large ranges and keep ticks distinct in logical pixels.
            qint64 interval = slider->tickInterval > 0 ? slider->tickInterval : qMax(1, slider->singleStep);
            interval = qMax(interval, (range + qMax(1, span/4) - 1) / qMax(1, span/4));
            p->save(); p->setPen(pal.color(QPalette::PlaceholderText));
            for (qint64 value = slider->minimum; value <= slider->maximum; value += interval) {
                QStyleOptionSlider tick(*slider); tick.sliderPosition = int(value);
                const QPoint c = subControlRect(control, &tick, SC_SliderHandle, widget).center();
                if (horizontal) {
                    if (slider->tickPosition & QSlider::TicksAbove)
                        p->drawLine(c.x(), option->rect.top(), c.x(), option->rect.top()+2);
                    if (slider->tickPosition & QSlider::TicksBelow)
                        p->drawLine(c.x(), option->rect.bottom()-2, c.x(), option->rect.bottom());
                } else {
                    if (slider->tickPosition & QSlider::TicksAbove)
                        p->drawLine(option->rect.left(), c.y(), option->rect.left()+2, c.y());
                    if (slider->tickPosition & QSlider::TicksBelow)
                        p->drawLine(option->rect.right()-2, c.y(), option->rect.right(), c.y());
                }
            }
            p->restore();
        }
        if (slider->subControls & SC_SliderGroove) {
            rounded(p, groove, pal.color(QPalette::Mid), Qt::transparent, 2);
            QRect fill = groove;
            if (slider->orientation == Qt::Horizontal) {
                if (slider->upsideDown) fill.setLeft(handle.center().x());
                else fill.setRight(handle.center().x());
            } else {
                if (slider->upsideDown) fill.setTop(handle.center().y());
                else fill.setBottom(handle.center().y());
            }
            rounded(p, fill, pal.color(QPalette::Highlight), Qt::transparent, 2);
        }
        if (slider->subControls & SC_SliderHandle) {
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            if ((option->state & State_Enabled) && (option->state & (State_MouseOver | State_HasFocus | State_Sunken))) {
                QColor halo = pal.color(QPalette::Highlight); halo.setAlpha(45);
                p->setPen(Qt::NoPen); p->setBrush(halo);
                p->drawEllipse(QRectF(handle).adjusted(.5, .5, -.5, -.5));
            }
            p->setPen(QPen(pal.color(QPalette::Highlight), 2));
            p->setBrush(pal.base());
            p->drawEllipse(QRectF(handle).adjusted(4, 4, -4, -4));
            p->restore();
        }
        return;
    }
    if (control == CC_ScrollBar) {
        const auto* scroll = qstyleoption_cast<const QStyleOptionSlider*>(option);
        if (!scroll) return;
        p->fillRect(option->rect, pal.window());
        for (SubControl sub : {SC_ScrollBarSubLine, SC_ScrollBarAddLine, SC_ScrollBarSlider}) {
            if (!(scroll->subControls & sub)) continue;
            QStyleOptionSlider part(*scroll);
            part.rect = subControlRect(control, option, sub, widget);
            if (!(scroll->activeSubControls & sub)) part.state &= ~(State_MouseOver | State_Sunken);
            if (sub == SC_ScrollBarSlider) {
                if (scroll->maximum > scroll->minimum) drawControl(CE_ScrollBarSlider, &part, p, widget);
            } else {
                if (part.state & (State_MouseOver | State_Sunken))
                    rounded(p, part.rect, pal.color(QPalette::Button), Qt::transparent, 2);
                const bool horizontal = scroll->orientation == Qt::Horizontal;
                const bool increase = sub == SC_ScrollBarAddLine;
                const bool right = increase != (scroll->direction == Qt::RightToLeft);
                arrow(p, part.rect, horizontal ? (right ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft)
                                               : (increase ? PE_IndicatorArrowDown : PE_IndicatorArrowUp),
                      pal.color(QPalette::ButtonText));
            }
        }
        return;
    }
    if (control == CC_ToolButton) {
        const auto* tool = qstyleoption_cast<const QStyleOptionToolButton*>(option);
        if (!tool) return;
        const QString windowControl = widget ? widget->property("bwWindowControl").toString() : QString();
        if (!windowControl.isEmpty()) {
            const bool enabled = tool->state & State_Enabled;
            const bool hover = enabled && (tool->state & State_MouseOver);
            const bool down = enabled && (tool->state & State_Sunken);
            const bool close = windowControl=="close";
            QColor background(Qt::transparent);
            QColor foreground=pal.color(QPalette::ButtonText);
            if (hover || down) {
                background = close ? buttonFill(option,widget) : pal.color(down ? QPalette::Dark : QPalette::Light);
                if (close) foreground=Qt::white;
                if (close && down) background=background.darker(120);
            }
            p->save();
            p->fillRect(option->rect,background);
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(QPen(foreground,1.25,Qt::SolidLine,Qt::SquareCap,Qt::MiterJoin));
            const QPointF center=QRectF(option->rect).center();
            const QRectF glyph(center.x()-4.5,center.y()-4.5,9,9);
            if (close) {
                p->drawLine(glyph.topLeft(),glyph.bottomRight());
                p->drawLine(glyph.topRight(),glyph.bottomLeft());
            } else if (windowControl=="minimize") {
                p->drawLine(QPointF(glyph.left(),center.y()+3),QPointF(glyph.right(),center.y()+3));
            } else if (windowControl=="restore") {
                p->drawRect(glyph.adjusted(2,-1,1,-2));
                p->fillRect(glyph,hover || down ? background : pal.color(QPalette::Window));
                p->drawRect(glyph);
            } else p->drawRect(glyph);
            if ((tool->state & State_HasFocus) && (tool->state & State_KeyboardFocusChange))
                rounded(p,option->rect.adjusted(2,2,-2,-2),Qt::transparent,foreground,1);
            p->restore();
            return;
        }
        QStyleOptionToolButton panel(*tool);
        drawPrimitive(PE_PanelButtonTool, &panel, p, widget);
        const bool hasMenu=tool->features & QStyleOptionToolButton::HasMenu;
        const bool split=tool->features & QStyleOptionToolButton::MenuButtonPopup;
        QRect labelRect=subControlRect(control,option,SC_ToolButton,widget);
        QRect menuRect;
        if (hasMenu) {
            if (split) menuRect=subControlRect(control,option,SC_ToolButtonMenu,widget);
            else {
                const int width=qMin(option->rect.width(),qMax(20,option->fontMetrics.height()+4));
                QRect logical=option->rect;
                logical.setLeft(logical.right()-width+1);
                menuRect=visualRect(option->direction,option->rect,logical);
                logical=option->rect;
                logical.setRight(logical.right()-width);
                labelRect=visualRect(option->direction,option->rect,logical);
            }
        }
        if (tool->subControls & SC_ToolButton) {
            QStyleOptionToolButton label(*tool);
            label.rect=labelRect.adjusted(4,3,-4,-3);
            // We own the menu arrow; the base style must only lay out the text/icon.
            label.features &= ~(QStyleOptionToolButton::HasMenu | QStyleOptionToolButton::MenuButtonPopup);
            if (!(tool->activeSubControls & SC_ToolButton)) label.state &= ~State_Sunken;
            drawControl(CE_ToolButtonLabel,&label,p,widget);
        }
        if (hasMenu && (!split || (tool->subControls & SC_ToolButtonMenu))) {
            if (split) {
                p->save(); p->setPen(pal.color(QPalette::Mid));
                const int x=tool->direction==Qt::RightToLeft ? menuRect.right() : menuRect.left();
                p->drawLine(x,menuRect.top()+6,x,menuRect.bottom()-6); p->restore();
            }
            arrow(p,menuRect,PE_IndicatorArrowDown,
                  filled(option,widget) && (option->state & State_Enabled)
                      ? pal.color(QPalette::HighlightedText) : pal.color(QPalette::ButtonText));
        }
        return;
    }
    if (control == CC_ComboBox) {
        if (const auto* combo = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            if (combo->subControls & SC_ComboBoxFrame)
                rounded(p, option->rect, pal.color(combo->editable ? QPalette::Base : QPalette::Button),
                        combo->frame ? edge(option) : QColor(Qt::transparent));
            if (combo->subControls & SC_ComboBoxArrow)
                arrow(p, subControlRect(control, option, SC_ComboBoxArrow, widget),
                      PE_IndicatorArrowDown, pal.color(QPalette::ButtonText));
            return;
        }
    }
    if (control == CC_SpinBox) {
        if (const auto* spin = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
            if (spin->subControls & SC_SpinBoxFrame)
                rounded(p, option->rect, pal.color(QPalette::Base), spin->frame ? edge(option) : QColor(Qt::transparent));
            for (SubControl sub : {SC_SpinBoxUp, SC_SpinBoxDown}) {
                if (!(spin->subControls & sub) || spin->buttonSymbols == QAbstractSpinBox::NoButtons) continue;
                const QRect r = subControlRect(control, option, sub, widget);
                const bool enabled = (spin->state & State_Enabled) && (spin->stepEnabled &
                    (sub == SC_SpinBoxUp ? QAbstractSpinBox::StepUpEnabled : QAbstractSpinBox::StepDownEnabled));
                if (enabled && (spin->activeSubControls & sub) && (spin->state & (State_MouseOver | State_Sunken)))
                    rounded(p, r, pal.color(QPalette::Light), Qt::transparent, 2);
                const QColor color = pal.color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText);
                if (spin->buttonSymbols == QAbstractSpinBox::PlusMinus) {
                    p->save(); p->setPen(QPen(color, 1.5));
                    p->drawLine(r.center()+QPoint(-3,0), r.center()+QPoint(3,0));
                    if (sub == SC_SpinBoxUp) p->drawLine(r.center()+QPoint(0,-3), r.center()+QPoint(0,3));
                    p->restore();
                } else arrow(p, r, sub == SC_SpinBoxUp ? PE_IndicatorArrowUp : PE_IndicatorArrowDown, color);
            }
            return;
        }
    }
    QProxyStyle::drawComplexControl(control, option, p, widget);
}
} // namespace basewidgets
