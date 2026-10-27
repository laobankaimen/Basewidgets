// 完整接入示例：高 DPI 设置→QApplication→初始化主题→组合主窗口。
// --smoke-test 遍历所有页面并退出；普通启动展示交互，--verify 开启可选回归验证。
#include <QApplication>
#include <QCloseEvent>
#include <QSettings>
#include <QScreen>
#include <QFont>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/Containers/MenuBar.h>
#include <basewidgets/Containers/ToolBar.h>
#include <basewidgets/Containers/StatusBar.h>
#include <basewidgets/Containers/TabWidget.h>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/Frame.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Inputs/MultiSelectComboBox.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/SpinBox.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/RadioButton.h>
#include <basewidgets/Inputs/Slider.h>
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Inputs/BusyIndicator.h>
#include <basewidgets/Inputs/ToggleSwitch.h>
#include <basewidgets/Inputs/TextEdit.h>
#include <basewidgets/Inputs/PlainTextEdit.h>
#include <basewidgets/Inputs/DatePicker.h>
#include <basewidgets/Inputs/TimePicker.h>
#include <basewidgets/Inputs/DateTimePicker.h>
#include <basewidgets/Inputs/DateRangePicker.h>
#include <basewidgets/Views/ListView.h>
#include <basewidgets/Views/TreeView.h>
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Views/TreeTable.h>
#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Views/TreeWidget.h>
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Gauges/CircularGauge.h>
#include <basewidgets/Gauges/BasicGauge.h>
#include <basewidgets/Gauges/SpeedGauge.h>
#include <basewidgets/Gauges/CompassGauge.h>
#include <basewidgets/Gauges/AttitudeMeter.h>
#include <QTimer>
#include <basewidgets/Charts/QCustomplot.h>
#include <basewidgets/Charts/StyledPlot.h>
#include <basewidgets/Charts/QCPLineChart.h>
#include <basewidgets/Charts/QCPBarChart.h>
#include <basewidgets/Charts/QCPWaterfallChart.h>
#include <QMenu>
#include <QtMath>
#include "pages/InputsPage.h"
#include "pages/DialogsPage.h"
#include "pages/PagedTablePage.h"
#include "pages/TextPage.h"
#include "pages/ViewsPage.h"

// 示例数据模型
#include <QStandardItemModel>
#include <QHeaderView>

using namespace basewidgets;

class DemoMainWindow : public FramelessWidgetWindow {
protected:
    void closeEvent(QCloseEvent* ev) override
    {
        if (!DialogUtils::askQuestion(this, "请确认", "是否关闭软件？")) {
            ev->ignore();
            return;
        }
        FramelessWidgetWindow::closeEvent(ev);
    }
};

// 创建视图和图表展示页面
StyledWidget* createViewsChartsShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scrollArea = new ScrollArea(page);
    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    // 图表控件组（折线图、柱状图与瀑布图分离展示）
    auto* chartGroup = new GroupBox("", content);
    auto* chartLayout = new QGridLayout(chartGroup);

    // 折线图 Plot（独立，使用通用基类封装交互与主题）
    auto* plotLine = new StyledPlot(chartGroup);
    plotLine->setMinimumSize(320, 300);

    // 折线图示例（多条折线，支持多纵轴与显示/隐藏）
    QVector<double> xs(200);
    for (int i = 0; i < xs.size(); ++i) xs[i] = i / 20.0;

    auto* line1 = new QCPLineChart(plotLine->xAxis, plotLine->yAxis);
    auto* line2 = new QCPLineChart(plotLine->xAxis, plotLine->yAxis);
    auto* line3 = new QCPLineChart(plotLine->xAxis, plotLine->yAxis);

    {
        QVector<double> y1(xs.size()), y2(xs.size()), y3(xs.size());
        for (int i = 0; i < xs.size(); ++i) {
            y1[i] = qSin(xs[i]);
            y2[i] = qCos(xs[i]);
            y3[i] = qSin(xs[i]) + 0.5 * qCos(xs[i]);
        }
        line1->setData(xs, y1);
        line2->setData(xs, y2);
        line3->setData(xs, y3);
        line1->setName("sin(x)");
        line2->setName("cos(x)");
        line3->setName("sin(x)+0.5cos(x)");

        line1->setOrientation(Qt::Vertical, plotLine);
        line2->setOrientation(Qt::Vertical, plotLine);
        line3->setOrientation(Qt::Vertical, plotLine);

        // 初始右侧纵轴给第一条线，便于演示多轴
        if (auto* yRight = line1->addValueAxisRight(plotLine)) {
            yRight->setRange(-1.2, 1.2);
            line1->setValueAxisByPtr(yRight);
        }

        // 每条折线的上下文菜单：切换方向、新增左右纵轴并绑定
        auto makeLineMenu = [plotLine](QCPLineChart* ln){
            auto* m = new QMenu(plotLine);
            m->addAction("切换横向/竖向", [ln, plotLine]{
                static Qt::Orientation o = Qt::Vertical;
                o = (o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical);
                ln->setOrientation(o, plotLine);
                plotLine->replot();
            });
            m->addAction("新增左侧纵轴并绑定", [ln, plotLine]{
                if (auto* ax = ln->addValueAxisLeft(plotLine)) {
                    ax->setRange(-1.2, 1.2);
                    ln->setValueAxisByPtr(ax);
                    plotLine->replot();
                }
            });
            m->addAction("新增右侧纵轴并绑定", [ln, plotLine]{
                if (auto* ax = ln->addValueAxisRight(plotLine)) {
                    ax->setRange(-1.2, 1.2);
                    ln->setValueAxisByPtr(ax);
                    plotLine->replot();
                }
            });
            return m;
        };
        auto* menu1 = makeLineMenu(line1);
        auto* menu2 = makeLineMenu(line2);
        auto* menu3 = makeLineMenu(line3);
        line1->setContextMenu(menu1); line1->installContextMenu(plotLine);
        line2->setContextMenu(menu2); line2->installContextMenu(plotLine);
        line3->setContextMenu(menu3); line3->installContextMenu(plotLine);
    }

    // 瀑布图 Plot（独立，使用通用基类封装交互与主题）
    auto* plotMap = new StyledPlot(chartGroup);
    plotMap->setMinimumSize(320, 300);

    // 瀑布图（基于 QCPColorMap，绑定到 plotMap）
    auto* cmap = new QCPWaterfallChart(plotMap->xAxis, plotMap->yAxis);
    {
        const int nx = 120, ny = 80;
        cmap->data()->setSize(nx, ny);
        cmap->data()->setRange(QCPRange(0, 6.0), QCPRange(0, 4.0));
        for (int x = 0; x < nx; ++x) {
            for (int y = 0; y < ny; ++y) {
                double xv = x * 6.0 / nx;
                double yv = y * 4.0 / ny;
                double v = qSin(xv) * qCos(yv) + 0.3 * qSin(3*xv) * qSin(2*yv);
                cmap->data()->setCell(x, y, v);
            }
        }
        // 初始渐变：与主题一致（secondaryLightColor -> primaryColor）
        {
            QCPColorGradient grad;
            auto* mgr = StyleManagerAdapter::instance();
            QColor low = mgr->themeColor("secondaryLightColor");
            QColor high = mgr->themeColor("primaryColor");
            if (!low.isValid()) low = QColor(120,120,255);
            if (!high.isValid()) high = QColor(255,80,80);
            grad.setColorStopAt(0.0, low);
            grad.setColorStopAt(1.0, high);
            cmap->setGradient(grad);
        }
        cmap->rescaleDataRange(true);
        // 设置图例名称
        cmap->setName("Waterfall");

        auto* cmenu = new QMenu(plotMap);
        cmenu->addAction("重置颜色范围", [cmap]{
            cmap->rescaleDataRange(true);
            if (cmap->parentPlot()) cmap->parentPlot()->replot();
        });
        cmap->setContextMenu(cmenu);
        cmap->installContextMenu(plotMap);
    }
    // 双击还原：分别重置两个图的范围
    QObject::connect(plotLine, &StyledPlot::mouseDoubleClick, [plotLine](QMouseEvent* /*ev*/){
        plotLine->xAxis->setRange(0, 10.0);
        plotLine->yAxis->setRange(-1.2, 1.2);
        plotLine->yAxis2->setRange(-1.2, 1.2);
        plotLine->replot();
    });
    QObject::connect(plotMap, &StyledPlot::mouseDoubleClick, [plotMap, cmap](QMouseEvent* /*ev*/){
        cmap->rescaleDataRange(true);
        plotMap->xAxis->setRange(0, 6.0);
        plotMap->yAxis->setRange(0, 4.0);
        plotMap->replot();
    });

    // 主题同步：曲线和渐变统一跟随主题 token，避免示例与样式系统脱节
    auto syncTheme = [plotLine, plotMap, line1, line2, line3, cmap](){
        auto* mgr = StyleManagerAdapter::instance();
        const QPalette pal = qApp->palette();
        auto resolveThemeColor = [mgr](const QString& token, const QColor& fallback) {
            const QColor themed = mgr->themeColor(token);
            return themed.isValid() ? themed : fallback;
        };

        const QColor primary = resolveThemeColor("primaryColor", pal.color(QPalette::Highlight));
        const QColor warning = resolveThemeColor("warningColor", QColor(241, 90, 34));
        const QColor success = resolveThemeColor("successColor", QColor(60, 179, 113));
        const QColor low = resolveThemeColor("secondaryLightColor", primary.lighter(150));

        line1->setPen(QPen(primary, 2));
        line2->setPen(QPen(warning, 2));
        line3->setPen(QPen(success, 2));
        plotLine->replot();

        QCPColorGradient grad;
        grad.setColorStopAt(0.0, low);
        grad.setColorStopAt(1.0, primary);
        cmap->setGradient(grad);
        plotMap->replot();
    };
    syncTheme();
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::themeChanged,
                     chartGroup, [syncTheme](){ syncTheme(); });

    // 由基类处理背景与坐标轴主题联动
    chartLayout->addWidget(new Label("折线图", chartGroup), 0, 0);
    chartLayout->addWidget(plotLine, 1, 0);
    // 折线图默认显示图例
    plotLine->legend->setVisible(true);
    // 序列显示/隐藏控制
    {
        auto* controls = new StyledWidget(chartGroup);
        auto* hl = new ResponsiveGridLayout(controls,220,2);
        hl->setContentsMargins(0, 0, 0, 0);
        auto* cb1 = new CheckBox("显示 sin(x)", controls);
        auto* cb2 = new CheckBox("显示 cos(x)", controls);
        auto* cb3 = new CheckBox("显示 sin(x)+0.5cos(x)", controls);
        auto* cbLegendLine = new CheckBox("显示图例(折线图)", controls);
        cb1->setChecked(true); cb2->setChecked(true); cb3->setChecked(true); cbLegendLine->setChecked(true);
        QObject::connect(cb1, &CheckBox::toggled, [plotLine, line1](bool v){ line1->setVisible(v); plotLine->replot(); });
        QObject::connect(cb2, &CheckBox::toggled, [plotLine, line2](bool v){ line2->setVisible(v); plotLine->replot(); });
        QObject::connect(cb3, &CheckBox::toggled, [plotLine, line3](bool v){ line3->setVisible(v); plotLine->replot(); });
        QObject::connect(cbLegendLine, &CheckBox::toggled, [plotLine](bool v){ plotLine->legend->setVisible(v); plotLine->replot(); });
        hl->addWidget(cb1);
        hl->addWidget(cb2);
        hl->addWidget(cb3);
        hl->addWidget(cbLegendLine);

        chartLayout->addWidget(controls, 2, 0);
    }
    // 柱状图 Plot（独立，使用通用基类封装交互与主题）
    auto* plotBar = new StyledPlot(chartGroup);
    plotBar->setMinimumSize(320, 280);
    auto* bars = new QCPBarChart(plotBar->xAxis, plotBar->yAxis);
    {
        // 示例数据
        QVector<double> k(6), v(6);
        for (int i = 0; i < 6; ++i) { k[i] = i + 1; }
        v[0]=2.0; v[1]=3.5; v[2]=1.2; v[3]=4.3; v[4]=3.0; v[5]=2.6;
        bars->setData(k, v);
        bars->setWidth(0.7);
        bars->setName("示例柱状");
        // 放入图例（确保图例中可见）
        bars->addToLegend();
        // 设置轴范围，保证柱状数据可见
        plotBar->xAxis->setRange(0.5, 6.5);
        plotBar->yAxis->setRange(0.0, 5.0);
        // 主题联动颜色（填充/边框）
        auto* mgr = StyleManagerAdapter::instance();
        QColor fill = mgr->themeColor("primaryColor");
        QColor border = mgr->themeColor("secondaryLightColor");
        if (!fill.isValid()) fill = QColor(66,135,245);
        if (!border.isValid()) border = QColor(255,255,255);
        bars->setBrush(QBrush(fill));
        bars->setPen(QPen(border));
        // 图例默认开启
        plotBar->legend->setVisible(true);
        plotBar->replot();
    }
    chartLayout->addWidget(new Label("柱状图", chartGroup), 3, 0);
    chartLayout->addWidget(plotBar, 4, 0);
    // 双击还原柱状图范围
    QObject::connect(plotBar, &StyledPlot::mouseDoubleClick, [plotBar](QMouseEvent* /*ev*/){
        plotBar->xAxis->setRange(0.5, 6.5);
        plotBar->yAxis->setRange(0.0, 5.0);
        plotBar->replot();
    });
    // 柱状图图例开关
    {
        auto* legendControlsBar = new StyledWidget(chartGroup);
        auto* hlb = new QHBoxLayout(legendControlsBar);
        hlb->setContentsMargins(0,0,0,0);
        auto* cbLegendBar = new CheckBox("显示图例(柱状图)", legendControlsBar);
        cbLegendBar->setChecked(true);
        QObject::connect(cbLegendBar, &CheckBox::toggled, [plotBar](bool v){ plotBar->legend->setVisible(v); plotBar->replot(); });
        hlb->addWidget(cbLegendBar);

        chartLayout->addWidget(legendControlsBar, 5, 0);
    }
    // 主题切换时同步柱状图配色
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::themeChanged,
                     chartGroup, [plotBar, bars](){
        auto* mgr = StyleManagerAdapter::instance();
        QColor fill = mgr->themeColor("primaryColor");
        QColor border = mgr->themeColor("secondaryLightColor");
        if (!fill.isValid()) fill = QColor(66,135,245);
        if (!border.isValid()) border = QColor(255,255,255);
        bars->setBrush(QBrush(fill));
        bars->setPen(QPen(border));
        plotBar->replot();
    });

    chartLayout->addWidget(new Label("瀑布图", chartGroup), 6, 0);
    // 瀑布图图例控制
    {
        // 默认显示图例
        plotMap->legend->setVisible(true);
        auto* legendControls = new StyledWidget(chartGroup);
        auto* hl2 = new QHBoxLayout(legendControls);
        hl2->setContentsMargins(0, 0, 0, 0);
        auto* cbLegendMap = new CheckBox("显示图例(瀑布图)", legendControls);
        cbLegendMap->setChecked(true);
        QObject::connect(cbLegendMap, &CheckBox::toggled, [plotMap](bool v){ plotMap->legend->setVisible(v); plotMap->replot(); });
        hl2->addWidget(cbLegendMap);

    chartLayout->addWidget(legendControls, 7, 0);
    }
    chartLayout->addWidget(plotMap, 8, 0);
    layout->addWidget(chartGroup, 0, 0);

    scrollArea->setWidget(content);
    
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);

    
    return page;
}

// 创建仪表盘展示页面（独立标签页）
StyledWidget* createGaugesShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scroll = new ScrollArea(page); scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new StyledWidget;
    auto* cards = new ResponsiveGridLayout(content,300,2);
    auto makeCard = [&](const QString& title) {
        auto* group = new GroupBox(title,content); cards->addWidget(group);
        return new QVBoxLayout(group);
    };
    auto addSlider = [](QVBoxLayout* layout,int maximum) {
        auto* slider = new Slider(Qt::Horizontal); slider->setRange(0,maximum); layout->addWidget(slider); return slider;
    };
    auto* basicLayout = makeCard("基础速度表");
    auto* basic = new BasicGauge; basic->setMinimumSize(240,240); basicLayout->addWidget(basic);
    QObject::connect(addSlider(basicLayout,100),&QSlider::valueChanged,basic,&BasicGauge::setValue);

    auto* compassLayout = makeCard("罗盘");
    auto* compass = new CompassGauge; compass->setMinimumSize(240,240); compassLayout->addWidget(compass);
    QObject::connect(addSlider(compassLayout,360),&QSlider::valueChanged,compass,&CompassGauge::setValue);

    auto* speedLayout = makeCard("经典速度表");
    auto* speed = new SpeedGauge; speed->setMinimumSize(240,240); speedLayout->addWidget(speed);
    QObject::connect(addSlider(speedLayout,80),&QSlider::valueChanged,speed,&SpeedGauge::setValue);

    auto* attitudeLayout = makeCard("姿态仪");
    auto* attitude = new AttitudeMeter; attitude->setMinimumSize(240,240); attitudeLayout->addWidget(attitude);
    auto* pitch = addSlider(attitudeLayout,45); pitch->setRange(-45,45);
    auto* roll = addSlider(attitudeLayout,90); roll->setRange(-90,90);
    pitch->setToolTip("俯仰角"); roll->setToolTip("横滚角");
    QObject::connect(pitch,&QSlider::valueChanged,attitude,&AttitudeMeter::setCurrentPitch);
    QObject::connect(roll,&QSlider::valueChanged,attitude,&AttitudeMeter::setCurrentRoll);
    auto* automatic = new CheckBox("自动更新"); attitudeLayout->addWidget(automatic);
    auto* timer = new QTimer(attitude); timer->setInterval(60);
    QObject::connect(automatic,&QCheckBox::toggled,timer,[timer](bool enabled) { enabled ? timer->start() : timer->stop(); });
    QObject::connect(timer,&QTimer::timeout,attitude,[attitude,t=0.0]() mutable {
        if (!attitude->isVisible()) return;
        t += .05; attitude->setCurrentPitch(20*qSin(t*1.2)); attitude->setCurrentRoll(45*qSin(t*.8));
    });

    // Configurable presets belong to CircularGauge, not SpeedGauge's old no-op shims.
    auto* circularLayout = makeCard("分段仪表与主题预设");
    auto* circular = new CircularGauge; circular->setMinimumSize(260,260); circularLayout->addWidget(circular);
    auto* presets = new ComboBox; presets->addItems({"经典三色","完成率","自定义分段"}); circularLayout->addWidget(presets);
    auto updatePreset = [circular](int index) {
        if (index==0) circular->useClassicThreeColorPercentPreset("完成率","%",30,50,20);
        else if (index==1) circular->useCompletionRatePreset("完成率","%");
        else {
            circular->setRange(0,100); circular->setRanges({{0,30,QColor("#409EFF"),12},{30,70,QColor("#E6A23C"),12},{70,100,QColor("#67C23A"),12}});
            circular->setShowValueArc(true);
        }
        circular->update();
    };
    QObject::connect(presets,QOverload<int>::of(&QComboBox::currentIndexChanged),circular,updatePreset);
    updatePreset(0);
    auto* value = addSlider(circularLayout,100);
    QObject::connect(value,&QSlider::valueChanged,circular,[circular](int number) { circular->setValue(number); });
    value->setValue(75);
    scroll->setWidget(content);
    auto* layout = new QVBoxLayout(page); layout->setContentsMargins(0,0,0,0); layout->addWidget(scroll);
    return page;
}

int main(int argc, char *argv[])
{
    StyleManagerAdapter::EnableHighDpi();

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("BaseWidgets");
    QCoreApplication::setApplicationName("widgets_demo");
    if (app.arguments().contains("--verify")) {
#ifdef UI_DEMO_CHECKS
        extern int runDemoChecks();
        return runDemoChecks();
#else
        qCritical() << "Configure with -DUI_DEMO_CHECKS=ON to enable --verify.";
        return 2;
#endif
    }
    // 默认读取 UI 库内嵌主题，不依赖工作目录，也无需复制 themes.rcc。
    StyleManagerAdapter::initialize();

    // 创建主窗口（使用基于 QWidget 的无边框主界面）
    auto* window = new DemoMainWindow();
    window->setObjectName("WidgetsDemoMainWindow");
    window->setTitle("basewidgets 组件全面展示");
    // 设置窗口图标（会自动同步到标题栏）
    window->setWindowIcon(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/logo/logo.svg"));

    // 创建菜单栏
    auto* contentRoot = new StyledWidget();
    auto* rootLayout = new QVBoxLayout(contentRoot);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* menuBar = new MenuBar(contentRoot);
    auto* fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction("新建(&N)", [](){});
    fileMenu->addAction("打开(&O)", [](){});
    /*QAction *openAct = new QAction(StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/user.svg"), "用户(&U)");
    fileMenu->addAction(openAct);*/
    fileMenu->addSeparator();
    auto* recentMenu = fileMenu->addMenu("最近文件(&R)");
    recentMenu->addAction("文件1.txt");
    recentMenu->addAction("文件2.txt");
    fileMenu->addSeparator();
    fileMenu->addAction("退出(&X)", [&app](){ app.quit(); });

    auto* helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("关于 basewidgets(&A)", [](){});
    window->setMenuBar(menuBar);

    // 创建工具栏
    auto* toolBar = new ToolBar(window);
    toolBar->setWindowTitle("主工具栏");
    toolBar->setObjectName("DemoToolBar");
    toolBar->setMovable(false);
    toolBar->addAction("新建", [](){});
    toolBar->addAction("打开", [](){});
    toolBar->addAction("保存", [](){});
    toolBar->addSeparator();
    
    // 添加视图模式切换
    auto* actionGroup = new QActionGroup(toolBar);
    auto* listAction = toolBar->addAction("列表视图");
    listAction->setCheckable(true);
    listAction->setChecked(true);
    actionGroup->addAction(listAction);
    
    auto* iconAction = toolBar->addAction("图标视图");
    iconAction->setCheckable(true);
    actionGroup->addAction(iconAction);
    
    toolBar->addSeparator();
    
    // 添加带下拉菜单的工具按钮
    auto* moreButton = new ToolButton(toolBar);
    moreButton->setText("更多选项");
    moreButton->setPopupMode(QToolButton::InstantPopup);
    auto* moreMenu = new QMenu(moreButton);
    moreMenu->addAction("导出数据");
    moreMenu->addAction("导入配置");
    moreMenu->addSeparator();
    moreMenu->addAction("设置");
    moreButton->setMenu(moreMenu);
    toolBar->addWidget(moreButton);

    auto* themeCombo = new ComboBox(toolBar);
    themeCombo->addItem("深色主题");
    themeCombo->addItem("浅色主题");
    toolBar->addWidget(themeCombo);
    
    window->setToolBar(toolBar);

    // 创建状态栏
    auto* statusBar = new StatusBar(window);
    statusBar->showMessage("basewidgets 组件展示 - 就绪");
    
    // 添加永久控件
    auto* networkLabel = new Label("网络: 已连接", statusBar);
    statusBar->addPermanentWidget(networkLabel);
    
    auto* progressBar = new ProgressBar(statusBar);
    progressBar->setRange(0, 100);
    progressBar->setValue(85);
    progressBar->setMaximumWidth(150);
    statusBar->addPermanentWidget(progressBar);
    
    // 状态栏置于底部
    // 先占位，稍后在设置中央内容后再添加

    // 创建中央控件 - 主标签页
    auto* centralWidget = new StyledWidget(contentRoot);
    auto* centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    auto* mainTabWidget = new TabWidget(centralWidget);
    mainTabWidget->setTabPosition(QTabWidget::North);
    mainTabWidget->setTabsClosable(false);

    // 添加主要展示页面
    // 顶部标签取消 Emoji 图标，避免跨平台字体缺失
    mainTabWidget->addTab(new InputsPage(mainTabWidget), "输入控件");
    mainTabWidget->addTab(new DialogsPage(mainTabWidget), "对话框");
    mainTabWidget->addTab(new ViewsPage(mainTabWidget), "视图控件");
    mainTabWidget->addTab(new TextPage(mainTabWidget), "文本&容器");
    // 图表控件页
    mainTabWidget->addTab(createViewsChartsShowcase(mainTabWidget), "图表控件");
    // 仪表盘控件页（独立）
    mainTabWidget->addTab(createGaugesShowcase(mainTabWidget), "仪表盘控件");
    // 高级表格（分页/筛选/排序/导出）
    mainTabWidget->addTab(new PagedTablePage(mainTabWidget), "高级表格");

    centralLayout->addWidget(mainTabWidget);

    // 主内容
    rootLayout->addWidget(centralWidget, 1);

    window->setContentWidget(contentRoot);
    window->setStatusBar(statusBar);

    // 主题切换逻辑保持不变

    // 取消标题栏注入与按钮控制示例，恢复默认布局

    // 切换主题色和 SVG 资源；StyleManagerAdapter 刷新自绘样式，不生成或应用 QSS。
    QObject::connect(themeCombo, QOverload<int>::of(&ComboBox::currentIndexChanged), 
                     [](int index) {
        auto* mgr = StyleManagerAdapter::instance();
        if (index == 0) {
            mgr->setCurrentTheme("dark");
        } else {
            mgr->setCurrentTheme("light");
        }
        mgr->updateTheme();
    });

    // 几何持久化属于应用；库不写入全局设置，多个窗口可选择不同 key。
    const bool smoke = app.arguments().contains("--smoke-test");
    QSettings settings;
    if (!smoke) {
        if (!window->restoreGeometry(settings.value("mainWindow/geometry").toByteArray())) {
            const QSize available = app.primaryScreen()->availableGeometry().size();
            window->resize(QSize(1100,760).boundedTo(available));
        }
        QObject::connect(&app, &QCoreApplication::aboutToQuit, window, [window] {
            QSettings settings; settings.setValue("mainWindow/geometry",window->saveGeometry());
        });
    } else {
        window->resize(1100,760);
    }
    window->show();
    // Reproducible headless smoke test of the real composite pages.
    if (smoke) {
        QTimer::singleShot(150, &app, [window, mainTabWidget, &app] {
            const QString captureDir = qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
            if (!captureDir.isEmpty()) QDir().mkpath(captureDir);
            bool ok = qApp->styleSheet().isEmpty();
            for (int width : {1100,640}) {
                window->resize(width,760);
                QApplication::processEvents();
                ok = window->width()==width && ok;
                for (const QString theme : {QString("dark"), QString("light")}) {
                    auto* manager = StyleManagerAdapter::instance();
                    ok = manager->setCurrentTheme(theme) && manager->updateTheme() && ok;
                    for (int i=0;i<mainTabWidget->count();++i) {
                        mainTabWidget->setCurrentIndex(i);
                        QApplication::processEvents();
                        const QPixmap image = window->grab();
                        ok = !image.isNull() && ok;
                        if (!captureDir.isEmpty())
                            ok = image.save(QDir(captureDir).filePath(QString("demo-%1-%2-%3.png").arg(width).arg(theme).arg(i))) && ok;
                    }
                }
            }
            for (QWidget* widget : QApplication::allWidgets()) ok = widget->styleSheet().isEmpty() && ok;
            app.exit(ok ? 0 : 1);
        });
    }
    return app.exec();
}
