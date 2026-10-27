#include <basewidgets/Charts/AcousticSpectrumPlot.h>
#include <cmath>
namespace basewidgets {
AcousticSpectrumPlot::AcousticSpectrumPlot(QWidget* parent) : StyledPlot(parent)
{
    setPlotTitle("单边频谱 / PSD"); xAxis->setLabel("频率 (Hz)"); yAxis->setLabel("PSD (dBFS/Hz)");
    xAxis->setRange(0,24000); yAxis->setRange(-120,0); legend->setVisible(true);
    m_spectrum=addGraph(); m_spectrum->setName("当前帧"); m_spectrum->setPen(QPen(Qt::black,1.5)); setSeriesColor(m_spectrum,0);
    captureResetView();
    m_hold=addGraph(); m_hold->setName("峰值保持"); m_hold->setPen(QPen(Qt::black,1,Qt::DashLine)); setSeriesColor(m_hold,1); m_hold->setVisible(false); m_hold->removeFromLegend();
}
bool AcousticSpectrumPlot::setFrame(const AcousticSpectrumFrame& frame)
{
    if(!frame.isValid() || !QCPRange::validRange(m_log ? frame.binWidth() : 0,frame.sampleRate/2)) return false;
    const bool changed=m_frame.fftSize!=frame.fftSize || m_frame.sampleRate!=frame.sampleRate || m_frame.unit!=frame.unit
        || m_frame.scaling!=frame.scaling || m_frame.window!=frame.window || m_frame.meanRemoved!=frame.meanRemoved || m_frame.floorDb!=frame.floorDb;
    m_frame=frame;
    if(changed || m_peak.size()!=frame.levelsDb.size()) m_peak=frame.levelsDb;
    else if(m_peakEnabled) for(int i=0;i<m_peak.size();++i) m_peak[i]=std::max(m_peak[i],frame.levelsDb[i]);
    m_spectrum->setData(frame.frequencies,frame.levelsDb,true); m_hold->setData(frame.frequencies,m_peak,true);
    yAxis->setLabel(frame.levelLabel());
    if(changed) { xAxis->setRange(m_log ? frame.binWidth() : 0,frame.sampleRate/2); captureResetView(); }
    replot(); return true;
}
void AcousticSpectrumPlot::setPeakHoldEnabled(bool enabled) { m_peakEnabled=enabled; m_hold->setVisible(enabled); if(enabled) m_hold->addToLegend(); else m_hold->removeFromLegend(); resetPeakHold(); }
void AcousticSpectrumPlot::resetPeakHold() { m_peak=m_frame.levelsDb; m_hold->setData(m_frame.frequencies,m_peak,true); replot(); }
bool AcousticSpectrumPlot::setFrequencyRange(double lower,double upper)
{
    if(!std::isfinite(lower)||!std::isfinite(upper)||lower<0||upper<=lower||(m_log && lower<=0)
        || (m_frame.isValid() && upper>m_frame.sampleRate/2)) return false;
    xAxis->setRange(lower,upper); setResetRanges(xAxis->range(),resetYRange()); replot(); return true;
}
bool AcousticSpectrumPlot::setLogFrequency(bool enabled)
{
    if(enabled && !m_frame.isValid()) return false;
    m_log=enabled; xAxis->setScaleType(enabled ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
    if(enabled) xAxis->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));
    else xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));
    if(m_frame.isValid()) xAxis->setRange(enabled ? m_frame.binWidth() : 0,m_frame.sampleRate/2);
    captureResetView();
    replot(); return true;
}
bool AcousticSpectrumPlot::setLevelRange(double lower,double upper)
{
    if(!std::isfinite(lower)||!std::isfinite(upper)||upper<=lower||!QCPRange::validRange(lower,upper)) return false;
    yAxis->setRange(lower,upper); setResetRanges(resetXRange(),yAxis->range()); replot(); return true;
}
double AcousticSpectrumPlot::bandPowerDb(double lower,double upper) const { return AcousticAnalysis::bandPowerDb(m_frame,lower,upper); }
double AcousticSpectrumPlot::peakFrequencyHz() const
{
    if(!m_frame.isValid()) return std::numeric_limits<double>::quiet_NaN();
    const auto peak=std::max_element(m_frame.linearPower.constBegin(),m_frame.linearPower.constEnd());
    return *peak>0 ? m_frame.frequencies[int(peak-m_frame.linearPower.constBegin())] : std::numeric_limits<double>::quiet_NaN();
}
}
