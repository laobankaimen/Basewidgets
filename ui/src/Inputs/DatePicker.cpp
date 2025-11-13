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
    // 组件级不再连接样式变更信号；应用级统一管理
}

void DatePicker::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void DatePicker::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
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