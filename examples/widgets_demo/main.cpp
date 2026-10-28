// 完整接入示例：高 DPI 设置→QApplication→初始化主题→组合主窗口。
// --smoke-test 遍历所有页面并退出；普通启动展示交互，--verify 开启可选回归验证。
#include <QApplication>
#include <QCommandLineParser>
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
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Core/WindowPlacement.h>
#include <basewidgets/Core/ScreenWindow.h>
#include <basewidgets/Core/MultiScreenWorkspace.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Views/AdvancedTable.h>
#include <QPainter>
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
#include <QTimer>
#include <basewidgets/Charts/QCustomplot.h>
#include <basewidgets/Charts/StyledPlot.h>
#include <QMenu>
#include <QtMath>
#include "pages/InputsPage.h"
#include "pages/CommonWidgetsPage.h"
#include "pages/DashboardPage.h"
#include "pages/GaugesPage.h"
#include "pages/AcousticPage.h"
#include "pages/DirectionAnalysisPage.h"
#include "pages/DataManagementPage.h"
#include "pages/DialogsPage.h"
#include "pages/PagedTablePage.h"
#include "pages/TextPage.h"
#include "DemoPageIds.h"

// 示例数据模型
#include <QStandardItemModel>
#include <QHeaderView>
#include <QFileInfo>
#include <QTabBar>

using namespace basewidgets;

class DemoMainWindow : public ScreenWindow {
protected:
    void closeEvent(QCloseEvent* ev) override
    {
        if (!DialogUtils::askQuestion(this, "请确认", "是否关闭软件？")) {
            ev->ignore();
            return;
        }
        ScreenWindow::closeEvent(ev);
    }
};


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
    QCommandLineParser arguments;
    arguments.setApplicationDescription("BaseWidgets 控件与配色示例"); arguments.addHelpOption();
    arguments.addOption({"styles-dir", "外部 styles 目录或主题 RCC；省略时使用库内嵌资源", "path"});
    arguments.addOption({"theme", "主题 XML 文件名（不含 .xml）；省略时读取 default_theme", "name"});
    arguments.addOption({"dashboard", "打开数据管理首页，使用居中标题与科技装饰"});
    arguments.addOption({"centered-title", "标题与图标整体居中，左侧展示业务动作"});
    arguments.addOption({"smoke-test", "遍历当前资源包的全部主题和页面后退出"});
    arguments.process(app);
    // 外部目录使用相同的配置协议。换配色只改 XML/JSON，应用无需重新编译。
    StyleManagerAdapter::initialize(arguments.value("styles-dir"), "themes", arguments.value("theme"));
    auto* themeManager = StyleManagerAdapter::instance();
    if (themeManager->themes().isEmpty() || themeManager->currentTheme().isEmpty()
        || (arguments.isSet("theme") && themeManager->currentTheme() != arguments.value("theme"))) {
        qCritical() << "Cannot load requested theme:" << arguments.value("theme") << themeManager->errorString();
        return 2;
    }

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

    // 应用菜单保持紧凑，命令栏提供常用操作；所有入口复用 QAction。
    auto icon=[](const QString& name) { return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/commands/"+name+".svg"); };
    auto* menuBar=new MenuBar(contentRoot);
    menuBar->setObjectName("WorkspaceMenuBar"); menuBar->setNativeMenuBar(false);
    QFont menuFont=menuBar->font(); menuFont.setWeight(QFont::Normal); menuBar->setFont(menuFont);
    auto* fileMenu=menuBar->addMenu("文件(&F)");
    auto* openAction=new QAction(icon("open"),"打开录音",window);
    openAction->setObjectName("WorkspaceOpenAction"); openAction->setShortcut(QKeySequence::Open);
    openAction->setToolTip("选择录音文件 · Ctrl+O"); fileMenu->addAction(openAction);
    auto* exportAction=new QAction(icon("export"),"导出 CSV",window);
    exportAction->setObjectName("WorkspaceExportAction"); exportAction->setToolTip("导出当前筛选后的录音记录"); fileMenu->addAction(exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction("退出(&X)",window,&QWidget::close);
    auto* helpMenu=menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("关于组件示例",window,[window] {
        DialogUtils::messageInfo(window,"BaseWidgets","水声分析与数据管理组件示例。\n界面中的采集、速率和仪表数据均为合成演示。");
    });
    window->setMenuBar(menuBar);

    auto* toolBar=new ToolBar(window);
    toolBar->setWindowTitle("工作区命令"); toolBar->setObjectName("DemoToolBar");
    toolBar->setMovable(false); toolBar->setFloatable(false);
    toolBar->setIconSize(QSize(18,18)); toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto* workspaceLabel=new Label("工作区",toolBar);
    workspaceLabel->setContentsMargins(10,0,6,0);
    auto updateWorkspaceLabel=[workspaceLabel] {
        auto pal=workspaceLabel->palette(); pal.setColor(QPalette::WindowText,ThemeTokens::instance()->textSecondary()); workspaceLabel->setPalette(pal);
    };
    updateWorkspaceLabel();
    QObject::connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,workspaceLabel,updateWorkspaceLabel);
    toolBar->addWidget(workspaceLabel);
    toolBar->addAction(openAction); toolBar->addAction(exportAction); toolBar->addSeparator();
    auto* actionGroup=new QActionGroup(toolBar);
    // The selected command follows the page, including pages outside this group.
    actionGroup->setExclusive(false);
    auto* recordsAction=toolBar->addAction(icon("records"),"录音"); recordsAction->setData(DemoPage::Records);
    auto* monitorAction=toolBar->addAction(icon("monitor"),"监控"); monitorAction->setData(DemoPage::Monitoring);
    auto* bearingAction=toolBar->addAction(icon("bearing"),"测向"); bearingAction->setData(DemoPage::Direction);
    for(auto* action:{recordsAction,monitorAction,bearingAction}) {
        action->setCheckable(true); actionGroup->addAction(action);
        action->setToolTip("打开"+action->text()+"工作区");
    }
    auto* spacer=new QWidget(toolBar); spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred); toolBar->addWidget(spacer);
    auto* themeAction=new QAction(icon("theme"),"主题",window);
    themeAction->setObjectName("WorkspaceThemeAction"); themeAction->setToolTip("选择配色主题");
    auto* themeMenu=new QMenu(window); themeMenu->setObjectName("WorkspaceThemeMenu"); themeAction->setMenu(themeMenu);
    auto* moreMenu=new QMenu(toolBar);
    auto* saveLayout=moreMenu->addAction(icon("layout"),"保存窗口布局");
    moreMenu->addAction(themeAction);
    auto* moreAction=new QAction("更多",toolBar); moreAction->setObjectName("WorkspaceMoreAction"); moreAction->setMenu(moreMenu);
    toolBar->addAction(moreAction);
    if(auto* button=qobject_cast<QToolButton*>(toolBar->widgetForAction(moreAction))) {
        button->setPopupMode(QToolButton::InstantPopup); button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    toolBar->addAction(themeAction);
    if (auto* button=qobject_cast<QToolButton*>(toolBar->widgetForAction(themeAction)))
        button->setPopupMode(QToolButton::InstantPopup);
    window->setToolBar(toolBar);

    // 一个主控窗口 + 多个子屏业务窗口。主屏可任选，不要求等于系统 primaryScreen。
    auto* workspace=new MultiScreenWorkspace(window);
    workspace->setObjectName("DemoMultiScreenWorkspace");
    auto* screensMenu=menuBar->addMenu("屏幕(&S)"); screensMenu->setObjectName("ScreenPlacementMenu");
    QObject::connect(screensMenu,&QMenu::aboutToShow,window,[screensMenu,workspace] {
        screensMenu->clear();
        auto* primary=screensMenu->addMenu("选择主控屏幕");
        for(auto* screen:QGuiApplication::screens()) {
            const QPointer<QScreen> target(screen);
            auto* action=primary->addAction(screen->name()); action->setCheckable(true); action->setChecked(screen==workspace->primaryScreen());
            QObject::connect(action,&QAction::triggered,workspace,[workspace,target] { if(target)workspace->setPrimaryScreen(target); });
        }
        auto* automatic=screensMenu->addAction("在其余屏幕自动创建子屏工作区"); automatic->setCheckable(true); automatic->setChecked(workspace->automaticSecondaryWindows());
        QObject::connect(automatic,&QAction::toggled,workspace,&MultiScreenWorkspace::setAutomaticSecondaryWindows);
        auto* reopen=screensMenu->addAction("重新显示所有子屏");
        QObject::connect(reopen,&QAction::triggered,workspace,&MultiScreenWorkspace::showAllSecondaryScreens);
        // 单屏开发也能查看角色差异，显式预览允许与主控共用物理屏幕。
        auto* preview=screensMenu->addMenu("子屏工作区预览 / 指定屏幕"); preview->setObjectName("MonitorScreensMenu");
        for(auto* screen:QGuiApplication::screens()) {
            const QPointer<QScreen> target(screen); auto* action=preview->addAction(screen->name());
            QObject::connect(action,&QAction::triggered,workspace,[workspace,target] {
                if(target)workspace->openSecondary("preview-"+MultiScreenWorkspace::screenKey(target),target,"子屏业务工作区 · 预览");
            });
        }
    });

    // 创建状态栏
    auto* statusBar = new StatusBar(window);
    statusBar->showMessage("basewidgets 组件展示 - 就绪");
    
    // 添加永久控件
    auto* networkLabel = new Label("主控 · 演示会话", statusBar);
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
    mainTabWidget->setObjectName("WorkspaceTabs");
    mainTabWidget->setDocumentMode(true);
    mainTabWidget->setHoverEffectEnabled(false);
    mainTabWidget->setContentHoverEffectEnabled(false);
    mainTabWidget->tabBar()->setDrawBase(false);
    mainTabWidget->tabBar()->setExpanding(false);
    mainTabWidget->tabBar()->setElideMode(Qt::ElideNone);

    // 添加主要展示页面
    // 顶部标签取消 Emoji 图标，避免跨平台字体缺失
    mainTabWidget->addTab(new InputsPage(mainTabWidget), "输入控件");
    mainTabWidget->addTab(new DialogsPage(mainTabWidget), "对话框");
    mainTabWidget->addTab(new CommonWidgetsPage(mainTabWidget), "常用组件");
    mainTabWidget->addTab(new TextPage(mainTabWidget), "文本布局");
    // 图表控件页
    mainTabWidget->addTab(new AcousticPage(mainTabWidget), "水声分析");
    mainTabWidget->addTab(new DirectionAnalysisPage(mainTabWidget), "测向解调");
    mainTabWidget->addTab(new DataManagementPage(mainTabWidget), "数据监控");
    // 仪表盘控件页（独立）
    mainTabWidget->addTab(new GaugesPage(mainTabWidget), "仪表盘");
    // 高级表格（分页/筛选/排序/导出）
    mainTabWidget->addTab(new PagedTablePage(mainTabWidget), "高级表格");
    auto* dashboard=new DashboardPage(mainTabWidget); mainTabWidget->addTab(dashboard,"数据首页");
    QObject::connect(dashboard,&DashboardPage::pageRequested,mainTabWidget,&QTabWidget::setCurrentIndex);
    if(arguments.isSet("dashboard"))mainTabWidget->setCurrentWidget(dashboard);

    // 导航状态只表达当前页面，不保留无实际作用的“列表/图标视图”开关。
    auto syncCommands=[actionGroup](int index) {
        for(auto* action:actionGroup->actions())action->setChecked(action->data().toInt()==index);
    };
    QObject::connect(actionGroup,&QActionGroup::triggered,mainTabWidget,[mainTabWidget,syncCommands](QAction* action) {
        mainTabWidget->setCurrentIndex(action->data().toInt());
        syncCommands(mainTabWidget->currentIndex()); // clicking the current page must not uncheck it
    });
    QObject::connect(mainTabWidget,&QTabWidget::currentChanged,toolBar,[syncCommands,statusBar,mainTabWidget](int index) {
        syncCommands(index); statusBar->showMessage("工作区 · "+mainTabWidget->tabText(index));
    });
    QObject::connect(openAction,&QAction::triggered,window,[window,statusBar,mainTabWidget] {
        const QString path=DialogUtils::openFile(window,"选择录音文件",QString(),"录音 (*.wav *.flac *.raw);;所有文件 (*)");
        if(path.isEmpty())return;
        mainTabWidget->setCurrentIndex(DemoPage::Records);
        statusBar->showMessage("已选择 "+QFileInfo(path).fileName()+" · 示例接入点，未执行文件解析",8000);
    });
    QObject::connect(exportAction,&QAction::triggered,window,[window,statusBar] {
        const auto* table=window->findChild<AdvancedTable*>("RecordingAdvancedTable"); if(!table)return;
        const QString path=DialogUtils::saveFile(window,"导出录音列表",QString(),"CSV (*.csv)"); if(path.isEmpty())return;
        QString error; AdvancedTable::CsvExportOptions options; options.filePath=path;
        const bool ok=table->exportCsv(options,&error);
        statusBar->showMessage(ok?"已导出录音列表："+QFileInfo(path).fileName():"导出失败："+error,8000);
    });
    QObject::connect(saveLayout,&QAction::triggered,window,[window,statusBar] {
        QSettings settings; settings.setValue("mainWindow/placement",WindowPlacement::save(window)); statusBar->showMessage("当前窗口布局已保存",4000);
    });
    auto themeLabel=[](const QString& name) {
        return name=="dark" ? QString("深色主题") : name=="light" ? QString("浅色主题") : name;
    };
    auto refreshThemeAction=[themeAction,themeMenu,themeLabel] {
        const QString current=StyleManagerAdapter::instance()->currentTheme();
        themeAction->setText(themeLabel(current));
        themeAction->setToolTip("当前主题："+themeLabel(current));
        for (auto* action:themeMenu->actions()) action->setChecked(action->data().toString()==current);
    };
    // 遍历 XML 发现的主题，新增 ocean.xml 等配置后自动出现在菜单中。
    auto rebuildThemeMenu=[themeMenu,themeLabel,refreshThemeAction,statusBar] {
        themeMenu->clear();
        for (const QString& name:StyleManagerAdapter::instance()->themes()) {
            auto* action=themeMenu->addAction(themeLabel(name)); action->setData(name); action->setCheckable(true);
            QObject::connect(action,&QAction::triggered,themeMenu,[name,refreshThemeAction,statusBar] {
                auto* manager=StyleManagerAdapter::instance();
                if (!manager->setCurrentTheme(name) || !manager->updateTheme())
                    statusBar->showMessage("主题加载失败："+manager->errorString(),8000);
                refreshThemeAction();
            });
        }
        refreshThemeAction();
    };
    rebuildThemeMenu();
    QObject::connect(StyleManagerAdapter::instance(),&acss::QtAdvancedStylesheet::currentStyleChanged,
                     themeMenu,rebuildThemeMenu);
    QObject::connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,themeMenu,refreshThemeAction);

    centralLayout->addWidget(mainTabWidget);

    // 主内容
    rootLayout->addWidget(centralWidget, 1);

    window->setContentWidget(contentRoot);
    window->setStatusBar(statusBar);

    // 子屏有自己的标题栏和状态栏，无主控的账户/全局搜索/菜单/工具栏。
    // 各屏可以同时打开不同分析页面；共享录音列表直接复用主屏源模型，不复制数据。
    QPointer<QAbstractItemModel> recordings=window->findChild<AdvancedTable*>("RecordingAdvancedTable")->sourceModel();
    QObject::connect(workspace,&MultiScreenWorkspace::windowCreated,window,[recordings,workspace](const QString& id,ScreenWindow* secondary) {
        auto* pages=new TabWidget;
        pages->addTab(new DirectionAnalysisPage(pages),"测向监视");
        pages->addTab(new DataManagementPage(pages),"数据监控");
        auto* table=new AdvancedTable(pages); table->setSourceModel(recordings); table->setPageSize(20);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        pages->addTab(table,"共享录音列表");
        pages->setCurrentIndex((workspace->secondaryIds().size()-1)%2);
        secondary->setContentWidget(pages);
        secondary->setContextText("模拟视图 · 共享录音源");
        secondary->titleBar()->setTitleAlignment(CustomTitleBar::TitleAlignment::Left);
        if(id.startsWith("screen-"))secondary->setWindowState(Qt::WindowMaximized);
        // 子屏之间独立切换页签/缩放；只有业务数据模型共享。
        QObject::connect(pages,&QTabWidget::currentChanged,secondary,[secondary,pages](int index) {
            secondary->setContextText(QString("%1 · 模拟视图").arg(pages->tabText(index)));
        });
    });
    QObject::connect(workspace,&MultiScreenWorkspace::compositorPlacementRequested,statusBar,[statusBar](const QString&,QScreen*) {
        statusBar->showMessage("已创建对应角色窗口；Wayland 下最终屏幕位置由合成器决定。",6000);
    });

    // 主标题栏独立配置，系统窗口标题/图标默认同步；这里演示覆盖展示标题。
    // 标题栏只发送搜索/操作信号，业务行为由应用实现。
    auto* titleBar=window->titleBar();
    titleBar->setTitle("水声数据管理中心");
    // 布局和颜色分别配置；--theme sg-blue --centered-title 可查看设计稿风格。
    // WithTitle 让图标与标题作为一个整体排列，左右按钮不会改变它的居中基准。
    titleBar->setIconPlacement(CustomTitleBar::IconPlacement::WithTitle);
    titleBar->setTitleAlignment((arguments.isSet("centered-title")||arguments.isSet("dashboard"))
        ?CustomTitleBar::TitleAlignment::Center:CustomTitleBar::TitleAlignment::Left);
    if(arguments.isSet("dashboard"))titleBar->setDecoration(CustomTitleBar::Decoration::Technical);
    titleBar->setSearchVisible(true); titleBar->setSettingsVisible(true); titleBar->setUserVisible(true);
    titleBar->setSearchPlaceholderText("搜索录音 · Ctrl+K");
    // 无照片时采用姓名字形占位，不把低分辨率示意头像放大。
    titleBar->setUserInfo("演示用户",QPixmap(),"本地示例 · 数据分析");
    auto* accountMenu=new QMenu(titleBar);
    auto* accountLabel=accountMenu->addAction("演示用户 · 本地账户"); accountLabel->setEnabled(false);
    accountMenu->addSeparator();
    auto* profileAction=accountMenu->addAction("用户信息");
    QObject::connect(profileAction,&QAction::triggered,window,[window] {
        DialogUtils::messageInfo(window,"用户信息","演示用户\n角色：数据分析\n此处可接入应用的账户信息和权限服务。");
    });
    titleBar->userButton()->setMenu(accountMenu);
    auto* titleSettings=new QMenu(titleBar); titleBar->settingsButton()->setMenu(titleSettings);
    auto* alignment=titleSettings->addMenu("标题位置"); auto* alignmentGroup=new QActionGroup(titleBar);
    auto* left=alignment->addAction("左侧"); auto* center=alignment->addAction("居中");
    for(auto* action:{left,center}) { action->setCheckable(true); alignmentGroup->addAction(action); }
    ((arguments.isSet("centered-title")||arguments.isSet("dashboard"))?center:left)->setChecked(true);
    QObject::connect(left,&QAction::triggered,titleBar,[titleBar] { titleBar->setTitleAlignment(CustomTitleBar::TitleAlignment::Left); });
    QObject::connect(center,&QAction::triggered,titleBar,[titleBar] { titleBar->setTitleAlignment(CustomTitleBar::TitleAlignment::Center); });
    auto* followIcon=titleSettings->addAction("图标随标题对齐"); followIcon->setCheckable(true); followIcon->setChecked(true);
    QObject::connect(followIcon,&QAction::toggled,titleBar,[titleBar](bool enabled) {
        titleBar->setIconPlacement(enabled?CustomTitleBar::IconPlacement::WithTitle:CustomTitleBar::IconPlacement::Leading);
    });
    auto* technical=titleSettings->addAction("科技风标题装饰"); technical->setCheckable(true); technical->setChecked(arguments.isSet("dashboard"));
    QObject::connect(technical,&QAction::toggled,titleBar,[titleBar](bool enabled) {
        titleBar->setDecoration(enabled?CustomTitleBar::Decoration::Technical:CustomTitleBar::Decoration::Plain);
    });
    auto* ownTitle=titleSettings->addAction("使用独立展示标题"); ownTitle->setCheckable(true); ownTitle->setChecked(true);
    QObject::connect(ownTitle,&QAction::toggled,titleBar,[titleBar](bool enabled) { if(enabled)titleBar->setTitle("水声数据管理中心"); else titleBar->resetTitle(); });
    auto* showIcon=titleSettings->addAction("显示应用图标"); showIcon->setCheckable(true); showIcon->setChecked(true);
    QObject::connect(showIcon,&QAction::toggled,titleBar,&CustomTitleBar::setIconVisible);
    auto* searchToggle=titleSettings->addAction("显示全局搜索"); searchToggle->setCheckable(true); searchToggle->setChecked(true);
    QObject::connect(searchToggle,&QAction::toggled,titleBar,&CustomTitleBar::setSearchVisible);
    titleSettings->addAction(themeAction);
    // 左侧为业务入口，QAction 仍由应用持有；点击仅演示页面导航，不启动真实采集。
    auto* analysisAction=new QAction("分析工作台",titleBar);
    titleBar->addTitleAction(analysisAction,CustomTitleBar::ActionPosition::Left);
    QObject::connect(analysisAction,&QAction::triggered,mainTabWidget,[mainTabWidget] { mainTabWidget->setCurrentIndex(DemoPage::Acoustic); });
    auto* showLeftActions=titleSettings->addAction("显示左侧业务入口"); showLeftActions->setCheckable(true); showLeftActions->setChecked(true);
    QObject::connect(showLeftActions,&QAction::toggled,analysisAction,&QAction::setVisible);
    // 可复用 QAction；窄窗口会自动出现在“更多操作”菜单，动作仍是同一个对象。
    auto* taskAction=new QAction("任务中心",titleBar); titleBar->addTitleAction(taskAction);
    QObject::connect(taskAction,&QAction::triggered,mainTabWidget,[mainTabWidget] { mainTabWidget->setCurrentIndex(DemoPage::Monitoring); });
    QObject::connect(titleBar,&CustomTitleBar::searchSubmitted,window,[window,mainTabWidget,statusBar](const QString& text) {
        auto* table=window->findChild<AdvancedTable*>("RecordingAdvancedTable");
        if(!table)return;
        table->setFilterText(text); mainTabWidget->setCurrentIndex(DemoPage::Records);
        statusBar->showMessage(text.isEmpty()?"已清空录音筛选":QString("录音搜索：%1 · %2 条结果").arg(text).arg(table->pager()->totalRows()),6000);
    });

    // 几何持久化属于应用；库不写入全局设置，多个窗口可选择不同 key。
    const bool smoke = app.arguments().contains("--smoke-test");
    QSettings settings;
    if (!smoke) {
        // 新格式保存稳定屏幕身份；旧 Qt 几何格式仅用于首次迁移。
        if (!WindowPlacement::restore(window,settings.value("mainWindow/placement").toByteArray()) &&
            !window->restoreGeometry(settings.value("mainWindow/geometry").toByteArray())) {
            const QSize available = app.primaryScreen()->availableGeometry().size();
            window->resize(QSize(1100,760).boundedTo(available));
        }
        QObject::connect(&app, &QCoreApplication::aboutToQuit, window, [window] {
            QSettings settings; settings.setValue("mainWindow/placement",WindowPlacement::save(window));
        });
    } else {
        window->resize(1100,760);
    }
    window->show();
    if(!smoke) {
        workspace->setPrimaryScreen(window->screen());
        workspace->setAutomaticSecondaryWindows(true);
    }
    // Reproducible headless smoke test of the real composite pages.
    if (smoke) {
        QTimer::singleShot(150, &app, [window, mainTabWidget, &app] {
            const QString captureDir = qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
            if (!captureDir.isEmpty()) QDir().mkpath(captureDir);
            bool ok = qApp->styleSheet().isEmpty();
            auto require=[&ok](bool passed,const char* label) { if(!passed)qWarning()<<"Demo smoke failed:"<<label; ok=passed&&ok; };
            require(mainTabWidget->count()==DemoPage::Count,"all consolidated demo pages are registered");
            require(mainTabWidget->tabText(DemoPage::Common)=="常用组件","views live in the common components tab");
            require(mainTabWidget->tabText(DemoPage::Dashboard)=="数据首页","dashboard navigation index is current");
            for (int width : {1100,640}) {
                window->resize(width,760);
                QApplication::processEvents();
                ok = window->width()==width && ok;
                for (const QString theme : StyleManagerAdapter::instance()->themes()) {
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
            // 宽屏首页用于核对四张指标卡与双列图表布局；不影响普通启动偏好。
            if(StyleManagerAdapter::instance()->themes().contains("sg-blue")) {
                auto* manager=StyleManagerAdapter::instance(); const QString previous=manager->currentTheme();
                auto* bar=window->titleBar(); const auto alignment=bar->titleAlignment(); const auto decoration=bar->decoration();
                manager->setCurrentTheme("sg-blue"); manager->updateTheme();
                bar->setTitleAlignment(CustomTitleBar::TitleAlignment::Center); bar->setDecoration(CustomTitleBar::Decoration::Technical);
                mainTabWidget->setCurrentIndex(DemoPage::Dashboard); window->resize(1600,1000); QApplication::processEvents();
                if(!captureDir.isEmpty())ok=window->grab().save(QDir(captureDir).filePath("dashboard-sg-blue-wide.png"))&&ok;
                bar->setTitleAlignment(alignment); bar->setDecoration(decoration); window->resize(640,760);
                manager->setCurrentTheme(previous); manager->updateTheme();
            }
            // Real command-bar actions navigate to business pages; a page outside this
            // group clears every check state. No file chooser or settings write in smoke.
            if(auto* toolbar=window->findChild<QToolBar*>("DemoToolBar")) {
                for(const auto& command:QList<QPair<QString,int>>{{"录音",DemoPage::Records},{"监控",DemoPage::Monitoring},{"测向",DemoPage::Direction}}) {
                    QAction* found=nullptr;
                    for(auto* action:toolbar->actions())if(action->text()==command.first) { found=action; break; }
                    require(found!=nullptr,"command action exists");
                    if(found) { found->trigger(); require(mainTabWidget->currentIndex()==command.second,"command opens expected page"); require(found->isChecked(),"command remains selected"); }
                }
                mainTabWidget->setCurrentIndex(DemoPage::Inputs);
                for(auto* action:toolbar->actions())if(action->isCheckable())require(!action->isChecked(),"unrelated page clears command selection");
                if(auto* theme=window->findChild<QAction*>("WorkspaceThemeAction")) {
                    auto* manager=StyleManagerAdapter::instance(); const QString before=manager->currentTheme();
                    require(theme->menu()!=nullptr,"theme command provides menu");
                    if(theme->menu()) {
                        require(theme->menu()->actions().size()==manager->themes().size(),"all configured themes are listed");
                        for(auto* choice:theme->menu()->actions()) {
                            choice->trigger(); require(manager->currentTheme()==choice->data().toString(),"theme menu selects configured theme");
                            require(choice->isChecked(),"active theme is checked");
                        }
                        manager->setCurrentTheme(before); manager->updateTheme();
                    }
                } else ok=false;
            } else ok=false;
            require(!mainTabWidget->hoverEffectEnabled() && !mainTabWidget->contentHoverEffectEnabled(),"page and tab hover default off");

            // Verify the title-bar business integration: submit a query and open the task page.
            auto* title=window->titleBar();
            if(title) {
                title->setSearchText("station_1");
                QMetaObject::invokeMethod(title->searchEdit(),"returnPressed",Qt::DirectConnection);
                auto* table=window->findChild<AdvancedTable*>("RecordingAdvancedTable");
                ok=table && table->pager()->totalRows()==50 && mainTabWidget->currentIndex()==DemoPage::Records && ok;
                title->setSearchText(""); QMetaObject::invokeMethod(title->searchEdit(),"returnPressed",Qt::DirectConnection);
                bool foundLeft=false;
                for(auto* action:title->findChildren<QAction*>())if(action->text()=="分析工作台") { action->trigger(); foundLeft=true; break; }
                require(foundLeft&&mainTabWidget->currentIndex()==DemoPage::Acoustic,"left title action opens analysis page");
                bool foundTask=false;
                for(auto* action:title->findChildren<QAction*>())if(action->text()=="任务中心") { action->trigger(); foundTask=true; break; }
                ok=foundTask && mainTabWidget->currentIndex()==DemoPage::Monitoring && ok;
            } else ok=false;

            // Exercise the role-based workspace on one offscreen surface as an explicit preview.
            auto* workspace=window->findChild<MultiScreenWorkspace*>("DemoMultiScreenWorkspace");
            if(workspace) {
                const QString id="smoke-secondary";
                auto* auxiliary=workspace->openSecondary(id,window->screen(),"子屏 · 测向 / 数据监控");
                ok=auxiliary && auxiliary->role()==ScreenWindow::Role::Secondary && auxiliary->parentWidget()==nullptr && ok;
                if(auxiliary) {
                    QApplication::processEvents();
                    ok=auxiliary->titleBar()->profile()==CustomTitleBar::Profile::SecondaryWindow && auxiliary->statusBar() && ok;
                    auto* shared=auxiliary->findChild<AdvancedTable*>();
                    auto* source=window->findChild<AdvancedTable*>("RecordingAdvancedTable");
                    ok=shared && source && shared->sourceModel()==source->sourceModel() && ok;
                    if(!captureDir.isEmpty())ok=auxiliary->grab().save(QDir(captureDir).filePath("secondary-workspace.png")) && ok;
                    ok=workspace->openSecondary(id,window->screen())==auxiliary && ok;
                    workspace->closeSecondary(id); ok=!auxiliary->isVisible() && ok;
                    workspace->openSecondary(id,window->screen()); ok=auxiliary->isVisible() && ok;
                    workspace->closeSecondary(id);
                }
            } else ok=false;
            for (QWidget* widget : QApplication::allWidgets()) ok = widget->styleSheet().isEmpty() && ok;
            app.exit(ok ? 0 : 1);
        });
    }
    return app.exec();
}
