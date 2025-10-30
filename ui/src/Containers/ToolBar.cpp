#include <QtAdvancedUI/Containers/ToolBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ToolBar::ToolBar(QWidget* parent)
    : QToolBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ToolBar::ToolBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QToolBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ToolBar::~ToolBar() = default;

void ToolBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ToolBar::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ToolBar::onStylesheetChanged);
    applyStylesheet();
}

void ToolBar::onStylesheetChanged()
{
    applyStylesheet();
}

void ToolBar::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI