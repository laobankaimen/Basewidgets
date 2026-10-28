#pragma once
#include <basewidgets/Core/StyledWidget.h>
/** 数据管理首页组合示例。只展示合成快照，导航交给宿主；不包含采集/文件业务。 */
class DashboardPage : public basewidgets::StyledWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent=nullptr);
signals:
    void pageRequested(int page); ///< widgets_demo 的既有页面编号，由 main.cpp 接线。
};
