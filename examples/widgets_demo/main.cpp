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


// 独立监控窗口可同时放到第二、第三屏，每个屏幕身份使用单独的应用设置键。
// 生产应用可把 DataManagementPage 换成共享业务模型驱动的真实监控页。
class DemoMonitorWindow : public FramelessWidgetWindow {
public:
    DemoMonitorWindow(const QString& key,QWidget* parent) : FramelessWidgetWindow(parent),m_key(key)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setTitle("数据管理 · 独立监控窗口（模拟）");
        setContentWidget(new DataManagementPage);
        resize(1000,760);
        if(!qApp->arguments().contains("--smoke-test")) {
            QSettings settings; WindowPlacement::restore(this,settings.value(m_key).toByteArray());
        }
        connect(qApp,&QCoreApplication::aboutToQuit,this,[this] { savePlacement(); });
    }
protected:
    void closeEvent(QCloseEvent* event) override
    {
        FramelessWidgetWindow::closeEvent(event);
        if(event->isAccepted())savePlacement();
    }
private:
    void savePlacement()
    {
        if(qApp->arguments().contains("--smoke-test"))return;
        QSettings settings; settings.setValue(m_key,WindowPlacement::save(this));
    }
    QString m_key;
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

    // 菜单每次展开重新枚举，允许运行中增减第二/第三屏；不缓存失效 QScreen 指针。
    auto* screensMenu=menuBar->addMenu("屏幕(&S)");
    screensMenu->setObjectName("ScreenPlacementMenu");
    QObject::connect(screensMenu,&QMenu::aboutToShow,window,[screensMenu,window] {
        screensMenu->clear();
        int number=0;
        for(auto* screen:QGuiApplication::screens()) {
            const QRect area=screen->availableGeometry();
            auto* action=screensMenu->addAction(QString("%1 · %2 · %3×%4 · %5%")
                .arg(++number).arg(screen->name()).arg(area.width()).arg(area.height()).arg(qRound(screen->devicePixelRatio()*100)));
            action->setCheckable(true); action->setChecked(window->screen()==screen);
            const QPointer<QScreen> target(screen);
            QObject::connect(action,&QAction::triggered,window,[window,target] {
                if(!target)return;
                const auto result=WindowPlacement::moveToScreen(window,target);
                if(result==WindowPlacement::MoveResult::CompositorManaged && window->statusBar())
                    window->statusBar()->showMessage("Wayland 下目标屏幕已请求，最终位置由桌面合成器决定。",6000);
            });
        }
        screensMenu->addSeparator();
        auto* separate=screensMenu->addMenu("在指定屏幕打开独立监控窗口");
        separate->setObjectName("MonitorScreensMenu");
        for(auto* screen:QGuiApplication::screens()) {
            const QPointer<QScreen> target(screen);
            auto* open=separate->addAction(screen->name());
            QObject::connect(open,&QAction::triggered,window,[window,target] {
                if(!target)return;
                const auto id=QString::fromLatin1(QCryptographicHash::hash((target->name()+"/"+target->serialNumber()).toUtf8(),QCryptographicHash::Sha256).toHex());
                const QString name="MonitorWindow-"+id;
                auto* monitor=window->findChild<FramelessWidgetWindow*>(name,Qt::FindDirectChildrenOnly);
                if(!monitor) {
                    monitor=new DemoMonitorWindow("monitorWindows/"+id+"/placement",window);
                    monitor->setObjectName(name);
                }
                WindowPlacement::moveToScreen(monitor,target);
                monitor->show(); monitor->raise(); monitor->activateWindow();
            });
        }
        auto* recover=screensMenu->addAction("恢复到可见区域");
        QObject::connect(recover,&QAction::triggered,window,[window] { WindowPlacement::ensureVisible(window); });
    });

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
            // Exercise the actual screen-menu action and auxiliary-window reuse without writing settings.
            if(auto* screens=window->findChild<QMenu*>("ScreenPlacementMenu")) {
                QMetaObject::invokeMethod(screens,"aboutToShow",Qt::DirectConnection);
                auto* monitors=screens->findChild<QMenu*>("MonitorScreensMenu");
                if(monitors && !monitors->actions().isEmpty()) {
                    monitors->actions().first()->trigger(); QApplication::processEvents();
                    int count=0; QPointer<FramelessWidgetWindow> auxiliary;
                    for(auto* candidate:window->findChildren<FramelessWidgetWindow*>(QString(),Qt::FindDirectChildrenOnly))
                        if(candidate->objectName().startsWith("MonitorWindow-")) { ++count; auxiliary=candidate; }
                    ok=count==1 && auxiliary && auxiliary->isVisible() && ok;
                    monitors->actions().first()->trigger(); QApplication::processEvents();
                    if(auxiliary) {
                        ok=auxiliary->contentWidget()!=nullptr && !auxiliary->grab().isNull() && ok;
                        auxiliary->close(); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
                        ok=auxiliary.isNull() && ok;
                    }
                } else ok=false;
            } else ok=false;
            for (QWidget* widget : QApplication::allWidgets()) ok = widget->styleSheet().isEmpty() && ok;
            app.exit(ok ? 0 : 1);
        });
    }
    return app.exec();
}
