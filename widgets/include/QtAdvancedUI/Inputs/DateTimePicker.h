/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef QTADVANCEDUI_DATETIMEPICKER_H
#define QTADVANCEDUI_DATETIMEPICKER_H

#include <QDateTimeEdit>
#include <QtAdvancedUI/Styles/QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT DateTimePicker : public QDateTimeEdit
{
    Q_OBJECT
public:
    explicit DateTimePicker(QWidget* parent = nullptr);
    explicit DateTimePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~DateTimePicker() override;

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

#endif // QTADVANCEDUI_DATETIMEPICKER_H

