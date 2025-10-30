#pragma once

#include <QtAdvancedUI/Charts/ChartBase.h>
#include <QVector>
#include <QStringList>


namespace QtAdvancedUI {

class ACSS_UI_EXPORT WaterfallChart : public ChartBase {
    Q_OBJECT
public:
    explicit WaterfallChart(QWidget* parent = nullptr);

    void setData(const QVector<double>& deltas, const QStringList& labels = {});
    // Compatibility wrappers for examples
    void setCategories(const QStringList& labels);
    void setValues(const QVector<double>& deltas);
    void clear();

protected:
    void updateCursorHint(const QPoint& pos) override;

private:
    void rebuild(const QVector<double>& deltas, const QStringList& labels);

    QCPBars* m_baseBars = nullptr;   // 不可见基座栏（用于堆叠定位）
    QCPBars* m_deltaBars = nullptr;  // 显示增量栏
    QVector<double> m_keys;          // 1..N
    QVector<double> m_bases;         // 前一累计值
    QVector<double> m_deltas;        // 当前增量
    QStringList m_labels;            // 类目标签
};

} // namespace QtAdvancedUI