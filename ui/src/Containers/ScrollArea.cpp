/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QtAdvancedUI/Containers/ScrollArea.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ScrollArea::ScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ScrollArea::ScrollArea(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ScrollArea::~ScrollArea() = default;

void ScrollArea::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ScrollArea::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void ScrollArea::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void ScrollArea::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI

