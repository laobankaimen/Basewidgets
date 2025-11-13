#include <QtAdvancedUI/Containers/ToolBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ToolBox::ToolBox(QWidget* parent)
    : QToolBox(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ToolBox::ToolBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QToolBox(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ToolBox::~ToolBox() = default;

void ToolBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ToolBox::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void ToolBox::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void ToolBox::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI