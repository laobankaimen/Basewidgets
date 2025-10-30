#include <QtAdvancedUI/Containers/MenuBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

MenuBar::MenuBar(QWidget* parent)
    : QMenuBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

MenuBar::MenuBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QMenuBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

MenuBar::~MenuBar() = default;

void MenuBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void MenuBar::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &MenuBar::onStylesheetChanged);
    applyStylesheet();
}

void MenuBar::onStylesheetChanged()
{
    applyStylesheet();
}

void MenuBar::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI