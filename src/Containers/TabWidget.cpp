/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Containers/TabWidget.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QTabBar>

using namespace acss;

namespace basewidgets {

TabWidget::TabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

TabWidget::TabWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTabWidget(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TabWidget::~TabWidget() = default;

void TabWidget::setHoverEffectEnabled(bool enabled)
{
    tabBar()->setProperty("bwTabHoverEnabled",enabled);
    tabBar()->update();
}

bool TabWidget::hoverEffectEnabled() const
{
    return tabBar()->property("bwTabHoverEnabled").toBool();
}

void TabWidget::setContentHoverEffectEnabled(bool enabled)
{
    setProperty("bwFrameHoverEnabled",enabled); update();
}
bool TabWidget::contentHoverEffectEnabled() const { return property("bwFrameHoverEnabled").toBool(); }

void TabWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TabWidget::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void TabWidget::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void TabWidget::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

