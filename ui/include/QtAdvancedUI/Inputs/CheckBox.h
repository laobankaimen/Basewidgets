#ifndef QTADVANCEDUI_CHECKBOX_H
#define QTADVANCEDUI_CHECKBOX_H

#include <QCheckBox>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT CheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit CheckBox(QWidget* parent = nullptr);
    explicit CheckBox(const QString& text, QWidget* parent = nullptr);
    explicit CheckBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~CheckBox() override;

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

#endif // QTADVANCEDUI_CHECKBOX_H
