/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TABLEVIEW_H
#define BASEWIDGETS_TABLEVIEW_H

#include <QTableView>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 表格视图包装；表头、模型和选择接口沿用 QTableView。 */
class ACSS_UI_EXPORT TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(QWidget* parent = nullptr);
    ~TableView() override;

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

#endif // BASEWIDGETS_TABLEVIEW_H

