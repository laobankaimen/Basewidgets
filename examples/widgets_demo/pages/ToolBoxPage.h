#ifndef WIDGETS_DEMO_TOOLBOXPAGE_H
#define WIDGETS_DEMO_TOOLBOXPAGE_H

#include <QtAdvancedUI/Core/StyledWidget.h>

class ToolBoxPage : public QtAdvancedUI::StyledWidget {
    Q_OBJECT
public:
    explicit ToolBoxPage(QWidget* parent = nullptr);
};

#endif // WIDGETS_DEMO_TOOLBOXPAGE_H