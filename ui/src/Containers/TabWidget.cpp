#include <QtAdvancedUI/Containers/TabWidget.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TabWidget::TabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

TabWidget::TabWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTabWidget(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TabWidget::~TabWidget() = default;

void TabWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TabWidget::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TabWidget::onStylesheetChanged);
    applyStylesheet();
}

void TabWidget::onStylesheetChanged()
{
    applyStylesheet();
}

void TabWidget::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI