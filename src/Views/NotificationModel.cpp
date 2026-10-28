#include <basewidgets/Views/NotificationModel.h>
#include <QPointer>
namespace basewidgets {
NotificationModel::NotificationModel(QObject* parent) : QAbstractListModel(parent) {}
int NotificationModel::rowCount(const QModelIndex& parent) const { return parent.isValid()?0:m_messages.size(); }
QVariant NotificationModel::data(const QModelIndex& index,int role) const
{
    if(!index.isValid()||index.model()!=this||index.column()!=0||index.row()<0||index.row()>=m_messages.size())return {};
    const auto& item=m_messages[index.row()];
    switch(role) {
    case Qt::DisplayRole: case TitleRole:return item.title;
    case Qt::AccessibleTextRole:return item.title+(item.read?tr("，已读"):tr("，未读"))+"\n"+item.body;
    case Qt::ToolTipRole:return item.title;
    case IdRole:return item.id; case BodyRole:return item.body; case TimestampRole:return item.timestamp;
    case ToneRole:return int(item.tone); case ReadRole:return item.read; case ActionTextRole:return item.actionText;
    default:return {};
    }
}
QHash<int,QByteArray> NotificationModel::roleNames() const
{
    return {{IdRole,"id"},{TitleRole,"title"},{BodyRole,"body"},{TimestampRole,"timestamp"},{ToneRole,"tone"},{ReadRole,"read"},{ActionTextRole,"actionText"}};
}
int NotificationModel::rowForId(const QString& id) const { for(int i=0;i<m_messages.size();++i)if(m_messages[i].id==id)return i; return -1; }
NotificationMessage NotificationModel::message(const QString& id) const { return messageAt(rowForId(id)); }
NotificationMessage NotificationModel::messageAt(int row) const { return row>=0&&row<m_messages.size()?m_messages[row]:NotificationMessage(); }
int NotificationModel::unreadCount() const { int count=0; for(const auto& item:m_messages)if(!item.read)++count; return count; }
void NotificationModel::publishCounts(int previousCount,int previousUnread)
{
    const int count=rowCount(),unread=unreadCount(); QPointer<NotificationModel> self(this);
    if(count!=previousCount)emit countChanged(count);
    if(self&&unread!=previousUnread)emit unreadCountChanged(unread);
}
bool NotificationModel::addMessage(const NotificationMessage& item)
{
    if(item.id.trimmed().isEmpty()||item.title.trimmed().isEmpty())return false;
    const int oldCount=rowCount(),oldUnread=unreadCount(),row=rowForId(item.id);
    if(row>=0) { m_messages[row]=item; emit dataChanged(index(row),index(row)); }
    else {
        if(m_messages.size()==m_maximum) { beginRemoveRows({},m_maximum-1,m_maximum-1); m_messages.removeLast(); endRemoveRows(); }
        beginInsertRows({},0,0); m_messages.prepend(item); endInsertRows();
    }
    publishCounts(oldCount,oldUnread); return true;
}
bool NotificationModel::removeMessage(const QString& id)
{
    const int row=rowForId(id); if(row<0)return false;
    const int count=rowCount(),unread=unreadCount(); beginRemoveRows({},row,row); m_messages.removeAt(row); endRemoveRows(); publishCounts(count,unread); return true;
}
bool NotificationModel::markRead(const QString& id,bool read)
{
    const int row=rowForId(id); if(row<0)return false;
    if(m_messages[row].read==read)return true;
    const int unread=unreadCount(); m_messages[row].read=read;
    emit dataChanged(index(row),index(row),{ReadRole,Qt::AccessibleTextRole}); publishCounts(rowCount(),unread); return true;
}
bool NotificationModel::setMaximumCount(int maximum)
{
    if(maximum<1||maximum>10000)return false;
    m_maximum=maximum; if(rowCount()<=maximum)return true;
    const int count=rowCount(),unread=unreadCount(); beginRemoveRows({},maximum,count-1); m_messages.resize(maximum); endRemoveRows(); publishCounts(count,unread); return true;
}
void NotificationModel::markAllRead()
{
    const int unread=unreadCount(); if(!unread)return;
    for(auto& item:m_messages)item.read=true;
    emit dataChanged(index(0),index(rowCount()-1),{ReadRole,Qt::AccessibleTextRole}); publishCounts(rowCount(),unread);
}
void NotificationModel::clear()
{
    const int count=rowCount(),unread=unreadCount(); if(!count)return;
    beginRemoveRows({},0,count-1); m_messages.clear(); endRemoveRows(); publishCounts(count,unread);
}
}
