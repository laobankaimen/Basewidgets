#include <QtAdvancedUI/Containers/GroupBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

GroupBox::GroupBox(QWidget* parent)
    : QGroupBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

GroupBox::GroupBox(const QString& title, QWidget* parent)
    : QGroupBox(title, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

GroupBox::GroupBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QGroupBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

GroupBox::~GroupBox() = default;

void GroupBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void GroupBox::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void GroupBox::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void GroupBox::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
