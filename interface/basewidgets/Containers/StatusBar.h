/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_STATUSBAR_H
#define BASEWIDGETS_STATUSBAR_H

#include <QStatusBar>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 状态栏包装；showMessage 表示临时消息，常驻状态请使用 addPermanentWidget。 */
class ACSS_UI_EXPORT StatusBar : public QStatusBar
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);
    explicit StatusBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~StatusBar() override;

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

#endif // BASEWIDGETS_STATUSBAR_H

