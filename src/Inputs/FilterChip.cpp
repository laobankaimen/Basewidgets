#include <basewidgets/Inputs/FilterChip.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include "../Widgets/FeedbackPainting_p.h"
#include <QKeyEvent>
#include <QResizeEvent>

namespace basewidgets {
FilterChip::FilterChip(const QString& label,QWidget* parent) : QAbstractButton(parent),m_remove(new feedbackPainting::CloseButton(this))
{
    StyleManagerAdapter::instance(); setText(label); setCheckable(true); setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed); setCursor(Qt::PointingHandCursor);
    m_remove->setObjectName("FilterChipRemove"); m_remove->setAccessibleName(tr("移除此筛选条件"));
    m_remove->setToolTip(tr("移除此筛选条件")); m_remove->hide();
    connect(m_remove,&QAbstractButton::clicked,this,&FilterChip::removeRequested);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); m_remove->update(); });
}
void FilterChip::setRemovable(bool removable)
{
    if(m_removable==removable)return;
    if(!removable&&m_remove->hasFocus())setFocus(Qt::OtherFocusReason);
    m_removable=removable; m_remove->setVisible(removable); layoutRemoveButton(); updateGeometry(); update();
}
void FilterChip::setTone(FeedbackTone tone) { m_tone=tone; update(); }
QSize FilterChip::sizeHint() const
{
    return QSize(fontMetrics().horizontalAdvance(text())+28+(isCheckable()?18:0)+(m_removable?m_remove->sizeHint().width()+4:0),
                 qMax(32,qMax(fontMetrics().height()+14,m_removable?m_remove->sizeHint().height()+4:0)));
}
QSize FilterChip::minimumSizeHint() const { return QSize(44+(isCheckable()?18:0)+(m_removable?m_remove->sizeHint().width()+4:0),sizeHint().height()); }
void FilterChip::layoutRemoveButton()
{
    const int side=qMin(qMax(0,height()-4),m_remove->sizeHint().height());
    QRect logical(width()-side-4,(height()-side)/2,side,side);
    m_remove->setGeometry(QStyle::visualRect(layoutDirection(),rect(),logical));
}
void FilterChip::resizeEvent(QResizeEvent* event) { QAbstractButton::resizeEvent(event); layoutRemoveButton(); }
void FilterChip::changeEvent(QEvent* event)
{
    QAbstractButton::changeEvent(event);
    if(event->type()==QEvent::FontChange||event->type()==QEvent::LayoutDirectionChange||event->type()==QEvent::StyleChange) {
        layoutRemoveButton(); updateGeometry(); update();
    }
}
bool FilterChip::hitButton(const QPoint& position) const { return rect().contains(position)&&(!m_removable||!m_remove->geometry().contains(position)); }
void FilterChip::keyPressEvent(QKeyEvent* event)
{
    if(!isEnabled()) { event->ignore(); return; }
    if(event->key()==Qt::Key_Delete&&event->modifiers()==Qt::NoModifier&&m_removable) { if(!event->isAutoRepeat())emit removeRequested(); event->accept(); return; }
    if(event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter) { if(!event->isAutoRepeat())click(); event->accept(); return; }
    QAbstractButton::keyPressEvent(event);
}
void FilterChip::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    const QColor accent=isEnabled()?feedbackPainting::toneColor(m_tone):t->textDisabled();
    QColor background=isChecked()?feedbackPainting::blend(t->bg1(),accent,.14):t->bg1();
    if(isEnabled()&&isDown())background=t->bgPressed();
    p.setBrush(background); p.setPen(isChecked()?QPen(accent,1):QPen(Qt::NoPen));
    p.drawRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),height()/2.,height()/2.);
    QRect logical=rect().adjusted(12,0,-12-(m_removable?m_remove->width()+2:0),0);
    if(isCheckable()) {
        QRect indicator(logical.left(),logical.center().y()-7,14,14);
        indicator=QStyle::visualRect(layoutDirection(),rect(),indicator);
        p.setPen(QPen(isChecked()?accent:t->border(),1.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
        if(isChecked()) { QPolygonF tick; tick<<indicator.topLeft()+QPoint(2,7)<<indicator.topLeft()+QPoint(5,10)<<indicator.topLeft()+QPoint(12,3); p.drawPolyline(tick); }
        else p.drawEllipse(QRectF(indicator).adjusted(3,3,-3,-3));
        logical.adjust(18,0,0,0);
    }
    const QRect label=QStyle::visualRect(layoutDirection(),rect(),logical);
    p.setPen(isEnabled()?t->textPrimary():t->textDisabled());
    p.drawText(label,Qt::AlignVCenter|Qt::AlignLeading|Qt::TextSingleLine,fontMetrics().elidedText(text(),Qt::ElideRight,qMax(0,label.width())));
    QStyleOptionButton option; option.initFrom(this);
    if(isEnabled()&&hasFocus()&&(option.state&QStyle::State_KeyboardFocusChange)) {
        p.setBrush(Qt::NoBrush); p.setPen(QPen(t->primary(),1,Qt::DashLine));
        p.drawRoundedRect(QRectF(rect()).adjusted(3,3,-3,-3),height()/2.,height()/2.);
    }
}
}
