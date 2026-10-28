#pragma once
#include <basewidgets/Inputs/ToolButton.h>
#include <QPointer>
namespace basewidgets {
class NotificationModel;
/** 矢量消息铃铛，默认显示未读数（超过 99 显示 99+），无闪烁或轮询。
 * 可独立设置数量，也可绑定 NotificationModel；clicked 沿用 QToolButton。
 * 不自动创建弹窗或标记已读，应用可连接 NotificationPopup::popup。GUI 线程使用。
 */
class ACSS_UI_EXPORT NotificationBell : public ToolButton {
    Q_OBJECT
    Q_PROPERTY(int unreadCount READ unreadCount WRITE setUnreadCount NOTIFY unreadCountChanged)
public:
    explicit NotificationBell(QWidget* parent=nullptr);
    void setModel(NotificationModel* model); ///< 借用；销毁后自动清零，nullptr 解绑并清零。
    NotificationModel* model() const;
    int unreadCount() const { return m_unread; }
    void setDotOnly(bool enabled); ///< 默认 false；true 只显示红点，完整数量仍在 tooltip 中。
    bool dotOnly() const { return m_dotOnly; }
    QSize sizeHint() const override;
public slots:
    void setUnreadCount(int count); ///< 负数按 0；模型绑定时下一模型更新会覆盖手动值。
signals:
    void unreadCountChanged(int count);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QPointer<NotificationModel> m_model;
    int m_unread=0;
    bool m_dotOnly=false;
};
}
