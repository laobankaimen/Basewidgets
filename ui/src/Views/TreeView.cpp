#include <QtAdvancedUI/Views/TreeView.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TreeView::TreeView(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTreeView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TreeView::~TreeView() = default;

void TreeView::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TreeView::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TreeView::onStylesheetChanged);
    applyStylesheet();
}

void TreeView::onStylesheetChanged()
{
    applyStylesheet();
}

void TreeView::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI