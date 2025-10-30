#pragma once

#include <QtAdvancedUI/Charts/ChartBase.h>
#include <QVector>
#include <QStringList>
#include <QColor>


namespace QtAdvancedUI {

class ACSS_UI_EXPORT PieChart : public ChartBase {
    Q_OBJECT
public:
    explicit PieChart(QWidget* parent = nullptr);

    // 设置数据：values 为扇形数值，labels 可选，colors 可选
    void setData(const QVector<double>& values,
                 const QStringList& labels = {},
                 const QVector<QColor>& colors = {});

    void clear();

    // 甜甜圈中空比例（0 表示实心，默认 0）
    void setHoleFraction(double f) { m_hole = qBound(0.0, f, 0.9); rebuild(); }

protected:
    void updateCursorHint(const QPoint& pos) override;

private:
    struct Slice {
        QCPPolarGraph* graph = nullptr;
        double startDeg = 0.0;
        double endDeg = 0.0;
        double value = 0.0;
        QString label;
        QColor color;
    };

    void rebuild();
    void buildSliceGeometry(Slice& s);
    QColor defaultColor(int index) const;

    // 极坐标轴
    QCPPolarAxisAngular* m_ang = nullptr;
    QCPPolarAxisRadial*  m_rad = nullptr;

    QVector<double>   m_values;
    QStringList       m_labels;
    QVector<QColor>   m_colors;
    QVector<Slice>    m_slices;
    double            m_sum = 0.0;
    double            m_hole = 0.0; // 0..0.9
};

} // namespace QtAdvancedUI