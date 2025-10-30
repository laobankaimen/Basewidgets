#include "QCustomplot.h"
#include <QtAdvancedUI/Charts/PieChart.h>
#include <QSharedPointer>
#include <QtMath>

using namespace QtAdvancedUI;

static double norm360(double deg) {
    while (deg < 0) deg += 360.0;
    while (deg >= 360.0) deg -= 360.0;
    return deg;
}

PieChart::PieChart(QWidget* parent)
    : ChartBase(parent)
{
    // 隐藏笛卡尔坐标轴，仅使用极坐标
    if (m_plot->xAxis) m_plot->xAxis->setVisible(false);
    if (m_plot->yAxis) m_plot->yAxis->setVisible(false);

    // 创建极坐标轴，并居中嵌入默认轴矩形
    m_ang = new QCPPolarAxisAngular(m_plot);
    m_rad = new QCPPolarAxisRadial(m_ang);
    if (m_plot->axisRect() && m_plot->axisRect()->insetLayout()) {
        m_plot->axisRect()->insetLayout()->addElement(m_ang, Qt::AlignCenter);
    }

    // 基本样式：关闭刻度与标签，仅保留背景
    m_plot->legend->setVisible(false);
    m_ang->setTicks(false);
    m_ang->setTickLabels(false);
    m_ang->setRange(0, 360);
    m_rad->setTicks(false);
    m_rad->setTickLabels(false);
    m_rad->setRange(0, 1.0);
}

void PieChart::setData(const QVector<double>& values,
                       const QStringList& labels,
                       const QVector<QColor>& colors)
{
    m_values = values;
    m_labels = labels;
    m_colors = colors;
    rebuild();
}

void PieChart::clear()
{
    for (auto& s : m_slices) {
        if (s.graph) {
            // QCustomPlot 没有直接 removePolarGraph，这里删除并释放对象
            delete s.graph;
            s.graph = nullptr;
        }
    }
    m_slices.clear();
    m_values.clear();
    m_labels.clear();
    m_colors.clear();
    m_sum = 0.0;
    requestReplotQueued();
}

void PieChart::rebuild()
{
    // 清除旧切片
    for (auto& s : m_slices) {
        if (s.graph) delete s.graph;
    }
    m_slices.clear();

    // 统计和，过滤非正数
    m_sum = 0.0;
    for (double v : m_values) {
        if (v > 0) m_sum += v;
    }
    if (m_sum <= 0) {
        requestReplotQueued();
        return;
    }

    // 构建切片
    double cur = 0.0;
    for (int i = 0; i < m_values.size(); ++i) {
        double v = m_values[i];
        if (v <= 0) continue;
        Slice s;
        s.value = v;
        s.startDeg = cur / m_sum * 360.0;
        s.endDeg = (cur + v) / m_sum * 360.0;
        s.label = (i < m_labels.size()) ? m_labels[i] : QString("%1").arg(i+1);
        s.color = (i < m_colors.size() && m_colors[i].isValid()) ? m_colors[i] : defaultColor(i);
        s.graph = new QCPPolarGraph(m_ang, m_rad);
        s.graph->setLineStyle(QCPPolarGraph::lsLine);
        s.graph->setScatterStyle(QCPScatterStyle::ssNone);
        s.graph->setPen(Qt::NoPen);
        s.graph->setBrush(QBrush(s.color));
        m_slices.push_back(s);
        buildSliceGeometry(m_slices.last());
        cur += v;
    }

    // 固定半径范围
    m_rad->setRange(m_hole, 1.0);
    requestReplotQueued();
}

void PieChart::buildSliceGeometry(Slice& s)
{
    if (!s.graph || !m_rad || !m_ang) return;
    const double r1 = qBound(0.0, m_hole, 0.95);
    const double r2 = 1.0;
    const double start = norm360(s.startDeg);
    const double end   = norm360(s.endDeg);
    // 处理跨越 360->0 的情况
    double span = end - start;
    if (span <= 0) span += 360.0;
    const int steps = qBound(12, int(span / 4.0), 128);

    QVector<double> keys; keys.reserve(steps + 4);
    QVector<double> vals; vals.reserve(steps + 4);

    // 先画内半径起点 -> 外半径起点
    keys.push_back(start); vals.push_back(r1);
    keys.push_back(start); vals.push_back(r2);
    // 沿外圆弧采样
    for (int i = 1; i < steps; ++i) {
        double t = i / double(steps);
        keys.push_back(start + t * span);
        vals.push_back(r2);
    }
    // 终点外半径
    keys.push_back(start + span); vals.push_back(r2);
    // 终点回到内半径
    keys.push_back(start + span); vals.push_back(r1);
    // 沿内圆弧回到起点（反向）
    for (int i = steps-1; i >= 1; --i) {
        double t = i / double(steps);
        keys.push_back(start + t * span);
        vals.push_back(r1);
    }

    s.graph->setData(keys, vals, /*alreadySorted*/true);
}

QColor PieChart::defaultColor(int index) const
{
    static const QVector<QColor> palette = {
        QColor(80, 170, 240), QColor(255, 99, 132), QColor(255, 159, 64),
        QColor(75, 192, 192), QColor(153, 102, 255), QColor(255, 205, 86),
        QColor(100, 181, 246), QColor(129, 199, 132), QColor(244, 143, 177)
    };
    return palette.at(index % palette.size());
}

void PieChart::updateCursorHint(const QPoint& pos)
{
    if (!m_ang || !m_rad) return;
    double ang = 0.0, rad = 0.0;
    m_ang->pixelToCoord(pos, ang, rad);
    ang = norm360(ang);
    if (rad < m_hole || rad > 1.05) { if (plot()) plot()->setToolTip(QString()); return; }

    for (const auto& s : m_slices) {
        double start = norm360(s.startDeg);
        double end   = norm360(s.endDeg);
        double span  = end - start; if (span <= 0) span += 360.0;
        // 判断 ang 是否落在 [start, start+span]
        double rel = ang - start; if (rel < 0) rel += 360.0;
        if (rel >= 0 && rel <= span) {
            double pct = (m_sum > 0) ? (s.value / m_sum * 100.0) : 0.0;
            if (plot()) plot()->setToolTip(QString("%1: %2 (%3%)")
                               .arg(s.label)
                               .arg(s.value, 0, 'f', 3)
                               .arg(pct, 0, 'f', 1));
            return;
        }
    }
    if (plot()) plot()->setToolTip(QString());
}