/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Charts/QCPWaterfallChart.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

namespace basewidgets {

QCPWaterfallChart::QCPWaterfallChart(QCPAxis* keyAxis, QCPAxis* valueAxis)
    : QCPColorMap(keyAxis, valueAxis) {}

void QCPWaterfallChart::setContextMenu(QMenu* menu)
{
    m_contextMenu = menu;
}

void QCPWaterfallChart::installContextMenu(QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(plot, &QWidget::customContextMenuRequested,
                     this, &QCPWaterfallChart::onContextMenuRequested);
}

void QCPWaterfallChart::onContextMenuRequested(const QPoint& pos)
{
    if (!m_plot || !m_contextMenu) return;
    QCPAbstractPlottable* hit = m_plot->plottableAt(pos, true);
    if (hit == this) {
        m_contextMenu->exec(m_plot->mapToGlobal(pos));
    }
}

void QCPWaterfallChart::installThemeSync(QCustomPlot* plot)
{
    if (!plot) return;
    m_plot = plot;
    applyThemeBackground();
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged,
                     this, &QCPWaterfallChart::onStylesheetChanged);
}

void QCPWaterfallChart::onStylesheetChanged()
{
    applyThemeBackground();
}

void QCPWaterfallChart::applyThemeBackground()
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

