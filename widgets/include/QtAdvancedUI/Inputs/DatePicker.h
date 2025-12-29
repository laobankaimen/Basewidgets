/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_DATEPICKER_H
#define QTADVANCEDUI_DATEPICKER_H

#include <QDateEdit>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT DatePicker : public QDateEdit
{
    Q_OBJECT
public:
    explicit DatePicker(QWidget* parent = nullptr);
    explicit DatePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~DatePicker() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    void setWeekNumbersEnabled(bool enabled);
    bool weekNumbersEnabled() const { return m_weekNumbers; }

protected:
    void applyStylesheet();

private:
    void attachToManager();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    bool m_weekNumbers = true;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_DATEPICKER_H

