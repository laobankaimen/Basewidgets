/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_MENUBAR_H
#define BASEWIDGETS_MENUBAR_H

#include <QMenuBar>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 菜单栏包装；普通窗口可直接使用，FramelessWidgetWindow 会放在自定义标题栏下方。 */
class ACSS_UI_EXPORT MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    explicit MenuBar(QWidget* parent = nullptr);
    explicit MenuBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~MenuBar() override;

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

#endif // BASEWIDGETS_MENUBAR_H

