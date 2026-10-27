#include <basewidgets/Charts/AcousticSpectrogramPlot.h>
namespace basewidgets {
AcousticSpectrogramPlot::AcousticSpectrogramPlot(QWidget* parent) : HistoryHeatmapPlot(parent)
{
    setPlotTitle("时频声谱图 · 空白表示缺帧"); xAxis->setLabel("频率 (Hz)"); yAxis->setLabel("帧中心时间 (s)");
}
bool AcousticSpectrogramPlot::configure(const AcousticSpectrumFrame& prototype,int capacity,double period)
{
    if(!prototype.isValid() || !configureGrid(0,prototype.sampleRate/2,prototype.levelsDb.size(),capacity,period,prototype.levelLabel())) return false;
    m_prototype=prototype; return true;
}
bool AcousticSpectrogramPlot::appendFrame(double time,const AcousticSpectrumFrame& frame)
{
    if(!frame.isValid() || frame.fftSize!=m_prototype.fftSize || frame.sampleRate!=m_prototype.sampleRate
        ||frame.unit!=m_prototype.unit||frame.scaling!=m_prototype.scaling||frame.window!=m_prototype.window
        ||frame.meanRemoved!=m_prototype.meanRemoved||frame.floorDb!=m_prototype.floorDb) return false;
    return appendRow(time,frame.levelsDb);
}
bool AcousticSpectrogramPlot::setFrequencyRange(double lower,double upper) { return setDisplayedXRange(lower,upper); }
}
