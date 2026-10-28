#include <basewidgets/Inputs/StatusBadge.h>
#include "../Widgets/FeedbackPainting_p.h"
#include <QEvent>
namespace basewidgets {
StatusBadge::StatusBadge(const QString& text,QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed); setFocusPolicy(Qt::NoFocus); setText(text);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
void StatusBadge::setText(const QString& text) { m_text=text; setToolTip(text); setAccessibleName(text); updateGeometry(); update(); }
void StatusBadge::setTone(FeedbackTone tone) { m_tone=tone; update(); }
void StatusBadge::setDotVisible(bool visible) { if(m_dotVisible==visible)return; m_dotVisible=visible; updateGeometry(); update(); }
QSize StatusBadge::sizeHint() const { return QSize(qMax(32,fontMetrics().horizontalAdvance(m_text)+20+(m_dotVisible?14:0)),qMax(26,fontMetrics().height()+8)); }
QSize StatusBadge::minimumSizeHint() const { return QSize(32,sizeHint().height()); }
void StatusBadge::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if(event->type()==QEvent::FontChange)updateGeometry();
    if(event->type()==QEvent::FontChange||event->type()==QEvent::EnabledChange||event->type()==QEvent::LayoutDirectionChange)update();
}
void StatusBadge::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); const QColor accent=isEnabled()?feedbackPainting::toneColor(m_tone):t->textDisabled();
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);
    p.setBrush(feedbackPainting::blend(t->bg1(),accent,.10)); p.drawRoundedRect(QRectF(rect()).adjusted(.5,.5,-.5,-.5),height()/2.,height()/2.);
    QRect area=rect().adjusted(10,2,-10,-2);
    if(m_dotVisible) {
        const int x=layoutDirection()==Qt::RightToLeft?area.right()-4:area.left()+4;
        p.setBrush(accent); p.drawEllipse(QPointF(x,height()/2.),3.5,3.5);
        if(layoutDirection()==Qt::RightToLeft)area.adjust(0,0,-14,0); else area.adjust(14,0,0,0);
    }
    p.setPen(isEnabled()?t->textPrimary():t->textDisabled());
    p.drawText(area,Qt::AlignVCenter|(layoutDirection()==Qt::RightToLeft?Qt::AlignRight:Qt::AlignLeft)|Qt::TextSingleLine,fontMetrics().elidedText(m_text,Qt::ElideRight,qMax(0,area.width())));
}
}
