#include <QApplication>
#include <QFont>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QtAdvancedUI/Core/StyledMainWindow.h>
#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/Core/StyledDialog.h>
#include <QtAdvancedUI/Core/FramelessDialog.h>
#include <QtAdvancedUI/Containers/MenuBar.h>
#include <QtAdvancedUI/Containers/ToolBar.h>
#include <QtAdvancedUI/Containers/StatusBar.h>
#include <QtAdvancedUI/Containers/TabWidget.h>
#include <QtAdvancedUI/Containers/GroupBox.h>
#include <QtAdvancedUI/Containers/Frame.h>
#include <QtAdvancedUI/Containers/ScrollArea.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>
#include <QtAdvancedUI/Inputs/ComboBox.h>
#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Inputs/SpinBox.h>
#include <QtAdvancedUI/Inputs/CheckBox.h>
#include <QtAdvancedUI/Inputs/RadioButton.h>
#include <QtAdvancedUI/Inputs/Slider.h>
#include <QtAdvancedUI/Inputs/ProgressBar.h>
#include <QtAdvancedUI/Inputs/BusyIndicator.h>
#include <QtAdvancedUI/Inputs/ToggleSwitch.h>
#include <QtAdvancedUI/Inputs/TextEdit.h>
#include <QtAdvancedUI/Inputs/PlainTextEdit.h>
#include <QtAdvancedUI/Views/ListView.h>
#include <QtAdvancedUI/Views/TreeView.h>
#include <QtAdvancedUI/Views/TableView.h>
#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QtAdvancedUI/Views/TreeWidget.h>
#include <QtAdvancedUI/Views/CircularGauge.h>
#include <QtAdvancedUI/Charts/LineChart.h>
#include <QtAdvancedUI/Charts/PieChart.h>
#include <QtAdvancedUI/Charts/HistogramChart.h>

using namespace QtAdvancedUI;

// 创建输入控件展示页面
StyledWidget* createInputsShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scrollArea = new ScrollArea(page);
    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // 按钮组
    auto* buttonGroup = new GroupBox("按钮控件", content);
    auto* buttonLayout = new QGridLayout(buttonGroup);
    
    auto* pushBtn = new PushButton("普通按钮", buttonGroup);
    auto* checkableBtn = new PushButton("可选按钮", buttonGroup);
    checkableBtn->setCheckable(true);
    auto* disabledBtn = new PushButton("禁用按钮", buttonGroup);
    disabledBtn->setEnabled(false);
    auto* toolBtn = new ToolButton(buttonGroup);
    toolBtn->setText("工具按钮");
    
    buttonLayout->addWidget(pushBtn, 0, 0);
    buttonLayout->addWidget(checkableBtn, 0, 1);
    buttonLayout->addWidget(disabledBtn, 1, 0);
    buttonLayout->addWidget(toolBtn, 1, 1);
    // 限制左右组最大宽度，避免控件被拉得过长
    buttonGroup->setMaximumWidth(560);
    
    // 输入框组
    auto* inputGroup = new GroupBox("输入控件", content);
    auto* inputLayout = new QGridLayout(inputGroup);
    
    auto* lineEdit = new LineEdit(inputGroup);
    lineEdit->setPlaceholderText("请输入文本...");
    auto* spinBox = new SpinBox(inputGroup);
    spinBox->setRange(0, 100);
    spinBox->setValue(50);
    auto* comboBox = new ComboBox(inputGroup);
    comboBox->addItems({"选项1", "选项2", "选项3"});
    auto* slider = new Slider(Qt::Horizontal, inputGroup);
    slider->setRange(0, 100);
    slider->setValue(30);
    
    inputLayout->addWidget(new Label("文本输入:", inputGroup), 0, 0);
    inputLayout->addWidget(lineEdit, 0, 1);
    inputLayout->addWidget(new Label("数值输入:", inputGroup), 1, 0);
    inputLayout->addWidget(spinBox, 1, 1);
    inputLayout->addWidget(new Label("下拉选择:", inputGroup), 2, 0);
    inputLayout->addWidget(comboBox, 2, 1);
    inputLayout->addWidget(new Label("滑块:", inputGroup), 3, 0);
    inputLayout->addWidget(slider, 3, 1);
    inputGroup->setMaximumWidth(560);
    
    // 选择控件组
    auto* selectGroup = new GroupBox("选择控件", content);
    auto* selectLayout = new QVBoxLayout(selectGroup);
    
    auto* checkBox1 = new CheckBox("复选框 1", selectGroup);
    auto* checkBox2 = new CheckBox("复选框 2", selectGroup);
    checkBox2->setChecked(true);
    auto* radioBtn1 = new RadioButton("单选按钮 1", selectGroup);
    auto* radioBtn2 = new RadioButton("单选按钮 2", selectGroup);
    radioBtn1->setChecked(true);
    auto* toggleSwitch = new ToggleSwitch(selectGroup);
    
    selectLayout->addWidget(checkBox1);
    selectLayout->addWidget(checkBox2);
    selectLayout->addWidget(radioBtn1);
    selectLayout->addWidget(radioBtn2);
    selectLayout->addWidget(new Label("开关控件:", selectGroup));
    selectLayout->addWidget(toggleSwitch);
    selectGroup->setMaximumWidth(560);
    
    // 进度指示器组
    auto* progressGroup = new GroupBox("进度指示器", content);
    auto* progressLayout = new QVBoxLayout(progressGroup);
    
    auto* progressBar = new ProgressBar(progressGroup);
    progressBar->setValue(65);
    auto* busyIndicator = new BusyIndicator(progressGroup);
    busyIndicator->start();
    
    progressLayout->addWidget(new Label("进度条:", progressGroup));
    progressLayout->addWidget(progressBar);
    progressLayout->addWidget(new Label("忙碌指示器:", progressGroup));
    progressLayout->addWidget(busyIndicator);
    progressGroup->setMaximumWidth(560);

    // 右侧：Dialogs 组件演示区域
    auto* dialogsGroup = new GroupBox("Dialogs 组件演示", content);
    auto* dialogsLayout = new QVBoxLayout(dialogsGroup);
    dialogsGroup->setMaximumWidth(420);

    // 结果展示标签
    auto* fileResult = new Label("文件结果: ", dialogsGroup);
    auto* dirResult = new Label("目录结果: ", dialogsGroup);
    auto* colorResult = new Label("颜色结果: ", dialogsGroup);
    auto* fontResult = new Label("字体结果: 使用默认字体", dialogsGroup);
    auto* inputResult = new Label("输入结果: ", dialogsGroup);

    // 文件对话框
    auto* openFileBtn = new PushButton("打开文件", dialogsGroup);
    auto* openFilesBtn = new PushButton("打开多个文件", dialogsGroup);
    auto* saveFileBtn = new PushButton("保存文件", dialogsGroup);
    auto* openDirBtn = new PushButton("打开目录", dialogsGroup);

    // 颜色与字体对话框
    auto* pickColorBtn = new PushButton("选择颜色", dialogsGroup);
    auto* pickFontBtn = new PushButton("选择字体", dialogsGroup);

    // 消息框
    auto* msgInfoBtn = new PushButton("信息消息", dialogsGroup);
    auto* msgWarnBtn = new PushButton("警告消息", dialogsGroup);
    auto* msgErrorBtn = new PushButton("错误消息", dialogsGroup);
    auto* msgSuccessBtn = new PushButton("成功消息", dialogsGroup);
    auto* askQuestionBtn = new PushButton("询问消息", dialogsGroup);

    // 输入对话框
    auto* inputTextBtn = new PushButton("输入文本", dialogsGroup);
    auto* inputIntBtn = new PushButton("输入整数", dialogsGroup);
    auto* inputDoubleBtn = new PushButton("输入小数", dialogsGroup);

    // 排版
    dialogsLayout->addWidget(openFileBtn);
    dialogsLayout->addWidget(openFilesBtn);
    dialogsLayout->addWidget(saveFileBtn);
    dialogsLayout->addWidget(openDirBtn);
    dialogsLayout->addWidget(fileResult);
    dialogsLayout->addWidget(dirResult);
    dialogsLayout->addSpacing(6);
    dialogsLayout->addWidget(pickColorBtn);
    dialogsLayout->addWidget(colorResult);
    dialogsLayout->addWidget(pickFontBtn);
    dialogsLayout->addWidget(fontResult);
    dialogsLayout->addSpacing(6);
    dialogsLayout->addWidget(msgInfoBtn);
    dialogsLayout->addWidget(msgWarnBtn);
    dialogsLayout->addWidget(msgErrorBtn);
    dialogsLayout->addWidget(msgSuccessBtn);
    dialogsLayout->addWidget(askQuestionBtn);
    dialogsLayout->addSpacing(6);
    dialogsLayout->addWidget(inputTextBtn);
    dialogsLayout->addWidget(inputIntBtn);
    dialogsLayout->addWidget(inputDoubleBtn);
    dialogsLayout->addWidget(inputResult);
    dialogsLayout->addSpacing(6);

    // 保留样式对话框展示
    auto* openStyledDialogBtn = new PushButton("打开样式对话框", dialogsGroup);
    auto* openFramelessDialogBtn = new PushButton("打开无边框对话框", dialogsGroup);
    dialogsLayout->addWidget(openStyledDialogBtn);
    dialogsLayout->addWidget(openFramelessDialogBtn);
    dialogsLayout->addStretch();

    // 连接：文件与目录
    QObject::connect(openFileBtn, &PushButton::clicked, [page, fileResult]() {
        const auto path = DialogUtils::openFile(page, "选择文件");
        fileResult->setText("文件结果: " + (path.isEmpty() ? QString("<空>") : path));
    });
    QObject::connect(openFilesBtn, &PushButton::clicked, [page, fileResult]() {
        const auto paths = DialogUtils::openFiles(page, "选择多个文件");
        fileResult->setText("文件结果: " + (paths.isEmpty() ? QString("<空>") : paths.join("; ")));
    });
    QObject::connect(saveFileBtn, &PushButton::clicked, [page, fileResult]() {
        const auto path = DialogUtils::saveFile(page, "保存文件");
        fileResult->setText("文件结果: " + (path.isEmpty() ? QString("<空>") : path));
    });
    QObject::connect(openDirBtn, &PushButton::clicked, [page, dirResult]() {
        const auto path = DialogUtils::openDirectory(page, "选择目录");
        dirResult->setText("目录结果: " + (path.isEmpty() ? QString("<空>") : path));
    });

    // 连接：颜色与字体
    QObject::connect(pickColorBtn, &PushButton::clicked, [page, colorResult]() {
        const auto c = DialogUtils::pickColor(page, QColor("#26A69A"), "选择颜色");
        colorResult->setText(QString("颜色结果: %1").arg(c.name()));
        colorResult->setStyleSheet(QString("background:%1; padding:6px;").arg(c.name()));
    });
    QObject::connect(pickFontBtn, &PushButton::clicked, [page, fontResult]() {
        bool ok = false;
        const auto f = DialogUtils::pickFont(page, fontResult->font(), &ok, "选择字体");
        if (ok) {
            fontResult->setText(QString("字体结果: %1, %2pt").arg(f.family()).arg(f.pointSize()));
            fontResult->setFont(f);
        }
    });

    // 连接：消息框
    QObject::connect(msgInfoBtn, &PushButton::clicked, [page]() {
        DialogUtils::messageInfo(page, "信息", "这是一个信息提示");
    });
    QObject::connect(msgWarnBtn, &PushButton::clicked, [page]() {
        DialogUtils::messageWarning(page, "警告", "这是一个警告提示");
    });
    QObject::connect(msgErrorBtn, &PushButton::clicked, [page]() {
        DialogUtils::messageError(page, "错误", "这是一个错误提示");
    });
    QObject::connect(msgSuccessBtn, &PushButton::clicked, [page]() {
        DialogUtils::messageSuccess(page, "成功", "这是一个成功提示");
    });
    QObject::connect(askQuestionBtn, &PushButton::clicked, [page, inputResult]() {
        const bool yes = DialogUtils::askQuestion(page, "询问", "是否继续操作？");
        inputResult->setText(QString("输入结果: 询问 = %1").arg(yes ? "是" : "否"));
    });

    // 连接：输入框
    QObject::connect(inputTextBtn, &PushButton::clicked, [page, inputResult]() {
        const auto t = DialogUtils::inputText(page, "输入文本", "请输入内容:");
        inputResult->setText("输入结果: 文本 = " + t);
    });
    QObject::connect(inputIntBtn, &PushButton::clicked, [page, inputResult]() {
        const int v = DialogUtils::inputInt(page, "输入整数", "请输入整数:", 5, 0, 100, 1);
        inputResult->setText(QString("输入结果: 整数 = %1").arg(v));
    });
    QObject::connect(inputDoubleBtn, &PushButton::clicked, [page, inputResult]() {
        const double v = DialogUtils::inputDouble(page, "输入小数", "请输入小数:", 1.2, 0.0, 10.0, 2);
        inputResult->setText(QString("输入结果: 小数 = %1").arg(v));
    });

    // 保留示例对话框的触发
    QObject::connect(openStyledDialogBtn, &PushButton::clicked, [page]() {
        StyledDialog dlg(page);
        dlg.setWindowTitle("样式对话框");
        auto* w = new StyledWidget(&dlg);
        auto* l = new QVBoxLayout(w);
        l->addWidget(new Label("这里是 StyledDialog 的内容", w));
        auto* closeBtn = new PushButton("关闭", w);
        QObject::connect(closeBtn, &PushButton::clicked, &dlg, &QDialog::accept);
        l->addWidget(closeBtn);
        w->setLayout(l);
        dlg.setLayout(new QVBoxLayout());
        dlg.layout()->addWidget(w);
        dlg.exec();
    });
    QObject::connect(openFramelessDialogBtn, &PushButton::clicked, [page]() {
        FramelessDialog dlg(page);
        dlg.setWindowTitle("无边框对话框");
        auto* w = new StyledWidget(&dlg);
        auto* l = new QVBoxLayout(w);
        l->addWidget(new Label("这是 FramelessDialog", w));
        auto* closeBtn = new PushButton("关闭", w);
        QObject::connect(closeBtn, &PushButton::clicked, &dlg, &QDialog::accept);
        l->addWidget(closeBtn);
        w->setLayout(l);
        dlg.setLayout(new QVBoxLayout());
        dlg.layout()->addWidget(w);
        dlg.exec();
    });
    
    layout->addWidget(buttonGroup, 0, 0);
    layout->addWidget(inputGroup, 0, 1);
    layout->addWidget(selectGroup, 1, 0);
    layout->addWidget(progressGroup, 1, 1);
    layout->addWidget(dialogsGroup, 0, 2, 2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 1);
    
    content->setLayout(layout);
    scrollArea->setWidget(content);
    
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    page->setLayout(pageLayout);
    
    return page;
}

// 创建视图和图表展示页面
StyledWidget* createViewsChartsShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scrollArea = new ScrollArea(page);
    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // 视图控件组
    auto* viewGroup = new GroupBox("视图控件", content);
    auto* viewLayout = new QGridLayout(viewGroup);
    
    auto* listView = new ListView(viewGroup);
    auto* treeView = new TreeView(viewGroup);
    auto* tableView = new TableView(viewGroup);
    auto* circularGauge = new CircularGauge(viewGroup);
    circularGauge->setValue(75);
    circularGauge->setMinimumSize(150, 150);
    
    viewLayout->addWidget(new Label("列表视图:", viewGroup), 0, 0);
    viewLayout->addWidget(listView, 1, 0);
    viewLayout->addWidget(new Label("树形视图:", viewGroup), 0, 1);
    viewLayout->addWidget(treeView, 1, 1);
    viewLayout->addWidget(new Label("表格视图:", viewGroup), 2, 0);
    viewLayout->addWidget(tableView, 3, 0);
    viewLayout->addWidget(new Label("圆形仪表:", viewGroup), 2, 1);
    viewLayout->addWidget(circularGauge, 3, 1);
    
    // 图表控件组
    auto* chartGroup = new GroupBox("图表控件", content);
    auto* chartLayout = new QGridLayout(chartGroup);
    
    auto* lineChart = new LineChart(chartGroup);
    lineChart->setMinimumSize(300, 200);
    auto* pieChart = new PieChart(chartGroup);
    pieChart->setMinimumSize(300, 200);
    auto* histogramChart = new HistogramChart(chartGroup);
    histogramChart->setMinimumSize(300, 200);
    
    chartLayout->addWidget(new Label("折线图:", chartGroup), 0, 0);
    chartLayout->addWidget(lineChart, 1, 0);
    chartLayout->addWidget(new Label("饼图:", chartGroup), 0, 1);
    chartLayout->addWidget(pieChart, 1, 1);
    chartLayout->addWidget(new Label("柱状图:", chartGroup), 2, 0, 1, 2);
    chartLayout->addWidget(histogramChart, 3, 0, 1, 2);
    
    layout->addWidget(viewGroup, 0, 0);
    layout->addWidget(chartGroup, 1, 0);
    
    content->setLayout(layout);
    scrollArea->setWidget(content);
    
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    page->setLayout(pageLayout);
    
    return page;
}

// 创建文本和对话框展示页面
StyledWidget* createTextDialogShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scrollArea = new ScrollArea(page);
    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    // 文本编辑器组
    auto* textGroup = new GroupBox("文本编辑器", content);
    auto* textLayout = new QGridLayout(textGroup);
    
    auto* textEdit = new TextEdit(textGroup);
    textEdit->setPlainText("这是一个富文本编辑器\n支持多种格式");
    textEdit->setMinimumHeight(120);
    
    auto* plainTextEdit = new PlainTextEdit(textGroup);
    plainTextEdit->setPlainText("这是一个纯文本编辑器\n适合代码编辑");
    plainTextEdit->setMinimumHeight(120);
    
    textLayout->addWidget(new Label("富文本编辑器:", textGroup), 0, 0);
    textLayout->addWidget(textEdit, 1, 0);
    textLayout->addWidget(new Label("纯文本编辑器:", textGroup), 0, 1);
    textLayout->addWidget(plainTextEdit, 1, 1);
    
    // 对话框和工具组
    auto* dialogGroup = new GroupBox("对话框和工具", content);
    auto* dialogLayout = new QVBoxLayout(dialogGroup);
    
    
    auto* frameExample = new Frame(dialogGroup);
    frameExample->setMinimumHeight(80);
    auto* frameLayout = new QVBoxLayout(frameExample);
    frameLayout->addWidget(new Label("这是一个 Frame 容器", frameExample));
    frameLayout->addWidget(new Label("可以包含其他控件", frameExample));
    frameExample->setLayout(frameLayout);
    
    dialogLayout->addWidget(new Label("Frame 容器示例:", dialogGroup));
    dialogLayout->addWidget(frameExample);
    
    layout->addWidget(textGroup, 0, 0);
    layout->addWidget(dialogGroup, 1, 0);
    
    content->setLayout(layout);
    scrollArea->setWidget(content);
    
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    page->setLayout(pageLayout);
    
    return page;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置字体
    QFont font("Microsoft YaHei UI", 9);
    app.setFont(font);

    // 初始化样式管理器
    StyleManagerAdapter::initialize();

    // 创建主窗口
    auto* window = new StyledMainWindow();
    window->setWindowTitle("QtAdvancedUI 组件全面展示");
    window->resize(1400, 900);

    // 创建菜单栏
    auto* menuBar = new MenuBar(window);
    auto* fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction("新建(&N)", [](){});
    fileMenu->addAction("打开(&O)", [](){});
    fileMenu->addSeparator();
    auto* recentMenu = fileMenu->addMenu("最近文件(&R)");
    recentMenu->addAction("文件1.txt");
    recentMenu->addAction("文件2.txt");
    fileMenu->addSeparator();
    fileMenu->addAction("退出(&X)", [&app](){ app.quit(); });

    auto* helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("关于 QtAdvancedUI(&A)", [](){});
    window->setMenuBar(menuBar);

    // 创建工具栏
    auto* toolBar = new ToolBar(window);
    toolBar->setWindowTitle("主工具栏");
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
    moreButton->setPopupMode(QToolButton::MenuButtonPopup);
    auto* moreMenu = new QMenu(moreButton);
    moreMenu->addAction("导出数据");
    moreMenu->addAction("导入配置");
    moreMenu->addSeparator();
    moreMenu->addAction("设置");
    moreButton->setMenu(moreMenu);
    toolBar->addWidget(moreButton);
    
    window->addToolBar(toolBar);

    // 创建状态栏
    auto* statusBar = new StatusBar(window);
    statusBar->showMessage("QtAdvancedUI 组件展示 - 就绪");
    
    // 添加永久控件
    auto* networkLabel = new Label("网络: 已连接", statusBar);
    statusBar->addPermanentWidget(networkLabel);
    
    auto* progressBar = new ProgressBar(statusBar);
    progressBar->setRange(0, 100);
    progressBar->setValue(85);
    progressBar->setMaximumWidth(150);
    statusBar->addPermanentWidget(progressBar);
    
    window->setStatusBar(statusBar);

    // 创建中央控件 - 主标签页
    auto* centralWidget = new StyledWidget(window);
    auto* centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    auto* mainTabWidget = new TabWidget(centralWidget);
    mainTabWidget->setTabPosition(QTabWidget::North);
    mainTabWidget->setTabsClosable(false);

    // 添加三个主要展示页面
    mainTabWidget->addTab(createInputsShowcase(mainTabWidget), "🎛️ 输入控件");
    mainTabWidget->addTab(createViewsChartsShowcase(mainTabWidget), "📊 视图图表");
    mainTabWidget->addTab(createTextDialogShowcase(mainTabWidget), "📝 文本对话框");

    centralLayout->addWidget(mainTabWidget);
    centralWidget->setLayout(centralLayout);
    window->setCentralWidget(centralWidget);

    // 创建主题切换停靠窗口
    auto* themeDock = new QDockWidget("主题设置", window);
    auto* themeWidget = new StyledWidget(themeDock);
    auto* themeLayout = new QVBoxLayout(themeWidget);

    auto* themeLabel = new Label("🎨 主题选择", themeWidget);
    QFont titleFont = themeLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(10);
    themeLabel->setFont(titleFont);

    auto* themeCombo = new ComboBox(themeWidget);
    themeCombo->addItem("🌙 深色主题");
    themeCombo->addItem("☀️ 浅色主题");
    
    auto* infoLabel = new Label("选择不同主题查看\n所有控件的样式变化", themeWidget);
    infoLabel->setWordWrap(true);
    
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeCombo);
    themeLayout->addWidget(infoLabel);
    themeLayout->addStretch();

    themeWidget->setLayout(themeLayout);
    themeDock->setWidget(themeWidget);
    window->addDockWidget(Qt::LeftDockWidgetArea, themeDock);

    // 连接主题切换
    QObject::connect(themeCombo, QOverload<int>::of(&ComboBox::currentIndexChanged), 
                     [](int index) {
        auto* mgr = StyleManagerAdapter::instance();
        if (index == 0) {
            mgr->setCurrentTheme("dark_teal");
        } else {
            mgr->setCurrentTheme("light_teal");
        }
        mgr->updateStylesheet();
    });

    window->show();
    return app.exec();
}