#ifndef WIDGETS_DEMO_FORMPAGE_H
#define WIDGETS_DEMO_FORMPAGE_H

#include <QtAdvancedUI/Core/StyledWidget.h>

class FormPage : public QtAdvancedUI::StyledWidget {
    Q_OBJECT
public:
    explicit FormPage(QWidget* parent = nullptr);
};

#endif // WIDGETS_DEMO_FORMPAGE_H