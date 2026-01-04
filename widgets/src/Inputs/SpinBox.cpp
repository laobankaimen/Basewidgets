/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/SpinBox.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

SpinBox::SpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

SpinBox::SpinBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QSpinBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

SpinBox::~SpinBox() = default;

void SpinBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void SpinBox::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void SpinBox::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void SpinBox::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

