#include <basewidgets/Gauges/DataManagementDashboards.h>
#include <basewidgets/Core/ThemeTokens.h>
#include "GaugePainting_p.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QSet>
#include <limits>
namespace basewidgets {
using namespace gaugePainting;
namespace {
const int QueueHeight=94;
QStringList taskLabels() { return {QObject::tr("排队"),QObject::tr("运行"),QObject::tr("成功"),QObject::tr("失败")}; }
QStringList qualityLabels() { return {QObject::tr("有效记录"),QObject::tr("文件缺失"),QObject::tr("损坏记录"),QObject::tr("重复记录"),QObject::tr("尚未核验")}; }
QVector<quint64> taskCounts(const TaskQueueSnapshot& queue) { return {queue.queued,queue.running,queue.succeeded,queue.failed}; }
QVector<QColor> taskColors() { auto* t=ThemeTokens::instance(); return {t->textSecondary(),t->primary(),t->success(),t->danger()}; }
QVector<QColor> qualityColors() { auto* t=ThemeTokens::instance(); return {t->success(),t->warning(),t->danger(),t->primary(),t->textSecondary()}; }
void surface(QPainter& painter,const QRect& rectangle)
{
    auto* t=ThemeTokens::instance(); painter.setPen(QPen(t->borderLight(),1)); painter.setBrush(t->bg1());
    painter.drawRoundedRect(QRectF(rectangle).adjusted(.5,.5,-.5,-.5),6,6);
}
void focus(QPainter& painter,const QRectF& rectangle)
{
    painter.setPen(QPen(ThemeTokens::instance()->primary(),1,Qt::DashLine)); painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rectangle.adjusted(1,1,-1,-1),3,3);
}
QString percentText(double value) { return std::isfinite(value)?QString::number(value,'f',1)+"%":QString::fromUtf8("—"); }
}
TaskQueueDashboard::TaskQueueDashboard(QWidget* parent) : StyledWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus); setAccessibleName(tr("数据处理任务队列"));
    setMinimumSize(minimumSizeHint());
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
}
QSize TaskQueueDashboard::sizeHint() const { return QSize(420,qMax(150,50+m_queues.size()*QueueHeight)); }
QSize TaskQueueDashboard::minimumSizeHint() const { return QSize(280,sizeHint().height()); }
bool TaskQueueDashboard::setQueues(const QVector<TaskQueueSnapshot>& queues)
{
    if(queues.size()>12)return false;
    QSet<QString> ids;
    for(const auto& queue:queues) {
        if(queue.id.trimmed().isEmpty()||queue.title.trimmed().isEmpty()||ids.contains(queue.id))return false;
        quint64 sum=0;
        for(quint64 count:taskCounts(queue)) {
            if(count>std::numeric_limits<quint64>::max()-sum)return false;
            sum+=count;
        }
        ids.insert(queue.id);
    }
    // Preserve keyboard focus by stable queue ID when snapshots reorder queues.
    const QString selectedId=m_selected/4<m_queues.size()?m_queues[m_selected/4].id:QString();
    const int selectedState=m_selected%4;
    m_queues=queues; m_selected=qBound(0,m_selected,qMax(0,m_queues.size()*4-1));
    for(int i=0;i<m_queues.size();++i)if(m_queues[i].id==selectedId)m_selected=i*4+selectedState;
    QString description=tr("任务队列；完成包含成功和失败，成功/失败分开统计。");
    const auto labels=taskLabels();
    for(const auto& queue:m_queues) {
        description+="\n"+queue.title; const auto counts=taskCounts(queue);
        for(int state=0;state<4;++state)description+=QString(" · %1 %2").arg(labels[state]).arg(counts[state]);
    }
    setAccessibleDescription(description); setToolTip(description);
    setMinimumHeight(minimumSizeHint().height()); updateGeometry(); update(); emit queuesChanged(); return true;
}
QRectF TaskQueueDashboard::stateRect(int queue,int state) const
{
    const qreal column=(width()-24)/4.; return QRectF(12+state*column,88+queue*QueueHeight,column,44);
}
int TaskQueueDashboard::hit(const QPoint& point) const
{
    for(int queue=0;queue<m_queues.size();++queue)for(int state=0;state<4;++state)
        if(stateRect(queue,state).contains(point))return queue*4+state;
    return -1;
}
void TaskQueueDashboard::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); surface(p,rect()); auto* t=ThemeTokens::instance();
    const QColor foreground=isEnabled()?t->textPrimary():t->textDisabled();
    text(p,QRectF(14,10,width()-28,24),tr("任务队列 / 处理进度"),sizedFont(font(),14,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
    if(m_queues.isEmpty()) { text(p,QRectF(14,62,width()-28,32),tr("暂无任务队列快照"),font(),t->textSecondary()); return; }
    const auto labels=taskLabels(); const auto colors=taskColors();
    for(int i=0;i<m_queues.size();++i) {
        const auto& queue=m_queues[i]; const auto counts=taskCounts(queue);
        quint64 total=0; for(auto count:counts)total+=count;
        const int y=44+i*QueueHeight;
        text(p,QRectF(14,y,width()-146,24),queue.title,sizedFont(font(),13,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
        text(p,QRectF(width()-132,y,116,24),total?tr("完成 %1%").arg(100.*double(queue.succeeded+queue.failed)/double(total),0,'f',1):tr("暂无任务"),sizedFont(font(),12),t->textSecondary(),Qt::AlignRight|Qt::AlignVCenter);
        const QRectF bar(14,y+29,width()-28,8); p.fillRect(bar,t->bg2()); double x=bar.left();
        for(int state=0;state<4;++state) {
            const qreal portion=total?bar.width()*double(counts[state])/double(total):0;
            p.fillRect(QRectF(x,bar.top(),portion,bar.height()),isEnabled()?colors[state]:t->textDisabled()); x+=portion;
            const auto box=stateRect(i,state);
            text(p,QRectF(box.x(),box.y(),box.width(),18),labels[state],sizedFont(font(),11),isEnabled()?colors[state]:t->textDisabled());
            text(p,QRectF(box.x(),box.y()+18,box.width(),24),QString::number(counts[state]),sizedFont(font(),15,true),foreground);
            if(hasFocus()&&m_selected==i*4+state)focus(p,box);
        }
        if(i+1<m_queues.size()) { p.setPen(t->borderLight()); p.drawLine(14,y+89,width()-14,y+89); }
    }
}
void TaskQueueDashboard::activate()
{
    if(m_selected>=0&&m_selected/4<m_queues.size())emit stateActivated(m_queues[m_selected/4].id,State(m_selected%4));
}
void TaskQueueDashboard::mousePressEvent(QMouseEvent* event)
{
    const int selected=hit(event->pos());
    if(event->button()==Qt::LeftButton&&selected>=0) { m_selected=selected; setFocus(Qt::MouseFocusReason); update(); activate(); event->accept(); return; }
    StyledWidget::mousePressEvent(event);
}
void TaskQueueDashboard::keyPressEvent(QKeyEvent* event)
{
    if(m_queues.isEmpty()) { StyledWidget::keyPressEvent(event); return; }
    int delta=0;
    switch(event->key()) {
    case Qt::Key_Left: delta=-1; break; case Qt::Key_Right: delta=1; break;
    case Qt::Key_Up: delta=-4; break; case Qt::Key_Down: delta=4; break;
    case Qt::Key_Enter: case Qt::Key_Return: case Qt::Key_Space: activate(); event->accept(); return;
    default: StyledWidget::keyPressEvent(event); return;
    }
    m_selected=qBound(0,m_selected+delta,m_queues.size()*4-1); update(); event->accept();
}
void TaskQueueDashboard::contextMenuEvent(QContextMenuEvent* event)
{
    const int selected=event->reason()==QContextMenuEvent::Keyboard?m_selected:hit(event->pos());
    if(selected<0||selected/4>=m_queues.size())return;
    m_selected=selected; update();
    auto* menu=new QMenu(this); menu->setAttribute(Qt::WA_DeleteOnClose);
    const QString description=accessibleDescription();
    connect(menu->addAction(tr("复制队列快照")),&QAction::triggered,this,[description] { QApplication::clipboard()->setText(description); });
    emit prepareContextMenu(menu,m_queues[selected/4].id,State(selected%4));
    menu->popup(event->reason()==QContextMenuEvent::Keyboard?mapToGlobal(stateRect(selected/4,selected%4).center().toPoint()):event->globalPos());
    event->accept();
}
void TaskQueueDashboard::focusInEvent(QFocusEvent* event) { StyledWidget::focusInEvent(event); update(); }
void TaskQueueDashboard::focusOutEvent(QFocusEvent* event) { StyledWidget::focusOutEvent(event); update(); }
DataQualityDashboard::DataQualityDashboard(QWidget* parent) : StyledWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus); setAccessibleName(tr("数据质量概览")); setMinimumSize(minimumSizeHint());
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,[this] { update(); });
    clear();
}
quint64 DataQualityDashboard::checkedRecords() const { return m_snapshot.valid+m_snapshot.missing+m_snapshot.corrupt+m_snapshot.duplicate; }
quint64 DataQualityDashboard::uncheckedRecords() const { return m_snapshot.total-checkedRecords(); }
double DataQualityDashboard::coveragePercent() const { return m_known&&m_snapshot.total?100.*double(checkedRecords())/double(m_snapshot.total):std::numeric_limits<double>::quiet_NaN(); }
double DataQualityDashboard::passPercent() const { const auto checked=checkedRecords(); return m_known&&checked?100.*double(m_snapshot.valid)/double(checked):std::numeric_limits<double>::quiet_NaN(); }
bool DataQualityDashboard::setSnapshot(const DataQualitySnapshot& snapshot)
{
    quint64 checked=0;
    for(auto count:{snapshot.valid,snapshot.missing,snapshot.corrupt,snapshot.duplicate}) {
        if(count>snapshot.total-checked)return false;
        checked+=count;
    }
    m_snapshot=snapshot; m_known=true;
    const QString description=tr("总记录 %1；有效 %2，缺失 %3，损坏 %4，重复 %5，未核验 %6。核验覆盖率 %7，已核验通过率 %8。")
        .arg(snapshot.total).arg(snapshot.valid).arg(snapshot.missing).arg(snapshot.corrupt).arg(snapshot.duplicate).arg(uncheckedRecords()).arg(percentText(coveragePercent()),percentText(passPercent()))
        +(snapshot.observedAt.isValid()?tr(" 采样于 %1").arg(snapshot.observedAt.toString(Qt::ISODate)):tr(" 未提供采样时间"));
    setAccessibleDescription(description); setToolTip(description); update(); emit snapshotChanged(); return true;
}
void DataQualityDashboard::clear()
{
    m_known=false; m_snapshot={}; m_selected=0; setAccessibleDescription(tr("尚未采样")); setToolTip(tr("尚未采样")); update(); emit snapshotChanged();
}
QRectF DataQualityDashboard::categoryRect(int category) const { return QRectF(12,169+category*34,width()-24,32); }
int DataQualityDashboard::hit(const QPoint& point) const
{
    if(!m_known)return -1;
    for(int category=0;category<5;++category)if(categoryRect(category).contains(point))return category;
    return -1;
}
void DataQualityDashboard::paintEvent(QPaintEvent*)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); surface(p,rect()); auto* t=ThemeTokens::instance();
    const QColor foreground=isEnabled()?t->textPrimary():t->textDisabled();
    text(p,QRectF(14,10,width()-28,24),tr("数据质量 / 核验概览"),sizedFont(font(),14,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
    const qreal half=(width()-28)/2.;
    for(int i=0;i<2;++i) {
        text(p,QRectF(14+i*half,46,half,20),i?tr("已核验通过率"):tr("核验覆盖率"),sizedFont(font(),12),t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
        text(p,QRectF(14+i*half,70,half,36),percentText(i?passPercent():coveragePercent()),sizedFont(font(),27,true),foreground,Qt::AlignLeft|Qt::AlignVCenter);
    }
    const QString status=!m_known?tr("尚未采样"):!m_snapshot.total?tr("空数据集"):tr("%1 / %2 条已核验").arg(checkedRecords()).arg(m_snapshot.total);
    text(p,QRectF(14,112,width()-28,22),status,sizedFont(font(),12),t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
    const QVector<quint64> counts{m_snapshot.valid,m_snapshot.missing,m_snapshot.corrupt,m_snapshot.duplicate,uncheckedRecords()};
    const auto colors=qualityColors(); const auto labels=qualityLabels();
    const QRectF bar(14,146,width()-28,10); p.fillRect(bar,t->bg2()); double x=bar.left();
    for(int i=0;i<5;++i) {
        const QColor color=isEnabled()?colors[i]:t->textDisabled();
        const double portion=m_snapshot.total?bar.width()*double(counts[i])/double(m_snapshot.total):0;
        p.fillRect(QRectF(x,bar.top(),portion,bar.height()),color); x+=portion;
        const auto box=categoryRect(i); p.fillRect(QRectF(box.x()+3,box.y()+12,7,7),color);
        text(p,QRectF(box.x()+18,box.y(),box.width()*.48-18,box.height()),labels[i],sizedFont(font(),12),foreground,Qt::AlignLeft|Qt::AlignVCenter);
        text(p,QRectF(box.x()+box.width()*.48,box.y(),box.width()*.52-4,box.height()),m_known?QString::number(counts[i]):QString::fromUtf8("—"),sizedFont(font(),14,true),foreground,Qt::AlignRight|Qt::AlignVCenter);
        if(m_known&&hasFocus()&&m_selected==i)focus(p,box);
    }
    text(p,QRectF(14,348,width()-28,20),m_snapshot.observedAt.isValid()?tr("采样 %1").arg(m_snapshot.observedAt.toLocalTime().toString("yyyy-MM-dd HH:mm:ss")):tr("未提供采样时间"),sizedFont(font(),11),t->textSecondary(),Qt::AlignLeft|Qt::AlignVCenter);
}
void DataQualityDashboard::mousePressEvent(QMouseEvent* event)
{
    const int category=hit(event->pos());
    if(event->button()==Qt::LeftButton&&category>=0) { m_selected=category; setFocus(Qt::MouseFocusReason); update(); emit categoryActivated(Category(category)); event->accept(); return; }
    StyledWidget::mousePressEvent(event);
}
void DataQualityDashboard::keyPressEvent(QKeyEvent* event)
{
    if(!m_known) { StyledWidget::keyPressEvent(event); return; }
    switch(event->key()) {
    case Qt::Key_Up: m_selected=qMax(0,m_selected-1); break;
    case Qt::Key_Down: m_selected=qMin(4,m_selected+1); break;
    case Qt::Key_Enter: case Qt::Key_Return: case Qt::Key_Space: emit categoryActivated(Category(m_selected)); break;
    default: StyledWidget::keyPressEvent(event); return;
    }
    update(); event->accept();
}
void DataQualityDashboard::contextMenuEvent(QContextMenuEvent* event)
{
    const int category=event->reason()==QContextMenuEvent::Keyboard?m_selected:hit(event->pos());
    if(!m_known||category<0)return;
    m_selected=category; update(); auto* menu=new QMenu(this); menu->setAttribute(Qt::WA_DeleteOnClose);
    const QString description=accessibleDescription();
    connect(menu->addAction(tr("复制质量快照")),&QAction::triggered,this,[description] { QApplication::clipboard()->setText(description); });
    emit prepareContextMenu(menu,Category(category));
    menu->popup(event->reason()==QContextMenuEvent::Keyboard?mapToGlobal(categoryRect(category).center().toPoint()):event->globalPos());
    event->accept();
}
void DataQualityDashboard::focusInEvent(QFocusEvent* event) { StyledWidget::focusInEvent(event); update(); }
void DataQualityDashboard::focusOutEvent(QFocusEvent* event) { StyledWidget::focusOutEvent(event); update(); }
}
