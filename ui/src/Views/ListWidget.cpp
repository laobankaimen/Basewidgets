#include <QtAdvancedUI/Views/ListWidget.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ListWidget::ListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ListWidget::ListWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QListWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ListWidget::~ListWidget() = default;

void ListWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ListWidget::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ListWidget::onStylesheetChanged);
    applyStylesheet();
}

void ListWidget::onStylesheetChanged()
{
    applyStylesheet();
}

void ListWidget::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI