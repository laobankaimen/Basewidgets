/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_GROUPBOX_H
#define BASEWIDGETS_GROUPBOX_H

#include <QGroupBox>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 带标题分组；可选中分组的启用/禁用子控件行为继承 QGroupBox。 */
class ACSS_UI_EXPORT GroupBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit GroupBox(QWidget* parent = nullptr);
    explicit GroupBox(const QString& title, QWidget* parent = nullptr);
    explicit GroupBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~GroupBox() override;

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

#endif // BASEWIDGETS_GROUPBOX_H

