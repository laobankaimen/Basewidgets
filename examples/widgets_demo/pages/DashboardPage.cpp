#include "DashboardPage.h"
#include "../DemoPageIds.h"
#include <basewidgets/Containers/DashboardPanel.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Gauges/StorageDonutMeter.h>
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Gauges/DataManagementDashboards.h>
#include <basewidgets/Charts/RateHistoryPlot.h>
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/Label.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QAction>
#include <QPainter>
using namespace basewidgets;
namespace {
// 直接把矢量图标画到逻辑坐标，避免 QIcon::pixmap 在高 DPI 下重复放大并裁切。
class MetricIcon : public QWidget {
public:
    MetricIcon() { setFixedSize(36,36); }
    void setIcon(const QIcon& icon) { m_icon=icon; update(); }
protected:
    void paintEvent(QPaintEvent*) override { QPainter painter(this); m_icon.paint(&painter,rect().adjusted(2,2,-2,-2)); }
private:
    QIcon m_icon;
};
// 统计卡是公开容器与 QLabel 的组合；业务可换成实时模型，无需复制绘制代码。
DashboardPanel* makeMetric(const QString& name,const QString& value,const QString& detail,const QString& iconName)
{
    auto* panel=new DashboardPanel;
    auto* row=new QHBoxLayout;
    auto* icon=new MetricIcon; row->addWidget(icon);
    auto updateIcon=[icon,iconName] {
        const auto svg=StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/commands/"+iconName+".svg");
        icon->setIcon(svg);
    };
    updateIcon(); QObject::connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,icon,updateIcon);
    auto* title=new Label(name); title->setTextFormat(Qt::PlainText); title->setWordWrap(true); row->addWidget(title,1); panel->contentLayout()->addLayout(row);
    auto* number=new Label(value); number->setTextFormat(Qt::PlainText); number->setWordWrap(true);
    QFont font=number->font(); font.setPixelSize(28); font.setBold(true); number->setFont(font); panel->contentLayout()->addWidget(number);
    auto* caption=new Label(detail); caption->setTextFormat(Qt::PlainText); caption->setWordWrap(true); panel->contentLayout()->addWidget(caption);
    auto updateColor=[caption] { auto colors=caption->palette(); colors.setColor(QPalette::WindowText,ThemeTokens::instance()->primary()); caption->setPalette(colors); };
    updateColor(); QObject::connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,caption,updateColor);
    panel->setAccessibleName(name+"，"+value+"，"+detail); return panel;
}
}
DashboardPage::DashboardPage(QWidget* parent) : StyledWidget(parent)
{
    setObjectName("DashboardOverview");
    auto* root=new QHBoxLayout(this); root->setContentsMargins(10,10,10,10); root->setSpacing(14);
    auto* navigation=new NavigationList(this); navigation->setObjectName("DashboardNavigation"); navigation->setFixedWidth(140);
    navigation->setSpacing(6); navigation->setFrameShape(QFrame::NoFrame); navigation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    for(const auto& item:QList<QPair<QString,int>>{{"首页",DemoPage::Dashboard},{"数据目录",DemoPage::Records},{"水声分析",DemoPage::Acoustic},{"存储监控",DemoPage::Monitoring},{"测向 / 解调",DemoPage::Direction}})
        navigation->addButton(item.first,item.second);
    navigation->setCurrentButton(DemoPage::Dashboard); root->addWidget(navigation);
    // activated 支持 Enter，clicked 支持鼠标；导航动作不读写外部业务数据。
    connect(navigation,&NavigationList::buttonClicked,this,[this,navigation](int id,const QString&,int) { navigation->setCurrentButton(DemoPage::Dashboard); emit pageRequested(id); });
    connect(navigation,&QListWidget::itemActivated,this,[this,navigation](QListWidgetItem*) { const int id=navigation->getCurrentButton(); navigation->setCurrentButton(DemoPage::Dashboard); emit pageRequested(id); });
    auto* scroll=new ScrollArea(this); scroll->setObjectName("DashboardScroll"); scroll->setFrameShape(QFrame::NoFrame); root->addWidget(scroll,1);
    auto* content=new StyledWidget; auto* layout=new QVBoxLayout(content); layout->setContentsMargins(0,0,4,0); layout->setSpacing(14);
    auto* note=new Label("数据管理总览 · 演示快照 / 非设备实测"); note->setWordWrap(true); layout->addWidget(note);
    auto* cards=new QWidget; auto* metrics=new ResponsiveGridLayout(cards,220,4); metrics->setContentsMargins(0,0,0,0); metrics->setSpacing(12);
    metrics->addWidget(makeMetric("组态数据总量","128,640 个","今日新增 1,286 个","records"));
    metrics->addWidget(makeMetric("数据总容量","400.00 TB","今日新增 2.46 TB","open"));
    metrics->addWidget(makeMetric("采集程序在线","18 / 20","在线 18 · 离线 1 · 未启动 1","monitor"));
    metrics->addWidget(makeMetric("进行中任务","5 项","数据回放 3 · 数据复盘 2","bearing")); layout->addWidget(cards);
    auto* middle=new QWidget; auto* middleGrid=new ResponsiveGridLayout(middle,360,2); middleGrid->setContentsMargins(0,0,0,0); middleGrid->setSpacing(14);
    auto* history=new DashboardPanel("丢包率 · 最近 24 小时"); auto* plot=new RateHistoryPlot;
    plot->setObjectName("DashboardPacketLoss"); plot->setPlotTitle(QString()); plot->configure(25,86400,"丢包率 (%)",3601); plot->addSeries("丢包率");
    plot->legend->setVisible(false); plot->setMinimumSize(240,290); plot->setAutoScaleY(false);
    plot->setSeriesColor(plot->graph(0),1); plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,5));
    auto timeTicker=QSharedPointer<QCPAxisTickerTime>::create(); timeTicker->setTimeFormat("%h:%m"); timeTicker->setTickCount(5); plot->xAxis->setTicker(timeTicker); plot->xAxis->setLabel("时间");
    // 丢包率输入为百分数，不是 0..1 比率；x 为从观察窗口起点算起的秒数。
    const QVector<double> losses{1,1.2,1.4,1.6,1.7,1.7,1.3,1.45,1.45,1.9,2.2,2.5,2.9,3.1,3.5,3.8,3.75,4.2,4,3.4,3.1,2.9,2.8,2.2,1.6};
    for(int i=0;i<losses.size();++i)plot->appendValues(i*3600.,{losses[i]});
    plot->setResetRanges(QCPRange(0,86400),QCPRange(0,5)); plot->resetView();
    auto updatePlot=[plot] { QColor fill=ThemeTokens::instance()->seriesColor(1); fill.setAlpha(36); plot->graph(0)->setBrush(fill); plot->replot(); };
    updatePlot(); connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,plot,updatePlot);
    history->contentLayout()->addWidget(plot); middleGrid->addWidget(history);
    auto* storage=new DashboardPanel("存储资源概况"); auto* clusters=new QWidget; auto* clusterGrid=new ResponsiveGridLayout(clusters,190,2); clusterGrid->setContentsMargins(0,0,0,0);
    for(const auto& name:{QString("实时存储集群"),QString("离线存储集群")}) {
        auto* cluster=new QWidget; auto* column=new QVBoxLayout(cluster); column->setContentsMargins(0,0,0,0);
        auto* label=new Label(name); label->setWordWrap(true); column->addWidget(label);
        auto* meter=new StorageDonutMeter; meter->setObjectName(name=="实时存储集群"?"DashboardRealtimeStorage":"DashboardOfflineStorage");
        // 参考稿使用 SI TB/PB，单位切换与真实 bytes 对应，200 TB / 2 PB = 10%。
        meter->setUnitSystem(StorageDonutMeter::UnitSystem::SI); meter->setCapacity(200000000000000ULL,2000000000000000ULL); column->addWidget(meter);
        auto* bandwidth=new Label(name=="实时存储集群"?"均值带宽 86.4 MB/s\n峰值带宽 128.6 MB/s":"均值带宽 42.6 MB/s\n峰值带宽 76.8 MB/s"); bandwidth->setWordWrap(true); column->addWidget(bandwidth); clusterGrid->addWidget(cluster);
    }
    storage->contentLayout()->addWidget(clusters); middleGrid->addWidget(storage); layout->addWidget(middle);
    auto* bottom=new QWidget; auto* bottomGrid=new ResponsiveGridLayout(bottom,280,3); bottomGrid->setContentsMargins(0,0,0,0); bottomGrid->setSpacing(14);
    auto* types=new DashboardPanel("数据类型分布 · 400 TB");
    // 分类占比的分母是数据总量，不是磁盘容量；三类合计 100% 不表示磁盘已满。
    for(const auto& item:QList<QPair<QString,int>>{{"原始录音 · 240 TB",60},{"处理结果 · 100 TB",25},{"索引 · 60 TB",15}}) {
        auto* label=new Label(item.first+QString(" · %1%").arg(item.second)); label->setWordWrap(true); types->contentLayout()->addWidget(label);
        auto* share=new ProgressBar; share->setRange(0,100); share->setValue(item.second); share->setTextVisible(false); share->setFixedHeight(8); share->setAccessibleName(item.first); types->contentLayout()->addWidget(share);
    }
    auto* browse=new PushButton("查看分类记录"); types->contentLayout()->addWidget(browse); types->contentLayout()->addStretch(); bottomGrid->addWidget(types);
    auto* tasks=new TaskQueueDashboard; tasks->setQueues({{"replay","数据回放",2,3,24,0},{"review","数据复盘",1,2,12,1}}); bottomGrid->addWidget(tasks);
    auto* activity=new DashboardPanel("最近操作"); auto* entries=new Label("09:42  索引构建完成\n09:30  导入东海站录音\n09:18  启动回放任务\n09:05  归档数据校验通过"); entries->setWordWrap(true); activity->contentLayout()->addWidget(entries);
    auto* all=new PushButton("查看数据记录"); all->setObjectName("DashboardOpenRecords"); activity->contentLayout()->addWidget(all); activity->contentLayout()->addStretch(); bottomGrid->addWidget(activity);
    connect(all,&QPushButton::clicked,this,[this] { emit pageRequested(DemoPage::Records); });
    connect(browse,&QPushButton::clicked,this,[this] { emit pageRequested(DemoPage::Records); });
    connect(tasks,&TaskQueueDashboard::stateActivated,this,[this](const QString&,TaskQueueDashboard::State) { emit pageRequested(DemoPage::Monitoring); });
    connect(plot,&StyledPlot::prepareContextMenu,this,[this](QMenu* menu,const QPointF&) {
        connect(menu->addAction("查看数据监控"),&QAction::triggered,this,[this] { emit pageRequested(DemoPage::Monitoring); });
    });
    layout->addWidget(bottom); layout->addStretch(); scroll->setWidget(content);
}
