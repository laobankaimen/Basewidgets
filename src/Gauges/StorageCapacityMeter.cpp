#include <basewidgets/Gauges/StorageCapacityMeter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <QMouseEvent>
#include <QSet>
#include <cmath>
namespace basewidgets {
using namespace gaugePainting;
namespace {
QString byteText(quint64 bytes)
{
    double value=double(bytes); const QStringList units{"B","KiB","MiB","GiB","TiB","PiB","EiB"}; int index=0;
    while(value>=1024 && index+1<units.size()) { value/=1024; ++index; }
    return QString("%1 %2").arg(value,0,'f',index?1:0).arg(units[index]);
}
}
StorageCapacityMeter::StorageCapacityMeter(QWidget* parent) : StyledWidget(parent)
{
    setMinimumSize(minimumSizeHint()); setAccessibleName(tr("存储容量分布"));
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
QSize StorageCapacityMeter::sizeHint() const { return QSize(380,qMax(210,158+((m_segments.size()+1)/2)*24)); }
bool StorageCapacityMeter::setCapacity(quint64 total,const QVector<StorageSegment>& segments)
{
    if(segments.size()>16||(!total&&!segments.isEmpty())) return false;
    quint64 used=0; QSet<QString> names;
    for(const auto& segment:segments) {
        if(segment.name.isEmpty()||names.contains(segment.name)||segment.bytes>total-used) return false;
        used+=segment.bytes; names.insert(segment.name);
    }
    m_total=total; m_used=used; m_segments=segments;
    QString description=total?tr("总量 %1，已用 %2，可用 %3").arg(byteText(total),byteText(used),byteText(total-used)):tr("容量未知");
    for(const auto& segment:segments) description+=QString("\n%1: %2 B").arg(segment.name).arg(segment.bytes);
    setAccessibleDescription(description); setToolTip(description); updateGeometry(); update(); return true;
}
bool StorageCapacityMeter::setWarningPercent(double percent) { if(!std::isfinite(percent)||percent<0||percent>100)return false; m_warning=percent; update(); return true; }
void StorageCapacityMeter::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); auto* t=ThemeTokens::instance();
    const QColor foreground=isEnabled()?t->textPrimary():t->textDisabled(); const QFont label=sizedFont(font(),12);
    text(p,QRectF(12,8,width()-24,22),tr("存储容量 / 分类"),label,t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    const double percent=m_total?100.*double(m_used)/m_total:0;
    text(p,QRectF(12,34,width()-24,34),m_total?QString("%1% 已用").arg(percent,0,'f',1):tr("容量未知"),sizedFont(font(),24,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
    const QRectF bar(12,86,width()-24,20); p.setPen(Qt::NoPen); p.setBrush(t->bg2()); p.drawRoundedRect(bar,4,4);
    static const QColor colors[]={QColor("#409EFF"),QColor("#67C23A"),QColor("#E6A23C"),QColor("#C084FC"),QColor("#22B8B0")};
    double x=bar.left();
    for(int i=0;i<m_segments.size();++i) {
        const auto& segment=m_segments[i]; const double width=m_total?bar.width()*double(segment.bytes)/m_total:0;
        const QColor color=isEnabled()?(segment.color.isValid()?segment.color:colors[i%5]):t->textDisabled();
        p.fillRect(QRectF(x,bar.top(),width,bar.height()),color); x+=width;
        const int col=i%2,row=i/2; const qreal columnWidth=(this->width()-24)/2.;
        p.fillRect(QRectF(12+col*columnWidth,148+row*24,8,8),color);
        text(p,QRectF(26+col*columnWidth,140+row*24,columnWidth-18,24),segment.name+" "+byteText(segment.bytes),sizedFont(font(),11),foreground,Qt::AlignLeft|Qt::AlignVCenter);
    }
    if(m_total) {
        const double warn=bar.left()+bar.width()*m_warning/100;
        p.setPen(QPen(t->warning(),2)); p.drawLine(QPointF(warn,bar.top()-3),QPointF(warn,bar.bottom()+3));
    }
    const QString status=m_total?tr("可用 %1 / %2%3").arg(byteText(freeBytes()),byteText(m_total),percent>=m_warning?tr(" · 达到警戒线"):QString()):tr("由业务传入磁盘或存储池快照");
    text(p,QRectF(12,112,width()-24,24),status,label,foreground,Qt::AlignLeft|Qt::AlignVCenter);
}
void StorageCapacityMeter::mousePressEvent(QMouseEvent* event)
{
    const QRectF bar(12,86,width()-24,20);
    if(event->button()==Qt::LeftButton&&m_total&&bar.contains(event->pos())) {
        double left=bar.left();
        for(int i=0;i<m_segments.size();++i) {
            const double right=left+bar.width()*double(m_segments[i].bytes)/m_total;
            if(event->pos().x()>=left&&event->pos().x()<right) { emit segmentSelected(i,m_segments[i].name,m_segments[i].bytes); return; } left=right;
        }
        emit segmentSelected(-1,tr("空闲"),freeBytes()); return;
    }
    StyledWidget::mousePressEvent(event);
}
}
