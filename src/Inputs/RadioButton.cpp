/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/RadioButton.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {


RadioButton::RadioButton(QWidget* parent)
    : QRadioButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

RadioButton::RadioButton(const QString& text, QWidget* parent)
    : QRadioButton(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

RadioButton::RadioButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QRadioButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

RadioButton::~RadioButton() = default;

void RadioButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void RadioButton::attachToManager() {}
void RadioButton::onStylesheetChanged() {}
void RadioButton::applyStylesheet() {}

} // namespace basewidgets
