#include <basewidgets/Charts/LofarDemonPlots.h>
namespace basewidgets {
LofarPlot::LofarPlot(QWidget* parent) : AcousticSpectrogramPlot(parent) { setPlotTitle("LOFAR · 窄带 PSD 历程"); }
bool LofarPlot::configure(const AcousticSpectrumFrame& frame,int capacity,double period,double lower,double upper)
{
    if(frame.scaling!=SpectrumScaling::PowerSpectralDensity||frame.unit==AcousticUnit::RelativeEnvelope
        ||upper<=lower||!QCPRange::validRange(lower,upper)||lower<0||upper>frame.sampleRate/2) return false;
    if(!AcousticSpectrogramPlot::configure(frame,capacity,period)) return false;
    return setFrequencyRange(lower,upper);
}
bool LofarPlot::appendFrame(double time,const AcousticSpectrumFrame& frame)
{
    if(frame.scaling!=SpectrumScaling::PowerSpectralDensity||frame.unit==AcousticUnit::RelativeEnvelope) return false;
    return AcousticSpectrogramPlot::appendFrame(time,frame);
}
DemonSpectrumPlot::DemonSpectrumPlot(QWidget* parent) : AcousticSpectrumPlot(parent)
{
    setPlotTitle("DEMON · 归一化包络调制谱"); xAxis->setLabel("调制频率 (Hz)");
}
bool DemonSpectrumPlot::setResult(const DemonResult& result)
{
    if(!result.isValid()) return false;
    const bool changed=!m_configured||!(m_settings==result.settings)||frame().sampleRate!=result.spectrum.sampleRate||frame().fftSize!=result.spectrum.fftSize;
    if(!setFrame(result.spectrum)) return false;
    if(changed) { resetPeakHold(); setFrequencyRange(xAxis->scaleType()==QCPAxis::stLogarithmic?result.spectrum.binWidth()/2:0,result.settings.maximumModulationHz); }
    m_settings=result.settings; m_configured=true; return true;
}
DemonHistoryPlot::DemonHistoryPlot(QWidget* parent) : AcousticSpectrogramPlot(parent)
{
    setPlotTitle("DEMON · 调制频率历程"); xAxis->setLabel("调制频率 (Hz)");
}
bool DemonHistoryPlot::configure(const DemonResult& result,int capacity,double period)
{
    if(!result.isValid()||!AcousticSpectrogramPlot::configure(result.spectrum,capacity,period)) return false;
    m_settings=result.settings; m_inputRate=result.inputSampleRate; m_inputCount=result.inputSampleCount;
    return setFrequencyRange(0,result.settings.maximumModulationHz);
}
bool DemonHistoryPlot::appendResult(double time,const DemonResult& result)
{
    if(!result.isValid()||!(result.settings==m_settings)||result.inputSampleRate!=m_inputRate||result.inputSampleCount!=m_inputCount) return false;
    return AcousticSpectrogramPlot::appendFrame(time,result.spectrum);
}
}
