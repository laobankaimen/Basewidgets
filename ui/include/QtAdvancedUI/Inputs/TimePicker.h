#ifndef QTADVANCEDUI_TIMEPICKER_H
#define QTADVANCEDUI_TIMEPICKER_H

#include <QTimeEdit>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT TimePicker : public QTimeEdit
{
    Q_OBJECT
public:
    explicit TimePicker(QWidget* parent = nullptr);
    explicit TimePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TimePicker() override;

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

#endif // QTADVANCEDUI_TIMEPICKER_H
