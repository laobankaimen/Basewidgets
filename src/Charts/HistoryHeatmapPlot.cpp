#include <basewidgets/Charts/HistoryHeatmapPlot.h>
#include <basewidgets/Core/FramelessDialog.h>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QActionGroup>
#include <cmath>
namespace basewidgets {
HistoryHeatmapPlot::HistoryHeatmapPlot(QWidget* parent) : StyledPlot(parent)
{
    yAxis->setLabel("时间 (s)");
    m_map=new QCPColorMap(xAxis,yAxis); m_map->setInterpolate(false); m_map->setTightBoundary(false); m_map->data()->fillAlpha(0);
    m_scale=new QCPColorScale(this); plotLayout()->addElement(1,1,m_scale); m_scale->setType(QCPAxis::atRight); m_map->setColorScale(m_scale);
    setColorPreset(ColorPreset::Viridis); m_map->setDataRange(QCPRange(m_lower,m_upper));
    // Layout and export dimensions must be known before choosing the display grid.
    connect(this,&QCustomPlot::afterLayout,this,&HistoryHeatmapPlot::synchronizeMap);
    connect(this,&StyledPlot::userNavigationStarted,this,[this] { setFollowLatest(false); });
    connect(this,&StyledPlot::viewReset,this,[this] { setFollowLatest(true); });
    connect(this,&StyledPlot::coordinateClicked,this,[this](double x,double time,Qt::KeyboardModifiers) {
        if(!m_hasTime || m_bins<2) return;
        const double index=(x-m_xMinimum)/(m_xMaximum-m_xMinimum)*(m_bins-1);
        const double rowIndex=(time-timeAt(0))/m_period;
        if(index<-.5||index>m_bins-.5||rowIndex<-.5||rowIndex>m_history.size()-.5) return;
        const int bin=qRound(index),row=qRound(rowIndex); const double value=levelAt(bin,row);
        emit cellSelected(timeAt(row),coordinateAt(bin),value,std::isfinite(value));
    });
    connect(m_scale,&QCPColorScale::dataRangeChanged,this,[this](const QCPRange& range) { setLevelRange(range.lower,range.upper); });
    refreshTheme();
}
bool HistoryHeatmapPlot::configureGrid(double lower,double upper,int bins,int capacity,double period,const QString& unit)
{
    if(upper<=lower||!QCPRange::validRange(lower,upper)||bins<2||bins>65536||capacity<2||capacity>2048||qint64(bins)*capacity>2097152
        ||!std::isfinite(period)||period<=0||period*capacity>86400*30||!QCPRange::validRange(0,period*(capacity-1))) return false;
    m_xMinimum=lower; m_xMaximum=upper; m_bins=bins; m_period=period; m_displayX=QCPRange(lower,upper);
    m_history=QVector<QVector<double>>(capacity); m_sourceCells=QRect(); m_displayGrid=QSize(); m_scale->setLabel(unit);
    xAxis->setRange(m_displayX); clearHistory(); return true;
}
void HistoryHeatmapPlot::clearHistory()
{
    for(auto& row:m_history) row.clear(); m_next=0; m_present=0; m_hasTime=false; m_last=0; m_dirty=true;
    if(!m_history.isEmpty()) {
        const QCPRange time(0,m_period*m_history.size()); setResetRanges(m_displayX,time);
        if(m_follow) setViewRanges(m_displayX,time);
    }
    replot();
}
void HistoryHeatmapPlot::push(const QVector<double>& levels)
{
    if(!m_history[m_next].isEmpty()) --m_present;
    m_history[m_next]=levels; if(!levels.isEmpty()) ++m_present;
    m_next=(m_next+1)%m_history.size();
}
bool HistoryHeatmapPlot::appendRow(double time,const QVector<double>& levels)
{
    if(m_history.isEmpty()||levels.size()!=m_bins||!std::isfinite(time)||std::abs(time)>1e12||time+m_period<=time) return false;
    for(double value:levels) if(std::isinf(value)) return false;
    if(m_hasTime) {
        const double steps=(time-m_last)/m_period;
        if(!std::isfinite(steps)||steps<.5||std::abs(steps-std::round(steps))>1e-4) return false;
        if(steps>=m_history.size()) { for(auto& row:m_history) row.clear(); m_next=0; m_present=0; }
        else for(int i=1;i<int(std::round(steps));++i) push({});
    }
    push(levels); m_last=time; m_hasTime=true; m_dirty=true;
    const QCPRange timeRange(timeAt(0)-m_period/2,time+m_period/2); setResetRanges(m_displayX,timeRange);
    if(m_follow) yAxis->setRange(timeRange);
    replot(); return true;
}
bool HistoryHeatmapPlot::setDisplayedXRange(double lower,double upper)
{
    if(upper<=lower||!QCPRange::validRange(lower,upper)||lower<m_xMinimum||upper>m_xMaximum) return false;
    m_displayX=QCPRange(lower,upper); xAxis->setRange(m_displayX); setResetRanges(m_displayX,resetYRange()); replot(); return true;
}
bool HistoryHeatmapPlot::setLevelRange(double lower,double upper)
{
    if(upper<=lower||!QCPRange::validRange(lower,upper)||!std::isfinite(lower)||!std::isfinite(upper)) return false;
    if(m_lower==lower && m_upper==upper) return true;
    m_lower=lower; m_upper=upper; m_map->setDataRange(QCPRange(lower,upper)); m_dirty=true;
    emit levelRangeChanged(lower,upper); replot(); return true;
}
bool HistoryHeatmapPlot::setDisplayThreshold(double value,bool enabled)
{
    if(!std::isfinite(value)) return false;
    m_threshold=value; m_thresholdEnabled=enabled; m_dirty=true; emit displayThresholdChanged(value,enabled); replot(); return true;
}
void HistoryHeatmapPlot::clearDisplayThreshold() { setDisplayThreshold(m_threshold,false); }
void HistoryHeatmapPlot::setColorPreset(ColorPreset preset)
{
    QCPColorGradient gradient;
    if(preset==ColorPreset::Gray) gradient=QCPColorGradient(QCPColorGradient::gpGrayscale);
    else if(preset==ColorPreset::Heat) gradient=QCPColorGradient(QCPColorGradient::gpHot);
    else {
        preset=ColorPreset::Viridis; gradient.setColorInterpolation(QCPColorGradient::ciRGB);
        gradient.setColorStopAt(0,QColor("#440154")); gradient.setColorStopAt(.25,QColor("#3B528B"));
        gradient.setColorStopAt(.5,QColor("#21918C")); gradient.setColorStopAt(.75,QColor("#5EC962")); gradient.setColorStopAt(1,QColor("#FDE725"));
    }
    m_preset=preset; m_map->setGradient(gradient); replot();
}
void HistoryHeatmapPlot::setFollowLatest(bool enabled)
{
    const bool changed=m_follow!=enabled; m_follow=enabled;
    if(enabled && m_hasTime) setViewRanges(m_displayX,QCPRange(timeAt(0)-m_period/2,m_last+m_period/2));
    if(changed) emit followLatestChanged(enabled); replot();
}
double HistoryHeatmapPlot::levelAt(int bin,int row) const
{
    if(bin<0||bin>=m_bins||row<0||row>=m_history.size()||!m_hasTime) return std::numeric_limits<double>::quiet_NaN();
    const auto& values=m_history[(m_next+row)%m_history.size()]; return values.isEmpty()?std::numeric_limits<double>::quiet_NaN():values[bin];
}
double HistoryHeatmapPlot::timeAt(int row) const
{
    return m_hasTime && row>=0 && row<m_history.size()?m_last-(m_history.size()-1-row)*m_period:std::numeric_limits<double>::quiet_NaN();
}
double HistoryHeatmapPlot::coordinateAt(int bin) const
{
    return bin>=0 && bin<m_bins && m_bins>1 ? m_xMinimum+(m_xMaximum-m_xMinimum)*double(bin)/(m_bins-1) : std::numeric_limits<double>::quiet_NaN();
}
QSize HistoryHeatmapPlot::displayGridSize() const { return m_map->visible()?m_displayGrid:QSize(); }
void HistoryHeatmapPlot::synchronizeMap()
{
    if(m_history.isEmpty())return;
    const double first=m_hasTime?timeAt(0):0;
    const double dx=(m_xMaximum-m_xMinimum)/(m_bins-1);
    // Convert only after clamping; zoomed-out ranges may exceed integer limits.
    auto visibleCells=[](const QCPRange& view,double origin,double step,int count) {
        const double lo=(view.lower-origin)/step,hi=(view.upper-origin)/step;
        if(hi<-.5||lo>count-.5)return QPair<int,int>(0,0);
        int begin=int(qBound(0.,std::floor(lo-.5),double(count-1)));
        int end=int(qBound(double(begin+1),std::ceil(hi+1.5),double(count)));
        // QCPColorMap requires non-zero range, including when one cell fills the viewport.
        if(end-begin<2) { begin=qMax(0,begin-1); end=qMin(count,begin+2); }
        return QPair<int,int>(begin,end);
    };
    const auto x=visibleCells(xAxis->range(),m_xMinimum,dx,m_bins);
    const auto y=visibleCells(yAxis->range(),first,m_period,m_history.size());
    if(x.first==x.second||y.first==y.second) { m_map->setVisible(false); return; }
    m_map->setVisible(true);
    const QRect cells(x.first,y.first,x.second-x.first,y.second-y.first);
    const QSize grid(qMin(cells.width(),qBound(2,axisRect()->width(),1024)),
                     qMin(cells.height(),qBound(2,axisRect()->height(),512)));
    if(!m_dirty && cells==m_sourceCells && grid==m_displayGrid)return;
    m_sourceCells=cells; m_displayGrid=grid;
    auto* data=m_map->data(); data->setSize(grid.width(),grid.height());
    // Keep original cell edges while replacing each group with one display cell.
    const double sx=dx*cells.width()/grid.width(),sy=m_period*cells.height()/grid.height();
    const double left=m_xMinimum+(cells.left()-.5)*dx,top=first+(cells.top()-.5)*m_period;
    data->setRange(QCPRange(left+sx/2,left+dx*cells.width()-sx/2),
                   QCPRange(top+sy/2,top+m_period*cells.height()-sy/2));
    QVector<double> peaks(grid.width());
    for(int row=0;row<grid.height();++row) {
        std::fill(peaks.begin(),peaks.end(),-std::numeric_limits<double>::infinity());
        const int rowBegin=cells.top()+row*cells.height()/grid.height();
        const int rowEnd=cells.top()+(row+1)*cells.height()/grid.height();
        for(int sourceRow=rowBegin;sourceRow<rowEnd;++sourceRow) {
            const auto& values=m_history[(m_next+sourceRow)%m_history.size()];
            if(!m_hasTime||values.isEmpty())continue;
            for(int col=0;col<grid.width();++col) {
                const int begin=cells.left()+col*cells.width()/grid.width();
                const int end=cells.left()+(col+1)*cells.width()/grid.width();
                double peak=peaks[col];
                for(int bin=begin;bin<end;++bin)if(std::isfinite(values[bin]))peak=qMax(peak,values[bin]);
                peaks[col]=peak;
            }
        }
        for(int col=0;col<grid.width();++col) {
            const double value=peaks[col]; const bool available=std::isfinite(value);
            data->setCell(col,row,available?value:m_lower);
            data->setAlpha(col,row,available&&(!m_thresholdEnabled||value>=m_threshold)?255:0);
        }
    }
    m_dirty=false;
}
void HistoryHeatmapPlot::extendContextMenu(QMenu* menu,const QPointF& point)
{
    StyledPlot::extendContextMenu(menu,point); menu->addSeparator();
    auto* edit=menu->addAction(tr("色标范围 / 显示门限…"),this,&HistoryHeatmapPlot::editColors); edit->setObjectName("history.colors");
    auto* colors=menu->addMenu(tr("色阶")); auto* group=new QActionGroup(colors);
    for(int i=0;i<3;++i) {
        auto* action=colors->addAction(QStringList{"Viridis",tr("灰度"),tr("热力")}.at(i)); action->setCheckable(true); action->setChecked(i==int(m_preset)); group->addAction(action);
        connect(action,&QAction::triggered,this,[this,i] { setColorPreset(static_cast<ColorPreset>(i)); });
    }
    auto* follow=menu->addAction(tr("跟随最新时间")); follow->setObjectName("history.follow"); follow->setCheckable(true); follow->setChecked(m_follow);
    connect(follow,&QAction::toggled,this,&HistoryHeatmapPlot::setFollowLatest);
    auto* clear=menu->addAction(tr("清空显示历史"),this,&HistoryHeatmapPlot::clearHistory); clear->setObjectName("history.clear");
}
void HistoryHeatmapPlot::editColors()
{
    FramelessDialog dialog(true,this); dialog.setTitle(tr("色标与显示门限"));
    auto* body=new QWidget; auto* form=new QFormLayout(body);
    auto make=[&](double value) { auto* spin=new QDoubleSpinBox(body); spin->setDecimals(3); spin->setRange(-1e12,1e12); spin->setValue(value); return spin; };
    auto* lower=make(m_lower); auto* upper=make(m_upper); auto* threshold=make(m_threshold);
    auto* enabled=new QCheckBox(tr("隐藏低于门限的单元格（不修改原始值）"),body); enabled->setChecked(m_thresholdEnabled);
    auto* error=new QLabel(body); error->setWordWrap(true);
    form->addRow(tr("色标下限"),lower); form->addRow(tr("色标上限"),upper); form->addRow(tr("显示门限"),threshold); form->addRow(enabled); form->addRow(error);
    dialog.setContentWidget(body);
    connect(&dialog,&FramelessDialog::okButtonClicked,&dialog,[&] {
        if(lower->value()>=upper->value()) { error->setText(tr("色标上限必须大于下限")); return; }
        setLevelRange(lower->value(),upper->value()); setDisplayThreshold(threshold->value(),enabled->isChecked()); dialog.accept();
    });
    connect(&dialog,&FramelessDialog::cancelButtonClicked,&dialog,&QDialog::reject); dialog.exec();
}
}
