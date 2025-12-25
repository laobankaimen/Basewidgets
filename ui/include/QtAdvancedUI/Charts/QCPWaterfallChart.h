#pragma once

#include <QtAdvancedUI/Charts/QCustomplot.h>
#include <QtWidgets/QMenu>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT QCPWaterfallChart : public QCPColorMap {
    Q_OBJECT
public:
    explicit QCPWaterfallChart(QCPAxis* keyAxis, QCPAxis* valueAxis);

    void setContextMenu(QMenu* menu);
    void installContextMenu(QCustomPlot* plot);

    // 根据当前主题自动设置图表背景，并监听主题变化
    void installThemeSync(QCustomPlot* plot);

private slots:
    void onContextMenuRequested(const QPoint& pos);
    void onStylesheetChanged();

private:
    void applyThemeBackground();
    QPointer<QCustomPlot> m_plot;
    QPointer<QMenu> m_contextMenu;
};

} // namespace QtAdvancedUI
