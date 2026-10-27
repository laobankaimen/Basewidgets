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

    /** 添加选项；id=-1 自动分配可用非负 ID，重复显式 ID 或空标题被忽略。 */
    void addButton(const QString &title,const int &id = -1,const QIcon &icon = QIcon());
    /** 在指定行插入；越界行号追加到末尾，ID 规则与 addButton 相同。 */
    void insertButton(const QString &title, int row = 0, const int &id = -1, const QIcon &icon = QIcon());
    //设置当前活动选项
    void setCurrentButton(const int &id);
    /** 返回当前业务 ID；未选中或无映射时返回 -1。 */
    int getCurrentButton() const;
    /** 删除指定 ID 的 item；不存在时忽略，item 由列表负责销毁。 */
    void removeButton(const int &id);

protected:
    void applyStylesheet();

private:
    void attachToManager();
    int findAvailableId();

signals:
    /** 点击信号：业务 ID、标题、当前列表行号（从 0 开始）。 */
    void buttonClicked(int id,QString title,int num);

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    QMap<int,QListWidgetItem*> m_items;
};

} // namespace basewidgets

#endif // BASEWIDGETS_NAVIVIEW_H

