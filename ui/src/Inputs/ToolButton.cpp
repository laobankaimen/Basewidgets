/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QtAdvancedUI/Inputs/ToolButton.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ToolButton::ToolButton(QWidget* parent)
    : QToolButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

ToolButton::ToolButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QToolButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ToolButton::~ToolButton() = default;

void ToolButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ToolButton::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void ToolButton::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void ToolButton::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI

