/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Charts/QCPLineChart.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

namespace basewidgets {

QCPLineChart::QCPLineChart(QCPAxis* keyAxis, QCPAxis* valueAxis)
    : QCPGraph(keyAxis, valueAxis) {}

void QCPLineChart::setOrientation(Qt::Orientation orientation, QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    m_orientation = orientation;
    if (orientation == Qt::Vertical) {
        setKeyAxis(plot->xAxis);
        setValueAxis(plot->yAxis);
    } else {
        setKeyAxis(plot->yAxis);
        setValueAxis(plot->xAxis);
    }
}

QCPAxis* QCPLineChart::addValueAxisRight(QCustomPlot* plot)
{
    if (!plot) return nullptr;
    m_plot = plot;
    QCPAxisRect* rect = plot->axisRect();
    if (!rect) return nullptr;
    QCPAxis* axis = rect->addAxis(QCPAxis::atRight);
    m_extraValueAxes.push_back(axis);
    return axis;
}

QCPAxis* QCPLineChart::addValueAxisLeft(QCustomPlot* plot)
{
    if (!plot) return nullptr;
    m_plot = plot;
    QCPAxisRect* rect = plot->axisRect();
    if (!rect) return nullptr;
    QCPAxis* axis = rect->addAxis(QCPAxis::atLeft);
    m_extraValueAxes.push_back(axis);
    return axis;
}

void QCPLineChart::setValueAxisByPtr(QCPAxis* axis)
{
    if (!axis) return;
    setValueAxis(axis);
}

void QCPLineChart::setContextMenu(QMenu* menu)
{
    m_contextMenu = menu;
}

void QCPLineChart::installContextMenu(QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(plot, &QWidget::customContextMenuRequested,
                     this, &QCPLineChart::onContextMenuRequested);
}

void QCPLineChart::onContextMenuRequested(const QPoint& pos)
{
    if (!m_plot || !m_contextMenu) return;
    QCPAbstractPlottable* hit = m_plot->plottableAt(pos, true);
    if (hit == this) {
        m_contextMenu->exec(m_plot->mapToGlobal(pos));
    }
}

void QCPLineChart::installThemeSync(QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    applyThemeBackground();
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::themeChanged,
                     this, &QCPLineChart::onStylesheetChanged);
}

void QCPLineChart::onStylesheetChanged()
{
    applyThemeBackground();
}

void QCPLineChart::applyThemeBackground()
{
    if (!m_plot) return;
    auto* mgr = StyleManagerAdapter::instance();
    const bool dark = mgr && mgr->isCurrentThemeDark();
    const QColor viewport = dark ? QColor(20,20,20) : QColor(255,255,255);
    const QColor canvas   = dark ? QColor(34,34,34) : QColor(255,255,255);
    m_plot->setBackground(QBrush(viewport));
    if (m_plot->axisRect()) {
        m_plot->axisRect()->setBackground(QBrush(canvas));
    }
    m_plot->replot();
}

} // namespace basewidgets

