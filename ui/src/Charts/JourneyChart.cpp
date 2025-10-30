#include "QCustomplot.h"
#include <QtAdvancedUI/Charts/JourneyChart.h>

using namespace QtAdvancedUI;

JourneyChart::JourneyChart(QWidget* parent)
    : ChartBase(parent)
{
    m_graph = m_plot->addGraph();
    m_graph->setName("Journey");
    m_graph->setPen(QPen(QColor(240, 120, 70), 2));
    m_graph->setAdaptiveSampling(true);
    m_plot->legend->setVisible(false);
    m_plot->xAxis->setLabel("X");
    m_plot->yAxis->setLabel("Y");
}

void JourneyChart::setMaxPoints(int maxPoints)
{
    m_maxPoints = qMax(1, maxPoints);
}

void JourneyChart::setMaxPointCount(int count)
{
    setMaxPoints(count);
}

void JourneyChart::clear()
{
    if (m_graph) {
        m_graph->data()->clear();
        requestReplotQueued();
    }
}

void JourneyChart::appendPath(const QVector<QPointF>& pts)
{
    m_px.reserve(m_px.size() + pts.size());
    m_py.reserve(m_py.size() + pts.size());
    for (const auto& p : pts) {
        m_px.push_back(p.x());
        m_py.push_back(p.y());
    }
    flushPending();
}

void JourneyChart::appendPoints(const QVector<QPointF>& pts)
{
    appendPath(pts);
}

void JourneyChart::flushPending()
{
    if (!m_graph || m_px.isEmpty()) return;
    m_graph->addData(m_px, m_py, /*alreadySorted*/false);
    m_px.clear();
    m_py.clear();

    captureInitialRangesIfNeeded();
    if (m_graph->dataCount() > m_maxPoints) {
        double cutoffKey = m_plot->xAxis->range().lower;
        // QCustomPlot 2.1.1: 使用数据容器的 removeBefore 来裁剪
        m_graph->data()->removeBefore(cutoffKey);
    }
    m_graph->rescaleAxes(true);
    requestReplotQueued();
}

void JourneyChart::updateCursorHint(const QPoint& pos)
{
    double x = m_plot->xAxis->pixelToCoord(pos.x());
    double y = m_plot->yAxis->pixelToCoord(pos.y());
    m_plot->setToolTip(QString("x: %1\ny: %2").arg(x, 0, 'f', 4).arg(y, 0, 'f', 4));
}