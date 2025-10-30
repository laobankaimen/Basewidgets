#include <QtAdvancedUI/Views/TreeWidget.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TreeWidget::TreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TreeWidget::TreeWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTreeWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TreeWidget::~TreeWidget() = default;

void TreeWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TreeWidget::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TreeWidget::onStylesheetChanged);
    applyStylesheet();
}

void TreeWidget::onStylesheetChanged()
{
    applyStylesheet();
}

void TreeWidget::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI