/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TOOLBOX_H
#define BASEWIDGETS_TOOLBOX_H

#include <QToolBox>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 折叠页面容器；页面管理及 currentChanged 信号沿用 QToolBox。 */
class ACSS_UI_EXPORT ToolBox : public QToolBox
{
    Q_OBJECT
public:
    explicit ToolBox(QWidget* parent = nullptr);
    explicit ToolBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ToolBox() override;

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

#endif // BASEWIDGETS_TOOLBOX_H

