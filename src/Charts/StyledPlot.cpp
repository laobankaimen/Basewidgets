#include <basewidgets/Charts/StyledPlot.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QActionGroup>
#include <QClipboard>
#include <QFileDialog>
#include <QScopedPointer>
#include <QTimer>
#include <basewidgets/Charts/SampledGraph.h>
#include <cmath>
namespace basewidgets {
StyledPlot::StyledPlot(QWidget* parent) : QCustomPlot(parent)
{
    StyleManagerAdapter::instance();
    // Dense non-cosmetic polylines can spend seconds in the Qt stroker. The
    // QCustomPlot segment path preserves the pen while bounding per-segment work.
    setPlottingHint(QCP::phFastPolylines,true);
    setPlottingHint(QCP::phImmediateRefresh,false);
    m_replotTimer=new QTimer(this); m_replotTimer->setSingleShot(true); m_replotTimer->setTimerType(Qt::PreciseTimer);
    connect(m_replotTimer,&QTimer::timeout,this,[this] { QCustomPlot::replot(QCustomPlot::rpQueuedRefresh); });
    connect(this,&QCustomPlot::beforeReplot,this,[this] { m_lastReplot.restart(); });
    setMinimumSize(280,220);
    axisRect()->setupFullAxesBox(true);
    plotLayout()->insertRow(0);
    m_title=new QCPTextElement(this,QString(),font()); plotLayout()->addElement(0,0,m_title);
    setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    setNavigationAxes(Qt::Orientations(Qt::Horizontal)|Qt::Vertical);
    setCursor(Qt::CrossCursor); setFocusPolicy(Qt::StrongFocus); setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint& position) {
        auto* menu=createContextMenu(position,this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->popup(mapToGlobal(position)); // nonblocking; deleting the plot also safely closes its menu
    });
    connect(selectionRect(),&QCPSelectionRect::accepted,this,[this](const QRect& rect,QMouseEvent*) { acceptRectangle(rect); });
    auto changed=[this](const QCPRange&) {
        emit viewRangesChanged(xAxis->range().lower,xAxis->range().upper,yAxis->range().lower,yAxis->range().upper);
    };
    connect(xAxis,QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),this,changed);
    connect(yAxis,QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),this,changed);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&StyledPlot::refreshTheme);
    refreshTheme();
}
void StyledPlot::setPlotTitle(const QString& title) { m_title->setText(title); replot(); }
QString StyledPlot::plotTitle() const { return m_title->text(); }
bool StyledPlot::setSeriesColor(QCPGraph* graph, int paletteIndex, const QColor& overrideColor)
{
    if (!graph || graph->parentPlot() != this || paletteIndex < 0) return false;
    for (int i = m_themedSeries.size() - 1; i >= 0; --i)
        if (!m_themedSeries[i].graph || m_themedSeries[i].graph == graph) m_themedSeries.removeAt(i);
    const QColor color = overrideColor.isValid() ? overrideColor : ThemeTokens::instance()->seriesColor(paletteIndex);
    QPen pen = graph->pen(); pen.setColor(color); graph->setPen(pen);
    if (!overrideColor.isValid()) m_themedSeries.append({graph, paletteIndex, color});
    replot(); return true;
}
void StyledPlot::refreshTheme()
{
    auto* tokens=ThemeTokens::instance();
    // Weak references tolerate removeGraph/clearGraphs; explicit business colors win.
    for (int i = m_themedSeries.size() - 1; i >= 0; --i) {
        auto& series = m_themedSeries[i];
        if (!series.graph || series.graph->pen().color() != series.appliedColor) {
            m_themedSeries.removeAt(i); continue;
        }
        series.appliedColor = tokens->seriesColor(series.paletteIndex);
        QPen pen = series.graph->pen(); pen.setColor(series.appliedColor); series.graph->setPen(pen);
    }
    setBackground(tokens->bg0());
    m_title->setTextColor(tokens->textPrimary());
    for(auto* rect:axisRects()) {
        rect->setBackground(tokens->bg1());
        for(auto* axis:rect->axes()) {
            axis->setBasePen(QPen(tokens->border())); axis->setTickPen(QPen(tokens->border()));
            axis->setSubTickPen(QPen(tokens->borderLight())); axis->setLabelColor(tokens->textPrimary());
            axis->setTickLabelColor(tokens->textSecondary());
            axis->grid()->setPen(QPen(tokens->borderLight(),.6,Qt::DotLine));
            axis->grid()->setZeroLinePen(QPen(tokens->border(),1));
        }
    }
    for(auto* scale:findChildren<QCPColorScale*>()) {
        auto* axis=scale->axis(); axis->setBasePen(QPen(tokens->border())); axis->setTickPen(QPen(tokens->border()));
        axis->setLabelColor(tokens->textPrimary()); axis->setTickLabelColor(tokens->textSecondary());
    }
    selectionRect()->setPen(QPen(tokens->primary(),1,Qt::DashLine));
    QColor selection=tokens->primary(); selection.setAlpha(35); selectionRect()->setBrush(selection);
    legend->setTextColor(tokens->textPrimary()); legend->setBrush(tokens->bg1()); legend->setBorderPen(QPen(tokens->borderLight()));
    replot();
}
QCPGraph* StyledPlot::addGraph(QCPAxis* key,QCPAxis* value)
{
    if(!key)key=xAxis; if(!value)value=yAxis;
    if(!key||!value||key->parentPlot()!=this||value->parentPlot()!=this)return nullptr;
    auto* graph=new SampledGraph(key,value); graph->setName(QString("Graph %1").arg(graphCount())); return graph;
}
bool StyledPlot::setMaximumFrameRate(int rate)
{
    if(rate<1||rate>120)return false;
    m_maximumFrameRate=rate;
    if(m_replotTimer->isActive()) { m_replotTimer->stop(); replot(); }
    return true;
}
void StyledPlot::replot(QCustomPlot::RefreshPriority priority)
{
    if(priority!=QCustomPlot::rpQueuedReplot) { m_replotTimer->stop(); QCustomPlot::replot(priority); return; }
    if(!m_replotTimer->isActive()) {
        const int interval=(1000+m_maximumFrameRate-1)/m_maximumFrameRate;
        const int delay=m_lastReplot.isValid()?int(qMax(qint64(0),interval-m_lastReplot.elapsed())):0;
        m_replotTimer->start(delay);
    }
}
void StyledPlot::setNavigationAxes(Qt::Orientations axes)
{
    m_navigationAxes=axes & (Qt::Orientations(Qt::Horizontal)|Qt::Vertical);
    axisRect()->setRangeDrag(m_navigationAxes); axisRect()->setRangeZoom(m_navigationAxes);
}
bool StyledPlot::setResetRanges(const QCPRange& x,const QCPRange& y)
{
    if(x.lower>=x.upper||y.lower>=y.upper||!QCPRange::validRange(x)||!QCPRange::validRange(y)
        || (xAxis->scaleType()==QCPAxis::stLogarithmic && x.lower<=0)
        || (yAxis->scaleType()==QCPAxis::stLogarithmic && y.lower<=0)) return false;
    m_homeX=x; m_homeY=y; m_hasHome=true; return true;
}
void StyledPlot::captureResetView() { setResetRanges(xAxis->range(),yAxis->range()); }
bool StyledPlot::setViewRanges(const QCPRange& x,const QCPRange& y)
{
    if(x.lower>=x.upper||y.lower>=y.upper||!QCPRange::validRange(x)||!QCPRange::validRange(y)
        || (xAxis->scaleType()==QCPAxis::stLogarithmic && x.lower<=0)
        || (yAxis->scaleType()==QCPAxis::stLogarithmic && y.lower<=0)) return false;
    xAxis->setRange(x); yAxis->setRange(y); replot(); return true;
}
void StyledPlot::resetView()
{
    if(m_hasHome) setViewRanges(m_homeX,m_homeY);
    else { rescaleAxes(); captureResetView(); }
    emit viewReset(); replot();
}
void StyledPlot::notifyUserNavigation()
{
    if(!m_hasHome) captureResetView();
    emit userNavigationStarted();
}
void StyledPlot::applyMode()
{
    const bool rectangle=m_temporaryZoom || m_mode==InteractionMode::RectangleZoom || m_mode==InteractionMode::RectangleSelect;
    setSelectionRectMode(rectangle ? QCP::srmCustom : QCP::srmNone);
    setInteraction(QCP::iRangeDrag,!rectangle && m_mode==InteractionMode::Pan);
}
void StyledPlot::setInteractionMode(InteractionMode mode)
{
    if(mode!=InteractionMode::Pan && mode!=InteractionMode::RectangleZoom && mode!=InteractionMode::RectangleSelect && mode!=InteractionMode::Inspect) return;
    if(selectionRect()->isActive()) selectionRect()->cancel();
    m_mode=mode; m_temporaryZoom=false; applyMode(); emit interactionModeChanged(mode);
}
void StyledPlot::acceptRectangle(const QRect& rectangle)
{
    const QRect area=rectangle.normalized().intersected(axisRect()->rect());
    if(area.width()<4 || area.height()<4) return;
    const double a=xAxis->pixelToCoord(area.left()),b=xAxis->pixelToCoord(area.right());
    const double c=yAxis->pixelToCoord(area.top()),d=yAxis->pixelToCoord(area.bottom());
    const QCPRange x(qMin(a,b),qMax(a,b)),y(qMin(c,d),qMax(c,d));
    emit selectionMade(x.lower,x.upper,y.lower,y.upper);
    if(m_mode==InteractionMode::RectangleZoom || m_temporaryZoom) {
        notifyUserNavigation();
        setViewRanges(m_navigationAxes.testFlag(Qt::Horizontal)?x:xAxis->range(),m_navigationAxes.testFlag(Qt::Vertical)?y:yAxis->range());
    }
}
void StyledPlot::mousePressEvent(QMouseEvent* event)
{
    m_pressed=event->button()==Qt::LeftButton && axisRect()->rect().contains(event->pos());
    if(m_pressed) {
        m_pressPixel=event->pos();
        m_temporaryZoom=m_mode==InteractionMode::Pan && event->modifiers().testFlag(Qt::ShiftModifier);
        applyMode();
        if(m_mode!=InteractionMode::Inspect) notifyUserNavigation();
    }
    QCustomPlot::mousePressEvent(event);
}
void StyledPlot::mouseReleaseEvent(QMouseEvent* event)
{
    const bool click=m_pressed && event->button()==Qt::LeftButton && !m_temporaryZoom
        && (m_mode==InteractionMode::Pan || m_mode==InteractionMode::Inspect)
        && (event->pos()-m_pressPixel).manhattanLength()<4 && axisRect()->rect().contains(event->pos());
    QCustomPlot::mouseReleaseEvent(event);
    if(click) emit coordinateClicked(xAxis->pixelToCoord(event->pos().x()),yAxis->pixelToCoord(event->pos().y()),event->modifiers());
    m_pressed=false; m_temporaryZoom=false; applyMode();
}
void StyledPlot::mouseMoveEvent(QMouseEvent* event)
{
    QCustomPlot::mouseMoveEvent(event);
    if(axisRect()->rect().contains(event->pos())) emit coordinateHovered(xAxis->pixelToCoord(event->pos().x()),yAxis->pixelToCoord(event->pos().y()));
}
void StyledPlot::mouseDoubleClickEvent(QMouseEvent* event)
{
    QCustomPlot::mouseDoubleClickEvent(event);
    if(event->button()==Qt::LeftButton && axisRect()->rect().contains(event->pos())) { m_pressed=false; resetView(); }
}
void StyledPlot::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const QPointF position=event->position();
#else
    const QPointF position=event->posF();
#endif
    if(!axisRect()->rect().contains(position.toPoint())) { QCustomPlot::wheelEvent(event); return; }
    emit mouseWheel(event);
    if(interactions().testFlag(QCP::iRangeZoom) && event->angleDelta().y()!=0) {
        notifyUserNavigation();
        // Same axis/anchor semantics as QCPAxisRect, with a queued frame instead
        // of a synchronous render for every wheel packet.
        const double steps=event->angleDelta().y()/120.;
        for(auto orientation:{Qt::Horizontal,Qt::Vertical})if(axisRect()->rangeZoom().testFlag(orientation)) {
            const double factor=std::pow(axisRect()->rangeZoomFactor(orientation),steps);
            if(!std::isfinite(factor)||factor<=0)continue;
            for(auto* axis:axisRect()->rangeZoomAxes(orientation))if(axis)
                axis->scaleRange(factor,axis->pixelToCoord(orientation==Qt::Horizontal?position.x():position.y()));
        }
        replot();
    }
    event->accept();
}
void StyledPlot::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_Home) { resetView(); event->accept(); return; }
    if(event->key()==Qt::Key_Escape) {
        selectionRect()->cancel(); m_pressed=false; m_temporaryZoom=false; applyMode(); event->accept(); return;
    }
    QWidget::keyPressEvent(event);
}
void StyledPlot::extendContextMenu(QMenu*,const QPointF&) {}
QMenu* StyledPlot::createContextMenu(const QPoint& pixel,QWidget* parent)
{
    auto* menu=new QMenu(parent);
    auto* reset=menu->addAction(tr("恢复视图（双击 / Home）"),this,&StyledPlot::resetView); reset->setObjectName("plot.reset");
    menu->addSeparator(); auto* group=new QActionGroup(menu); group->setExclusive(true);
    const QStringList names{tr("平移 / 滚轮缩放"),tr("框选放大"),tr("框选区域（发出选择信号）"),tr("坐标拾取 / 游标")};
    for(int i=0;i<names.size();++i) {
        auto* action=menu->addAction(names[i]); action->setCheckable(true); action->setChecked(int(m_mode)==i);
        action->setObjectName(QString("plot.mode.%1").arg(i)); group->addAction(action);
        connect(action,&QAction::triggered,this,[this,i] { setInteractionMode(static_cast<InteractionMode>(i)); });
    }
    const QPointF coordinate(xAxis->pixelToCoord(pixel.x()),yAxis->pixelToCoord(pixel.y()));
    menu->addSeparator();
    auto* copy=menu->addAction(tr("复制当前位置坐标")); copy->setEnabled(axisRect()->rect().contains(pixel)); copy->setObjectName("plot.copyCoordinate");
    connect(copy,&QAction::triggered,this,[this,coordinate] { QApplication::clipboard()->setText(QString("%1 = %2\n%3 = %4").arg(xAxis->label()).arg(coordinate.x(),0,'g',14).arg(yAxis->label()).arg(coordinate.y(),0,'g',14)); });
    auto* save=menu->addAction(tr("导出当前图像 PNG…")); save->setObjectName("plot.exportPng");
    connect(save,&QAction::triggered,this,[this] {
        const QString path=QFileDialog::getSaveFileName(this,tr("导出图像"),QString(),"PNG (*.png)");
        if(!path.isEmpty()) saveSnapshot(path);
    });
    extendContextMenu(menu,coordinate);
    emit prepareContextMenu(menu,coordinate);
    return menu;
}
bool StyledPlot::saveSnapshot(const QString& fileName,int width,int height,double scale)
{
    if(fileName.isEmpty() || width<0 || height<0 || !std::isfinite(scale) || scale<=0 || scale>8) return false;
    replot(QCustomPlot::rpImmediateRefresh); // flush pending history maps before synchronous export
    return savePng(fileName,width,height,scale);
}

}
