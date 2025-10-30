#include <QtAdvancedUI/Views/ListView.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ListView::ListView(QWidget* parent)
    : QListView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ListView::ListView(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QListView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ListView::~ListView() = default;

void ListView::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ListView::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ListView::onStylesheetChanged);
    applyStylesheet();
}

void ListView::onStylesheetChanged()
{
    applyStylesheet();
}

void ListView::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI