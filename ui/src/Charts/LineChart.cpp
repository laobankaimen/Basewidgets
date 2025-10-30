#include "QCustomplot.h"
#include <QtAdvancedUI/Charts/LineChart.h>

using namespace QtAdvancedUI;

LineChart::LineChart(QWidget* parent)
    : ChartBase(parent)
{
    m_graph = m_plot->addGraph();
    m_graph->setName("Line");
    m_graph->setPen(QPen(QColor(80, 170, 240), 2));
    m_graph->setAdaptiveSampling(true);
    m_plot->xAxis->setLabel("X");
    m_plot->yAxis->setLabel("Y");
    m_plot->legend->setVisible(false);
}

void LineChart::setMaxPoints(int maxPoints)
{
    m_maxPoints = qMax(1, maxPoints);
}

void LineChart::setMaxPointCount(int count)
{
    setMaxPoints(count);
}

void LineChart::clear()
{
    if (m_graph) {
        m_graph->data()->clear();
        requestReplotQueued();
    }
}

void LineChart::appendPoints(const QVector<QPointF>& pts)
{
    m_pendingX.reserve(m_pendingX.size() + pts.size());
    m_pendingY.reserve(m_pendingY.size() + pts.size());
    for (const auto& p : pts) {
        m_pendingX.push_back(p.x());
        m_pendingY.push_back(p.y());
    }
    flushPending();
}

void LineChart::appendPoint(const QPointF& pt)
{
    appendPoints(QVector<QPointF>{pt});
}

void LineChart::flushPending()
{
    if (!m_graph || m_pendingX.isEmpty()) return;
    m_graph->addData(m_pendingX, m_pendingY, /*alreadySorted*/false);
    m_pendingX.clear();
    m_pendingY.clear();

    captureInitialRangesIfNeeded();
    // 超过最大点数时移除较早数据（按 x 轴键裁剪）
    if (m_graph->dataCount() > m_maxPoints) {
        // 估算裁剪的键：以当前范围左侧为基准略向左偏移
        double cutoffKey = m_plot->xAxis->range().lower;
        // QCustomPlot 2.1.1: 使用数据容器的 removeBefore 来裁剪
        m_graph->data()->removeBefore(cutoffKey);
    }
    m_graph->rescaleAxes(true);
    requestReplotQueued();
}

void LineChart::updateCursorHint(const QPoint& pos)
{
    if (!m_plot || !m_graph) return;
    double x = m_plot->xAxis->pixelToCoord(pos.x());
    double y = m_plot->yAxis->pixelToCoord(pos.y());
    m_plot->setToolTip(QString("x: %1\ny: %2").arg(x, 0, 'f', 4).arg(y, 0, 'f', 4));
}