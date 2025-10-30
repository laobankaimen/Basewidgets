#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
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
    QGridLayout* grid = new QGridLayout(central);

    // 顶部：主题切换 + 背景配置
    auto* themeLabel = new Label("主题：");
    auto* themeCombo = new ComboBox();
    auto* bgLabel = new Label("背景：");
    auto* bgCombo = new ComboBox();

    auto* mgr = StyleManagerAdapter::instance();
    const QStringList themes = mgr->themes();
    themeCombo->addItems(themes);
    int curIndex = themes.indexOf(mgr->currentTheme());
    if (curIndex >= 0) themeCombo->setCurrentIndex(curIndex);
    QObject::connect(themeCombo, static_cast<void(ComboBox::*)(int)>(&ComboBox::currentIndexChanged),
        [mgr, themeCombo, bgCombo](int index){
            const QString theme = themeCombo->itemText(index);
            mgr->setCurrentTheme(theme);
            // 如果当前存在背景配置项，切换主题后继续应用该值
            const QString color = bgCombo->itemData(bgCombo->currentIndex()).toString();
            if (!color.isEmpty()) {
                mgr->setThemeVariableValue("central.background", color);
            }
            mgr->updateStylesheet();
        }
    );

    // 背景配置选项：默认（跟随主题），以及常用暗色与浅灰
    bgCombo->addItem("默认", QVariant());
    bgCombo->addItem("#2b2d31", QString("#2b2d31"));
    bgCombo->addItem("#1e1e1e", QString("#1e1e1e"));
    bgCombo->addItem("#e6e6e6", QString("#e6e6e6"));
    QObject::connect(bgCombo, static_cast<void(ComboBox::*)(int)>(&ComboBox::currentIndexChanged),
        [mgr, bgCombo]() {
            const QString color = bgCombo->itemData(bgCombo->currentIndex()).toString();
            mgr->setThemeVariableValue("central.background", color);
            mgr->updateStylesheet();
        }
    );

    // 设置默认背景变量：若未配置，深色主题用 #2b2d31，浅色主题用 #e6e6e6
    if (mgr->themeVariableValue("central.background").isEmpty()) {
        mgr->setThemeVariableValue("central.background", mgr->isCurrentThemeDark() ? "#2b2d31" : "#e6e6e6");
    }

    // 初始与动态应用背景与文本颜色：优先使用主题变量 central.background / central.foreground
    auto applyCentralBg = [central, &win, mgr]() {
        QString bg = mgr->themeVariableValue("central.background");
        if (bg.isEmpty()) {
            bg = mgr->isCurrentThemeDark() ? "#2b2d31" : "#e6e6e6";
        }
        QString fg = mgr->themeVariableValue("central.foreground");
        if (fg.isEmpty()) {
            fg = mgr->isCurrentThemeDark() ? "#e8e8e8" : "#222222";
        }
        const QString patch = "\nQMainWindow { background-color: " + bg + "; }\n#central { background-color: " + bg + "; color: " + fg + "; }\n";
        // 将补丁样式应用到主窗口（避免 StyledWidget 覆盖顺序问题）
        win.setStyleSheet(mgr->styleSheet() + patch);
    };
    applyCentralBg();
    QObject::connect(mgr, &acss::QtAdvancedStylesheet::stylesheetChanged, central, applyCentralBg, Qt::QueuedConnection);

    // Buttons
    auto* btn1 = new PushButton("主按钮");
    auto* btn2 = new PushButton("次按钮");
    auto* toolBtn = new ToolButton();
    toolBtn->setText("工具");
    toolBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // Label + LineEdit + SpinBox
    auto* label = new Label("名称：");
    auto* edit = new LineEdit();
    edit->setPlaceholderText("请输入内容...");

    auto* spinLabel = new Label("数量：");
    auto* spin = new SpinBox();
    spin->setRange(0, 100);
    spin->setValue(10);

    // CheckBox
    auto* chk = new CheckBox("启用功能");

    // RadioButtons
    auto* radio1 = new RadioButton("选项 A");
    auto* radio2 = new RadioButton("选项 B");
    QHBoxLayout* radioRow = new QHBoxLayout();
    radioRow->addWidget(radio1);
    radioRow->addWidget(radio2);
    radioRow->addStretch();

    // ComboBox 示例
    auto* combo = new ComboBox();
    combo->addItems({"项目 1", "项目 2", "项目 3"});

    // Slider with label
    auto* sliderLabel = new Label("滑块：50");
    auto* slider = new Slider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(50);
    QObject::connect(slider, &Slider::valueChanged, [sliderLabel](int v){
        sliderLabel->setText(QString("滑块：%1").arg(v));
    });

    // TabWidget 组合页（拆分为独立页面）
    auto* tabs = new TabWidget();
    tabs->addTab(new FormPage(), "表单");
    tabs->addTab(new ContainersPage(), "容器");

    // 独立页面：视图、对话框、工具箱
    tabs->addTab(new ViewsPage(), "视图");
    tabs->addTab(new DialogsPage(), "对话框");
    tabs->addTab(new ToolBoxPage(), "工具箱");

    // 网格布局排布
    grid->setContentsMargins(12, 12, 12, 12);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);

    int row = 0;
    grid->addWidget(themeLabel, row, 0);
    grid->addWidget(themeCombo, row, 1);
    grid->addWidget(bgLabel, row, 2);
    grid->addWidget(bgCombo, row, 3);

    row++;
    grid->addWidget(btn1, row, 0);
    grid->addWidget(btn2, row, 1);

    row++;
    grid->addWidget(new Label("选择："), row, 0);
    grid->addWidget(combo, row, 1);
    grid->addWidget(chk, row, 2);

    row++;
    grid->addWidget(new Label("选项："), row, 0);
    grid->addLayout(radioRow, row, 1, 1, 3);

    row++;
    grid->addWidget(tabs, row, 0, 1, 4);

    central->setLayout(grid);
    win.setCentralWidget(central);
    win.setWindowTitle("QtAdvancedUI 组件演示");
    win.resize(900, 620);
    win.show();

    return app.exec();
}