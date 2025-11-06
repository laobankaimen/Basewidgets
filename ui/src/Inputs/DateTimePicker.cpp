#include <QtAdvancedUI/Inputs/DateTimePicker.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

DateTimePicker::DateTimePicker(QWidget* parent)
    : QDateTimeEdit(parent)
{
    setCalendarPopup(true);
    setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    setStyleManager(StyleManagerAdapter::instance());
}

DateTimePicker::DateTimePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QDateTimeEdit(parent)
{
    setCalendarPopup(true);
    setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

DateTimePicker::~DateTimePicker() = default;

void DateTimePicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void DateTimePicker::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &DateTimePicker::onStylesheetChanged);
    applyStylesheet();
}

void DateTimePicker::onStylesheetChanged()
{
    applyStylesheet();
}

void DateTimePicker::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI