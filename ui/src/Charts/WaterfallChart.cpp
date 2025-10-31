#include <QtAdvancedUI/Charts/QCustomplot.h>
#include <QtAdvancedUI/Charts/WaterfallChart.h>

using namespace QtAdvancedUI;

WaterfallChart::WaterfallChart(QWidget* parent)
    : ChartBase(parent)
{
    m_baseBars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
    m_deltaBars = new QCPBars(m_plot->xAxis, m_plot->yAxis);
    m_baseBars->setName("Base");
    m_deltaBars->setName("Delta");
    // 基座透明，不显示，仅用于定位
    m_baseBars->setPen(Qt::NoPen);
    m_baseBars->setBrush(Qt::NoBrush);
    // 增量颜色
    m_deltaBars->setPen(QPen(QColor(80, 170, 240)));
    m_deltaBars->setBrush(QBrush(QColor(80, 170, 240, 180)));
    m_deltaBars->moveAbove(m_baseBars);
    m_plot->legend->setVisible(false);
}

void WaterfallChart::setData(const QVector<double>& deltas, const QStringList& labels)
{
    rebuild(deltas, labels);
}

void WaterfallChart::setCategories(const QStringList& labels)
{
    // 保留现有增量数据，更新标签
    rebuild(m_deltas, labels);
}

void WaterfallChart::setValues(const QVector<double>& deltas)
{
    // 保留现有标签，更新增量数据
    rebuild(deltas, m_labels);
}

void WaterfallChart::clear()
{
    m_keys.clear();
    m_bases.clear();
    m_deltas.clear();
    m_labels.clear();
    m_baseBars->data()->clear();
    m_deltaBars->data()->clear();
    requestReplotQueued();
}

void WaterfallChart::rebuild(const QVector<double>& deltas, const QStringList& labels)
{
    int n = deltas.size();
    m_keys.resize(n);
    m_bases.resize(n);
    m_deltas = deltas;
    m_labels = labels;

    double cum = 0.0;
    for (int i = 0; i < n; ++i) {
        m_keys[i] = i + 1;
        m_bases[i] = cum;
        cum += deltas[i];
    }
    m_baseBars->setData(m_keys, m_bases);
    m_deltaBars->setData(m_keys, m_deltas);
    m_baseBars->setWidth(0.6);
    m_deltaBars->setWidth(0.6);

    // y 轴范围：包含累计最大值与最小值
    double minY = 0.0, maxY = 0.0, cur = 0.0;
    for (double d : deltas) {
        double next = cur + d;
        minY = qMin(minY, qMin(cur, next));
        maxY = qMax(maxY, qMax(cur, next));
        cur = next;
    }
    m_plot->yAxis->setRange(minY * 1.1, maxY * 1.1);
    m_plot->xAxis->setRange(0, n + 1);

    // 自定义类目标签
    if (labels.size() == n) {
        QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
        for (int i = 0; i < n; ++i) {
            textTicker->addTick(m_keys[i], labels[i]);
        }
        m_plot->xAxis->setTicker(textTicker);
    }

    captureInitialRangesIfNeeded();
    requestReplotQueued();
}

void WaterfallChart::updateCursorHint(const QPoint& pos)
{
    double x = m_plot->xAxis->pixelToCoord(pos.x());
    int idx = qBound(0, int(std::round(x)) - 1, m_deltas.size()-1);
    if (idx >= 0) {
        double base = m_bases.value(idx);
        double delta = m_deltas.value(idx);
        double after = base + delta;
        QString label = (idx < m_labels.size()) ? m_labels[idx] : QString::number(idx+1);
        m_plot->setToolTip(QString("%1\nbase=%2 delta=%3 -> %4")
            .arg(label)
            .arg(base, 0, 'f', 3)
            .arg(delta, 0, 'f', 3)
            .arg(after, 0, 'f', 3));
    }
}