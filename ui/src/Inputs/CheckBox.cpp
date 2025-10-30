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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &CheckBox::onStylesheetChanged);
    applyStylesheet();
}

void CheckBox::onStylesheetChanged()
{
    applyStylesheet();
}

void CheckBox::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI