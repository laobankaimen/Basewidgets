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

/** 日期时间编辑器；默认 yyyy-MM-dd HH:mm:ss；业务默认值由调用方设置。 */
class ACSS_UI_EXPORT DateTimePicker : public QDateTimeEdit
{
    Q_OBJECT
public:
    explicit DateTimePicker(QWidget* parent = nullptr);
    explicit DateTimePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~DateTimePicker() override;

    /** 兼容管理器入口；不转移所有权。全应用主题请通过 StyleManagerAdapter 设置。 */
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

