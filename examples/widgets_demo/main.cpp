#include <QApplication>
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
#include "pages/PagedTablePage.h"

// 示例数据模型
#include <QStandardItemModel>
#include <QHeaderView>

using namespace basewidgets;

// 创建输入控件展示页面
StyledWidget* createInputsShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scrollArea = new ScrollArea(page);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(6, 0, 6, 6);
    // 减小主网格的间距，让输入控件更贴近选择控件
    layout->setSpacing(4);
    layout->setVerticalSpacing(2);

    // 按钮组
    auto* buttonGroup = new GroupBox("", content);
    auto* buttonLayout = new QGridLayout(buttonGroup);
    buttonLayout->setContentsMargins(4, 4, 4, 4);
    buttonLayout->setHorizontalSpacing(8);
    buttonLayout->setVerticalSpacing(4);
    
    auto* pushBtn = new PushButton("普通按钮", buttonGroup);
    auto* checkableBtn = new PushButton("可选按钮", buttonGroup);
    checkableBtn->setCheckable(true);
    auto* disabledBtn = new PushButton("禁用按钮", buttonGroup);
    disabledBtn->setEnabled(false);
    auto* toolBtn = new ToolButton(buttonGroup);
    toolBtn->setText("工具按钮");
    // 固定按钮尺寸，避免在布局中被拉伸
    pushBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    checkableBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    disabledBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    buttonLayout->addWidget(pushBtn, 0, 0);
    buttonLayout->addWidget(checkableBtn, 0, 1);
    buttonLayout->addWidget(disabledBtn, 1, 0);
    buttonLayout->addWidget(toolBtn, 1, 1);
    // 限制左右组最大宽度，避免控件被拉得过长
    buttonGroup->setMaximumWidth(420);
    buttonGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // 输入框组
    auto* inputGroup = new GroupBox("", content);
    auto* inputLayout = new QGridLayout(inputGroup);
    inputLayout->setContentsMargins(4, 4, 4, 4);
    inputLayout->setHorizontalSpacing(8);
    inputLayout->setVerticalSpacing(4);
    
    auto* lineEdit = new LineEdit(inputGroup);
    lineEdit->setPlaceholderText("请输入文本...");
    auto* spinBox = new SpinBox(inputGroup);
    spinBox->setRange(0, 100);
    spinBox->setValue(50);
    auto* comboBox = new ComboBox(inputGroup);
    comboBox->addItems({"选项1", "选项2", "选项3"});
    auto* multiCombo = new MultiSelectComboBox(inputGroup);
    multiCombo->setOptions({"后台", "前端", "移动端", "AI", "测试", "DevOps"});
    multiCombo->setSelectedOptions({"前端", "AI"});
    multiCombo->setJoinSelectedText(true, ", ");
    // 增强：启用搜索过滤、允许所有项、设置选择上限与提示
    multiCombo->setSearchEnabled(true);
    // 纯文字展示：不显示选项图标
    multiCombo->setShowOptionIcons(false);
    // 允许所有选项可选
    // multiCombo->setDisabledOptions({});
    // 示例：设置最大选择上限为 3，并在超过时提示
    multiCombo->setMaxSelection(3);
    QObject::connect(multiCombo, &MultiSelectComboBox::selectionLimitReached,
        multiCombo, [multiCombo](int limit){
            DialogUtils::messageWarning(multiCombo, "选择上限",
                QString("不允许超过上限 %1，请取消部分选择。").arg(limit));
        });
    // 示例中不设置图标以保持纯文字
    auto* slider = new Slider(Qt::Horizontal, inputGroup);
    slider->setRange(0, 100);
    slider->setValue(30);
    
    inputLayout->addWidget(new Label("文本输入:", inputGroup), 0, 0);
    inputLayout->addWidget(lineEdit, 0, 1);
    inputLayout->addWidget(new Label("数值输入:", inputGroup), 1, 0);
    inputLayout->addWidget(spinBox, 1, 1);
    inputLayout->addWidget(new Label("下拉选择:", inputGroup), 2, 0);
    inputLayout->addWidget(comboBox, 2, 1);
    inputLayout->addWidget(new Label("多选下拉:", inputGroup), 3, 0);
    inputLayout->addWidget(multiCombo, 3, 1);
    inputLayout->addWidget(new Label("滑块:", inputGroup), 4, 0);
    inputLayout->addWidget(slider, 4, 1);

    // 日期时间控件组
    auto* datetimeGroup = new GroupBox("", content);
    auto* dtLayout = new QGridLayout(datetimeGroup);
    dtLayout->setContentsMargins(4, 4, 4, 4);
    dtLayout->setHorizontalSpacing(8);
    dtLayout->setVerticalSpacing(4);
    auto* datePicker = new DatePicker(datetimeGroup);
    datePicker->setWeekNumbersEnabled(true);
    auto* timePicker = new TimePicker(datetimeGroup);
    auto* dateTimePicker = new DateTimePicker(datetimeGroup);
    auto* rangePicker = new DateRangePicker(datetimeGroup);
    dtLayout->addWidget(new Label("日期选择:", datetimeGroup), 0, 0);
    dtLayout->addWidget(datePicker, 0, 1);
    dtLayout->addWidget(new Label("时间选择:", datetimeGroup), 1, 0);
    dtLayout->addWidget(timePicker, 1, 1);
    dtLayout->addWidget(new Label("日期时间:", datetimeGroup), 2, 0);
    dtLayout->addWidget(dateTimePicker, 2, 1);
    dtLayout->addWidget(new Label("日期范围:", datetimeGroup), 3, 0);
    dtLayout->addWidget(rangePicker, 3, 1);
    datetimeGroup->setMaximumWidth(420);
    datetimeGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    inputGroup->setMaximumWidth(560);
    // 允许纵向按内容压缩，避免单元格留高导致上方空白
    inputGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    
    // 选择控件组
    auto* selectGroup = new GroupBox("", content);
    // 改为两列横排布局
    auto* selectLayout = new QGridLayout(selectGroup);
    // 压缩选择组内部边距与间距
    selectLayout->setContentsMargins(4, 4, 4, 4);
    selectLayout->setHorizontalSpacing(8);
    selectLayout->setVerticalSpacing(4);
    
    auto* checkBox1 = new CheckBox("复选框 1", selectGroup);
    auto* checkBox2 = new CheckBox("复选框 2", selectGroup);
    checkBox2->setChecked(true);
    auto* radioBtn1 = new RadioButton("单选按钮 1", selectGroup);
    auto* radioBtn2 = new RadioButton("单选按钮 2", selectGroup);
    radioBtn1->setChecked(true);
    auto* toggleSwitch = new ToggleSwitch(selectGroup);
    
    // 每行两个控件：复选、单选两行；开关与标签一行
    selectLayout->addWidget(checkBox1, 0, 0);
    selectLayout->addWidget(checkBox2, 0, 1);
    selectLayout->addWidget(radioBtn1, 1, 0);
    selectLayout->addWidget(radioBtn2, 1, 1);
    selectLayout->addWidget(new Label("开关控件:", selectGroup), 2, 0);
    selectLayout->addWidget(toggleSwitch, 2, 1);
    selectGroup->setMaximumWidth(420);
    selectGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    
    // 进度指示器组
    auto* progressGroup = new GroupBox("", content);
    auto* progressLayout = new QVBoxLayout(progressGroup);
    progressLayout->setContentsMargins(4, 4, 4, 4);
    progressLayout->setSpacing(4);
    
    auto* progressBar1 = new ProgressBar(progressGroup);
    progressBar1->setValue(30);
    auto* progressBar2 = new ProgressBar(progressGroup);
    progressBar2->setProperty("state", QVariant("success"));
    progressBar2->setValue(40);
    auto* progressBar3 = new ProgressBar(progressGroup);
    progressBar3->setProperty("state", QVariant("danger"));
    progressBar3->setValue(50);
    auto* progressBar4 = new ProgressBar(progressGroup);
    progressBar4->setProperty("state", QVariant("warning"));
    progressBar4->setValue(60);
    auto* progressBar5 = new ProgressBar(progressGroup);
    progressBar5->setRange(0,0);
    auto* progressBar6 = new ProgressBar(progressGroup);
    progressBar6->setValue(70);
    progressBar6->setDisabled(true);
    auto* busyIndicator = new BusyIndicator(progressGroup);
    busyIndicator->start();
    
    progressLayout->addWidget(new Label("进度条:", progressGroup));
    progressLayout->addWidget(progressBar1);
    progressLayout->addWidget(progressBar2);
    progressLayout->addWidget(progressBar3);
    progressLayout->addWidget(progressBar4);
    progressLayout->addWidget(progressBar6);
    progressLayout->addWidget(new Label("忙碌指示器:", progressGroup));
    progressLayout->addWidget(progressBar5);
    progressLayout->addWidget(busyIndicator);
    progressGroup->setMaximumWidth(420);
    progressGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 导航列表组
    auto* navGroup = new GroupBox("", content);
    auto* navLayout = new QHBoxLayout(navGroup);
    navLayout->setContentsMargins(4, 4, 4, 4);
    navLayout->setSpacing(8);

    auto* navList = new NavigationList(navGroup);
    // 添加按钮 - 使用自动ID
    navList->addButton("首页");
    navList->addButton("设置");
    navList->addButton("关于");
    // 插入按钮 - 混合使用自动ID和指定ID
    navList->insertButton("个人中心", 1); // 在位置1插入，自动分配ID
    navList->insertButton("消息", 2, 10); // 在位置2插入，指定ID为10
    navList->addButton("帮助", 11); // 添加按钮，指定ID为11
    // 设置默认选中
    navList->setCurrentButton(0);

    navLayout->addWidget(navList);

    // 当前选中项显示
    auto* navCurrentLabel = new Label("当前选中: 首页 (ID: 0)", navGroup);
    navCurrentLabel->setWordWrap(true);
    navLayout->addWidget(navCurrentLabel);

    navGroup->setMaximumWidth(420);
    navGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 右侧：Dialogs 组件演示区域
    auto* dialogsGroup = new GroupBox("", content);
    auto* dialogsLayout = new QGridLayout(dialogsGroup);
    dialogsLayout->setHorizontalSpacing(10);
    dialogsLayout->setVerticalSpacing(20);
    dialogsLayout->setContentsMargins(6, 6, 6, 6);
    dialogsLayout->setColumnStretch(0, 1);
    dialogsLayout->setColumnStretch(1, 1);
    dialogsGroup->setMaximumWidth(480);
    dialogsGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // 结果展示标签
    auto* fileResult = new Label("文件结果: ", dialogsGroup);
    auto* dirResult = new Label("目录结果: ", dialogsGroup);
    auto* colorResult = new Label("颜色结果: ", dialogsGroup);
    auto* fontResult = new Label("字体结果: 使用默认字体", dialogsGroup);
    auto* inputResult = new Label("输入结果: ", dialogsGroup);

    // 文件对话框
    auto* openFileBtn = new PushButton("打开文件", dialogsGroup);
    auto* openFilesBtn = new PushButton("多个文件", dialogsGroup);
    auto* saveFileBtn = new PushButton("保存文件", dialogsGroup);
    auto* openDirBtn = new PushButton("打开目录", dialogsGroup);
    // 固定按钮尺寸，避免在布局中被拉伸
    openFileBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    openFilesBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    saveFileBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    openDirBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 颜色与字体对话框
    auto* pickColorBtn = new PushButton("选择颜色", dialogsGroup);
    auto* pickFontBtn = new PushButton("选择字体", dialogsGroup);
    pickColorBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pickFontBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 消息框
    auto* msgInfoBtn = new PushButton("信息消息", dialogsGroup);
    auto* msgWarnBtn = new PushButton("警告消息", dialogsGroup);
    auto* msgErrorBtn = new PushButton("错误消息", dialogsGroup);
    auto* msgSuccessBtn = new PushButton("成功消息", dialogsGroup);
    auto* askQuestionBtn = new PushButton("询问消息", dialogsGroup);
    msgInfoBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    msgWarnBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    msgErrorBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    msgSuccessBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    askQuestionBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 输入对话框
    auto* inputTextBtn = new PushButton("输入文本", dialogsGroup);
    auto* inputIntBtn = new PushButton("输入整数", dialogsGroup);
    auto* inputDoubleBtn = new PushButton("输入小数", dialogsGroup);
    inputTextBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    inputIntBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    inputDoubleBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 调整按钮尺寸以适配两排紧凑布局
    const int dlgBtnH = 32;
    const int dlgBtnW = 96;
    openFileBtn->setFixedSize(dlgBtnW, dlgBtnH);
    openFilesBtn->setFixedSize(dlgBtnW, dlgBtnH);
    saveFileBtn->setFixedSize(dlgBtnW, dlgBtnH);
    openDirBtn->setFixedSize(dlgBtnW, dlgBtnH);
    pickColorBtn->setFixedSize(dlgBtnW, dlgBtnH);
    pickFontBtn->setFixedSize(dlgBtnW, dlgBtnH);
    msgInfoBtn->setFixedSize(dlgBtnW, dlgBtnH);
    msgWarnBtn->setFixedSize(dlgBtnW, dlgBtnH);
    msgErrorBtn->setFixedSize(dlgBtnW, dlgBtnH);
    msgSuccessBtn->setFixedSize(dlgBtnW, dlgBtnH);
    askQuestionBtn->setFixedSize(dlgBtnW, dlgBtnH);
    inputTextBtn->setFixedSize(dlgBtnW, dlgBtnH);
    inputIntBtn->setFixedSize(dlgBtnW, dlgBtnH);
    inputDoubleBtn->setFixedSize(dlgBtnW, dlgBtnH);

    // 两列竖排布局（左列 7 个，右列 7 个）
    dialogsLayout->addWidget(openFileBtn,    0, 0);
    dialogsLayout->addWidget(openFilesBtn,   1, 0);
    dialogsLayout->addWidget(saveFileBtn,    2, 0);
    dialogsLayout->addWidget(openDirBtn,     3, 0);
    dialogsLayout->addWidget(pickColorBtn,   4, 0);
    dialogsLayout->addWidget(pickFontBtn,    5, 0);
    dialogsLayout->addWidget(msgInfoBtn,     6, 0);
    dialogsLayout->addWidget(msgWarnBtn,     0, 1);
    dialogsLayout->addWidget(msgErrorBtn,    1, 1);
    dialogsLayout->addWidget(msgSuccessBtn,  2, 1);
    dialogsLayout->addWidget(askQuestionBtn, 3, 1);
    dialogsLayout->addWidget(inputTextBtn,   4, 1);
    dialogsLayout->addWidget(inputIntBtn,    5, 1);
    dialogsLayout->addWidget(inputDoubleBtn, 6, 1);

    // 结果标签置于两排按钮下方，减少高度占用
    fileResult->setWordWrap(true);
    dirResult->setWordWrap(true);
    colorResult->setWordWrap(true);
    fontResult->setWordWrap(true);
    inputResult->setWordWrap(true);
    dialogsLayout->addWidget(fileResult,  7, 0, 1, 2);
    dialogsLayout->addWidget(dirResult,   8, 0, 1, 2);
    dialogsLayout->addWidget(colorResult, 9, 0, 1, 2);
    dialogsLayout->addWidget(fontResult, 10, 0, 1, 2);
    dialogsLayout->addWidget(inputResult, 11, 0, 1, 2);

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
        // 传入 QColor() 表示使用库中记录的上次颜色
        // 用户也可以通过 DialogUtils::getLastPickedColor() 获取
        const auto c = DialogUtils::pickColor(page, QColor(), "选择颜色");
        
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

    // 已移除示例对话框按钮与触发逻辑
    
    // 左侧列：使用一个垂直容器紧密堆叠控制类分组，减少行间空白
    auto* leftPanel = new QWidget(content);
    auto* leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(4, 4, 4, 4);
    leftPanelLayout->setSpacing(6);
    // 让分组在垂直方向尽量压缩
    buttonGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    selectGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    inputGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    progressGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    datetimeGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    navGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    leftPanelLayout->addWidget(dialogsGroup);
    leftPanelLayout->addWidget(buttonGroup);
    leftPanelLayout->addWidget(selectGroup);
    leftPanelLayout->addWidget(inputGroup);
    leftPanelLayout->addWidget(progressGroup);
    leftPanelLayout->addWidget(datetimeGroup);
    leftPanelLayout->addWidget(navGroup);
    leftPanel->setLayout(leftPanelLayout);

    // 左列放置该容器，整体顶部对齐
    layout->addWidget(leftPanel, 0, 0, 5, 1, Qt::AlignTop);

    // 视图控件组（合并自“视图图表”页）
    {
        auto* viewGroup = new GroupBox("", content);
        auto* viewLayout = new QGridLayout(viewGroup);

        auto* listView = new ListView(viewGroup);
        auto* tableView = new TableView(viewGroup);
        listView->setMinimumHeight(140);
        tableView->setMinimumHeight(180);
        auto* treeTable = new TreeTable(viewGroup);
        // 列表视图模型
        {
            auto* listModel = new QStandardItemModel(listView);
            listModel->setColumnCount(1);
            listModel->appendRow(new QStandardItem("任务 A"));
            listModel->appendRow(new QStandardItem("任务 B"));
            listModel->appendRow(new QStandardItem("任务 C"));
            listView->setModel(listModel);
        }
        // 表格视图模型
        {
            auto* tableModel = new QStandardItemModel(tableView);
            tableModel->setColumnCount(3);
            tableModel->setHorizontalHeaderLabels({"产品", "区域", "销量"});
            tableModel->appendRow(QList<QStandardItem*>{new QStandardItem("A"), new QStandardItem("华北"), new QStandardItem("120")});
            tableModel->appendRow(QList<QStandardItem*>{new QStandardItem("B"), new QStandardItem("华东"), new QStandardItem("98")});
            tableModel->appendRow(QList<QStandardItem*>{new QStandardItem("C"), new QStandardItem("华南"), new QStandardItem("150")});
            tableView->setModel(tableModel);
            tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        }
        // TreeTable 示例模型
        auto* tmodel = new QStandardItemModel(treeTable);
        tmodel->setColumnCount(3);
        tmodel->setHorizontalHeaderLabels({"类别", "项目", "数值"});
        auto* sales = new QStandardItem("销售");
        auto* rnd = new QStandardItem("研发");
        tmodel->appendRow(QList<QStandardItem*>{sales, new QStandardItem(), new QStandardItem()});
        tmodel->appendRow(QList<QStandardItem*>{rnd, new QStandardItem(), new QStandardItem()});
        sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q1"), new QStandardItem(), new QStandardItem("120.5")});
        sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q2"), new QStandardItem(), new QStandardItem("98.0")});
        sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q3"), new QStandardItem(), new QStandardItem("150.3")});
        sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q4"), new QStandardItem(), new QStandardItem("110.0")});
        rnd->appendRow(QList<QStandardItem*>{new QStandardItem("平台"), new QStandardItem(), new QStandardItem("80.0")});
        rnd->appendRow(QList<QStandardItem*>{new QStandardItem("算法"), new QStandardItem(), new QStandardItem("132.7")});
        rnd->appendRow(QList<QStandardItem*>{new QStandardItem("客户端"), new QStandardItem(), new QStandardItem("64.2")});
        treeTable->setModel(tmodel);
        treeTable->clearColumnAggregations();
        treeTable->setColumnAggregation(2, TreeTable::Sum);
        treeTable->expandAll();
        treeTable->header()->setSectionResizeMode(QHeaderView::Stretch);

        viewLayout->addWidget(new Label("列表视图:", viewGroup), 0, 0);
        viewLayout->addWidget(listView, 1, 0);
        viewLayout->addWidget(new Label("表格视图:", viewGroup), 2, 0);
        viewLayout->addWidget(tableView, 3, 0);
        viewLayout->addWidget(new Label("树形表格:", viewGroup), 4, 0);
        viewLayout->addWidget(treeTable, 5, 0);
        viewLayout->setColumnStretch(0, 1);
        // 视图控件置于右列第二行
    layout->addWidget(viewGroup, 1, 1);
    }

    // 图表控件组（已迁移至 createViewsChartsShowcase，旧组移除）

    // 文本编辑器组与示例容器（合并自“文本对话框”页）
    {
        auto* textGroup = new GroupBox("", content);
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
        // 文本编辑器组置于右列第四行
        layout->addWidget(textGroup, 3, 1);

        auto* dialogGroup = new GroupBox("", content);
        auto* dialogLayout = new QVBoxLayout(dialogGroup);
        auto* frameExample = new Frame(dialogGroup);
        frameExample->setMinimumHeight(80);
        auto* frameLayout = new QVBoxLayout(frameExample);
        frameLayout->addWidget(new Label("这是一个 Frame 容器", frameExample));
        frameLayout->addWidget(new Label("可以包含其他控件", frameExample));
        frameExample->setLayout(frameLayout);
        dialogLayout->addWidget(new Label("Frame 容器示例:", dialogGroup));
        dialogLayout->addWidget(frameExample);
        // 工具示例容器并入右列底部，与 dialogsGroup 同层次
        layout->addWidget(dialogGroup, 4, 1);
    }

    // 列伸缩：左列保持最小，右列作为主显示区
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 1);
    // 行伸缩：前四行固定，底部一行吸收多余空间
    layout->setRowStretch(0, 0);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 0);
    layout->setRowStretch(3, 0);
    layout->setRowStretch(4, 1);

    // 连接NavigationList信号以更新当前选中项显示
    // 使用currentItemChanged信号，无论是点击还是程序设置都会触发
    QObject::connect(navList, &NavigationList::currentItemChanged,
        [navList, navCurrentLabel](QListWidgetItem* current, QListWidgetItem* previous){
            if(current) {
                int id = navList->getCurrentButton();
                QString title = current->text();
                navCurrentLabel->setText(QString("当前选中: %1 (ID: %2)").arg(title).arg(id));
            }
        });

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
    // 图表控件组（折线图、柱状图与瀑布图分离展示）
    auto* chartGroup = new GroupBox("", content);
    auto* chartLayout = new QGridLayout(chartGroup);

    // 折线图 Plot（独立，使用通用基类封装交互与主题）
    auto* plotLine = new StyledPlot(chartGroup);
    plotLine->setMinimumSize(600, 300);

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
    plotMap->setMinimumSize(600, 300);

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

    // 主题同步：配色跟随主题变化（背景由组件内部处理）
    auto syncTheme = [plotLine, plotMap, line1, line2, line3, cmap](){
        auto* mgr = StyleManagerAdapter::instance();
        const QPalette pal = qApp->palette();
        // 多折线采用不同颜色
        QPen p1(QColor(66,135,245), 2);   // 蓝
        QPen p2(QColor(241,90,34), 2);    // 橙
        QPen p3(QColor(60,179,113), 2);   // 绿
        line1->setPen(p1);
        line2->setPen(p2);
        line3->setPen(p3);
        plotLine->replot();
        // 瀑布图渐变采用固定蓝->红
        QCPColorGradient grad;
        grad.setColorStopAt(0.0, QColor(0,0,255));
        grad.setColorStopAt(1.0, QColor(255,0,0));
        cmap->setGradient(grad);
        plotMap->replot();
    };
    syncTheme();
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged,
                     chartGroup, [syncTheme](){ syncTheme(); });

    // 由基类处理背景与坐标轴主题联动
    chartLayout->addWidget(new Label("折线图", chartGroup), 0, 0);
    chartLayout->addWidget(plotLine, 1, 0);
    // 折线图默认显示图例
    plotLine->legend->setVisible(true);
    // 序列显示/隐藏控制
    {
        auto* controls = new StyledWidget(chartGroup);
        auto* hl = new QHBoxLayout(controls);
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
        controls->setLayout(hl);
        chartLayout->addWidget(controls, 2, 0);
    }
    // 柱状图 Plot（独立，使用通用基类封装交互与主题）
    auto* plotBar = new StyledPlot(chartGroup);
    plotBar->setMinimumSize(600, 280);
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
        legendControlsBar->setLayout(hlb);
        chartLayout->addWidget(legendControlsBar, 5, 0);
    }
    // 主题切换时同步柱状图配色
    QObject::connect(StyleManagerAdapter::instance(), &acss::QtAdvancedStylesheet::stylesheetChanged,
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
        legendControls->setLayout(hl2);
    chartLayout->addWidget(legendControls, 7, 0);
    }
    chartLayout->addWidget(plotMap, 8, 0);
    layout->addWidget(chartGroup, 0, 0);
    
    content->setLayout(layout);
    scrollArea->setWidget(content);
    
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    page->setLayout(pageLayout);
    
    return page;
}

// 创建仪表盘展示页面（独立标签页）
StyledWidget* createGaugesShowcase(QWidget* parent) {
    auto* page = new StyledWidget(parent);
    auto* scrollArea = new ScrollArea(page);
    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    auto* gaugeGroup = new GroupBox("", content);
    auto* gaugeLayout = new QGridLayout(gaugeGroup);

    // Basic 速度表
    {
        auto* g = new BasicGauge(gaugeGroup);
        g->setMinimumSize(240, 240);
        auto* slider = new Slider(Qt::Horizontal, gaugeGroup);
        slider->setRange(0, 100);
        slider->setValue(0);
        QObject::connect(slider, &Slider::valueChanged, [g](int v){ g->setValue(v); });
        gaugeLayout->addWidget(new Label("Basic 速度表", gaugeGroup), 0, 0);
        gaugeLayout->addWidget(g, 1, 0);
        gaugeLayout->addWidget(slider, 2, 0);
    }

    // Compass 罗盘
    {
        auto* g = new CompassGauge(gaugeGroup);
        g->setMinimumSize(240, 240);
        auto* slider = new Slider(Qt::Horizontal, gaugeGroup);
        slider->setRange(0, 360);
        slider->setValue(0);
        QObject::connect(slider, &Slider::valueChanged, [g](int v){ g->setValue(v); });
        gaugeLayout->addWidget(new Label("Compass 罗盘", gaugeGroup), 0, 1);
        gaugeLayout->addWidget(g, 1, 1);
        gaugeLayout->addWidget(slider, 2, 1);
    }

    // SpeedGauge 速度表 + 主题预设
    {
        auto* g = new SpeedGauge(gaugeGroup);
        g->setMinimumSize(240, 240);
        auto* slider = new Slider(Qt::Horizontal, gaugeGroup);
        slider->setRange(0, 80);
        slider->setValue(0);
        QObject::connect(slider, &Slider::valueChanged, [g](int v){ g->setValue(v); });

        auto* row = new StyledWidget(gaugeGroup);
        auto* hl = new QHBoxLayout(row); hl->setContentsMargins(0,0,0,0);
        hl->addWidget(new Label("主题预设:", row));
        auto* preset = new ComboBox(row);
        preset->addItem("经典三色预设");
        preset->addItem("完成率预设");
        preset->addItem("自定义分段");
        hl->addWidget(preset);
        row->setLayout(hl);

        QObject::connect(preset, QOverload<int>::of(&ComboBox::currentIndexChanged), [g](int idx){
            if (idx == 0) {
                g->useClassicThreeColorPercentPreset(QString(), QString("Km/h"), 30, 50, 20,
                                                     QColor(162, 208, 190), QColor(103,133,160), QColor(203,80,80));
                g->setRange(0, 80);
            } else if (idx == 1) {
                g->useCompletionRatePreset(QString(), QString("Km/h"));
                g->setRange(0, 80);
            } else {
                QVector<CircularGauge::GaugeRange> ranges;
                ranges.push_back({0, 30, QColor(66, 135, 245), 12});   // 蓝
                ranges.push_back({30, 60, QColor(241, 90, 34), 12});  // 橙
                ranges.push_back({60, 80, QColor(203, 80, 80), 12});  // 红
                g->setRanges(ranges);
                g->setShowValueArc(true);
                g->setDialColor(QColor(40,40,40));
                g->setNeedleColor(QColor(50, 120, 255));
                g->setUnit("Km/h");
                g->setRange(0, 80);
                g->setTickCount(9);
                g->setShowTickLabels(true);
            }
            g->update();
        });
        preset->setCurrentIndex(0);

        gaugeLayout->addWidget(new Label("SpeedGauge 速度表", gaugeGroup), 3, 0);
        gaugeLayout->addWidget(g, 4, 0);
        gaugeLayout->addWidget(slider, 5, 0);
        gaugeLayout->addWidget(row, 6, 0);
    }

    // AttitudeMeter 姿态仪 + 自动更新动画
    {
        auto* g = new AttitudeMeter(gaugeGroup);
        g->setMinimumSize(240, 240);
        auto* pitch = new Slider(Qt::Horizontal, gaugeGroup);
        auto* roll  = new Slider(Qt::Horizontal, gaugeGroup);
        pitch->setRange(-45, 45); pitch->setValue(0);
        roll->setRange(-90, 90);  roll->setValue(0);
        QObject::connect(pitch, &Slider::valueChanged, [g](int v){ g->setCurrentPitch(v); });
        QObject::connect(roll,  &Slider::valueChanged, [g](int v){ g->setCurrentRoll(v); });

        auto* animRow = new StyledWidget(gaugeGroup);
        auto* hl = new QHBoxLayout(animRow); hl->setContentsMargins(0,0,0,0);
        auto* cb = new CheckBox("自动更新", animRow);
        hl->addWidget(cb);
        animRow->setLayout(hl);
        auto* timer = new QTimer(animRow);
        timer->setInterval(60);
        QObject::connect(cb, &CheckBox::toggled, [timer](bool on){ on ? timer->start() : timer->stop(); });
        QObject::connect(timer, &QTimer::timeout, [g]{
            static double t = 0.0; t += 0.05;
            g->setCurrentPitch(20 * qSin(t * 1.2));
            g->setCurrentRoll(45 * qSin(t * 0.8));
        });

        gaugeLayout->addWidget(new Label("AttitudeMeter 姿态仪", gaugeGroup), 3, 1);
        gaugeLayout->addWidget(g, 4, 1);
        auto* row = new StyledWidget(gaugeGroup);
        auto* sliders = new QHBoxLayout(row); sliders->setContentsMargins(0,0,0,0);
        sliders->addWidget(new Label("Pitch:", row)); sliders->addWidget(pitch);
        sliders->addWidget(new Label("Roll:", row));  sliders->addWidget(roll);
        row->setLayout(sliders);
        gaugeLayout->addWidget(row, 5, 1);
        gaugeLayout->addWidget(animRow, 6, 1);
    }

    // CircularGauge 圆形仪表（迁移自首页）
    {
        auto* g = new CircularGauge(gaugeGroup);
        // 保持与迁移前一致的初始化参数
        g->useClassicThreeColorPercentPreset("完成率", "%", 30, 50, 20);
        g->setTickLabelFontScale(0.75);
        g->setValue(75);
        g->setMinimumSize(260, 260);
        gaugeLayout->addWidget(new Label("CircularGauge 圆形仪表", gaugeGroup), 7, 0);
        gaugeLayout->addWidget(g, 8, 0);
    }

    layout->addWidget(gaugeGroup, 0, 0);
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
    auto* textGroup = new GroupBox("", content);
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
    auto* dialogGroup = new GroupBox("", content);
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
    StyleManagerAdapter::EnableHighDpi();

    QApplication app(argc, argv);
    // 初始化样式管理器；不传 stylesDir 时会默认查找应用目录下的 styles/ 与 themes.rcc
    StyleManagerAdapter::initialize();

    // 创建主窗口（使用基于 QWidget 的无边框主界面）
    auto* window = new FramelessWidgetWindow();
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
    auto* toolBar = new ToolBar(contentRoot);
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
    
    rootLayout->addWidget(toolBar);

    // 创建状态栏
    auto* statusBar = new StatusBar(contentRoot);
    statusBar->showMessage("basewidgets 组件展示 - 就绪");
    
    // 添加永久控件
    auto* networkLabel = new Label("网络: 已连接", statusBar);
    statusBar->addWidget(networkLabel);
    
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
    mainTabWidget->addTab(createInputsShowcase(mainTabWidget), "输入控件");
    // 图表控件页
    mainTabWidget->addTab(createViewsChartsShowcase(mainTabWidget), "图表控件");
    // 仪表盘控件页（独立）
    mainTabWidget->addTab(createGaugesShowcase(mainTabWidget), "仪表盘控件");
    // 高级表格（分页/筛选/排序/导出）
    mainTabWidget->addTab(new PagedTablePage(mainTabWidget), "高级表格");

    centralLayout->addWidget(mainTabWidget);
    centralWidget->setLayout(centralLayout);
    // 工具栏（主题切换）置于顶部工具栏区域下方
    auto* themeToolbar = new ToolBar(contentRoot);
    themeToolbar->setMovable(false);
    rootLayout->addWidget(themeToolbar);
    themeToolbar->addWidget(new Label("二号工具栏", themeToolbar));

    // 主内容
    rootLayout->addWidget(centralWidget, 1);
    // 底部状态栏
    rootLayout->addWidget(statusBar);
    contentRoot->setLayout(rootLayout);
    window->setContentWidget(contentRoot);

    // 主题切换逻辑保持不变

    // 取消标题栏注入与按钮控制示例，恢复默认布局

    // 连接主题切换：仅修改当前主题并生成样式表，由 StyleManagerAdapter 统一应用到 qApp
    QObject::connect(themeCombo, QOverload<int>::of(&ComboBox::currentIndexChanged), 
                     [](int index) {
        auto* mgr = StyleManagerAdapter::instance();
        if (index == 0) {
            mgr->setCurrentTheme("dark");
        } else {
            mgr->setCurrentTheme("light");
        }
        mgr->updateStylesheet();
    });

    window->show();
    return app.exec();
}
