#include "QCustomplot.h"
#include <QtAdvancedUI/Charts/HistogramChart.h>

using namespace QtAdvancedUI;

HistogramChart::HistogramChart(QWidget* parent)
    : ChartBase(parent)
{
    m_bars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
    m_bars->setName("Histogram");
    m_bars->setAntialiased(false);
    m_bars->setPen(QPen(QColor(90, 180, 90)));
    m_bars->setBrush(QBrush(QColor(90, 180, 90, 140)));

    configureBins(20, 0.0, 1.0);
    m_plot->legend->setVisible(false);
}

void HistogramChart::configureBins(int binCount, double minValue, double maxValue)
{
    m_binCount = qMax(1, binCount);
    m_minValue = minValue;
    m_maxValue = maxValue > minValue ? maxValue : (minValue + 1.0);
    m_bins.fill(0, m_binCount);

    // x 轴使用分箱中心，y 轴为频数
    QVector<double> keys, vals;
    keys.reserve(m_binCount);
    vals.fill(0, m_binCount);
    double width = (m_maxValue - m_minValue) / m_binCount;
    for (int i = 0; i < m_binCount; ++i) {
        keys.push_back(m_minValue + (i + 0.5) * width);
    }
    m_bars->setWidth(width * 0.9);
    m_bars->setData(keys, vals);
    m_plot->xAxis->setRange(m_minValue, m_maxValue);
    m_plot->yAxis->setRange(0, 1);
    captureInitialRangesIfNeeded();
    requestReplotQueued();
}

void HistogramChart::setBinCount(int binCount)
{
    configureBins(binCount, m_minValue, m_maxValue);
}

void HistogramChart::addSamples(const QVector<double>& values)
{
    if (values.isEmpty()) return;
    double width = (m_maxValue - m_minValue) / m_binCount;
    for (double v : values) {
        if (v < m_minValue || v > m_maxValue) continue;
        int idx = int((v - m_minValue) / width);
        if (idx >= m_binCount) idx = m_binCount - 1;
        ++m_bins[idx];
    }
    updateBars();
}

void HistogramChart::clear()
{
    m_bins.fill(0, m_binCount);
    updateBars();
}

void HistogramChart::updateBars()
{
    QVector<double> keys;
    QVector<double> vals;
    keys.reserve(m_binCount);
    vals.reserve(m_binCount);
    double width = (m_maxValue - m_minValue) / m_binCount;
    int maxCount = 1;
    for (int i = 0; i < m_binCount; ++i) {
        keys.push_back(m_minValue + (i + 0.5) * width);
        vals.push_back(m_bins.value(i));
        maxCount = qMax(maxCount, m_bins.value(i));
    }
    m_bars->setData(keys, vals);
    m_plot->yAxis->setRange(0, maxCount * 1.1);
    requestReplotQueued();
}

void HistogramChart::updateCursorHint(const QPoint& pos)
{
    double x = m_plot->xAxis->pixelToCoord(pos.x());
    double width = (m_maxValue - m_minValue) / m_binCount;
    int idx = int((x - m_minValue) / width);
    if (idx >= 0 && idx < m_binCount) {
        m_plot->setToolTip(QString("bin[%1]: count=%2").arg(idx).arg(m_bins.value(idx)));
    }
}