/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TIMEPICKER_H
#define BASEWIDGETS_TIMEPICKER_H

#include <QTimeEdit>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

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

} // namespace basewidgets

#endif // BASEWIDGETS_TIMEPICKER_H

