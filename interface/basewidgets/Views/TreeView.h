/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TREEVIEW_H
#define BASEWIDGETS_TREEVIEW_H

#include <QTreeView>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 树视图包装；外部模型不转移所有权，展开/排序行为沿用 QTreeView。 */
class ACSS_UI_EXPORT TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget* parent = nullptr);
    ~TreeView() override;

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

#endif // BASEWIDGETS_TREEVIEW_H

