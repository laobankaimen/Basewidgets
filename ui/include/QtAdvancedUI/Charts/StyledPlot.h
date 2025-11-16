#pragma once

#include <QtAdvancedUI/Charts/QCustomplot.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QApplication>

namespace QtAdvancedUI {

// 通用图表基类：封装交互与主题联动背景/坐标轴颜色
class StyledPlot : public QCustomPlot {
    Q_OBJECT
public:
    explicit StyledPlot(QWidget* parent = nullptr);

private slots:
    void onStylesheetChanged();

private:
    void configureDefaultInteractions();
    void applyTheme();
};

} // namespace QtAdvancedUI