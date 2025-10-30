#include <QtAdvancedUI/Containers/ScrollArea.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ScrollArea::ScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ScrollArea::ScrollArea(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ScrollArea::~ScrollArea() = default;

void ScrollArea::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ScrollArea::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ScrollArea::onStylesheetChanged);
    applyStylesheet();
}

void ScrollArea::onStylesheetChanged()
{
    applyStylesheet();
}

void ScrollArea::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI