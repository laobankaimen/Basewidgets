/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#pragma once

#include <QtAdvancedUI/Charts/QCustomplot.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QApplication>

namespace QtAdvancedUI {

// 通用图表基类：封装交互与主题联动背景/坐标轴颜色
class ACSS_UI_EXPORT StyledPlot : public QCustomPlot {
    Q_OBJECT
public:
    explicit StyledPlot(QWidget* parent = nullptr);

    // 重新暴露基类方法以避免链接错误（因为 QCustomPlot 未导出）
    void replot(QCustomPlot::RefreshPriority priority = QCustomPlot::rpRefreshHint);

signals:
    // 转发基类信号
    void mouseDoubleClick(QMouseEvent* event);
    void mousePress(QMouseEvent* event);
    void mouseRelease(QMouseEvent* event);
    void mouseMove(QMouseEvent* event);
    void mouseWheel(QWheelEvent* event);

private slots:
    void onStylesheetChanged();

private:
    void configureDefaultInteractions();
    void applyTheme();
};

} // namespace QtAdvancedUI

