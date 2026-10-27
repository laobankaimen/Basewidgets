/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/DateRangePicker.h>
#include <basewidgets/Inputs/DatePicker.h>
#include <basewidgets/Inputs/TimePicker.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QGridLayout>
#include <QCalendarWidget>
#include <QSignalBlocker>

using namespace acss;

namespace basewidgets {

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

    m_layout->setContentsMargins(0,0,0,0);
    m_layout->addWidget(m_start,0,0); m_layout->addWidget(m_end,0,1);
    m_layout->addWidget(m_startTime,1,0); m_layout->addWidget(m_endTime,1,1);
    m_layout->addWidget(m_presets,2,0); m_layout->addWidget(m_zoneCombo,2,1);

    rebuildPresets();
    // 默认时区列表（常用 + 示例）
    updateTimeZoneCombo({"本地时区", "UTC", "Asia/Shanghai", "Asia/Tokyo", "Europe/London", "Europe/Berlin", "America/Los_Angeles", "America/New_York"});
    connect(m_presets, QOverload<int>::of(&ComboBox::currentIndexChanged), [this](int){
        applyPreset(m_presets->currentText());
    });
    connect(m_start,&QDateEdit::dateChanged,this,[this] { notifyRangeChanged(true); });
    connect(m_end,&QDateEdit::dateChanged,this,[this] { notifyRangeChanged(false); });
    connect(m_startTime,&QTimeEdit::timeChanged,this,[this] { if (m_includeTime) notifyRangeChanged(true); });
    connect(m_endTime,&QTimeEdit::timeChanged,this,[this] { if (m_includeTime) notifyRangeChanged(false); });
    connect(m_zoneCombo,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int index) {
        setTimeZone(QTimeZone(m_zoneCombo->itemData(index).toByteArray()));
    });
    setIncludeTime(false);
    setFirstDayOfWeek(m_firstDay);
    applyPreset("今天");
}

DateRangePicker::DateRangePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : DateRangePicker(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
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
    if (!zone.isValid() || m_zone==zone) return;
    m_zone = zone;
    const QSignalBlocker block(m_zoneCombo);
    int index = m_zoneCombo->findData(zone.id());
    if (index<0) { m_zoneCombo->addItem(QString::fromUtf8(zone.id()),zone.id()); index=m_zoneCombo->count()-1; }
    m_zoneCombo->setCurrentIndex(index);
    const auto selected = range();
    emit rangeChanged(selected.first,selected.second);
}

QPair<QDateTime,QDateTime> DateRangePicker::range() const
{
    return {QDateTime(m_start->date(),m_includeTime ? m_startTime->time() : QTime(0,0),m_zone),
            QDateTime(m_end->date(),m_includeTime ? m_endTime->time() : QTime(0,0),m_zone)};
}

void DateRangePicker::setRange(const QDateTime& start, const QDateTime& end)
{
    if (!start.isValid() || !end.isValid()) return;
    QDateTime first = start.toTimeZone(m_zone), last = end.toTimeZone(m_zone);
    if (last<first) qSwap(first,last);
    const QSignalBlocker a(m_start), b(m_end), c(m_startTime), d(m_endTime), preset(m_presets);
    m_start->setDate(first.date()); m_end->setDate(last.date());
    m_startTime->setTime(first.time()); m_endTime->setTime(last.time());
    m_presets->setCurrentText("自定义");
    const auto selected = range(); emit rangeChanged(selected.first,selected.second);
}

void DateRangePicker::notifyRangeChanged(bool startEdited)
{
    const auto selected = range();
    if (selected.second<selected.first) {
        const auto endpoint = startEdited ? selected.first : selected.second;
        setRange(endpoint,endpoint);
    } else {
        const QSignalBlocker preset(m_presets); m_presets->setCurrentText("自定义");
        emit rangeChanged(selected.first,selected.second);
    }
}

void DateRangePicker::rebuildPresets()
{
    m_presets->clear();
    m_presets->addItems({"今天", "昨天", "近7天", "近30天", "本周", "本月", "上月", "本季度", "自定义"});
}

void DateRangePicker::applyPreset(const QString& presetKey)
{
    if (presetKey=="自定义") return;
    const QDate today = QDateTime::currentDateTimeUtc().toTimeZone(m_zone).date();
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
    setRange(QDateTime(startDate,QTime(0,0),m_zone),QDateTime(endDate,QTime(0,0),m_zone));
    const QSignalBlocker preset(m_presets); m_presets->setCurrentText(presetKey);
}

void DateRangePicker::setIncludeTime(bool enabled)
{
    const bool changed = m_includeTime != enabled;
    m_includeTime = enabled;
    m_startTime->setVisible(enabled);
    m_endTime->setVisible(enabled);
    if (changed) notifyRangeChanged(true);
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
    const QSignalBlocker block(m_zoneCombo);
    m_zoneCombo->clear();
    for (const QString& id : ids) {
        const QTimeZone zone = id=="本地时区" ? QTimeZone::systemTimeZone() : QTimeZone(id.toUtf8());
        if (zone.isValid()) m_zoneCombo->addItem(id,zone.id());
    }
    int index = m_zoneCombo->findData(m_zone.id());
    if (index<0) { m_zoneCombo->addItem(QString::fromUtf8(m_zone.id()),m_zone.id()); index=m_zoneCombo->count()-1; }
    m_zoneCombo->setCurrentIndex(index);
}

} // namespace basewidgets

