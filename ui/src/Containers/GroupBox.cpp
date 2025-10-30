#include <QtAdvancedUI/Containers/GroupBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

GroupBox::GroupBox(QWidget* parent)
    : QGroupBox(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

GroupBox::GroupBox(const QString& title, QWidget* parent)
    : QGroupBox(title, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

GroupBox::GroupBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QGroupBox(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

GroupBox::~GroupBox() = default;

void GroupBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void GroupBox::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &GroupBox::onStylesheetChanged);
    applyStylesheet();
}

void GroupBox::onStylesheetChanged()
{
    applyStylesheet();
}

void GroupBox::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI