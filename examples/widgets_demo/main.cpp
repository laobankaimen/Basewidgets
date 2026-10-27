// 完整接入示例：高 DPI 设置→QApplication→初始化主题→组合主窗口。
// --smoke-test 遍历所有页面并退出；普通启动展示交互，--verify 开启可选回归验证。
#include <QApplication>
#include <QCloseEvent>
#include <QSettings>
#include <QCryptographicHash>
#include <QScreen>
#include <QFont>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

#include <basewidgets/Core/StyleManagerAdapter.h>
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
#include "pages/GaugesPage.h"
#include "pages/AcousticPage.h"
#include "pages/DirectionAnalysisPage.h"
#include "pages/DataManagementPage.h"
#include "pages/DialogsPage.h"
#include "pages/PagedTablePage.h"
#include "pages/TextPage.h"
#include "pages/ViewsPage.h"

// 示例数据模型
#include <QStandardItemModel>
#include <QHeaderView>

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

    // 添加主要展示页面
    // 顶部标签取消 Emoji 图标，避免跨平台字体缺失
    mainTabWidget->addTab(new InputsPage(mainTabWidget), "输入控件");
    mainTabWidget->addTab(new DialogsPage(mainTabWidget), "对话框");
    mainTabWidget->addTab(new ViewsPage(mainTabWidget), "视图控件");
    mainTabWidget->addTab(new TextPage(mainTabWidget), "文本&容器");
    // 图表控件页
    mainTabWidget->addTab(new AcousticPage(mainTabWidget), "水声分析");
    mainTabWidget->addTab(new DirectionAnalysisPage(mainTabWidget), "测向 / 解调");
    mainTabWidget->addTab(new DataManagementPage(mainTabWidget), "数据监控");
    // 仪表盘控件页（独立）
    mainTabWidget->addTab(new GaugesPage(mainTabWidget), "仪表盘控件");
    // 高级表格（分页/筛选/排序/导出）
    mainTabWidget->addTab(new PagedTablePage(mainTabWidget), "高级表格");

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
    left->setChecked(true);
    QObject::connect(left,&QAction::triggered,titleBar,[titleBar] { titleBar->setTitleAlignment(CustomTitleBar::TitleAlignment::Left); });
    QObject::connect(center,&QAction::triggered,titleBar,[titleBar] { titleBar->setTitleAlignment(CustomTitleBar::TitleAlignment::Center); });
    auto* ownTitle=titleSettings->addAction("使用独立展示标题"); ownTitle->setCheckable(true); ownTitle->setChecked(true);
    QObject::connect(ownTitle,&QAction::toggled,titleBar,[titleBar](bool enabled) { if(enabled)titleBar->setTitle("水声数据管理中心"); else titleBar->resetTitle(); });
    auto* showIcon=titleSettings->addAction("显示应用图标"); showIcon->setCheckable(true); showIcon->setChecked(true);
    QObject::connect(showIcon,&QAction::toggled,titleBar,&CustomTitleBar::setIconVisible);
    auto* searchToggle=titleSettings->addAction("显示全局搜索"); searchToggle->setCheckable(true); searchToggle->setChecked(true);
    QObject::connect(searchToggle,&QAction::toggled,titleBar,&CustomTitleBar::setSearchVisible);
    auto* themeAction=titleSettings->addAction("切换明暗主题");
    QObject::connect(themeAction,&QAction::triggered,themeCombo,[themeCombo] { themeCombo->setCurrentIndex(1-themeCombo->currentIndex()); });
    // 可复用 QAction；窄窗口会自动出现在“更多操作”菜单，动作仍是同一个对象。
    auto* taskAction=new QAction("任务中心",titleBar); titleBar->addTitleAction(taskAction);
    QObject::connect(taskAction,&QAction::triggered,mainTabWidget,[mainTabWidget] { mainTabWidget->setCurrentIndex(6); });
    QObject::connect(titleBar,&CustomTitleBar::searchSubmitted,window,[window,mainTabWidget,statusBar](const QString& text) {
        auto* table=window->findChild<AdvancedTable*>("RecordingAdvancedTable");
        if(!table)return;
        table->setFilterText(text); mainTabWidget->setCurrentIndex(8);
        statusBar->showMessage(text.isEmpty()?"已清空录音筛选":QString("录音搜索：%1 · %2 条结果").arg(text).arg(table->pager()->totalRows()),6000);
    });

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
            // Verify the title-bar business integration: submit a query and open the task page.
            auto* title=window->titleBar();
            if(title) {
                title->setSearchText("station_1");
                QMetaObject::invokeMethod(title->searchEdit(),"returnPressed",Qt::DirectConnection);
                auto* table=window->findChild<AdvancedTable*>("RecordingAdvancedTable");
                ok=table && table->pager()->totalRows()==50 && mainTabWidget->currentIndex()==8 && ok;
                title->setSearchText(""); QMetaObject::invokeMethod(title->searchEdit(),"returnPressed",Qt::DirectConnection);
                bool foundTask=false;
                for(auto* action:title->findChildren<QAction*>())if(action->text()=="任务中心") { action->trigger(); foundTask=true; break; }
                ok=foundTask && mainTabWidget->currentIndex()==6 && ok;
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
