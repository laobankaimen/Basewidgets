/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_LISTVIEW_H
#define BASEWIDGETS_LISTVIEW_H

#include <QListView>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 列表视图包装；外部模型不转移所有权，建议以视图或页面作为模型父对象。 */
class ACSS_UI_EXPORT ListView : public QListView
{
    Q_OBJECT
public:
    explicit ListView(QWidget* parent = nullptr);
    ~ListView() override;

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

#endif // BASEWIDGETS_LISTVIEW_H

