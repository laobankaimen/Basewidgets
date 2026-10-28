#pragma once
#include <basewidgets/Core/StyledWidget.h>
/** 集中展示搜索/复制/状态/折叠参数、基础视图和筛选反馈；按宽度换行。 */
class CommonWidgetsPage : public basewidgets::StyledWidget {
public:
    explicit CommonWidgetsPage(QWidget* parent=nullptr);
};
