#include <QtAdvancedUI/Inputs/RadioButton.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

RadioButton::RadioButton(QWidget* parent)
    : QRadioButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

RadioButton::RadioButton(const QString& text, QWidget* parent)
    : QRadioButton(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

RadioButton::RadioButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QRadioButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

RadioButton::~RadioButton() = default;

void RadioButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void RadioButton::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &RadioButton::onStylesheetChanged);
    applyStylesheet();
}

void RadioButton::onStylesheetChanged()
{
    applyStylesheet();
}

void RadioButton::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI