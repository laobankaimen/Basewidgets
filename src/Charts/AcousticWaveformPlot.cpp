#include <basewidgets/Charts/AcousticWaveformPlot.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <cmath>
namespace basewidgets {
AcousticWaveformPlot::AcousticWaveformPlot(QWidget* parent) : StyledPlot(parent)
{
    setPlotTitle("多通道时域波形"); xAxis->setLabel("时间 (s)"); yAxis->setLabel(AcousticAnalysis::amplitudeLabel(AcousticUnit::FullScale));
    xAxis->setRange(0,1); yAxis->setRange(-1.05,1.05); legend->setVisible(true);
    m_lineA=new QCPItemStraightLine(this); m_lineB=new QCPItemStraightLine(this);
    for(auto* line:{m_lineA,m_lineB}) { line->setVisible(false); line->setPen(QPen(ThemeTokens::instance()->plotCursor(),1,Qt::DashLine)); }
    connect(ThemeTokens::instance(), &ThemeTokens::tokensChanged, this, [this] {
        for (auto* line : {m_lineA, m_lineB}) { QPen pen = line->pen(); pen.setColor(ThemeTokens::instance()->plotCursor()); line->setPen(pen); }
        replot();
    });
    setNavigationAxes(Qt::Horizontal); captureResetView();
    connect(this,&StyledPlot::userNavigationStarted,this,[this] { setFollowLatest(false); });
    connect(this,&StyledPlot::viewReset,this,[this] { setFollowLatest(true); });
    connect(this,&StyledPlot::interactionModeChanged,this,[this](InteractionMode mode) {
        m_cursorMode=mode==InteractionMode::Inspect; m_nextCursorB=false;
        if(m_cursorMode) setFollowLatest(false);
    });
    connect(this,&StyledPlot::coordinateClicked,this,[this](double at,double,Qt::KeyboardModifiers) {
        if(!m_cursorMode) return;
        if(m_nextCursorB) setCursors(m_cursorA,at); else setCursors(at,at);
        m_nextCursorB=!m_nextCursorB;
    });
}
bool AcousticWaveformPlot::configure(double rate,int capacity,AcousticUnit unit)
{
    if(!std::isfinite(rate) || rate<=0 || rate>1e9 || capacity<16 || capacity>1048576 || AcousticAnalysis::amplitudeLabel(unit).isEmpty()) return false;
    m_sampleRate=rate; m_capacity=capacity; yAxis->setLabel(AcousticAnalysis::amplitudeLabel(unit)); clearSamples(); return true;
}
int AcousticWaveformPlot::addChannel(const QString& name,const QColor& color)
{
    if(m_channels.size()>=16 || name.isEmpty()) return -1;
    auto* curve=addGraph(); curve->setName(name); curve->setPen(QPen(Qt::black,1.3)); setSeriesColor(curve,m_channels.size(),color); curve->setAdaptiveSampling(true);
    m_channels.append({curve,false,0}); replot(); return m_channels.size()-1;
}
bool AcousticWaveformPlot::appendSamples(int channel,double start,const QVector<double>& samples)
{
    if(channel<0 || channel>=m_channels.size() || samples.isEmpty() || samples.size()>m_capacity
        || !std::isfinite(start) || std::abs(start)>1e12) return false;
    const double end=start+(samples.size()-1)/m_sampleRate;
    if(!std::isfinite(end) || std::abs(end)>1e12 || start+1/m_sampleRate<=start || end+1/m_sampleRate<=end || (samples.size()>1 && end<=start)) return false;
    for(double value:samples) if(std::isinf(value)) return false;
    auto& target=m_channels[channel];
    if(target.hasData && start<target.lastTime+1/m_sampleRate-.25/m_sampleRate) return false;
    QVector<double> times,values; times.reserve(samples.size()+1); values.reserve(samples.size()+1);
    if(target.hasData && start>target.lastTime+1.5/m_sampleRate) {
        times.append(target.lastTime+.5/m_sampleRate); values.append(std::numeric_limits<double>::quiet_NaN());
    }
    for(int i=0;i<samples.size();++i) { times.append(start+i/m_sampleRate); values.append(samples[i]); }
    auto data=target.graph->data();
    target.graph->addData(times,values,true);
    if(data->size()>m_capacity) {
        const auto keep=data->constBegin()+(data->size()-m_capacity);
        data->removeBefore(keep->key);
    }
    target.lastTime=end; target.hasData=true; m_latest=m_hasLatest ? std::max(m_latest,end) : end; m_hasLatest=true;
    setResetRanges(QCPRange(m_latest-m_duration,m_latest),resetYRange());
    if(m_follow) xAxis->setRange(m_latest-m_duration,m_latest);
    replot(); return true;
}
void AcousticWaveformPlot::clearSamples()
{
    for(auto& channel:m_channels) { channel.graph->data()->clear(); channel.hasData=false; }
    m_hasLatest=false; m_latest=0; m_cursorA=0; m_cursorB=0; m_lineA->setVisible(false); m_lineB->setVisible(false); m_nextCursorB=false;
    xAxis->setRange(0,m_duration); setResetRanges(xAxis->range(),resetYRange()); replot();
}
int AcousticWaveformPlot::storedPointCount(int channel) const { return channel>=0 && channel<m_channels.size() ? m_channels[channel].graph->data()->size() : 0; }
void AcousticWaveformPlot::setChannelVisible(int channel,bool visible) { if(channel>=0 && channel<m_channels.size()) { m_channels[channel].graph->setVisible(visible); replot(); } }
void AcousticWaveformPlot::setFollowLatest(bool enabled)
{
    if(m_follow==enabled) return; m_follow=enabled;
    if(enabled && m_hasLatest) xAxis->setRange(m_latest-m_duration,m_latest);
    emit followLatestChanged(enabled); replot();
}
bool AcousticWaveformPlot::setVisibleDuration(double seconds)
{
    if(!std::isfinite(seconds) || seconds<=0 || seconds>86400*30 || !QCPRange::validRange(0,seconds)) return false;
    m_duration=seconds;
    setResetRanges(QCPRange(m_hasLatest?m_latest-seconds:0,m_hasLatest?m_latest:seconds),resetYRange());
    if(m_follow) xAxis->setRange(m_hasLatest ? m_latest-seconds : 0,m_hasLatest ? m_latest : seconds);
    replot(); return true;
}
bool AcousticWaveformPlot::setCursors(double a,double b)
{
    if(!std::isfinite(a)||!std::isfinite(b)||std::abs(a)>1e12||std::abs(b)>1e12) return false;
    m_cursorA=a; m_cursorB=b;
    m_lineA->point1->setCoords(a,0); m_lineA->point2->setCoords(a,1);
    m_lineB->point1->setCoords(b,0); m_lineB->point2->setCoords(b,1);
    m_lineA->setVisible(true); m_lineB->setVisible(true); emit cursorsChanged(a,b); replot(); return true;
}
void AcousticWaveformPlot::setCursorModeEnabled(bool enabled)
{
    setInteractionMode(enabled ? InteractionMode::Inspect : InteractionMode::Pan);
}
bool AcousticWaveformPlot::fitAmplitude()
{
    double minimum=std::numeric_limits<double>::infinity(),maximum=-minimum;
    for(const auto& channel:m_channels) if(channel.graph->visible()) for(auto it=channel.graph->data()->constBegin();it!=channel.graph->data()->constEnd();++it) {
        if(std::isfinite(it->value)) { minimum=std::min(minimum,it->value); maximum=std::max(maximum,it->value); }
    }
    if(!std::isfinite(minimum)||!std::isfinite(maximum)) return false;
    const double padding=std::max((maximum-minimum)*.1,std::max(std::abs(maximum)*.01,1e-9));
    if(!std::isfinite(minimum-padding)||!std::isfinite(maximum+padding)) return false;
    yAxis->setRange(minimum-padding,maximum+padding); setResetRanges(resetXRange(),yAxis->range()); replot(); return true;
}
}
