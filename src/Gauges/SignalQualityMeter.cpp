#include <basewidgets/Gauges/SignalQualityMeter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <cmath>
namespace basewidgets {
using namespace gaugePainting;
namespace {
QString dbText(double value) { return std::isnan(value) ? QString::fromUtf8("—") : std::isinf(value) ? QString::fromUtf8("−∞") : QString::number(value,'f',1); }
}
SignalQualityMeter::SignalQualityMeter(QWidget* parent) : StyledWidget(parent),m_name(tr("信号质量"))
{
    setAccessibleName(m_name); setMinimumSize(minimumSizeHint()); connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this]{ update(); });
}
void SignalQualityMeter::setChannelName(const QString& name) { m_name=name; setAccessibleName(name); update(); }
void SignalQualityMeter::setSamples(const QVector<double>& samples) { setStatistics(AcousticAnalysis::statistics(samples,1)); }
bool SignalQualityMeter::setStatistics(const SignalStatistics& value)
{
    if(value.sampleCount<0 || value.validCount<0 || value.invalidCount<0 || value.clippedCount<0 || value.clippedCount>value.validCount
        || qint64(value.validCount)+value.invalidCount!=value.sampleCount || (value.valid() && (!std::isfinite(value.rms) || !std::isfinite(value.peak)
        || !std::isfinite(value.mean) || value.rms<0 || value.peak<value.rms-1e-12*std::max(value.peak,value.rms)))) return false;
    m_statistics=value;
    // Derive dB from linear measures so the panel never mixes inconsistent caller fields.
    if(value.valid()) {
        m_statistics.rmsDb=value.rms>0 ? 20*std::log10(value.rms) : -std::numeric_limits<double>::infinity();
        m_statistics.peakDb=value.peak>0 ? 20*std::log10(value.peak) : -std::numeric_limits<double>::infinity();
        m_statistics.crestFactorDb=value.rms>0 ? m_statistics.peakDb-m_statistics.rmsDb : std::numeric_limits<double>::quiet_NaN();
    } else {
        m_statistics.rmsDb=m_statistics.peakDb=m_statistics.crestFactorDb=std::numeric_limits<double>::quiet_NaN();
    }
    setAccessibleDescription(QString("RMS %1 dBFS; Peak %2 dBFS; 削波 %3; 缺样 %4").arg(dbText(m_statistics.rmsDb),dbText(m_statistics.peakDb)).arg(value.clippedCount).arg(value.invalidCount));
    update(); return true;
}
bool SignalQualityMeter::setDisplayRange(double lower,double upper)
{
    if(!std::isfinite(lower)||!std::isfinite(upper)||upper<=lower||!std::isfinite(upper-lower)) return false;
    m_lower=lower; m_upper=upper; update(); return true;
}
void SignalQualityMeter::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); auto* t=ThemeTokens::instance();
    const QColor textColor=isEnabled()?t->textPrimary():t->textDisabled();
    const QFont label=sizedFont(font(),12),large=sizedFont(font(),25,true);
    text(p,QRectF(12,8,width()-24,22),m_name,label,textColor,Qt::AlignLeft|Qt::AlignVCenter);
    text(p,QRectF(12,34,width()-24,34),m_statistics.valid()?dbText(m_statistics.rmsDb):QString::fromUtf8("—"),large,textColor,Qt::AlignLeft|Qt::AlignVCenter);
    text(p,QRectF(12,70,width()-24,18),"RMS dBFS  ·  Peak "+(m_statistics.valid()?dbText(m_statistics.peakDb):QString::fromUtf8("—"))+" dBFS",label,t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    const QRectF track(12,100,width()-24,16);
    p.setPen(Qt::NoPen); p.setBrush(t->bg2()); p.drawRoundedRect(track,4,4);
    if(m_statistics.valid()) {
        const double ratio=qBound(0.,(m_statistics.rmsDb-m_lower)/(m_upper-m_lower),1.);
        QRectF fill=track; fill.setWidth(track.width()*ratio);
        p.setBrush(isEnabled() ? (m_statistics.clippedCount ? t->danger() : t->primary()) : t->textDisabled());
        if(fill.width()>0) p.drawRoundedRect(fill,4,4);
        const double peak=qBound(0.,(m_statistics.peakDb-m_lower)/(m_upper-m_lower),1.);
        const double x=track.left()+track.width()*peak;
        p.setPen(QPen(t->warning(),2)); p.drawLine(QPointF(x,track.top()-3),QPointF(x,track.bottom()+3));
    }
    text(p,QRectF(12,120,width()-24,18),QString("%1 … %2 dBFS").arg(m_lower).arg(m_upper),sizedFont(font(),10),t->textSecondary());
    const QString quality=!m_statistics.valid()?tr("无有效样本") : m_statistics.clippedCount?tr("削波 %1 / %2").arg(m_statistics.clippedCount).arg(m_statistics.validCount)
        : m_statistics.invalidCount?tr("缺样 %1 / %2").arg(m_statistics.invalidCount).arg(m_statistics.sampleCount):tr("块内未检测到削波或缺样");
    text(p,QRectF(12,146,width()-24,20),quality,label,textColor,Qt::AlignLeft|Qt::AlignVCenter);
    const QString detail=m_statistics.valid()?QString("DC %1 FS  ·  峰均比 %2 dB").arg(m_statistics.mean,0,'g',4).arg(dbText(m_statistics.crestFactorDb)):QString();
    text(p,QRectF(12,174,width()-24,20),detail,label,t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
}
}
