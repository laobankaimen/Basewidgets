/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TOOLBAR_H
#define BASEWIDGETS_TOOLBAR_H

#include <QToolBar>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 工具栏包装；使用 addAction/addWidget，主窗口通过原生 toolbar area 管理。 */
class ACSS_UI_EXPORT ToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit ToolBar(QWidget* parent = nullptr);
    explicit ToolBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ToolBar() override;

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

#endif // BASEWIDGETS_TOOLBAR_H

