#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

PushButton::PushButton(QWidget* parent)
    : QPushButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

PushButton::PushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

PushButton::PushButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QPushButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

PushButton::~PushButton() = default;

void PushButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void PushButton::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void PushButton::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void PushButton::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
