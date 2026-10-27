#include <basewidgets/Charts/RateHistoryPlot.h>
#include <cmath>
namespace basewidgets {
RateHistoryPlot::RateHistoryPlot(QWidget* parent) : StyledPlot(parent)
{
    setPlotTitle("速率历程"); xAxis->setLabel("时间 (s)"); yAxis->setLabel("速率 (MiB/s)"); xAxis->setRange(0,60); yAxis->setRange(0,1); legend->setVisible(true); captureResetView();
    connect(this,&StyledPlot::userNavigationStarted,this,[this] { setFollowLatest(false); });
    connect(this,&StyledPlot::viewReset,this,[this] { setFollowLatest(true); });
}
bool RateHistoryPlot::configure(int capacity,double window,const QString& unit,double gap)
{
    if(capacity<2||capacity>8192||!std::isfinite(window)||window<=0||window>86400*30||!QCPRange::validRange(0,window)||unit.isEmpty()||!std::isfinite(gap)||gap<=0) return false;
    m_capacity=capacity; m_window=window; m_gap=gap; yAxis->setLabel(unit); clearSamples(); return true;
}
int RateHistoryPlot::addSeries(const QString& name,const QColor& color)
{
    if(name.isEmpty()||m_series.size()>=16||m_hasData) return -1;
    static const QColor colors[]={QColor("#409EFF"),QColor("#67C23A"),QColor("#E6A23C"),QColor("#C084FC")};
    auto* curve=addGraph(); curve->setName(name); curve->setPen(QPen(color.isValid()?color:colors[m_series.size()%4],1.5));
    m_series.append(curve); replot(); return m_series.size()-1;
}
bool RateHistoryPlot::appendValues(double time,const QVector<double>& values)
{
    if(m_series.isEmpty()||values.size()!=m_series.size()||!std::isfinite(time)||std::abs(time)>1e12||(m_hasData&&time<=m_last)) return false;
    for(double value:values) if(std::isinf(value)||(std::isfinite(value)&&std::abs(value)>QCPRange::maxRange/100)) return false;
    const bool gap=m_hasData && time-m_last>m_gap;
    for(int i=0;i<m_series.size();++i) {
        auto* curve=m_series[i];
        if(gap) curve->addData(m_last+(time-m_last)/2,std::numeric_limits<double>::quiet_NaN());
        curve->addData(time,values[i]);
        if(curve->data()->size()>m_capacity) {
            const auto keep=curve->data()->constBegin()+curve->data()->size()-m_capacity; curve->data()->removeBefore(keep->key);
        }
    }
    m_last=time; m_hasData=true; updateDataView(); replot(); return true;
}
void RateHistoryPlot::updateDataView()
{
    QCPRange y=resetYRange();
    if(m_autoY) {
        double low=0,high=0;
        for(auto* curve:m_series) if(curve->visible()) for(auto it=curve->data()->constBegin();it!=curve->data()->constEnd();++it)
            if(std::isfinite(it->value)) { low=std::min(low,it->value); high=std::max(high,it->value); }
        const double pad=std::max((high-low)*.1,1e-6); y=QCPRange(low<0?low-pad:0,high+pad);
    }
    const QCPRange x=m_hasData?QCPRange(m_last-m_window,m_last):QCPRange(0,m_window);
    if(setResetRanges(x,y)&&m_follow) setViewRanges(x,y);
}
void RateHistoryPlot::clearSamples() { for(auto* curve:m_series) curve->data()->clear(); m_hasData=false; m_last=0; updateDataView(); replot(); }
void RateHistoryPlot::setFollowLatest(bool enabled) { const bool changed=m_follow!=enabled; m_follow=enabled; updateDataView(); if(changed)emit followLatestChanged(enabled); }
int RateHistoryPlot::storedPointCount() const { return m_series.isEmpty()?0:m_series.first()->data()->size(); }
void RateHistoryPlot::setAutoScaleY(bool enabled) { m_autoY=enabled; updateDataView(); }
}
