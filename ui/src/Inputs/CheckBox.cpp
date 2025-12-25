#include <QtAdvancedUI/Inputs/CheckBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

CheckBox::CheckBox(QWidget* parent)
    : QCheckBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

CheckBox::CheckBox(const QString& text, QWidget* parent)
    : QCheckBox(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

CheckBox::CheckBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QCheckBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

CheckBox::~CheckBox() = default;

void CheckBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void CheckBox::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void CheckBox::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void CheckBox::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
