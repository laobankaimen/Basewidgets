#include <QtAdvancedUI/Views/TableWidget.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TableWidget::TableWidget(QWidget* parent)
    : QTableWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TableWidget::TableWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTableWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TableWidget::~TableWidget() = default;

void TableWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TableWidget::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TableWidget::onStylesheetChanged);
    applyStylesheet();
}

void TableWidget::onStylesheetChanged()
{
    applyStylesheet();
}

void TableWidget::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI