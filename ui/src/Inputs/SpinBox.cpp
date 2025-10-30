#include <QtAdvancedUI/Inputs/SpinBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

SpinBox::SpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

SpinBox::SpinBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QSpinBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

SpinBox::~SpinBox() = default;

void SpinBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void SpinBox::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &SpinBox::onStylesheetChanged);
    applyStylesheet();
}

void SpinBox::onStylesheetChanged()
{
    applyStylesheet();
}

void SpinBox::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI