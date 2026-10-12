/*
 * Author: zhoutao@zkhx.com
 *
 * 导航栏按钮列表，继承自QListWidget，可实现自定义按钮id
 *
 * Date: 2026-03-16
 */
#ifndef BASEWIDGETS_NAVIVIEW_H
#define BASEWIDGETS_NAVIVIEW_H

#include <QListWidget>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

class ACSS_UI_EXPORT NavigationList : public QListWidget
{
    Q_OBJECT
public:
    explicit NavigationList(QWidget* parent = nullptr);
    ~NavigationList() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    //新增一个选项
    void addButton(const QString &title,const int &id = -1,const QIcon &icon = QIcon());
    //插入一个选项
    void insertButton(const QString &title, int row = 0, const int &id = -1, const QIcon &icon = QIcon());
    //设置当前活动选项
    void setCurrentButton(const int &id);
    //获取当前活动选项
    int getCurrentButton() const;
    //删除一个选项
    void removeButton(const int &id);

protected:
    void applyStylesheet();

private:
    void attachToManager();
    int findAvailableId();

signals:
    void buttonClicked(int id,QString title,int num);

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    QMap<int,QListWidgetItem*> m_items;
};

} // namespace basewidgets

#endif // BASEWIDGETS_NAVIVIEW_H

