#pragma once
#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>
#include <basewidgets/Core/FeedbackTone.h>
#include <basewidgets/QtAdvancedUIExport.h>
namespace basewidgets {
/** 应用提供的消息快照；id 为稳定业务标识，文字均按纯文本展示。
 * timestamp 为空表示未提供时间；控件不会伪造服务器时间或执行 actionText 对应业务。
 */
struct ACSS_UI_EXPORT NotificationMessage {
    QString id,title,body;
    QDateTime timestamp;
    FeedbackTone tone=FeedbackTone::Info;
    bool read=false;
    QString actionText;
};
/** GUI 线程消息列表，可供多个铃铛/面板共享；不访问网络、不保存到磁盘。
 * 默认最多 200 条（可设 1..10000）；新 ID 插入最前，满额淘汰末尾旧消息。
 * 相同 ID 原位替换快照，包括 read 状态；不会改变排列位置。
 */
class ACSS_UI_EXPORT NotificationModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role { IdRole=Qt::UserRole+1,TitleRole,BodyRole,TimestampRole,ToneRole,ReadRole,ActionTextRole };
    Q_ENUM(Role)
    explicit NotificationModel(QObject* parent=nullptr);
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant data(const QModelIndex& index,int role=Qt::DisplayRole) const override;
    QHash<int,QByteArray> roleNames() const override;
    /** 拒绝空 id/标题；失败不改列表。不要在 model 信号回调内重入修改本模型。 */
    bool addMessage(const NotificationMessage& message);
    bool removeMessage(const QString& id);
    bool markRead(const QString& id,bool read=true); ///< 不存在返回 false；相同状态不重复发信号。
    NotificationMessage message(const QString& id) const; ///< 未找到返回 id 为空的快照。
    NotificationMessage messageAt(int row) const;
    int unreadCount() const;
    bool setMaximumCount(int maximum); ///< 缩小容量立即淘汰末尾条目，非法值保持旧设置。
    int maximumCount() const { return m_maximum; }
public slots:
    void markAllRead();
    void clear();
signals:
    void unreadCountChanged(int count);
    void countChanged(int count);
private:
    int rowForId(const QString& id) const;
    void publishCounts(int previousCount,int previousUnread);
    QVector<NotificationMessage> m_messages;
    int m_maximum=200;
};
}
Q_DECLARE_METATYPE(basewidgets::NotificationMessage)
