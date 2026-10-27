/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_DATERANGEPICKER_H
#define BASEWIDGETS_DATERANGEPICKER_H

#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>
#include <QDateTime>
#include <QTimeZone>

class QGridLayout;

namespace basewidgets {

class DatePicker;
class ComboBox;

/** 日期/可选时间范围编辑器。默认“今天”，时区按显示的本地墙钟时间解释。
 * 两端反序会规范为 start<=end；批量 setRange 只发出一次 rangeChanged。
 */
class ACSS_UI_EXPORT DateRangePicker : public StyledWidget
{
    Q_OBJECT
public:
    explicit DateRangePicker(QWidget* parent = nullptr);
    explicit DateRangePicker(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~DateRangePicker() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    void setPresetRangesEnabled(bool enabled);
    bool presetRangesEnabled() const { return m_presetsEnabled; }

    // Interpret the displayed dates/times as wall-clock values in this zone.
    void setTimeZone(const QTimeZone& zone);
    QTimeZone timeZone() const { return m_zone; }

    // 扩展：控制是否包含时间范围（时分秒）
    void setIncludeTime(bool enabled);
    bool includeTime() const { return m_includeTime; }

    // 扩展：设置 ISO 周起始（周一或其他）
    void setFirstDayOfWeek(Qt::DayOfWeek day);

    // 扩展：提供更丰富时区列表
    void setTimeZones(const QStringList& ids);

    /** 返回所选时区中的时间；includeTime=false 时两端时间均为 00:00。 */
    QPair<QDateTime, QDateTime> range() const;
    /** 有效输入先转换到当前时区，再更新控件；无效输入不改变状态。 */
    void setRange(const QDateTime& start, const QDateTime& end);

signals:
    void rangeChanged(const QDateTime& start, const QDateTime& end);

protected:
    void applyStylesheet();

private:
    void attachToManager();
    void rebuildPresets();
    void notifyRangeChanged(bool startEdited);
    void applyPreset(const QString& presetKey);
    void updateTimeZoneCombo(const QStringList& ids);

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    DatePicker* m_start = nullptr;
    DatePicker* m_end = nullptr;
    class TimePicker* m_startTime = nullptr;
    class TimePicker* m_endTime = nullptr;
    ComboBox* m_presets = nullptr;
    ComboBox* m_zoneCombo = nullptr;
    QGridLayout* m_layout = nullptr;
    bool m_presetsEnabled = true;
    QTimeZone m_zone = QTimeZone::systemTimeZone();
    bool m_includeTime = false;
    Qt::DayOfWeek m_firstDay = Qt::Monday;
};

} // namespace basewidgets

#endif // BASEWIDGETS_DATERANGEPICKER_H

