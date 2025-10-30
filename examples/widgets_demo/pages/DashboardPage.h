#ifndef WIDGETS_DEMO_DASHBOARDPAGE_H
#define WIDGETS_DEMO_DASHBOARDPAGE_H

#include <QtAdvancedUI/Core/StyledWidget.h>

class DashboardPage : public QtAdvancedUI::StyledWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);
};

#endif // WIDGETS_DEMO_DASHBOARDPAGE_H