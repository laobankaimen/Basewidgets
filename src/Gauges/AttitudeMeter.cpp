#include <basewidgets/Gauges/AttitudeMeter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <QPainterPath>
namespace basewidgets {
using namespace gaugePainting;
AttitudeMeter::AttitudeMeter(QWidget* parent) : StyledWidget(parent),m_title(tr("姿态"))
{
    setMinimumSize(minimumSizeHint()); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); }); refreshDescription();
}
void AttitudeMeter::setAttitude(double pitch,double roll)
{
    if (!std::isfinite(pitch) || !std::isfinite(roll)) { setValueAvailable(false); return; }
    pitch=qBound(-90.,pitch,90.);
    roll=std::fmod(roll,360.); if (roll>=180) roll-=360; if (roll<-180) roll+=360;
    const bool changed=pitch!=m_pitch || roll!=m_roll;
    m_pitch=pitch; m_roll=roll; m_available=true; refreshDescription(); update();
    if (changed) emit attitudeChanged(pitch,roll);
}
void AttitudeMeter::setTitle(const QString& title) { m_title=title; refreshDescription(); update(); }
void AttitudeMeter::setValueAvailable(bool available) { m_available=available; refreshDescription(); update(); }
void AttitudeMeter::refreshDescription()
{
    setAccessibleName(m_title);
    setAccessibleDescription(m_available ? tr("俯仰 %1 度，横滚 %2 度").arg(locale().toString(m_pitch,'f',1),locale().toString(m_roll,'f',1)) : tr("无数据"));
    setToolTip(accessibleDescription());
}
void AttitudeMeter::paintEvent(QPaintEvent*)
{
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    const auto* tokens=ThemeTokens::instance();
    const QColor foreground=isEnabled() ? tokens->textPrimary() : tokens->textDisabled();
    const QColor secondary=isEnabled() ? tokens->textSecondary() : tokens->textDisabled();
    const qreal diameter=qMax(1,qMin(width()-24,height()-80)),radius=qMax(1.,diameter/2-14);
    const QPointF center(width()/2.,36+diameter/2);
    text(painter,QRectF(12,8,width()-24,22),m_title,sizedFont(font(),13),secondary);
    QPainterPath dial; dial.addEllipse(center,radius,radius);
    painter.save(); painter.setClipPath(dial);
    if (m_available) {
        painter.translate(center); painter.rotate(-m_roll);
        const qreal horizon=m_pitch/45.*radius;
        const QColor sky=isEnabled() ? QColor("#245B83") : tokens->bg2();
        const QColor ground=isEnabled() ? QColor("#704D35") : tokens->bgCtrl();
        painter.fillRect(QRectF(-3*radius,-3*radius,6*radius,3*radius+horizon),sky);
        painter.fillRect(QRectF(-3*radius,horizon,6*radius,3*radius-horizon),ground);
        painter.setPen(QPen(isEnabled() ? Qt::white : secondary,2));
        painter.drawLine(QPointF(-3*radius,horizon),QPointF(3*radius,horizon));
        const QFont ladder=sizedFont(font(),qBound(9.,diameter*.043,12.));
        for (int pitch=-60;pitch<=60;pitch+=10) {
            if (!pitch) continue;
            const qreal y=horizon-pitch/45.*radius;
            const qreal half=radius*(pitch%20==0 ? .27 : .15);
            painter.setPen(QPen(isEnabled() ? Qt::white : secondary,1));
            painter.drawLine(QPointF(-half,y),QPointF(half,y));
            if (diameter>=170 && pitch%20==0)
                text(painter,QRectF(half+3,y-8,24,16),QString::number(std::abs(pitch)),ladder,isEnabled() ? QColor(Qt::white) : secondary);
        }
    } else painter.fillPath(dial,tokens->bg2());
    painter.restore();
    painter.setPen(QPen(tokens->border(),2)); painter.setBrush(Qt::NoBrush); painter.drawEllipse(center,radius,radius);
    // Bank ticks stay fixed; only horizon/pitch ladder rotate. The aircraft symbol is fixed too.
    for (int bank : {-60,-30,0,30,60}) {
        const qreal angle=90-bank;
        painter.setPen(QPen(foreground,bank==0 ? 2 : 1));
        painter.drawLine(polar(center,radius+4,angle),polar(center,radius+10,angle));
    }
    if (m_available) {
        const QColor marker=isEnabled() ? tokens->warning() : tokens->textDisabled();
        painter.setPen(QPen(marker,3,Qt::SolidLine,Qt::RoundCap));
        painter.drawLine(center+QPointF(-radius*.38,0),center+QPointF(-8,0));
        painter.drawLine(center+QPointF(8,0),center+QPointF(radius*.38,0));
        painter.drawLine(center+QPointF(-8,0),center+QPointF(0,5));
        painter.drawLine(center+QPointF(0,5),center+QPointF(8,0));
        const qreal angle=90+m_roll;
        const QPointF top=polar(center,radius-4,angle);
        painter.setBrush(marker); painter.setPen(Qt::NoPen);
        painter.drawPolygon(QPolygonF(QVector<QPointF>{top,polar(center,radius-12,angle-4),polar(center,radius-12,angle+4)}));
    } else text(painter,QRectF(center.x()-50,center.y()-15,100,30),tr("无数据"),sizedFont(font(),16,true),secondary);
    const QString pitch=m_available ? tr("俯仰 %1°").arg(locale().toString(m_pitch,'f',1)) : tr("俯仰 —");
    const QString roll=m_available ? tr("横滚 %1°").arg(locale().toString(m_roll,'f',1)) : tr("横滚 —");
    text(painter,QRectF(6,height()-44,width()-12,22),pitch,sizedFont(font(),15,true),foreground);
    text(painter,QRectF(6,height()-22,width()-12,18),roll,sizedFont(font(),12),secondary);
}
}
