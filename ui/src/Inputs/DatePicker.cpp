#include <QtAdvancedUI/Inputs/DatePicker.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QCalendarWidget>

using namespace acss;

namespace QtAdvancedUI {

DatePicker::DatePicker(QWidget* parent)
    : QDateEdit(parent)
{
    setCalendarPopup(true);
    setDisplayFormat("yyyy-MM-dd");
    setStyleManager(StyleManagerAdapter::instance());
    setWeekNumbersEnabled(true);
}

DatePicker::DatePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QDateEdit(parent)
{
    setCalendarPopup(true);
    setDisplayFormat("yyyy-MM-dd");
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
    setWeekNumbersEnabled(true);
}

DatePicker::~DatePicker() = default;

void DatePicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void DatePicker::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &DatePicker::onStylesheetChanged);
    applyStylesheet();
}

void DatePicker::onStylesheetChanged()
{
    applyStylesheet();
}

void DatePicker::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

void DatePicker::setWeekNumbersEnabled(bool enabled)
{
    m_weekNumbers = enabled;
    QCalendarWidget* cal = this->calendarWidget();
    if (!cal) {
        cal = new QCalendarWidget(this);
        this->setCalendarWidget(cal);
    }
    cal->setVerticalHeaderFormat(enabled ? QCalendarWidget::ISOWeekNumbers
                                         : QCalendarWidget::NoVerticalHeader);
}

} // namespace QtAdvancedUI