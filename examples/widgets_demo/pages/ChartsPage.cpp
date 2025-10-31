#include "ChartsPage.h"

#include <QtAdvancedUI/Containers/GroupBox.h>
#include <QtAdvancedUI/Containers/ScrollArea.h>
#include <QtAdvancedUI/Inputs/Label.h>

#include <QtAdvancedUI/Charts/LineChart.h>
#include <QtAdvancedUI/Charts/HistogramChart.h>
#include <QtAdvancedUI/Charts/JourneyChart.h>
#include <QtAdvancedUI/Charts/WaterfallChart.h>
#include <QtAdvancedUI/Charts/PieChart.h>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QRandomGenerator>
#include <QtMath>

using namespace QtAdvancedUI;

ChartsPage::ChartsPage(QWidget* parent)
    : StyledWidget(parent)
{
    // 滚动内容外层
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    auto* scrollArea = new ScrollArea(this);
    auto* content = new StyledWidget(scrollArea);
    auto* grid = new QGridLayout(content);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    // 折线图（实时追加）
    auto* lineChart = new LineChart(content);
    lineChart->setMinimumHeight(220);
    lineChart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lineChart->setMaxPointCount(600);
    auto* gbLine = new GroupBox("LineChart（实时数据）：", content);
    auto* vLine = new QVBoxLayout(gbLine); vLine->addWidget(lineChart); gbLine->setLayout(vLine);
    // 模拟数据源
    QTimer* lineTimer = new QTimer(this);
    lineTimer->setInterval(60);
    QObject::connect(lineTimer, &QTimer::timeout, [lineChart](){
        static double x = 0.0;
        double v = 0.5 * qSin(x / 10.0) + 0.5 * QRandomGenerator::global()->bounded(0.8);
        lineChart->appendPoint(QPointF(x, v));
        x += 1.0;
    });
    lineTimer->start();

    // 直方图（动态采样）
    auto* histogram = new HistogramChart(content);
    histogram->setMinimumHeight(220);
    histogram->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    histogram->setBinCount(20);
    auto* gbHist = new GroupBox("Histogram（随机采样）：", content);
    auto* vHist = new QVBoxLayout(gbHist); vHist->addWidget(histogram); gbHist->setLayout(vHist);
    QTimer* histTimer = new QTimer(this);
    histTimer->setInterval(100);
    QObject::connect(histTimer, &QTimer::timeout, [histogram](){
        QVector<double> samples;
        samples.reserve(50);
        for (int i = 0; i < 50; ++i) {
            // 生成混合分布样本
            double a = QRandomGenerator::global()->bounded(1.0);
            double b = QRandomGenerator::global()->bounded(1.0);
            samples.append(0.6 * a + 0.4 * b * b);
        }
        histogram->addSamples(samples);
    });
    histTimer->start();

    // 行进路径（曲线+散点）
    auto* journey = new JourneyChart(content);
    journey->setMinimumHeight(220);
    journey->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    journey->setMaxPointCount(1000);
    auto* gbJourney = new GroupBox("Journey（轨迹）：", content);
    auto* vJourney = new QVBoxLayout(gbJourney); vJourney->addWidget(journey); gbJourney->setLayout(vJourney);
    QTimer* journeyTimer = new QTimer(this);
    journeyTimer->setInterval(80);
    QObject::connect(journeyTimer, &QTimer::timeout, [journey](){
        static double t = 0.0;
        t += 0.06;
        double x = t * 0.8 + 0.5 * qSin(t);
        double y = 0.5 * qCos(t) + 0.3 * QRandomGenerator::global()->bounded(1.0);
        journey->appendPoints({ QPointF(x, y) });
    });
    journeyTimer->start();

    // 瀑布图（类财务汇总）
    auto* waterfall = new WaterfallChart(content);
    waterfall->setMinimumHeight(220);
    waterfall->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    waterfall->setCategories({"起始", "收入", "成本", "税费", "其他", "结余"});
    waterfall->setValues({ 1000, +400, -320, -150, +80, 0 });
    auto* gbWater = new GroupBox("Waterfall（累计变化）：", content);
    auto* vWater = new QVBoxLayout(gbWater); vWater->addWidget(waterfall); gbWater->setLayout(vWater);

    // 饼图（极坐标）
    auto* pie = new PieChart(content);
    pie->setMinimumHeight(300);
    pie->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pie->setHoleFraction(0.15);
    pie->setData({ 40, 25, 20, 15 }, {"A", "B", "C", "D"});
    auto* gbPie = new GroupBox("Pie（占比）：", content);
    auto* vPie = new QVBoxLayout(gbPie); vPie->addWidget(pie); gbPie->setLayout(vPie);

    int row = 0;
    grid->addWidget(gbLine,    row, 0);
    grid->addWidget(gbHist,    row, 1); ++row;
    grid->addWidget(gbJourney, row, 0);
    grid->addWidget(gbWater,   row, 1); ++row;
    grid->addWidget(gbPie,     row, 0, 1, 2);

    content->setLayout(grid);
    //scrollArea->setWidget(content);
    outerLayout->addWidget(content);
    outerLayout->addStretch();
    setLayout(outerLayout);
}