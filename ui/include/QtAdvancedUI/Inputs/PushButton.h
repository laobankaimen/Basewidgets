#ifndef QTADVANCEDUI_PUSHBUTTON_H
#define QTADVANCEDUI_PUSHBUTTON_H

#include <QPushButton>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT PushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButton(QWidget* parent = nullptr);
    explicit PushButton(const QString& text, QWidget* parent = nullptr);
    explicit PushButton(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~PushButton() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

protected:
    void applyStylesheet();

private:
    void attachToManager();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_PUSHBUTTON_H