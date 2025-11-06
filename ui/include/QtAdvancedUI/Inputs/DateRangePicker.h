#ifndef QTADVANCEDUI_DATERANGEPICKER_H
#define QTADVANCEDUI_DATERANGEPICKER_H

#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QDateTime>
#include <QTimeZone>

class QGridLayout;

namespace QtAdvancedUI {

class DatePicker;
class ComboBox;

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

    void setTimeZone(const QTimeZone& zone);
    QTimeZone timeZone() const { return m_zone; }

    // 扩展：控制是否包含时间范围（时分秒）
    void setIncludeTime(bool enabled);
    bool includeTime() const { return m_includeTime; }

    // 扩展：设置 ISO 周起始（周一或其他）
    void setFirstDayOfWeek(Qt::DayOfWeek day);

    // 扩展：提供更丰富时区列表
    void setTimeZones(const QStringList& ids);

    QPair<QDateTime, QDateTime> range() const;
    void setRange(const QDateTime& start, const QDateTime& end);

signals:
    void rangeChanged(const QDateTime& start, const QDateTime& end);

protected:
    void applyStylesheet();

private:
    void attachToManager();
    void rebuildPresets();
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

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_DATERANGEPICKER_H