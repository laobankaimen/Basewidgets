#include "DataManagementPage.h"
#include <basewidgets/Gauges/StorageCapacityMeter.h>
#include <basewidgets/Gauges/DataManagementDashboards.h>
#include <basewidgets/Gauges/TransferRateMeter.h>
#include <basewidgets/Charts/RateHistoryPlot.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <QDoubleSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>
using namespace basewidgets;
DataManagementPage::DataManagementPage(QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(10,10,10,10);
    auto* heading=new Label("模拟数据管理监控 · 容量使用 IEC 字节单位，网速 Mbps 与 MiB/s 明确区分",this); heading->setWordWrap(true); outer->addWidget(heading);
    auto* row=new QHBoxLayout; m_run=new CheckBox("继续模拟",this); m_run->setObjectName("DataManagementRun"); row->addWidget(m_run);
    auto* resetCounter=new PushButton("模拟计数器重置",this); row->addWidget(resetCounter);
    auto* resetViews=new PushButton("恢复所有视图",this); row->addWidget(resetViews); row->addStretch(); outer->addLayout(row);
    auto* unitRow=new QHBoxLayout; auto* unit=new ComboBox(this); unit->addItems({"Mbps（bit/s）","MiB/s（byte/s）"});
    auto* capacity=new QDoubleSpinBox(this); capacity->setRange(0,1000000); capacity->setValue(1000); capacity->setSuffix(" Mbps");
    unitRow->addWidget(new Label("速率表显示",this)); unitRow->addWidget(unit); unitRow->addWidget(new Label("单向链路容量",this)); unitRow->addWidget(capacity); unitRow->addStretch(); outer->addLayout(unitRow);
    auto* output=new Label("色块可点击以筛选目录；速率曲线可右键扩展、框选和双击恢复。",this); output->setWordWrap(true); outer->addWidget(output);
    auto* scroll=new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame); outer->addWidget(scroll,1);
    auto* content=new StyledWidget; auto* layout=new QVBoxLayout(content); layout->setContentsMargins(0,0,0,0);
    auto* panels=new QWidget; auto* grid=new ResponsiveGridLayout(panels,280,2); grid->setContentsMargins(0,0,0,0);
    m_capacity=new StorageCapacityMeter; m_capacity->setObjectName("DataManagementCapacity");
    const quint64 gib=quint64(1024)*1024*1024;
    m_capacity->setCapacity(1024*gib,{{"原始录音",520*gib,{}},{"处理结果",120*gib,{}},{"索引/缓存",96*gib,{}}}); grid->addWidget(m_capacity);
    m_rate=new TransferRateMeter; m_rate->setObjectName("DataManagementRate"); m_rate->setLinkCapacity(1e9); grid->addWidget(m_rate); layout->addWidget(panels);
    // 队列统计互不相加；同一文件在导入、索引和归档阶段可以对应不同业务任务。
    auto* businessPanels=new QWidget;
    auto* businessGrid=new ResponsiveGridLayout(businessPanels,280,2); businessGrid->setContentsMargins(0,0,0,0);
    m_tasks=new TaskQueueDashboard; m_tasks->setObjectName("DataManagementTasks");
    m_quality=new DataQualityDashboard; m_quality->setObjectName("DataManagementQuality");
    businessGrid->addWidget(m_tasks); businessGrid->addWidget(m_quality); layout->addWidget(businessPanels);
    connect(m_tasks,&TaskQueueDashboard::stateActivated,output,[output](const QString& id,TaskQueueDashboard::State state) {
        const QStringList states{"排队","运行","成功","失败"};
        output->setText(QString("业务筛选：queueId=%1，状态=%2（可连接 AdvancedTable 的源模型筛选）").arg(id,states[int(state)]));
    });
    connect(m_quality,&DataQualityDashboard::categoryActivated,output,[output](DataQualityDashboard::Category category) {
        const QStringList categories{"有效","缺失","损坏","重复","未核验"};
        output->setText("业务筛选："+categories[int(category)]+"记录。核验覆盖率与通过率使用不同分母。");
    });
    connect(m_tasks,&TaskQueueDashboard::prepareContextMenu,output,[output](QMenu* menu,const QString& id,TaskQueueDashboard::State state) {
        auto* action=menu->addAction("业务扩展：打开此队列");
        connect(action,&QAction::triggered,output,[output,id,state] { output->setText(QString("打开队列 %1，状态枚举 %2").arg(id).arg(int(state))); });
    });
    connect(m_quality,&DataQualityDashboard::prepareContextMenu,output,[output](QMenu* menu,DataQualityDashboard::Category category) {
        auto* action=menu->addAction("业务扩展：查看核验记录");
        connect(action,&QAction::triggered,output,[output,category] { output->setText(QString("核验结果类别 %1：此处连接业务记录列表。").arg(int(category))); });
    });
    auto makePlot=[&](const QString& title,const QString& name,const QString& unit) {
        auto* plot=new RateHistoryPlot; plot->setObjectName(name); plot->setPlotTitle(title); plot->configure(120,60,unit,5); plot->setMinimumHeight(280); layout->addWidget(plot); return plot;
    };
    m_network=makePlot("网络吞吐历程（区间平均）","DataManagementNetwork","吞吐 (MiB/s)"); m_network->addSeries("接收"); m_network->addSeries("发送");
    m_disk=makePlot("存储 I/O 历程","DataManagementDisk","I/O (MiB/s)"); m_disk->addSeries("写入"); m_disk->addSeries("读取");
    m_indexing=makePlot("索引处理历程","DataManagementIndexing","文件数 / s"); m_indexing->addSeries("已处理文件/s");
    scroll->setWidget(content);
    connect(m_rate,&TransferRateMeter::ratesUpdated,this,[this,output](double time,double rx,double tx) {
        m_network->appendValues(time,{rx/1048576.,tx/1048576.});
        if(output->property("awaitingRate").toBool()) {
            output->setProperty("awaitingRate",false);
            output->setText("速率采样已恢复。点击容量分段、任务状态或核验分类可联动业务筛选；右键可扩展操作。");
        }
    });
    connect(m_rate,&TransferRateMeter::baselineReset,this,[this,output](const QString& reason) {
        output->setProperty("awaitingRate",true);
        const double missing=std::numeric_limits<double>::quiet_NaN(); m_network->appendValues(m_time,{missing,missing}); output->setText(reason+"：等待下一帧，不制造负速率或尖峰。");
    });
    connect(unit,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int index) { m_rate->setDisplayUnit(index?TransferRateMeter::DisplayUnit::BinaryMiBPerSecond:TransferRateMeter::DisplayUnit::NetworkMbps); });
    connect(capacity,QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[this](double value) { m_rate->setLinkCapacity(value*1e6); });
    connect(m_capacity,&StorageCapacityMeter::segmentSelected,output,[output](int index,const QString& name,quint64 bytes) { output->setText(QString("容量分段 index=%1 · %2 · %3 B（可关联文件列表筛选）").arg(index).arg(name).arg(bytes)); });
    connect(resetCounter,&QPushButton::clicked,this,[this] { ++m_time; m_received=m_sent=0; m_rate->addCounters(m_time,0,0); });
    const QList<StyledPlot*> plots{m_network,m_disk,m_indexing};
    connect(resetViews,&QPushButton::clicked,this,[plots] { for(auto* plot:plots)plot->resetView(); });
    for(auto* plot:plots) {
        connect(plot,&StyledPlot::selectionMade,output,[output,plot](double a,double b,double c,double d) {
            output->setText(QString("%1：时间 %2–%3 s，值 %4–%5").arg(plot->plotTitle()).arg(a,0,'f',2).arg(b,0,'f',2).arg(c,0,'f',2).arg(d,0,'f',2));
        });
        connect(plot,&StyledPlot::prepareContextMenu,output,[plot,output](QMenu* menu,const QPointF& at) {
            auto* action=menu->addAction("业务扩展：查看此时刻任务"); connect(action,&QAction::triggered,output,[plot,output,at] { output->setText(QString("%1 · 查询 %2 s 的业务任务").arg(plot->plotTitle()).arg(at.x(),0,'f',2)); });
        });
    }
    m_rate->addCounters(0,0,0); for(int i=0;i<60;++i)advance();
    m_timer=new QTimer(this); m_timer->setObjectName("DataManagementTimer"); m_timer->setInterval(250);
    connect(m_timer,&QTimer::timeout,this,&DataManagementPage::advance);
    connect(m_run,&QCheckBox::toggled,this,[this](bool on) { if(on&&isVisible())m_timer->start(); else m_timer->stop(); });
}
void DataManagementPage::advance()
{
    ++m_time;
    const quint64 done=quint64(m_time)%120;
    m_tasks->setQueues({{"import","录音导入",120-done,4,400+done,3},
                       {"index","索引构建",160-done,8,350+done,2},
                       {"archive","归档迁移",80,2,260+done,1}});
    // 数据质量类别互斥；未核验部分由控件计算。不把尚未核验的数据当作有效。
    DataQualitySnapshot quality; quality.total=10000; quality.valid=7200+done*10;
    quality.missing=18; quality.corrupt=7; quality.duplicate=125;
    quality.observedAt=QDateTime::fromSecsSinceEpoch(1793062800+qint64(m_time),Qt::UTC); // 固定模拟时钟，非本机采样时间
    m_quality->setSnapshot(quality);
    m_received+=quint64((40+18*std::sin(m_time*.12))*1048576); m_sent+=quint64((12+6*std::cos(m_time*.17))*1048576);
    m_rate->addCounters(m_time,m_received,m_sent);
    m_disk->appendValues(m_time,{22+9*std::sin(m_time*.13),14+5*std::cos(m_time*.19)});
    m_indexing->appendValues(m_time,{80+30*std::sin(m_time*.09)});
}
void DataManagementPage::showEvent(QShowEvent* event) { StyledWidget::showEvent(event); if(m_run->isChecked())m_timer->start(); }
void DataManagementPage::hideEvent(QHideEvent* event) { m_timer->stop(); StyledWidget::hideEvent(event); }
