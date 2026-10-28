#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/Views/NotificationModel.h>
#include <QPointer>
class QLabel;
class QListView;
class QSortFilterProxyModel;
class QStackedWidget;
class QPlainTextEdit;
namespace basewidgets {
class CheckBox;
class PushButton;
/** 可嵌入的消息中心：未读筛选、全部已读、清空、详情和业务动作。
 * 列表用 Model/View 绘制，不为每条消息创建 QWidget；正文按纯文本处理。
 * 点击/Enter 查看详情并标记该条已读；打开面板本身不改变已读状态。
 * 内置空模型归面板持有，外部模型仅借用、销毁后切回内置模型。GUI 线程使用。
 */
class ACSS_UI_EXPORT NotificationPanel : public StyledWidget {
    Q_OBJECT
public:
    explicit NotificationPanel(QWidget* parent=nullptr);
    void setModel(NotificationModel* model); ///< nullptr 恢复面板自己的模型。
    NotificationModel* model() const;
    void setTitle(const QString& title);
    void setUnreadOnly(bool enabled);
    bool unreadOnly() const;
    QSize sizeHint() const override { return QSize(380,480); }
public slots:
    void openMessage(const QString& id); ///< 不存在时无操作；详情显示最新模型快照。
    void showList();
signals:
    void messageActivated(const QString& id);
    void actionRequested(const QString& id); ///< 仅通知业务，不隐式跳转/执行/删除消息。
private:
    void refresh();
    NotificationModel* m_owned=nullptr;
    QPointer<NotificationModel> m_model;
    QSortFilterProxyModel* m_proxy=nullptr;
    QLabel *m_title=nullptr,*m_count=nullptr,*m_empty=nullptr,*m_detailTitle=nullptr,*m_time=nullptr;
    QListView* m_list=nullptr;
    QStackedWidget* m_stack=nullptr;
    QWidget *m_listPage=nullptr,*m_detailPage=nullptr;
    QPlainTextEdit* m_body=nullptr;
    CheckBox* m_unread=nullptr;
    PushButton *m_markAll=nullptr,*m_clear=nullptr,*m_action=nullptr;
    QString m_current;
};
}
