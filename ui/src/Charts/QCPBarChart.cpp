/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QtAdvancedUI/Charts/QCPBarChart.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

namespace QtAdvancedUI {

QCPBarChart::QCPBarChart(QCPAxis* keyAxis, QCPAxis* valueAxis)
    : QCPBars(keyAxis, valueAxis) {}

void QCPBarChart::setOrientation(Qt::Orientation orientation, QCustomPlot* plot)
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

QCPAxis* QCPBarChart::addValueAxisRight(QCustomPlot* plot)
{
    if (!plot) return nullptr;
    m_plot = plot;
    QCPAxisRect* rect = plot->axisRect();
    if (!rect) return nullptr;
    QCPAxis* axis = rect->addAxis(QCPAxis::atRight);
    m_extraValueAxes.push_back(axis);
    return axis;
}

QCPAxis* QCPBarChart::addValueAxisLeft(QCustomPlot* plot)
{
    if (!plot) return nullptr;
    m_plot = plot;
    QCPAxisRect* rect = plot->axisRect();
    if (!rect) return nullptr;
    QCPAxis* axis = rect->addAxis(QCPAxis::atLeft);
    m_extraValueAxes.push_back(axis);
    return axis;
}

void QCPBarChart::setValueAxisByPtr(QCPAxis* axis)
{
    if (!axis) return;
    setValueAxis(axis);
}

void QCPBarChart::setContextMenu(QMenu* menu)
{
    m_contextMenu = menu;
}

void QCPBarChart::installContextMenu(QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(plot, &QWidget::customContextMenuRequested,
                     this, &QCPBarChart::onContextMenuRequested);
}

void QCPBarChart::onContextMenuRequested(const QPoint& pos)
{
    if (!m_plot || !m_contextMenu) return;
    QCPAbstractPlottable* hit = m_plot->plottableAt(pos, true);
    if (hit == this) {
        m_contextMenu->exec(m_plot->mapToGlobal(pos));
    }
}

void QCPBarChart::installThemeSync(QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    applyThemeBackground();
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged,
                     this, &QCPBarChart::onStylesheetChanged);
}

void QCPBarChart::onStylesheetChanged()
{
    applyThemeBackground();
}

void QCPBarChart::applyThemeBackground()
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

} // namespace QtAdvancedUI

