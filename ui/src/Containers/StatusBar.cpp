#include <QtAdvancedUI/Containers/StatusBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

StatusBar::StatusBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QStatusBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

StatusBar::~StatusBar() = default;

void StatusBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void StatusBar::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &StatusBar::onStylesheetChanged);
    applyStylesheet();
}

void StatusBar::onStylesheetChanged()
{
    applyStylesheet();
}

void StatusBar::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI