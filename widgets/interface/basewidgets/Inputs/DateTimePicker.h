/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_DATETIMEPICKER_H
#define BASEWIDGETS_DATETIMEPICKER_H

#include <QDateTimeEdit>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

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

} // namespace basewidgets

#endif // BASEWIDGETS_DATETIMEPICKER_H

