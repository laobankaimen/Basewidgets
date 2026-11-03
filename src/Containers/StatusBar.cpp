/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Containers/StatusBar.h>
#include <basewidgets/Inputs/StatusBadge.h>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

StatusBar::StatusBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QStatusBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

StatusBar::~StatusBar() = default;

StatusBadge* StatusBar::addIndicator(const QString& text,FeedbackTone tone)
{
    auto* badge=new StatusBadge(text,this); badge->setTone(tone); addPermanentWidget(badge); return badge;
}
void StatusBar::setVersionText(const QString& version)
{
    if(!m_version) { m_version=new QLabel(this); m_version->setObjectName("StatusVersion"); m_version->setTextFormat(Qt::PlainText); addPermanentWidget(m_version); }
    m_version->setText(version); m_version->setAccessibleName(tr("版本 %1").arg(version)); m_version->setVisible(!version.isEmpty());
}
void StatusBar::setClockVisible(bool visible)
{
    if(!m_clock && visible) {
        m_clock=new QLabel(this); m_clock->setObjectName("StatusClock"); m_clock->setToolTip(tr("本机时间；不是数据更新时间")); addPermanentWidget(m_clock);
        m_clockTimer=new QTimer(this); m_clockTimer->setInterval(1000);
        connect(m_clockTimer,&QTimer::timeout,this,[this] { m_clock->setText(QDateTime::currentDateTime().toString("MM-dd HH:mm:ss")); });
    }
    if(!m_clock)return;
    m_clock->setVisible(visible);
    if(visible) { m_clock->setText(QDateTime::currentDateTime().toString("MM-dd HH:mm:ss")); m_clockTimer->start(); }
    else m_clockTimer->stop();
}

void StatusBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void StatusBar::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void StatusBar::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void StatusBar::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

