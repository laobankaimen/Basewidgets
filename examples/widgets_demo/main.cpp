#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDockWidget>
#include <QTabBar>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Inputs/CheckBox.h>
#include <QtAdvancedUI/Inputs/RadioButton.h>
#include <QtAdvancedUI/Inputs/ComboBox.h>
#include <QtAdvancedUI/Inputs/Slider.h>
#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/SpinBox.h>
#include <QtAdvancedUI/Containers/TabWidget.h>
#include <QtAdvancedUI/Core/StyledMainWindow.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>
#include <QtAdvancedUI/Containers/MenuBar.h>
#include <QtAdvancedUI/Containers/ToolBar.h>
#include <QtAdvancedUI/Containers/StatusBar.h>

#include "pages/FormPage.h"
#include "pages/ContainersPage.h"
#include "pages/ViewsPage.h"
#include "pages/DialogsPage.h"
#include "pages/ToolBoxPage.h"
#include "pages/DashboardPage.h"
#include "pages/PagedTablePage.h"
#include "pages/ChartsPage.h"
#include <QMenu>
#include <QActionGroup>
#include <QToolButton>
#include <QProgressBar>
#include <QStyle>
#include <QKeySequence>
#include <QTranslator>
#include <QLibraryInfo>
 
using namespace QtAdvancedUI;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 加载 Qt 基础中文翻译，覆盖标准按钮/对话框文案
    QTranslator qtbaseTranslator;
    qtbaseTranslator.load("qtbase_zh_CN", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    if (!qtbaseTranslator.isEmpty()) {
        app.installTranslator(&qtbaseTranslator);
    }

    // 设置全局中文字体与字号
    QFont baseFont = app.font();
    baseFont.setFamily("Microsoft YaHei");
    baseFont.setPointSize(11);
    app.setFont(baseFont);
    // 统一字号（也覆盖库样式中的默认字体大小）
    app.setStyleSheet("QWidget{font-size:14px;} QToolTip{font-size:12px;}");

    // 初始化样式管理器（使用库的默认 UI_STYLES_DIR、样式与主题）
    StyleManagerAdapter::initialize();

    StyledMainWindow win;
    // 配置菜单栏、工具栏与状态栏（样式化组件）
    auto* menuBar = new MenuBar(&win);
    win.setMenuBar(menuBar);
    auto* fileMenu = menuBar->addMenu("文件");
    QAction* actNew = new QAction("新建", &win);
    QAction* actQuit = new QAction("退出", &win);
    QAction* actOpen = new QAction("打开", &win);
    // 图标与快捷键
    actNew->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    actNew->setShortcut(QKeySequence::New);
    actOpen->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
    actOpen->setShortcut(QKeySequence::Open);
    actQuit->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));
    actQuit->setShortcut(QKeySequence("Ctrl+Q"));
    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    // 最近文件子菜单
    QMenu* recentMenu = fileMenu->addMenu("最近文件");
    const QStringList recentFiles = { "README.md", "demo.txt", "report.csv" };
    for (const QString& fn : recentFiles) {
        QAction* a = recentMenu->addAction(QApplication::style()->standardIcon(QStyle::SP_FileIcon), fn);
        QObject::connect(a, &QAction::triggered, [&win, fn](){
            if (win.statusBar()) win.statusBar()->showMessage(QString("打开: %1").arg(fn), 1500);
        });
    }
    fileMenu->addSeparator();
    fileMenu->addAction(actQuit);
    QObject::connect(actQuit, &QAction::triggered, [](){ qApp->quit(); });

    auto* helpMenu = menuBar->addMenu("帮助");
    QAction* actAbout = new QAction("关于", &win);
    actAbout->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    actAbout->setShortcut(QKeySequence("F1"));
    helpMenu->addAction(actAbout);

    auto* toolBar = new ToolBar(&win);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    win.addToolBar(toolBar);
    QAction* actHello = new QAction("你好", &win);
    QAction* actInfo = new QAction("信息", &win);
    actHello->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    actInfo->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    toolBar->addAction(actHello);
    toolBar->addAction(actInfo);
    toolBar->addSeparator();
    // 切换动作组
    QActionGroup* viewModeGroup = new QActionGroup(&win);
    viewModeGroup->setExclusive(true);
    QAction* actModeA = new QAction("模式A", &win);
    QAction* actModeB = new QAction("模式B", &win);
    actModeA->setCheckable(true);
    actModeB->setCheckable(true);
    actModeA->setActionGroup(viewModeGroup);
    actModeB->setActionGroup(viewModeGroup);
    actModeA->setChecked(true);
    toolBar->addAction(actModeA);
    toolBar->addAction(actModeB);
    // 弹出菜单按钮
    QToolButton* moreBtn = new QToolButton();
    moreBtn->setText("更多");
    moreBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    moreBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
    QMenu* moreMenu = new QMenu(&win);
    moreMenu->addAction("设置");
    moreMenu->addAction("反馈");
    moreBtn->setMenu(moreMenu);
    moreBtn->setPopupMode(QToolButton::InstantPopup);
    toolBar->addWidget(moreBtn);
    QObject::connect(viewModeGroup, &QActionGroup::triggered, [&win](QAction* a){
        if (win.statusBar()) win.statusBar()->showMessage(QString("切换到 %1").arg(a->text()), 1200);
    });
    QObject::connect(actHello, &QAction::triggered, [&win](){
        if (win.statusBar()) win.statusBar()->showMessage("\"你好\" 动作已触发", 1500);
    });
    QObject::connect(actInfo, &QAction::triggered, [&win](){
        if (win.statusBar()) win.statusBar()->showMessage("\"信息\" 动作已触发", 1500);
    });

    auto* statusBar = new StatusBar(&win);
    win.setStatusBar(statusBar);
    // 永久小部件：网络状态与进度
    QLabel* netLabel = new QLabel("网络: 在线");
    QProgressBar* sbProgress = new QProgressBar();
    sbProgress->setRange(0, 100);
    sbProgress->setValue(66);
    sbProgress->setFixedWidth(120);
    statusBar->addPermanentWidget(netLabel);
    statusBar->addPermanentWidget(sbProgress);
    statusBar->showMessage("准备就绪", 2000);

    // 使用 StyledWidget 作为中心窗口，自动应用与刷新样式
    StyledWidget* central = new StyledWidget(&win);
    central->setObjectName("central");
    central->setAttribute(Qt::WA_StyledBackground, true);
    central->setAutoFillBackground(true);
    QVBoxLayout* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(12, 12, 12, 12);
    centralLayout->setSpacing(8);

    // 右侧 Dock：主题切换 + 背景配置（参考 full_features 的 Dock 布局）
    auto* themeDock = new QDockWidget("外观", &win);
    themeDock->setObjectName("ThemeDock");
    themeDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    StyledWidget* dockWidget = new StyledWidget(themeDock);
    QVBoxLayout* dockLayout = new QVBoxLayout(dockWidget);
    dockLayout->setContentsMargins(10, 10, 10, 10);
    dockLayout->setSpacing(10);
    auto* themeLabel = new Label("主题：");
    auto* themeCombo = new ComboBox();

    auto* mgr = StyleManagerAdapter::instance();
    const QStringList themes = mgr->themes();
    themeCombo->addItems(themes);
    int curIndex = themes.indexOf(mgr->currentTheme());
    if (curIndex >= 0) themeCombo->setCurrentIndex(curIndex);
    QObject::connect(themeCombo, static_cast<void(ComboBox::*)(int)>(&ComboBox::currentIndexChanged),
        [mgr, themeCombo](int index){
            const QString theme = themeCombo->itemText(index);
            mgr->setCurrentTheme(theme);
            mgr->updateStylesheet();
        }
    );
    
    // 直接跟随当前样式表，不再单独设置主界面背景颜色
    win.setStyleSheet(mgr->styleSheet());
    
    // TabWidget 组合页（左侧侧边栏式标签页，参考 full_features 的 East/West 标签布局）
    auto* tabs = new TabWidget();
    tabs->setTabPosition(QTabWidget::North);
    tabs->setDocumentMode(true);
    tabs->setIconSize(QSize(20, 20));
    // 添加页面并设置图标
    tabs->addTab(new DashboardPage(), QApplication::style()->standardIcon(QStyle::SP_ComputerIcon), "概览");
    // 将“图表”页面提前，避免被标签数量挤到滚动区
    tabs->addTab(new ChartsPage(), QApplication::style()->standardIcon(QStyle::SP_BrowserStop), "图表");
    tabs->addTab(new FormPage(), QApplication::style()->standardIcon(QStyle::SP_FileIcon), "表单");
    tabs->addTab(new ContainersPage(), QApplication::style()->standardIcon(QStyle::SP_DirIcon), "容器");
    tabs->addTab(new ViewsPage(), QApplication::style()->standardIcon(QStyle::SP_DesktopIcon), "视图");
    tabs->addTab(new DialogsPage(), QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), "对话框");
    tabs->addTab(new ToolBoxPage(), QApplication::style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton), "工具箱");
    tabs->addTab(new PagedTablePage(), QApplication::style()->standardIcon(QStyle::SP_DirHomeIcon), "高级表格");
    // 调整侧边栏宽度以获得更好的视觉效果
    if (tabs->tabBar()) {
        tabs->tabBar()->setUsesScrollButtons(true);
    }
    centralLayout->addWidget(tabs);

    // 将主题/背景控件放入 Dock
    dockLayout->addWidget(themeLabel);
    dockLayout->addWidget(themeCombo);
    dockLayout->addSpacing(6);
    dockLayout->addStretch();
    themeDock->setWidget(dockWidget);
    win.addDockWidget(Qt::RightDockWidgetArea, themeDock);

    central->setLayout(centralLayout);
    win.setCentralWidget(central);
    win.setWindowTitle("QtAdvancedUI 组件演示");
    win.resize(1360, 900);
    win.show();

    return app.exec();
}