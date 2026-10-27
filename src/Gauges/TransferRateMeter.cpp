#include <basewidgets/Gauges/TransferRateMeter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <cmath>
namespace basewidgets {
using namespace gaugePainting;
TransferRateMeter::TransferRateMeter(QWidget* parent) : StyledWidget(parent)
{
    setMinimumSize(minimumSizeHint()); setAccessibleName(tr("双向网络 / 传输速率"));
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
double TransferRateMeter::receiveBytesPerSecond() const { return hasRates()?m_rx:std::numeric_limits<double>::quiet_NaN(); }
double TransferRateMeter::sendBytesPerSecond() const { return hasRates()?m_tx:std::numeric_limits<double>::quiet_NaN(); }
bool TransferRateMeter::setMaximumSampleGap(double seconds) { if(!std::isfinite(seconds)||seconds<=0)return false; m_gap=seconds; return true; }
bool TransferRateMeter::setLinkCapacity(double bits) { if(!std::isfinite(bits)||bits<0)return false; m_capacityBits=bits; update(); return true; }
void TransferRateMeter::setDisplayUnit(DisplayUnit unit) { if(unit!=DisplayUnit::NetworkMbps&&unit!=DisplayUnit::BinaryMiBPerSecond)return; m_unit=unit; update(); }
void TransferRateMeter::reset() { const bool valid=hasRates(); m_hasCounters=false; m_hasRates=false; m_stale=false; m_last=0; m_received=m_sent=0; if(valid)emit rateValidityChanged(false); update(); }
void TransferRateMeter::setStale(bool stale) { const bool old=hasRates(); m_stale=stale; if(old!=hasRates())emit rateValidityChanged(hasRates()); update(); }
bool TransferRateMeter::setRates(double time,double rx,double tx)
{
    if(!std::isfinite(time)||std::abs(time)>1e12||!std::isfinite(rx)||!std::isfinite(tx)||rx<0||tx<0||rx>1e15||tx>1e15
        ||((m_hasRates||m_hasCounters)&&time<=m_last)) return false;
    const bool valid=hasRates(); m_last=time; m_rx=rx; m_tx=tx; m_hasRates=true; m_hasCounters=false; m_stale=false;
    setAccessibleDescription(tr("接收 %1 B/s，发送 %2 B/s").arg(rx).arg(tx));
    if(!valid)emit rateValidityChanged(true); emit ratesUpdated(time,rx,tx); update(); return true;
}
bool TransferRateMeter::addCounters(double time,quint64 received,quint64 sent)
{
    if(!std::isfinite(time)||std::abs(time)>1e12||((m_hasCounters||m_hasRates)&&time<=m_last)) return false;
    if(!m_hasCounters||received<m_received||sent<m_sent||time-m_last>m_gap) {
        const QString reason=!m_hasCounters?tr("建立计数基准"):(received<m_received||sent<m_sent)?tr("计数器重置 / 回绕"):tr("采样间隔超过上限");
        const bool valid=hasRates(); m_last=time; m_received=received; m_sent=sent; m_hasCounters=true; m_hasRates=false; m_stale=false;
        if(valid)emit rateValidityChanged(false); emit baselineReset(reason); update(); return true;
    }
    const double rx=double(received-m_received)/(time-m_last),tx=double(sent-m_sent)/(time-m_last);
    if(!std::isfinite(rx)||!std::isfinite(tx)||rx>1e15||tx>1e15) return false;
    const bool valid=hasRates();
    m_last=time; m_rx=rx; m_tx=tx; m_received=received; m_sent=sent; m_hasCounters=true; m_hasRates=true; m_stale=false;
    setAccessibleDescription(tr("接收 %1 B/s，发送 %2 B/s").arg(rx).arg(tx));
    if(!valid)emit rateValidityChanged(true); emit ratesUpdated(time,rx,tx); update(); return true;
}
void TransferRateMeter::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); auto* t=ThemeTokens::instance();
    const QColor foreground=isEnabled()?t->textPrimary():t->textDisabled();
    const bool valid=hasRates(); const double scale=m_unit==DisplayUnit::NetworkMbps?8./1e6:1./1048576;
    const QString unit=m_unit==DisplayUnit::NetworkMbps?"Mbps":"MiB/s";
    text(p,QRectF(12,8,width()-24,22),tr("双向速率 · %1").arg(unit),sizedFont(font(),12),t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    for(int row=0;row<2;++row) {
        const double rate=row?m_tx:m_rx; const QString name=row?tr("发送"):tr("接收"); const int y=40+row*64;
        text(p,QRectF(12,y,width()-24,28),valid?name+" "+QString::number(rate*scale,'f',2)+" "+unit:name+" —",sizedFont(font(),19,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
        const QRectF track(12,y+33,width()-24,10); p.setPen(Qt::NoPen); p.setBrush(t->bg2()); p.drawRoundedRect(track,3,3);
        if(valid&&m_capacityBits>0) {
            QRectF fill=track; fill.setWidth(track.width()*qBound(0.,rate*8/m_capacityBits,1.));
            p.setBrush(isEnabled()?(row?t->success():t->primary()):t->textDisabled()); if(fill.width()>0)p.drawRoundedRect(fill,3,3);
        }
    }
    const QString state=m_stale?tr("数据已过期"):!m_hasRates?tr("等待两个有效计数快照 / 直接速率"):m_capacityBits>0?tr("单方向容量 %1 Mbps").arg(m_capacityBits/1e6,0,'f',1):tr("未配置链路容量，不显示占用比例");
    text(p,QRectF(12,181,width()-24,30),state,sizedFont(font(),11),t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
}
}
