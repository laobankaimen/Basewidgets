#pragma once
#include <basewidgets/Core/StyledWidget.h>
/** 集中展示导航/列表/表格/树形视图、筛选条件、消息反馈和空状态；按宽度换行。 */
class CommonWidgetsPage : public basewidgets::StyledWidget {
public:
    explicit CommonWidgetsPage(QWidget* parent=nullptr);
};
