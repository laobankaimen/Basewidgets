#include <basewidgets/Charts/BeamPlots.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <cmath>
namespace basewidgets {
BeamPatternPlot::BeamPatternPlot(QWidget* parent) : StyledPlot(parent)
{
    setPlotTitle("多测向波束 · 点击拾取方位"); xAxis->setLabel("方位 (°，北=0，顺时针)"); yAxis->setLabel("波束电平 (dB)");
    xAxis->setRange(0,360); yAxis->setRange(-60,0); legend->setVisible(true); captureResetView();
    m_cursor=new QCPItemStraightLine(this); m_cursor->setPen(QPen(QColor("#E6A23C"),1,Qt::DashLine)); m_cursor->setVisible(false);
    connect(this,&StyledPlot::coordinateClicked,this,[this](double x,double y,Qt::KeyboardModifiers) {
        setBearingCursor(x); double best=std::numeric_limits<double>::infinity(),pickedX=0,pickedY=0; QString id;
        for(const auto& beam:m_beams) if(beam.graph->visible()) {
            const auto data=beam.graph->data(); const auto after=data->findBegin(x,false);
            auto consider=[&](QCPGraphDataContainer::const_iterator it) {
                if(it==data->constEnd()||!std::isfinite(it->value)) return;
                const double dx=xAxis->coordToPixel(it->key)-xAxis->coordToPixel(x),dy=yAxis->coordToPixel(it->value)-yAxis->coordToPixel(y);
                const double distance=dx*dx+dy*dy;
                if(distance<best) { best=distance; id=beam.id; pickedX=it->key; pickedY=it->value; }
            };
            consider(after); if(after!=data->constBegin()) consider(after-1);
        }
        if(!id.isEmpty()) emit beamPicked(id,pickedX,pickedY);
    });
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] {
        for(const auto& beam:m_beams) if(auto* item=legend->itemWithPlottable(beam.graph))
            item->setTextColor(beam.graph->visible()?ThemeTokens::instance()->textPrimary():ThemeTokens::instance()->textDisabled());
    });
    setInteraction(QCP::iSelectLegend,true);
    connect(this,&QCustomPlot::legendClick,this,[this](QCPLegend*,QCPAbstractLegendItem* item,QMouseEvent*) {
        auto* legendItem=qobject_cast<QCPPlottableLegendItem*>(item); if(!legendItem) return;
        for(const auto& beam:m_beams) if(beam.graph==legendItem->plottable()) {
            setBeamVisible(beam.id,!beam.graph->visible()); legendItem->setSelected(false); break;
        }
    });
}
bool BeamPatternPlot::setBearingRange(double lower,double upper)
{
    if(!std::isfinite(lower)||!std::isfinite(upper)||upper<=lower||upper-lower>360||lower< -180||upper>360) return false;
    // Changing the angular convention with data present requires an explicit clear.
    if(!m_beams.isEmpty()&&(lower!=m_lower||upper!=m_upper)) return false;
    m_lower=lower; m_upper=upper; xAxis->setRange(lower,upper); setResetRanges(xAxis->range(),resetYRange()); replot(); return true;
}
bool BeamPatternPlot::setLevelRange(double lower,double upper)
{
    if(upper<=lower||!QCPRange::validRange(lower,upper)) return false;
    yAxis->setRange(lower,upper); setResetRanges(resetXRange(),yAxis->range()); replot(); return true;
}
void BeamPatternPlot::setLevelUnit(const QString& label) { yAxis->setLabel(label); replot(); }
bool BeamPatternPlot::setBeam(const QString& id,const QString& name,const QVector<double>& bearings,const QVector<double>& levels,const QColor& color)
{
    if(id.isEmpty()||name.isEmpty()||bearings.size()!=levels.size()||bearings.size()<2||bearings.size()>16384) return false;
    for(int i=0;i<bearings.size();++i) if(!std::isfinite(bearings[i])||bearings[i]<m_lower||bearings[i]>m_upper
        ||(i&&bearings[i]<=bearings[i-1])||std::isinf(levels[i])) return false;
    int index=-1; for(int i=0;i<m_beams.size();++i) if(m_beams[i].id==id) index=i;
    if(index<0) {
        if(m_beams.size()>=32) return false;
        static const QColor colors[]={QColor("#409EFF"),QColor("#E6A23C"),QColor("#67C23A"),QColor("#C084FC"),QColor("#F472B6")};
        auto* curve=addGraph(); curve->setPen(QPen(color.isValid()?color:colors[m_beams.size()%5],1.5));
        m_beams.append({id,curve}); index=m_beams.size()-1;
    }
    auto* curve=m_beams[index].graph; curve->setName(name); if(color.isValid()) curve->setPen(QPen(color,1.5));
    curve->setData(bearings,levels,true); replot(); return true;
}
bool BeamPatternPlot::setBeamVisible(const QString& id,bool visible)
{
    for(auto& beam:m_beams) if(beam.id==id) {
        beam.graph->setVisible(visible);
        if(auto* item=legend->itemWithPlottable(beam.graph)) item->setTextColor(visible?ThemeTokens::instance()->textPrimary():ThemeTokens::instance()->textDisabled());
        replot(); return true;
    } return false;
}
bool BeamPatternPlot::removeBeam(const QString& id)
{
    for(int i=0;i<m_beams.size();++i) if(m_beams[i].id==id) { removeGraph(m_beams[i].graph); m_beams.removeAt(i); replot(); return true; } return false;
}
void BeamPatternPlot::clearBeams() { for(auto& beam:m_beams) removeGraph(beam.graph); m_beams.clear(); m_cursor->setVisible(false); replot(); }
bool BeamPatternPlot::setBearingCursor(double degrees)
{
    if(!std::isfinite(degrees)||degrees<m_lower||degrees>m_upper) return false;
    m_cursor->point1->setCoords(degrees,0); m_cursor->point2->setCoords(degrees,1); m_cursor->setVisible(true);
    emit bearingCursorChanged(degrees); replot(); return true;
}
void BeamPatternPlot::extendContextMenu(QMenu* menu,const QPointF& coordinate)
{
    StyledPlot::extendContextMenu(menu,coordinate); auto* beams=menu->addMenu(tr("波束显隐"));
    for(const auto& beam:m_beams) {
        auto* action=beams->addAction(beam.graph->name()); action->setCheckable(true); action->setChecked(beam.graph->visible());
        connect(action,&QAction::toggled,this,[this,id=beam.id](bool visible) { setBeamVisible(id,visible); });
    }
}
BearingHistoryPlot::BearingHistoryPlot(QWidget* parent) : HistoryHeatmapPlot(parent)
{
    setPlotTitle("测向历程 · 显示门限可调"); xAxis->setLabel("方位 (°，北=0，顺时针)");
    connect(this,&HistoryHeatmapPlot::cellSelected,this,&BearingHistoryPlot::bearingSelected);
}
bool BearingHistoryPlot::configure(double lower,double upper,int bins,int frames,double period,const QString& unit)
{
    if(lower< -180||upper>360||upper-lower>360||unit.isEmpty()) return false;
    return configureGrid(lower,upper,bins,frames,period,unit);
}
bool BearingHistoryPlot::appendFrame(double time,const QVector<double>& levels) { return appendRow(time,levels); }
}
