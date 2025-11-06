#include <QtAdvancedUI/Inputs/TimePicker.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TimePicker::TimePicker(QWidget* parent)
    : QTimeEdit(parent)
{
    setDisplayFormat("HH:mm:ss");
    setStyleManager(StyleManagerAdapter::instance());
}

TimePicker::TimePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTimeEdit(parent)
{
    setDisplayFormat("HH:mm:ss");
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TimePicker::~TimePicker() = default;

void TimePicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TimePicker::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TimePicker::onStylesheetChanged);
    applyStylesheet();
}

void TimePicker::onStylesheetChanged()
{
    applyStylesheet();
}

void TimePicker::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI