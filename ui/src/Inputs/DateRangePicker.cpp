/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QtAdvancedUI/Inputs/DateRangePicker.h>
#include <QtAdvancedUI/Inputs/DatePicker.h>
#include <QtAdvancedUI/Inputs/TimePicker.h>
#include <QtAdvancedUI/Inputs/ComboBox.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QGridLayout>
#include <QCalendarWidget>

using namespace acss;

namespace QtAdvancedUI {

DateRangePicker::DateRangePicker(QWidget* parent)
    : StyledWidget(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
    m_layout = new QGridLayout(this);
    m_start = new DatePicker(this);
    m_end = new DatePicker(this);
    m_presets = new ComboBox(this);
    m_zoneCombo = new ComboBox(this);
    m_startTime = new TimePicker(this);
    m_endTime = new TimePicker(this);

    m_layout->addWidget(m_start, 0, 0);
    m_layout->addWidget(m_end, 0, 1);
    m_layout->addWidget(m_startTime, 0, 2);
    m_layout->addWidget(m_endTime, 0, 3);
    m_layout->addWidget(m_presets, 1, 0, 1, 2);
    m_layout->addWidget(m_zoneCombo, 1, 2, 1, 2);
    setLayout(m_layout);

    rebuildPresets();
    // 默认时区列表（常用 + 示例）
    updateTimeZoneCombo({"本地时区", "UTC", "Asia/Shanghai", "Asia/Tokyo", "Europe/London", "Europe/Berlin", "America/Los_Angeles", "America/New_York"});
    connect(m_presets, QOverload<int>::of(&ComboBox::currentIndexChanged), [this](int){
        applyPreset(m_presets->currentText());
    });
    connect(m_start, &QDateEdit::dateChanged, [this](const QDate&){ emit rangeChanged(m_start->dateTime(), m_end->dateTime()); });
    connect(m_end, &QDateEdit::dateChanged, [this](const QDate&){ emit rangeChanged(m_start->dateTime(), m_end->dateTime()); });
    connect(m_startTime, &QTimeEdit::timeChanged, [this](const QTime&){ if (m_includeTime) emit rangeChanged(range().first, range().second); });
    connect(m_endTime, &QTimeEdit::timeChanged, [this](const QTime&){ if (m_includeTime) emit rangeChanged(range().first, range().second); });
    setIncludeTime(false);
    setFirstDayOfWeek(m_firstDay);
}

DateRangePicker::DateRangePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : StyledWidget(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
    m_layout = new QGridLayout(this);
    m_start = new DatePicker(this);
    m_start->calendarWidget()->setGridVisible(true);
    m_end = new DatePicker(this);
    m_end->calendarWidget()->setGridVisible(true);
    m_presets = new ComboBox(this);
    m_zoneCombo = new ComboBox(this);
    m_startTime = new TimePicker(this);
    m_endTime = new TimePicker(this);
    m_layout->addWidget(m_start, 0, 0);
    m_layout->addWidget(m_end, 0, 1);
    m_layout->addWidget(m_startTime, 0, 2);
    m_layout->addWidget(m_endTime, 0, 3);
    m_layout->addWidget(m_presets, 1, 0, 1, 2);
    m_layout->addWidget(m_zoneCombo, 1, 2, 1, 2);
    setLayout(m_layout);
    rebuildPresets();
    updateTimeZoneCombo({"本地时区", "UTC", "Asia/Shanghai", "Asia/Tokyo", "Europe/London", "Europe/Berlin", "America/Los_Angeles", "America/New_York"});
    connect(m_presets, QOverload<int>::of(&ComboBox::currentIndexChanged), [this](int){
        applyPreset(m_presets->currentText());
    });
    connect(m_start, &QDateEdit::dateChanged, [this](const QDate&){ emit rangeChanged(m_start->dateTime(), m_end->dateTime()); });
    connect(m_end, &QDateEdit::dateChanged, [this](const QDate&){ emit rangeChanged(m_start->dateTime(), m_end->dateTime()); });
    connect(m_startTime, &QTimeEdit::timeChanged, [this](const QTime&){ if (m_includeTime) emit rangeChanged(range().first, range().second); });
    connect(m_endTime, &QTimeEdit::timeChanged, [this](const QTime&){ if (m_includeTime) emit rangeChanged(range().first, range().second); });
    setIncludeTime(false);
    setFirstDayOfWeek(m_firstDay);
}

DateRangePicker::~DateRangePicker() = default;

void DateRangePicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void DateRangePicker::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void DateRangePicker::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void DateRangePicker::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

void DateRangePicker::setPresetRangesEnabled(bool enabled)
{
    m_presetsEnabled = enabled;
    m_presets->setVisible(enabled);
    if (enabled && m_presets->count() == 0) rebuildPresets();
}

void DateRangePicker::setTimeZone(const QTimeZone& zone)
{
    m_zone = zone;
}

QPair<QDateTime, QDateTime> DateRangePicker::range() const
{
    if (!m_includeTime) {
        return { m_start->dateTime(), m_end->dateTime() };
    }
    const QDate sDate = m_start->date();
    const QDate eDate = m_end->date();
    const QTime sTime = m_startTime->time();
    const QTime eTime = m_endTime->time();
    return { QDateTime(sDate, sTime), QDateTime(eDate, eTime) };
}

void DateRangePicker::setRange(const QDateTime& start, const QDateTime& end)
{
    m_start->setDateTime(start);
    m_end->setDateTime(end);
    emit rangeChanged(m_start->dateTime(), m_end->dateTime());
}

void DateRangePicker::rebuildPresets()
{
    m_presets->clear();
    m_presets->addItems({"今天", "昨天", "近7天", "近30天", "本周", "本月", "上月", "本季度"});
}

void DateRangePicker::applyPreset(const QString& presetKey)
{
    const QDate today = QDate::currentDate();
    QDate startDate = today;
    QDate endDate = today;
    if (presetKey == "今天") {
        startDate = endDate = today;
    } else if (presetKey == "昨天") {
        startDate = endDate = today.addDays(-1);
    } else if (presetKey == "近7天") {
        startDate = today.addDays(-6);
        endDate = today;
    } else if (presetKey == "近30天") {
        startDate = today.addDays(-29);
        endDate = today;
    } else if (presetKey == "本周") {
        int delta = (static_cast<int>(m_firstDay) - static_cast<int>(today.dayOfWeek()));
        if (delta > 0) delta -= 7; // 调整到过去的同一周起始
        startDate = today.addDays(delta);
        endDate = startDate.addDays(6);
    } else if (presetKey == "本月") {
        startDate = QDate(today.year(), today.month(), 1);
        endDate = startDate.addMonths(1).addDays(-1);
    } else if (presetKey == "上月") {
        const QDate firstThis = QDate(today.year(), today.month(), 1);
        endDate = firstThis.addDays(-1);
        startDate = QDate(endDate.year(), endDate.month(), 1);
    } else if (presetKey == "本季度") {
        int q = (today.month() - 1) / 3; // 0-based
        int startMonth = q * 3 + 1;
        startDate = QDate(today.year(), startMonth, 1);
        endDate = QDate(today.year(), startMonth + 2, 1).addMonths(1).addDays(-1);
    }
    m_start->setDate(startDate);
    m_end->setDate(endDate);
    emit rangeChanged(m_start->dateTime(), m_end->dateTime());
}

void DateRangePicker::setIncludeTime(bool enabled)
{
    m_includeTime = enabled;
    m_startTime->setVisible(enabled);
    m_endTime->setVisible(enabled);
}

void DateRangePicker::setFirstDayOfWeek(Qt::DayOfWeek day)
{
    m_firstDay = day;
    if (m_start->calendarWidget()) m_start->calendarWidget()->setFirstDayOfWeek(day);
    if (m_end->calendarWidget()) m_end->calendarWidget()->setFirstDayOfWeek(day);
}

void DateRangePicker::setTimeZones(const QStringList& ids)
{
    updateTimeZoneCombo(ids);
}

void DateRangePicker::updateTimeZoneCombo(const QStringList& ids)
{
    m_zoneCombo->clear();
    m_zoneCombo->addItems(ids);
}

} // namespace QtAdvancedUI

