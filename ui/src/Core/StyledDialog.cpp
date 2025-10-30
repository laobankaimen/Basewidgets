#include <QtAdvancedUI/Core/StyledDialog.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

StyledDialog::StyledDialog(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

StyledDialog::StyledDialog(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

StyledDialog::~StyledDialog() = default;

void StyledDialog::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void StyledDialog::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &StyledDialog::onStylesheetChanged);
    applyStylesheet();
}

void StyledDialog::onStylesheetChanged()
{
    applyStylesheet();
}

void StyledDialog::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI