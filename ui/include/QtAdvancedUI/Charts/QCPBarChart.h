#pragma once

#include <QtAdvancedUI/Charts/QCustomplot.h>
#include <QtWidgets/QMenu>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT QCPBarChart : public QCPBars {
    Q_OBJECT
public:
    explicit QCPBarChart(QCPAxis* keyAxis, QCPAxis* valueAxis);

    void setOrientation(Qt::Orientation orientation, QCustomPlot* plot);
    QCPAxis* addValueAxisRight(QCustomPlot* plot);
    QCPAxis* addValueAxisLeft(QCustomPlot* plot);
    void setValueAxisByPtr(QCPAxis* axis);

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
    Qt::Orientation m_orientation { Qt::Vertical };
    QVector<QPointer<QCPAxis>> m_extraValueAxes;
};

} // namespace QtAdvancedUI
