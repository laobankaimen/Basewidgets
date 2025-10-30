#include <QtAdvancedUI/Core/StyledMainWindow.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

StyledMainWindow::StyledMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

StyledMainWindow::StyledMainWindow(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

StyledMainWindow::~StyledMainWindow() = default;

void StyledMainWindow::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void StyledMainWindow::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &StyledMainWindow::onStylesheetChanged);
    applyStylesheet();
}

void StyledMainWindow::onStylesheetChanged()
{
    applyStylesheet();
}

void StyledMainWindow::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI