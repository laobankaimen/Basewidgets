/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_DATEPICKER_H
#define BASEWIDGETS_DATEPICKER_H

#include <QDateEdit>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 日期编辑器；默认 yyyy-MM-dd 并启用日历弹窗；不代表自动选择今天。 */
class ACSS_UI_EXPORT DatePicker : public QDateEdit
{
    Q_OBJECT
public:
    explicit DatePicker(QWidget* parent = nullptr);
    explicit DatePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~DatePicker() override;

    /** 兼容管理器入口；不转移所有权。全应用主题请通过 StyleManagerAdapter 设置。 */
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

} // namespace basewidgets

#endif // BASEWIDGETS_DATEPICKER_H

