#include <basewidgets/Gauges/CompassGauge.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
namespace basewidgets {
using namespace gaugePainting;
CompassGauge::CompassGauge(QWidget* parent) : StyledWidget(parent),m_title(tr("航向"))
{
    setMinimumSize(minimumSizeHint()); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
    refreshDescription();
}
void CompassGauge::setHeading(double degrees)
{
    if (!std::isfinite(degrees)) { setValueAvailable(false); return; }
    double normalized=std::fmod(degrees,360.); if (normalized<0) normalized+=360.;
    const bool changed=normalized!=m_heading;
    m_heading=normalized; m_available=true; refreshDescription(); update();
    if (changed) emit headingChanged(m_heading);
}
void CompassGauge::setValueAvailable(bool available) { m_available=available; refreshDescription(); update(); }
void CompassGauge::setTitle(const QString& title) { m_title=title; refreshDescription(); update(); }
QString CompassGauge::directionText() const
{
    if (!m_available) return tr("无数据");
    const QStringList directions{tr("北"),tr("东北"),tr("东"),tr("东南"),tr("南"),tr("西南"),tr("西"),tr("西北")};
    return directions.at(int(std::floor((m_heading+22.5)/45.))%8);
}
void CompassGauge::refreshDescription()
{
    setAccessibleName(m_title);
    setAccessibleDescription(m_available ? tr("航向 %1 度，%2").arg(locale().toString(m_heading,'f',1),directionText()) : tr("无数据"));
    setToolTip(accessibleDescription());
}
void CompassGauge::paintEvent(QPaintEvent*)
{
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    const auto* tokens=ThemeTokens::instance();
    const QColor foreground=isEnabled() ? tokens->textPrimary() : tokens->textDisabled();
    const QColor secondary=isEnabled() ? tokens->textSecondary() : tokens->textDisabled();
    const QColor accent=isEnabled() ? tokens->primary() : tokens->textDisabled();
    const qreal diameter=qMax(1,qMin(width()-24,height()-80)), radius=diameter/2-4;
    const QPointF center(width()/2.,36+diameter/2);
    text(painter,QRectF(12,8,width()-24,22),m_title,sizedFont(font(),13),secondary);
    painter.setPen(QPen(tokens->borderLight(),2)); painter.setBrush(Qt::NoBrush); painter.drawEllipse(center,radius,radius);
    const QFont cardinal=sizedFont(font(),qBound(12.,diameter*.065,18.),true);
    for (int degrees=0;degrees<360;degrees+=15) {
        const double angle=90-degrees;
        painter.setPen(QPen(degrees%90==0 ? foreground : tokens->border(),degrees%90==0 ? 2 : 1));
        painter.drawLine(polar(center,radius-5,angle),polar(center,radius-(degrees%90==0 ? 13 : 9),angle));
        if (degrees%90==0) {
            const QString label=QStringList{"N","E","S","W"}.at(degrees/90);
            const QPointF location=polar(center,radius-26,angle);
            text(painter,QRectF(location.x()-14,location.y()-12,28,24),label,cardinal,foreground);
        }
    }
    if (m_available) {
        const double angle=90-m_heading;
        const QPointF sideways=polar(QPointF(),4,angle+90);
        painter.setPen(Qt::NoPen); painter.setBrush(tokens->border());
        painter.drawPolygon(QPolygonF(QVector<QPointF>{center+sideways,polar(center,radius*.48,angle+180),center-sideways}));
        painter.setBrush(accent);
        painter.drawPolygon(QPolygonF(QVector<QPointF>{center+sideways,polar(center,radius*.67,angle),center-sideways}));
        painter.setBrush(tokens->bg1()); painter.setPen(QPen(accent,2)); painter.drawEllipse(center,4,4);
    } else text(painter,QRectF(center.x()-30,center.y()-18,60,36),QString::fromUtf8("—"),sizedFont(font(),28,true),secondary);
    const QString value=m_available ? QString("%1°").arg(qRound(m_heading)%360,3,10,QChar('0')) : QString::fromUtf8("—");
    text(painter,QRectF(12,height()-44,width()-24,25),value,sizedFont(font(),22,true),foreground);
    text(painter,QRectF(12,height()-20,width()-24,18),directionText(),sizedFont(font(),11),secondary);
}
}
