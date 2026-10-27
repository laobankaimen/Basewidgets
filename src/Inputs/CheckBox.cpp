/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {


CheckBox::CheckBox(QWidget* parent)
    : QCheckBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

CheckBox::CheckBox(const QString& text, QWidget* parent)
    : QCheckBox(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

CheckBox::CheckBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QCheckBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
    setAttribute(Qt::WA_Hover);
}

CheckBox::~CheckBox() = default;

void CheckBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void CheckBox::attachToManager() {}
void CheckBox::onStylesheetChanged() {}
void CheckBox::applyStylesheet() {}

} // namespace basewidgets
