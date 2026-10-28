#include <basewidgets/Gauges/StorageDonutMeter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QPainter>
#include <QLocale>
#include <limits>
namespace basewidgets {
StorageDonutMeter::StorageDonutMeter(QWidget* parent) : StyledWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred); refreshDescription();
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
bool StorageDonutMeter::setCapacity(quint64 used,quint64 total)
{
    if(used>total)return false;
    m_used=used; m_total=total; refreshDescription(); update(); return true;
}
double StorageDonutMeter::percent() const { return m_total?100.*(double(m_used)/double(m_total)):std::numeric_limits<double>::quiet_NaN(); }
void StorageDonutMeter::setUnitSystem(UnitSystem system) { m_units=system; refreshDescription(); update(); }
QString StorageDonutMeter::formatBytes(quint64 bytes) const
{
    const QStringList units=m_units==UnitSystem::IEC?QStringList{"B","KiB","MiB","GiB","TiB","PiB","EiB"}:QStringList{"B","kB","MB","GB","TB","PB","EB"};
    const double base=m_units==UnitSystem::IEC?1024.:1000.; double value=double(bytes); int i=0;
    while(value>=base&&i<units.size()-1) { value/=base; ++i; }
    return locale().toString(value,'f',i?2:0)+" "+units[i];
}
QString StorageDonutMeter::usedText() const { return m_total?formatBytes(m_used):QString::fromUtf8("—"); }
QString StorageDonutMeter::totalText() const { return m_total?formatBytes(m_total):QString::fromUtf8("—"); }
void StorageDonutMeter::refreshDescription()
{
    const QString text=m_total?tr("已用 %1，总容量 %2，占用 %3%").arg(usedText(),totalText(),locale().toString(percent(),'f',1)):tr("容量未知");
    setAccessibleName(text); setToolTip(text);
}
QSize StorageDonutMeter::minimumSizeHint() const { return QSize(qMax(180,fontMetrics().horizontalAdvance(tr("总容量 18.45 EB"))+16),180+fontMetrics().height()*2); }
QSize StorageDonutMeter::sizeHint() const { return minimumSizeHint().expandedTo(QSize(240,250)); }
void StorageDonutMeter::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    const int line=fontMetrics().height()+5; const int diameter=qMax(0,qMin(width()-20,height()-line*2-20));
    if(diameter<40)return;
    const QRectF ring((width()-diameter)/2.,8,diameter,diameter); const double stroke=qBound(8.,diameter*.10,18.);
    const QRectF arc=ring.adjusted(stroke/2,stroke/2,-stroke/2,-stroke/2);
    p.setPen(QPen(t->bgCtrl(),stroke)); p.drawEllipse(arc);
    if(m_total&&m_used) {
        p.setPen(QPen(t->primary(),stroke,Qt::SolidLine,Qt::FlatCap));
        p.drawArc(arc,90*16,-qRound(percent()/100.*360*16));
    }
    const QRectF inner=ring.adjusted(stroke+8,stroke+4,-stroke-8,-stroke-4);
    QFont valueFont=font(); valueFont.setPixelSize(qMax(12,qMin(30,int(diameter*.18)))); valueFont.setBold(true); p.setFont(valueFont); p.setPen(t->textPrimary());
    const QString value=m_total?locale().toString(percent(),'f',1)+"%":QString::fromUtf8("—");
    p.drawText(inner.adjusted(0,-10,0,-10),Qt::AlignCenter,value);
    p.setFont(font()); p.setPen(t->textSecondary());
    p.drawText(QRectF(inner.left(),ring.center().y()+12,inner.width(),line),Qt::AlignHCenter|Qt::AlignTop,m_total?tr("已用容量"):tr("容量未知"));
    const int y=qRound(ring.bottom())+8;
    p.drawText(QRect(4,y,width()-8,line),Qt::AlignCenter,fontMetrics().elidedText(tr("已用 %1").arg(usedText()),Qt::ElideRight,width()-8));
    p.drawText(QRect(4,y+line,width()-8,line),Qt::AlignCenter,fontMetrics().elidedText(tr("总容量 %1").arg(totalText()),Qt::ElideRight,width()-8));
}
}
