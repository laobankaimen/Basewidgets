#include <basewidgets/Containers/NotificationPanel.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Views/ListView.h>
#include <basewidgets/Inputs/PlainTextEdit.h>
#include "../Widgets/FeedbackPainting_p.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QStyleOptionFocusRect>
#include <QDateTime>
#include <QTextLayout>
#include <cmath>
namespace basewidgets {
namespace {
class UnreadProxy : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    bool unreadOnly=false;
    void setUnreadOnly(bool enabled) { unreadOnly=enabled; invalidateFilter(); }
protected:
    bool filterAcceptsRow(int row,const QModelIndex& parent) const override { return !unreadOnly||!sourceModel()->index(row,0,parent).data(NotificationModel::ReadRole).toBool(); }
};
int messageLineHeight(const QFont& font)
{
    // Chinese fallback fonts may be taller than QStyleOption's Latin fontMetrics.
    QTextLayout text(QStringLiteral("消息Ag"),font); text.beginLayout(); auto line=text.createLine(); line.setLineWidth(1000); text.endLayout();
    return int(std::ceil(line.height()))+1;
}
class MessageDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem& option,const QModelIndex&) const override { return QSize(120,messageLineHeight(option.font)*4+28); }
    void paint(QPainter* p,const QStyleOptionViewItem& option,const QModelIndex& index) const override {
        auto* t=ThemeTokens::instance(); p->save(); p->setClipRect(option.rect); p->setRenderHint(QPainter::Antialiasing);
        const bool unread=!index.data(NotificationModel::ReadRole).toBool();
        const bool selected=option.state&QStyle::State_Selected,enabled=option.state&QStyle::State_Enabled;
        const auto tone=static_cast<FeedbackTone>(index.data(NotificationModel::ToneRole).toInt());
        const QColor accent=enabled?feedbackPainting::toneColor(tone):t->textDisabled();
        p->fillRect(option.rect,selected?feedbackPainting::blend(t->bg1(),t->primary(),.18):t->bg1());
        QRect content=option.rect.adjusted(14,10,-12,-8); const int line=messageLineHeight(option.font);
        const bool rtl=option.direction==Qt::RightToLeft;
        p->setPen(Qt::NoPen); p->setBrush(accent);
        const int x=rtl?content.right()-3:content.left()+3; p->drawEllipse(QPointF(x,content.top()+line/2.),3,3);
        if(rtl)content.adjust(0,0,-15,0); else content.adjust(15,0,0,0);
        QFont heading=option.font; heading.setBold(unread); p->setFont(heading); p->setPen(enabled?t->textPrimary():t->textDisabled());
        const int align=Qt::AlignVCenter|(rtl?Qt::AlignRight:Qt::AlignLeft);
        p->drawText(QRect(content.left(),content.top(),content.width(),line+2),align,p->fontMetrics().elidedText(index.data(NotificationModel::TitleRole).toString(),Qt::ElideRight,content.width()));
        p->setFont(option.font); p->setPen(enabled?t->textSecondary():t->textDisabled());
        const QString preview=index.data(NotificationModel::BodyRole).toString().left(400);
        QTextLayout body(preview,option.font); QTextOption textOption; textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); textOption.setTextDirection(option.direction); body.setTextOption(textOption);
        body.beginLayout(); qreal y=0;
        for(int i=0;i<2;++i) { auto row=body.createLine(); if(!row.isValid())break; row.setLineWidth(content.width()); row.setPosition(QPointF(0,y)); y+=row.height(); }
        body.endLayout(); body.draw(p,QPointF(content.left(),content.top()+line+4));
        const auto timestamp=index.data(NotificationModel::TimestampRole).toDateTime();
        QString footer=unread?tr("未读"):tr("已读"); if(timestamp.isValid())footer+=" · "+timestamp.toLocalTime().toString("MM-dd HH:mm");
        p->drawText(QRect(content.left(),content.top()+line*3+9,content.width(),line),align,option.fontMetrics.elidedText(footer,Qt::ElideRight,content.width()));
        p->setPen(t->borderLight()); p->drawLine(option.rect.bottomLeft(),option.rect.bottomRight());
        if(option.state&QStyle::State_HasFocus) { p->setPen(QPen(t->primary(),1,Qt::DashLine)); p->setBrush(Qt::NoBrush); p->drawRect(option.rect.adjusted(2,2,-3,-3)); }
        p->restore();
    }
};
QLabel* textLabel(QWidget* parent) { auto* label=new QLabel(parent); label->setTextFormat(Qt::PlainText); label->setWordWrap(true); label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred); return label; }
}
NotificationPanel::NotificationPanel(QWidget* parent) : StyledWidget(parent)
{
    auto* layout=new QVBoxLayout(this); layout->setContentsMargins(12,12,12,12); layout->setSpacing(10);
    m_title=textLabel(this); QFont bold=m_title->font(); bold.setBold(true); m_title->setFont(bold); setTitle(tr("消息中心")); layout->addWidget(m_title);
    auto* tools=new QHBoxLayout; m_unread=new CheckBox(tr("仅未读"),this); m_unread->setObjectName("NotificationUnreadOnly");
    m_markAll=new PushButton(tr("全部已读"),this); m_markAll->setObjectName("NotificationMarkAll"); m_markAll->setAppearance(ButtonAppearance::Subtle);
    tools->addWidget(m_unread); tools->addStretch(); tools->addWidget(m_markAll); layout->addLayout(tools);
    m_stack=new QStackedWidget(this); layout->addWidget(m_stack,1);
    m_listPage=new QWidget(m_stack); auto* listLayout=new QVBoxLayout(m_listPage); listLayout->setContentsMargins(0,0,0,0);
    m_list=new ListView(m_listPage); m_list->setObjectName("NotificationList"); m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setFrameShape(QFrame::NoFrame); m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); m_list->setUniformItemSizes(true); m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setItemDelegate(new MessageDelegate(m_list)); m_list->setAccessibleName(tr("消息列表")); listLayout->addWidget(m_list);
    m_empty=textLabel(m_listPage); m_empty->setAlignment(Qt::AlignCenter); listLayout->addWidget(m_empty,1); m_stack->addWidget(m_listPage);
    m_detailPage=new QWidget(m_stack); auto* detail=new QVBoxLayout(m_detailPage); detail->setContentsMargins(0,0,0,0);
    auto* back=new PushButton(tr("返回消息列表"),m_detailPage); back->setObjectName("NotificationBack"); back->setAppearance(ButtonAppearance::Subtle); detail->addWidget(back,0,Qt::AlignLeading);
    m_detailTitle=textLabel(m_detailPage); m_detailTitle->setFont(bold); detail->addWidget(m_detailTitle);
    m_time=textLabel(m_detailPage); detail->addWidget(m_time);
    m_body=new PlainTextEdit(m_detailPage); m_body->setObjectName("NotificationBody"); m_body->setReadOnly(true); m_body->setFrameShape(QFrame::NoFrame); m_body->setAccessibleName(tr("消息正文")); detail->addWidget(m_body,1);
    m_action=new PushButton(m_detailPage); m_action->setObjectName("NotificationAction"); m_action->setAppearance(ButtonAppearance::Primary); detail->addWidget(m_action,0,Qt::AlignLeading); m_stack->addWidget(m_detailPage);
    auto* footer=new QHBoxLayout; m_count=textLabel(this); footer->addWidget(m_count,1);
    m_clear=new PushButton(tr("清空"),this); m_clear->setObjectName("NotificationClear"); m_clear->setAppearance(ButtonAppearance::Subtle); footer->addWidget(m_clear); layout->addLayout(footer);
    setFocusProxy(m_list); m_proxy=new UnreadProxy(this); m_proxy->setFilterRole(NotificationModel::ReadRole); m_list->setModel(m_proxy); m_owned=new NotificationModel(this);
    connect(m_unread,&QCheckBox::toggled,this,[this](bool enabled) { static_cast<UnreadProxy*>(m_proxy)->setUnreadOnly(enabled); refresh(); });
    connect(m_markAll,&QPushButton::clicked,this,[this] { if(m_model)m_model->markAllRead(); });
    connect(m_clear,&QPushButton::clicked,this,[this] { if(m_model)m_model->clear(); });
    auto activate=[this](const QModelIndex& index) { openMessage(index.data(NotificationModel::IdRole).toString()); };
    connect(m_list,&QListView::clicked,this,activate); connect(m_list,&QListView::activated,this,activate);
    connect(back,&QPushButton::clicked,this,&NotificationPanel::showList);
    connect(m_action,&QPushButton::clicked,this,[this] { const QString id=m_current; if(!id.isEmpty())emit actionRequested(id); });
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { m_list->viewport()->update(); update(); }); setModel(m_owned);
}
NotificationModel* NotificationPanel::model() const { return m_model; }
void NotificationPanel::setModel(NotificationModel* model)
{
    if(!model)model=m_owned; if(m_model==model)return;
    if(m_model)disconnect(m_model,nullptr,this,nullptr); m_model=model; m_proxy->setSourceModel(model); m_current.clear(); m_stack->setCurrentWidget(m_listPage);
    connect(model,&QAbstractItemModel::dataChanged,this,[this] { refresh(); });
    connect(model,&QAbstractItemModel::rowsInserted,this,[this] { refresh(); });
    connect(model,&QAbstractItemModel::rowsRemoved,this,[this] { refresh(); });
    connect(model,&QAbstractItemModel::modelReset,this,[this] { refresh(); });
    if(model!=m_owned)connect(model,&QObject::destroyed,this,[this] { m_model=nullptr; setModel(m_owned); }); refresh();
}
void NotificationPanel::setTitle(const QString& title) { m_title->setText(title); }
void NotificationPanel::setUnreadOnly(bool enabled) { m_unread->setChecked(enabled); }
bool NotificationPanel::unreadOnly() const { return m_unread->isChecked(); }
void NotificationPanel::refresh()
{
    if(!m_model)return;
    const int unread=m_model->unreadCount(); m_count->setText(tr("%1 条消息 · %2 条未读").arg(m_model->rowCount()).arg(unread));
    m_markAll->setEnabled(unread>0); m_clear->setEnabled(m_model->rowCount()>0);
    const bool empty=m_proxy->rowCount()==0; m_empty->setText(unreadOnly()?tr("暂无未读消息"):tr("暂无消息")); m_empty->setVisible(empty); m_list->setVisible(!empty);
    if(!m_current.isEmpty()) {
        const auto item=m_model->message(m_current); if(item.id.isEmpty()) { showList(); return; }
        m_detailTitle->setText(item.title); m_time->setText(item.timestamp.isValid()?item.timestamp.toLocalTime().toString("yyyy-MM-dd HH:mm:ss"):QString()); m_time->setVisible(item.timestamp.isValid());
        if(m_body->toPlainText()!=item.body)m_body->setPlainText(item.body);
        m_action->setText(item.actionText); m_action->setVisible(!item.actionText.isEmpty());
    }
}
void NotificationPanel::openMessage(const QString& id)
{
    if(!m_model||id.isEmpty()||m_model->message(id).id.isEmpty()||m_current==id)return;
    m_current=id; const QPointer<NotificationPanel> self(this); m_model->markRead(id); if(!self)return;
    if(m_current!=id||!m_model||m_model->message(id).id.isEmpty())return;
    m_stack->setCurrentWidget(m_detailPage); refresh(); m_body->setFocus(Qt::OtherFocusReason); emit messageActivated(id);
}
void NotificationPanel::showList() { m_current.clear(); m_stack->setCurrentWidget(m_listPage); if(isVisible())m_list->setFocus(Qt::OtherFocusReason); }
}
