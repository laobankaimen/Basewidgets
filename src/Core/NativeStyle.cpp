#include <basewidgets/Core/NativeStyle.h>
#include <basewidgets/Core/ButtonAppearance.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
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
// Keep configured button text readable (4.5:1) on filled primary/danger surfaces.
// Theme primaryColor stays unchanged for sliders/indicators; button fills are adjusted locally.
QColor buttonFill(const QStyleOption* option, const QWidget* widget)
{
    const auto* tokens = ThemeTokens::instance();
    QColor color = appearance(widget)==ButtonAppearance::Danger
        ? tokens->dangerButton() : tokens->primaryButton();
    auto linear = [](double value) { return value<=.04045 ? value/12.92 : std::pow((value+.055)/1.055,2.4); };
    const QColor ink = option->palette.color(QPalette::BrightText);
    const double inkLuminance = .2126*linear(ink.redF())+.7152*linear(ink.greenF())+.0722*linear(ink.blueF());
    // Custom themes may choose dark button text. Move the surface toward the
    // opposite luminance first. Derive interaction states from this corrected base
    // so contrast correction cannot collapse normal and hover to the same color.
    const int target = inkLuminance > .179 ? 0 : 255;
    for (int i=0;i<64;++i) {
        const double luminance = .2126*linear(color.redF())+.7152*linear(color.greenF())+.0722*linear(color.blueF());
        if ((qMax(inkLuminance,luminance)+.05)/(qMin(inkLuminance,luminance)+.05)>=4.5) break;
        auto step = [target](int value) { return target == 0 ? qMax(0,value-qMax(1,qRound(value*.08)))
                                                           : qMin(255,value+qMax(1,qRound((255-value)*.08))); };
        color.setRgb(step(color.red()),step(color.green()),step(color.blue()));
    }
    if(option->state & QStyle::State_Enabled) {
        const qreal amount=(option->state & QStyle::State_Sunken)?0.30:
                           (option->state & QStyle::State_MouseOver)?0.16:0.0;
        auto shift=[target,amount](int channel) { return qRound(channel+(target-channel)*amount); };
        color.setRgb(shift(color.red()),shift(color.green()),shift(color.blue()));
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
        if (name == "bwPrimary" || name == "primary" || name == "active" || name == "bwAppearance" || name == "bwWindowControl" || name == "bwFrameHoverEnabled")
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
    case PM_TabBarTabHSpace: return 28;
    case PM_TabBarTabVSpace: return 16;
    case PM_ToolBarItemSpacing: return 6;
    case PM_ToolBarItemMargin: return 3;
    case PM_ToolBarFrameWidth: return 0;
    case PM_ToolBarSeparatorExtent: return 16;
    case PM_ToolBarExtensionExtent: return 28;
    case PM_MenuBarPanelWidth: return 0;
    case PM_MenuBarItemSpacing: return 2;
    case PM_MenuBarHMargin: return 8;
    case PM_MenuBarVMargin: return 2;
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
    if(type==CT_MenuBarItem) {
        if(const auto* item=qstyleoption_cast<const QStyleOptionMenuItem*>(option))
            result.setWidth(qMax(result.width(),option->fontMetrics.size(Qt::TextShowMnemonic,item->text).width()+20));
        result.setHeight(qMax(result.height(),option->fontMetrics.height()+8));
    }
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

QIcon NativeStyle::standardIcon(StandardPixmap icon,const QStyleOption* option,const QWidget* widget) const
{
    if(icon==SP_ToolBarHorizontalExtensionButton || icon==SP_ToolBarVerticalExtensionButton)
        return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/titlebar/more.svg");
    return QProxyStyle::standardIcon(icon,option,widget);
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
        if (enabled && down) fill = strong ? buttonFill(option,widget) : pal.color(QPalette::Dark);
        else if (hover) fill = strong ? buttonFill(option,widget) : pal.color(QPalette::Light);
        QColor border(Qt::transparent);
        if(strong && (hover || down)) {
            const QColor ink=pal.color(QPalette::BrightText);
            const qreal amount=down?.22:.38;
            border=QColor(qRound(fill.red()+(ink.red()-fill.red())*amount),
                          qRound(fill.green()+(ink.green()-fill.green())*amount),
                          qRound(fill.blue()+(ink.blue()-fill.blue())*amount));
        }
        if (appearance(widget)==ButtonAppearance::Outline) border=pal.color(QPalette::Mid);
        if (checked) border=pal.color(QPalette::Highlight);
        // Mouse clicks do not leave every button outlined. Keyboard focus must remain visible.
        if (enabled && (option->state & State_HasFocus) && (option->state & State_KeyboardFocusChange))
            border=pal.color(strong ? QPalette::BrightText : QPalette::Highlight);
        rounded(p, option->rect, fill, border);
        return;
    }
    case PE_IndicatorCheckBox: case PE_IndicatorRadioButton: case PE_IndicatorItemViewItemCheck: {
        const bool radio = element == PE_IndicatorRadioButton;
        const bool checked = option->state & State_On;
        const bool partial = option->state & State_NoChange;
        // Shared normalized geometry for checkboxes, item checks and radio buttons.
        // Proportions remain stable at fractional DPR and custom indicator sizes.
        const qreal side=qMin(option->rect.width(),option->rect.height());
        const QRectF box(option->rect.x()+(option->rect.width()-side)/2.,option->rect.y()+(option->rect.height()-side)/2.,side,side);
        const QRectF r=box.adjusted(1,1,-1,-1);
        const QColor fill=enabled&&(checked||partial)?pal.color(QPalette::Highlight):pal.color(QPalette::Base);
        p->save(); p->setRenderHint(QPainter::Antialiasing);
        p->setBrush(fill);
        p->setPen(QPen(enabled&&(checked||partial)?fill:edge(option),1.0));
        if(radio)p->drawEllipse(r.adjusted(.5,.5,-.5,-.5));
        else p->drawRoundedRect(r.adjusted(.5,.5,-.5,-.5),2.5,2.5);
        const QColor ink=enabled?pal.color(QPalette::HighlightedText):pal.color(QPalette::Text);
        p->setPen(QPen(ink,qMax(1.25,side*.1),Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        auto point=[&](qreal x,qreal y) { return QPointF(box.left()+side*x,box.top()+side*y); };
        if(partial&&!radio) {
            p->drawLine(point(.3,.5),point(.7,.5));
        } else if(checked&&radio) {
            p->setBrush(ink); p->setPen(Qt::NoPen); p->drawEllipse(box.center(),side*.14,side*.14);
        } else if(checked) {
            QPolygonF tick; tick<<point(.27,.5)<<point(.43,.66)<<point(.74,.34); p->drawPolyline(tick);
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
    case PE_FrameLineEdit:
        rounded(p,option->rect,Qt::transparent,edge(option));
        return;
    case PE_Frame: case PE_FrameGroupBox: case PE_FrameTabWidget: {
        // Containers are not inputs. Hovering a page/card must not outline the entire
        // content area. Explicit opt-in is independent of tab-label hover preference.
        const bool hoverBorder=enabled && widget && widget->property("bwFrameHoverEnabled").toBool() && (option->state&State_MouseOver);
        const bool keyboardFocus=enabled && (option->state&State_HasFocus) && (option->state&State_KeyboardFocusChange);
        rounded(p,option->rect,Qt::transparent,(hoverBorder||keyboardFocus)?pal.color(QPalette::Highlight):ThemeTokens::instance()->borderLight());
        return;
    }
    case PE_PanelMenuBar:
        p->fillRect(option->rect,ThemeTokens::instance()->bg0());
        return;
    case PE_PanelToolBar:
        p->fillRect(option->rect,ThemeTokens::instance()->bg1());
        return;
    case PE_IndicatorToolBarSeparator: {
        p->save(); p->setPen(ThemeTokens::instance()->borderLight());
        const QRect r=option->rect;
        if(option->state&State_Horizontal)p->drawLine(r.center().x(),r.top()+8,r.center().x(),r.bottom()-8);
        else p->drawLine(r.left()+8,r.center().y(),r.right()-8,r.center().y());
        p->restore(); return;
    }
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
                  filled(option, widget) && (option->state & State_Enabled) ? pal.color(QPalette::BrightText) : pal.color(QPalette::ButtonText));
        }
        return;
    }
    if (element == CE_PushButtonLabel) {
        if (const auto* button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            QStyleOptionButton copy(*button);
            if ((option->state & State_Enabled) && filled(option, widget))
                copy.palette.setColor(QPalette::ButtonText, pal.color(QPalette::BrightText));
            QProxyStyle::drawControl(element, &copy, p, widget);
            return;
        }
    }
    if (element == CE_ToolButtonLabel) {
        if (const auto* button = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
            QStyleOptionToolButton copy(*button);
            if ((option->state & State_Enabled) && filled(option, widget))
                copy.palette.setColor(QPalette::ButtonText, pal.color(QPalette::BrightText));
            if (!copy.icon.isNull() && (copy.toolButtonStyle==Qt::ToolButtonIconOnly || copy.toolButtonStyle==Qt::ToolButtonTextBesideIcon)) {
                // Paint vector engines into the actual painter, including fractional-DPR
                // offscreen targets, rather than rasterizing at the widget's old screen DPR.
                const QSize iconSize=copy.iconSize.boundedTo(copy.rect.size());
                const QIcon::Mode mode=!(copy.state&State_Enabled)?QIcon::Disabled:(copy.state&State_MouseOver)?QIcon::Active:QIcon::Normal;
                const QIcon::State state=(copy.state&State_On)?QIcon::On:QIcon::Off;
                if(copy.toolButtonStyle==Qt::ToolButtonIconOnly) {
                    copy.icon.paint(p,alignedRect(copy.direction,Qt::AlignCenter,iconSize,copy.rect),Qt::AlignCenter,mode,state);
                } else {
                    const int gap=copy.text.isEmpty()?0:6;
                    const int textWidth=copy.fontMetrics.size(Qt::TextShowMnemonic,copy.text).width();
                    const int total=qMin(copy.rect.width(),iconSize.width()+gap+textWidth);
                    const int left=copy.rect.left()+(copy.rect.width()-total)/2;
                    const QRect logicalIcon(left,copy.rect.top()+(copy.rect.height()-iconSize.height())/2,iconSize.width(),iconSize.height());
                    const QRect logicalText(left+iconSize.width()+gap,copy.rect.top(),qMax(0,total-iconSize.width()-gap),copy.rect.height());
                    copy.icon.paint(p,visualRect(copy.direction,copy.rect,logicalIcon),Qt::AlignCenter,mode,state);
                    p->save(); p->setFont(copy.font);
                    const QString text=copy.fontMetrics.elidedText(copy.text,Qt::ElideRight,logicalText.width(),Qt::TextShowMnemonic);
                    drawItemText(p,visualRect(copy.direction,copy.rect,logicalText),Qt::AlignVCenter|(copy.direction==Qt::RightToLeft?Qt::AlignRight:Qt::AlignLeft)|Qt::TextSingleLine|Qt::TextShowMnemonic,
                                 copy.palette,copy.state&State_Enabled,text,QPalette::ButtonText);
                    p->restore();
                }
                return;
            }
            QProxyStyle::drawControl(element, &copy, p, widget);
            return;
        }
    }
    if(element==CE_MenuBarEmptyArea) { p->fillRect(option->rect,ThemeTokens::instance()->bg0()); return; }
    if(element==CE_MenuBarItem) {
        if(const auto* item=qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            p->fillRect(item->rect,ThemeTokens::instance()->bg0());
            if(item->state&(State_Selected|State_Sunken))rounded(p,item->rect.adjusted(1,1,-1,-1),pal.color(QPalette::Button),Qt::transparent,4);
            if(!item->icon.isNull()) {
                const QIcon::Mode mode=!(item->state&State_Enabled)?QIcon::Disabled:(item->state&State_Selected)?QIcon::Active:QIcon::Normal;
                item->icon.paint(p,item->rect.adjusted(8,4,-8,-4),Qt::AlignCenter,mode); return;
            }
            p->save(); p->setFont(item->font);
            drawItemText(p,item->rect.adjusted(10,3,-10,-3),Qt::AlignCenter|Qt::TextShowMnemonic,pal,item->state&State_Enabled,item->text,QPalette::WindowText);
            p->restore(); return;
        }
    }
    if(element==CE_ToolBar) {
        p->fillRect(option->rect,ThemeTokens::instance()->bg1());
        p->save(); p->setPen(ThemeTokens::instance()->borderLight());
        if(option->state&State_Horizontal)p->drawLine(option->rect.bottomLeft(),option->rect.bottomRight());
        else p->drawLine(option->rect.topRight(),option->rect.bottomRight());
        p->restore(); return;
    }
    if(element==CE_TabBarTabLabel) {
        if(const auto* tab=qstyleoption_cast<const QStyleOptionTab*>(option)) {
            QStyleOptionTab label(*tab);
            label.palette.setColor(QPalette::WindowText,!(tab->state&State_Enabled)?ThemeTokens::instance()->textDisabled():
                                   (tab->state&State_Selected)?ThemeTokens::instance()->textPrimary():ThemeTokens::instance()->textSecondary());
            QProxyStyle::drawControl(element,&label,p,widget); return;
        }
    }
    if (element == CE_TabBarTabShape) {
        if (const auto* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
            const bool active=option->state&State_Selected;
            if(active || (option->state&State_MouseOver)) {
                const QRect shape=option->rect.adjusted(3,2,-3,-2);
                rounded(p,shape,active?ThemeTokens::instance()->bg1():ThemeTokens::instance()->bgHover(),Qt::transparent,5);
            }
            if(active) {
                QRect r=option->rect;
                switch(tab->shape) {
                case QTabBar::RoundedSouth: case QTabBar::TriangularSouth: r.setHeight(3); r.adjust(10,0,-10,0); break;
                case QTabBar::RoundedWest: case QTabBar::TriangularWest: r.setLeft(r.right()-2); r.adjust(0,8,0,-8); break;
                case QTabBar::RoundedEast: case QTabBar::TriangularEast: r.setWidth(3); r.adjust(0,8,0,-8); break;
                default: r.setTop(r.bottom()-2); r.adjust(10,0,-10,0); break;
                }
                rounded(p,r,pal.color(QPalette::Highlight),Qt::transparent,1);
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
            p->save(); p->setPen(ThemeTokens::instance()->textPlaceholder());
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
                if (close) foreground=pal.color(QPalette::BrightText);
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
                p->drawLine(QPointF(glyph.left(),center.y()),QPointF(glyph.right(),center.y()));
            } else if (windowControl=="restore") {
                // Draw only the exposed part of the rear window. An opaque erase
                // rectangle would leave a patch on a gradient title-bar surface.
                const QRectF front=glyph.translated(-.5,.5);
                const QRectF rear=front.adjusted(2,-1,1,-2);
                QPainterPath outline; outline.moveTo(rear.left(),front.top());
                outline.lineTo(rear.topLeft()); outline.lineTo(rear.topRight());
                outline.lineTo(rear.bottomRight()); outline.lineTo(front.right(),rear.bottom());
                p->drawPath(outline); p->drawRect(front);
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
                      ? pal.color(QPalette::BrightText) : pal.color(QPalette::ButtonText));
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
