/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

ComboBox::ComboBox(QWidget* parent)
    : QComboBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

ComboBox::ComboBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QComboBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ComboBox::~ComboBox() = default;

void ComboBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ComboBox::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void ComboBox::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void ComboBox::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

