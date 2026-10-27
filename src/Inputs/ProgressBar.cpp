/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

ProgressBar::ProgressBar(QWidget* parent)
    : QProgressBar(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

ProgressBar::ProgressBar(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QProgressBar(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ProgressBar::~ProgressBar() = default;

void ProgressBar::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ProgressBar::attachToManager() {}
void ProgressBar::onStylesheetChanged() {}
void ProgressBar::applyStylesheet() {}

} // namespace basewidgets
