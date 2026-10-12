/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Containers/MenuBar.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

MenuBar::MenuBar(QWidget* parent)
    : QMenuBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

MenuBar::MenuBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QMenuBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

MenuBar::~MenuBar() = default;

void MenuBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void MenuBar::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void MenuBar::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void MenuBar::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

