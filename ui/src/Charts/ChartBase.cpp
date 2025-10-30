#include "QCustomplot.h"
#include <QtAdvancedUI/Charts/ChartBase.h>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QResizeEvent>

using namespace QtAdvancedUI;

QCustomPlot* ChartBase::plot() const { return m_plot; }

ChartBase::ChartBase(QWidget* parent)
    : StyledWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_plot = new QCustomPlot(this);
    layout->addWidget(m_plot);
    setLayout(layout);

    // 交互与性能优化
    QCP::Interactions interactions;
    interactions.setFlag(QCP::iRangeDrag, true);
    interactions.setFlag(QCP::iRangeZoom, true);
    interactions.setFlag(QCP::iSelectPlottables, true);
    m_plot->setInteractions(interactions);
    m_plot->setNoAntialiasingOnDrag(true);
    m_plot->setPlottingHints(QCP::phFastPolylines);
    if (m_plot->axisRect())
        m_plot->axisRect()->setupFullAxesBox();

    // 事件过滤器用于双击复位与光标跟踪
    m_plot->installEventFilter(this);

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);
    connect(m_updateTimer, &QTimer::timeout, [this]{
        if (m_plot)
            m_plot->replot(QCustomPlot::rpQueuedReplot);
    });
    m_updateTimer->start();
}

ChartBase::~ChartBase() {}

void ChartBase::setCursorTrackingEnabled(bool enable)
{
    m_cursorTracking = enable;
}

void ChartBase::setDoubleClickResetEnabled(bool enable)
{
    m_resetOnDoubleClick = enable;
}

void ChartBase::setUpdateIntervalMs(int ms)
{
    if (m_updateTimer)
        m_updateTimer->setInterval(qMax(1, ms));
}

bool ChartBase::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_plot) {
        switch (ev->type()) {
        case QEvent::MouseButtonDblClick:
            if (m_resetOnDoubleClick) {
                restoreInitialRanges();
                return true;
            }
            break;
        case QEvent::MouseMove:
            if (m_cursorTracking) {
                auto* me = static_cast<QMouseEvent*>(ev);
                updateCursorHint(me->pos());
            }
            break;
        default:
            break;
        }
    }
    return StyledWidget::eventFilter(obj, ev);
}

void ChartBase::resizeEvent(QResizeEvent* e)
{
    StyledWidget::resizeEvent(e);
    requestReplotQueued();
}

void ChartBase::captureInitialRangesIfNeeded()
{
    if (!m_initialCaptured && m_plot) {
        m_xLower = m_plot->xAxis->range().lower;
        m_xUpper = m_plot->xAxis->range().upper;
        m_yLower = m_plot->yAxis->range().lower;
        m_yUpper = m_plot->yAxis->range().upper;
        m_initialCaptured = true;
    }
}

void ChartBase::restoreInitialRanges()
{
    if (m_plot) {
        if (!m_initialCaptured)
            captureInitialRangesIfNeeded();
        m_plot->xAxis->setRange(m_xLower, m_xUpper);
        m_plot->yAxis->setRange(m_yLower, m_yUpper);
        m_plot->replot(QCustomPlot::rpQueuedReplot);
    }
}

void ChartBase::updateCursorHint(const QPoint& pos)
{
    if (!m_plot) return;
    double x = m_plot->xAxis->pixelToCoord(pos.x());
    double y = m_plot->yAxis->pixelToCoord(pos.y());
    m_plot->setToolTip(QString("x: %1\ny: %2").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
}

void ChartBase::requestReplotQueued()
{
    if (m_plot)
        m_plot->replot(QCustomPlot::rpQueuedReplot);
}