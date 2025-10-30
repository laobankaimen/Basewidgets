#include <QtAdvancedUI/Inputs/ComboBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ComboBox::ComboBox(QWidget* parent)
    : QComboBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

ComboBox::ComboBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QComboBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ComboBox::~ComboBox() = default;

void ComboBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ComboBox::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ComboBox::onStylesheetChanged);
    applyStylesheet();
}

void ComboBox::onStylesheetChanged()
{
    applyStylesheet();
}

void ComboBox::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI