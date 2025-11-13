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
    // 组件级不再连接样式变更信号；应用级统一管理
}

void DateTimePicker::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void DateTimePicker::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI