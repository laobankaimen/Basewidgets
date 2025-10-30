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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &PushButton::onStylesheetChanged);
    applyStylesheet();
}

void PushButton::onStylesheetChanged()
{
    applyStylesheet();
}

void PushButton::applyStylesheet()
{
    if (!m_manager) return;
    // 为简化，应用全局样式表；后续可细化状态样式
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI