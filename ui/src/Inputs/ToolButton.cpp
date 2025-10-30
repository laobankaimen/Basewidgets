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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ToolButton::onStylesheetChanged);
    applyStylesheet();
}

void ToolButton::onStylesheetChanged()
{
    applyStylesheet();
}

void ToolButton::applyStylesheet()
{
    if (!m_manager) return;
    // 关闭按钮由 FramelessDialog 的父级样式控制，避免全局样式覆盖其图标和大小
    if (objectName() == QStringLiteral("FramelessDialogCloseButton")) {
        return;
    }
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI