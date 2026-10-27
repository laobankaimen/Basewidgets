/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

PushButton::PushButton(QWidget* parent)
    : QPushButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

PushButton::PushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

PushButton::PushButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QPushButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

PushButton::~PushButton() = default;

void PushButton::setAppearance(ButtonAppearance appearance)
{
    // Dynamic properties also let NativeStyle render plain Qt child buttons consistently.
    setProperty("bwAppearance", static_cast<int>(appearance));
    update();
}

ButtonAppearance PushButton::appearance() const
{
    return static_cast<ButtonAppearance>(property("bwAppearance").toInt());
}

void PushButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void PushButton::attachToManager() {}
void PushButton::onStylesheetChanged() {}
void PushButton::applyStylesheet() {}

} // namespace basewidgets
