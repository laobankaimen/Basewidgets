#include <QtAdvancedUI/Containers/ToolBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ToolBox::ToolBox(QWidget* parent)
    : QToolBox(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ToolBox::ToolBox(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QToolBox(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ToolBox::~ToolBox() = default;

void ToolBox::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ToolBox::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ToolBox::onStylesheetChanged);
    applyStylesheet();
}

void ToolBox::onStylesheetChanged()
{
    applyStylesheet();
}

void ToolBox::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI