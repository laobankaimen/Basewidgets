#include <basewidgets/Gauges/AcquisitionStatusPanel.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <cmath>
#include <limits>
namespace basewidgets {
using namespace gaugePainting;
AcquisitionStatusPanel::AcquisitionStatusPanel(QWidget* parent) : StyledWidget(parent)
{
    setMinimumSize(minimumSizeHint()); setAccessibleName(tr("采集与存储状态"));
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this]{ update(); });
}
bool AcquisitionStatusPanel::setStatus(const AcquisitionStatus& status)
{
    if(!std::isfinite(status.sampleRate)||status.sampleRate<0||!std::isfinite(status.writeBytesPerSecond)||status.writeBytesPerSecond<0
        || status.channels<0 || status.channels>4096 || status.freeBytes>status.totalBytes
        || (status.state!=AcquisitionState::Idle && status.state!=AcquisitionState::Recording && status.state!=AcquisitionState::Paused && status.state!=AcquisitionState::Disconnected)) return false;
    if(status.state==AcquisitionState::Recording && (status.sampleRate<=0 || status.channels==0)) return false;
    m_status=status; m_valid=true;
    setAccessibleDescription(tr("采样率 %1 Hz，%2 通道，接收 %3 样点，丢失 %4 样点，可用 %5 字节")
        .arg(status.sampleRate).arg(status.channels).arg(status.receivedSamples).arg(status.lostSamples).arg(status.freeBytes));
    update(); return true;
}
double AcquisitionStatusPanel::lossPercent() const
{
    const long double total=static_cast<long double>(m_status.receivedSamples)+m_status.lostSamples;
    return m_valid && total>0 ? double(m_status.lostSamples/total*100) : std::numeric_limits<double>::quiet_NaN();
}
double AcquisitionStatusPanel::remainingSeconds() const
{
    if(!m_valid || m_status.totalBytes==0 || m_status.writeBytesPerSecond<=0) return std::numeric_limits<double>::quiet_NaN();
    const double value=m_status.freeBytes/m_status.writeBytesPerSecond;
    return std::isfinite(value)?value:std::numeric_limits<double>::quiet_NaN();
}
void AcquisitionStatusPanel::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); auto* t=ThemeTokens::instance(); const QFont label=sizedFont(font(),12);
    const QColor foreground=isEnabled()?t->textPrimary():t->textDisabled();
    const QString state=!m_valid ? tr("状态未提供") : m_status.state==AcquisitionState::Recording ? tr("录制中") : m_status.state==AcquisitionState::Paused ? tr("已暂停") : m_status.state==AcquisitionState::Disconnected ? tr("已断开") : tr("待机");
    text(p,QRectF(12,8,width()-24,24),tr("采集 / 存储"),label,t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    text(p,QRectF(12,36,width()-24,30),state,sizedFont(font(),23,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
    const QString rate=m_status.sampleRate>0 ? QString("%1 Hz").arg(m_status.sampleRate,0,'g',6) : tr("采样率 —");
    const QString channels=m_status.channels>0 ? tr("%1 通道").arg(m_status.channels) : tr("通道 —");
    const QString setup=m_valid ? QString("%1 · %2 · %3 MiB/s").arg(rate,channels).arg(m_status.writeBytesPerSecond/1048576,0,'f',2) : QString::fromUtf8("—");
    text(p,QRectF(12,72,width()-24,20),setup,label,t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    const QRectF track(12,104,width()-24,14); p.setPen(Qt::NoPen); p.setBrush(t->bg2()); p.drawRoundedRect(track,4,4);
    const double free=m_status.totalBytes ? double(m_status.freeBytes)/m_status.totalBytes : 0;
    if(m_valid && m_status.totalBytes) { QRectF used=track; used.setWidth(track.width()*(1-free)); p.setBrush(isEnabled()?t->primary():t->textDisabled()); p.drawRoundedRect(used,4,4); }
    const QString storage=m_valid && m_status.totalBytes ? tr("可用 %1 / %2 GiB").arg(m_status.freeBytes/1073741824.,0,'f',1).arg(m_status.totalBytes/1073741824.,0,'f',1) : tr("存储容量未知");
    text(p,QRectF(12,122,width()-24,20),storage,label,foreground,Qt::AlignLeft|Qt::AlignVCenter);
    const double remaining=remainingSeconds();
    const QString duration=std::isfinite(remaining) ? tr("按当前速率预计可录 %1 h").arg(remaining/3600,0,'f',2) : tr("预计可录时间 —");
    text(p,QRectF(12,150,width()-24,20),duration,label,t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    const double loss=lossPercent();
    text(p,QRectF(12,178,width()-24,20),std::isfinite(loss)?tr("丢样 %1 · 丢样率 %2%").arg(m_status.lostSamples).arg(loss,0,'f',3):tr("样点计数未提供"),label,foreground,Qt::AlignLeft|Qt::AlignVCenter);
}
}
