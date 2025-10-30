#include <QtAdvancedUI/Inputs/ProgressBar.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

ProgressBar::ProgressBar(QWidget* parent)
    : QProgressBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

ProgressBar::ProgressBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QProgressBar(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ProgressBar::~ProgressBar() = default;

void ProgressBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ProgressBar::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ProgressBar::onStylesheetChanged);
    applyStylesheet();
}

void ProgressBar::onStylesheetChanged()
{
    applyStylesheet();
}

void ProgressBar::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI