#include "InputsPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Inputs/BusyIndicator.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Inputs/DatePicker.h>
#include <basewidgets/Inputs/DateRangePicker.h>
#include <basewidgets/Inputs/DateTimePicker.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/RadioButton.h>
#include <basewidgets/Inputs/Slider.h>
#include <basewidgets/Inputs/SpinBox.h>
#include <basewidgets/Inputs/TimePicker.h>
#include <basewidgets/Inputs/ToggleSwitch.h>
#include <basewidgets/Inputs/ToolButton.h>

using namespace basewidgets;

InputsPage::InputsPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* scrollArea = new ScrollArea(this);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignTop);

    auto* content = new StyledWidget(scrollArea);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* contentLayout = new QGridLayout(content);
    contentLayout->setContentsMargins(6, 6, 6, 6);
    contentLayout->setSpacing(12);
    contentLayout->setColumnStretch(0, 1);
    contentLayout->setColumnStretch(1, 1);
    contentLayout->setRowStretch(0, 1);

    auto* leftPanel = new StyledWidget(content);
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);
    leftLayout->setAlignment(Qt::AlignTop);

    auto* buttonGroup = new GroupBox("", content);
    {
        auto* layout = new QGridLayout(buttonGroup);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setHorizontalSpacing(8);
        layout->setVerticalSpacing(4);
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 1);

        auto* pushBtn = new PushButton("普通按钮", buttonGroup);
        auto* checkableBtn = new PushButton("可选按钮", buttonGroup);
        checkableBtn->setCheckable(true);
        auto* disabledBtn = new PushButton("禁用按钮", buttonGroup);
        disabledBtn->setEnabled(false);
        auto* toolBtn = new ToolButton(buttonGroup);
        toolBtn->setText("工具按钮");

        pushBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        checkableBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        disabledBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        toolBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        layout->addWidget(pushBtn, 0, 0);
        layout->addWidget(checkableBtn, 0, 1);
        layout->addWidget(disabledBtn, 1, 0);
        layout->addWidget(toolBtn, 1, 1);
    }
    buttonGroup->setMinimumWidth(360);
    buttonGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(buttonGroup);

    auto* inputGroup = new GroupBox("", content);
    {
        auto* layout = new QGridLayout(inputGroup);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setHorizontalSpacing(8);
        layout->setVerticalSpacing(4);
        layout->setColumnStretch(1, 1);

        auto* lineEdit = new LineEdit(inputGroup);
        lineEdit->setPlaceholderText("请输入文本...");
        auto* spinBox = new SpinBox(inputGroup);
        spinBox->setRange(0, 100);
        spinBox->setValue(50);
        auto* slider = new Slider(Qt::Horizontal, inputGroup);
        slider->setRange(0, 100);
        slider->setValue(30);

        layout->addWidget(new Label("输入框:", inputGroup), 0, 0);
        layout->addWidget(lineEdit, 0, 1);
        layout->addWidget(new Label("数值框:", inputGroup), 1, 0);
        layout->addWidget(spinBox, 1, 1);
        layout->addWidget(new Label("滑块:", inputGroup), 2, 0);
        layout->addWidget(slider, 2, 1);
    }
    inputGroup->setMinimumWidth(360);
    inputGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(inputGroup);

    auto* selectGroup = new GroupBox("", content);
    {
        auto* layout = new QGridLayout(selectGroup);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setHorizontalSpacing(8);
        layout->setVerticalSpacing(4);
        layout->setColumnStretch(1, 1);
        layout->setColumnStretch(2, 1);

        auto* checkBox1 = new CheckBox("复选框 1", selectGroup);
        auto* checkBox2 = new CheckBox("复选框 2", selectGroup);
        auto* radioBtn1 = new RadioButton("单选按钮 1", selectGroup);
        auto* radioBtn2 = new RadioButton("单选按钮 2", selectGroup);
        auto* toggleSwitch = new ToggleSwitch(selectGroup);
        auto* combo = new ComboBox(selectGroup);
        combo->addItems({"选项 A", "选项 B", "选项 C"});

        layout->addWidget(new Label("复选框:", selectGroup), 0, 0);
        layout->addWidget(checkBox1, 0, 1);
        layout->addWidget(checkBox2, 0, 2);
        layout->addWidget(new Label("单选按钮:", selectGroup), 1, 0);
        layout->addWidget(radioBtn1, 1, 1);
        layout->addWidget(radioBtn2, 1, 2);
        layout->addWidget(new Label("开关控件:", selectGroup), 2, 0);
        layout->addWidget(toggleSwitch, 2, 1);
        layout->addWidget(new Label("下拉框:", selectGroup), 3, 0);
        layout->addWidget(combo, 3, 1, 1, 2);
    }
    selectGroup->setMinimumWidth(360);
    selectGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(selectGroup);

    auto* datetimeGroup = new GroupBox("", content);
    {
        auto* layout = new QGridLayout(datetimeGroup);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setHorizontalSpacing(8);
        layout->setVerticalSpacing(4);
        layout->setColumnStretch(1, 1);

        auto* datePicker = new DatePicker(datetimeGroup);
        auto* timePicker = new TimePicker(datetimeGroup);
        auto* dateTimePicker = new DateTimePicker(datetimeGroup);
        auto* rangePicker = new DateRangePicker(datetimeGroup);

        layout->addWidget(new Label("日期选择:", datetimeGroup), 0, 0);
        layout->addWidget(datePicker, 0, 1);
        layout->addWidget(new Label("时间选择:", datetimeGroup), 1, 0);
        layout->addWidget(timePicker, 1, 1);
        layout->addWidget(new Label("日期时间:", datetimeGroup), 2, 0);
        layout->addWidget(dateTimePicker, 2, 1);
        layout->addWidget(new Label("日期范围:", datetimeGroup), 3, 0);
        layout->addWidget(rangePicker, 3, 1);
    }
    datetimeGroup->setMinimumWidth(360);
    datetimeGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(datetimeGroup);

    auto* progressGroup = new GroupBox("", content);
    {
        auto* layout = new QVBoxLayout(progressGroup);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setSpacing(6);

        auto* progressBar1 = new ProgressBar(progressGroup);
        progressBar1->setValue(25);
        auto* progressBar2 = new ProgressBar(progressGroup);
        progressBar2->setValue(50);
        auto* progressBar3 = new ProgressBar(progressGroup);
        progressBar3->setValue(75);
        auto* busyIndicator = new BusyIndicator(progressGroup);
        busyIndicator->start();

        layout->addWidget(new Label("进度条:", progressGroup));
        layout->addWidget(progressBar1);
        layout->addWidget(progressBar2);
        layout->addWidget(progressBar3);
        layout->addWidget(new Label("忙碌指示器:", progressGroup));
        layout->addWidget(busyIndicator);
    }
    progressGroup->setMinimumWidth(360);
    progressGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(progressGroup);
    leftLayout->addStretch();

    auto* dialogsGroup = new GroupBox("", content);
    {
        auto* layout = new QGridLayout(dialogsGroup);
        layout->setHorizontalSpacing(10);
        layout->setVerticalSpacing(12);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 1);

        auto* fileResult = new Label("文件结果: ", dialogsGroup);
        auto* dirResult = new Label("目录结果: ", dialogsGroup);
        auto* colorResult = new Label("颜色结果: ", dialogsGroup);
        auto* fontResult = new Label("字体结果: 使用默认字体", dialogsGroup);
        auto* inputResult = new Label("输入结果: ", dialogsGroup);

        auto* openFileBtn = new PushButton("打开文件", dialogsGroup);
        auto* openFilesBtn = new PushButton("多个文件", dialogsGroup);
        auto* saveFileBtn = new PushButton("保存文件", dialogsGroup);
        auto* openDirBtn = new PushButton("打开目录", dialogsGroup);
        auto* pickColorBtn = new PushButton("选择颜色", dialogsGroup);
        auto* pickFontBtn = new PushButton("选择字体", dialogsGroup);
        auto* msgInfoBtn = new PushButton("信息消息", dialogsGroup);
        auto* msgWarnBtn = new PushButton("警告消息", dialogsGroup);
        auto* msgErrorBtn = new PushButton("错误消息", dialogsGroup);
        auto* msgSuccessBtn = new PushButton("成功消息", dialogsGroup);
        auto* askQuestionBtn = new PushButton("询问消息", dialogsGroup);
        auto* inputTextBtn = new PushButton("输入文本", dialogsGroup);
        auto* inputIntBtn = new PushButton("输入整数", dialogsGroup);
        auto* inputDoubleBtn = new PushButton("输入小数", dialogsGroup);

        const int dlgBtnH = 32;
        const int dlgBtnW = 96;
        for (auto* b : {openFileBtn, openFilesBtn, saveFileBtn, openDirBtn,
                        pickColorBtn, pickFontBtn,
                        msgInfoBtn, msgWarnBtn, msgErrorBtn, msgSuccessBtn, askQuestionBtn,
                        inputTextBtn, inputIntBtn, inputDoubleBtn}) {
            b->setFixedSize(dlgBtnW, dlgBtnH);
            b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        }

        layout->addWidget(openFileBtn,    0, 0);
        layout->addWidget(msgWarnBtn,     0, 1);
        layout->addWidget(openFilesBtn,   1, 0);
        layout->addWidget(msgErrorBtn,    1, 1);
        layout->addWidget(saveFileBtn,    2, 0);
        layout->addWidget(msgSuccessBtn,  2, 1);
        layout->addWidget(openDirBtn,     3, 0);
        layout->addWidget(askQuestionBtn, 3, 1);
        layout->addWidget(pickColorBtn,   4, 0);
        layout->addWidget(inputTextBtn,   4, 1);
        layout->addWidget(pickFontBtn,    5, 0);
        layout->addWidget(inputIntBtn,    5, 1);
        layout->addWidget(msgInfoBtn,     6, 0);
        layout->addWidget(inputDoubleBtn, 6, 1);

        fileResult->setWordWrap(true);
        dirResult->setWordWrap(true);
        colorResult->setWordWrap(true);
        fontResult->setWordWrap(true);
        inputResult->setWordWrap(true);
        layout->addWidget(fileResult,  7, 0, 1, 2);
        layout->addWidget(dirResult,   8, 0, 1, 2);
        layout->addWidget(colorResult, 9, 0, 1, 2);
        layout->addWidget(fontResult, 10, 0, 1, 2);
        layout->addWidget(inputResult, 11, 0, 1, 2);

        QObject::connect(openFileBtn, &PushButton::clicked, [this, fileResult]() {
            const auto path = DialogUtils::openFile(this, "选择文件");
            fileResult->setText("文件结果: " + (path.isEmpty() ? QString("<空>") : path));
        });
        QObject::connect(openFilesBtn, &PushButton::clicked, [this, fileResult]() {
            const auto paths = DialogUtils::openFiles(this, "选择多个文件");
            fileResult->setText("文件结果: " + (paths.isEmpty() ? QString("<空>") : paths.join("; ")));
        });
        QObject::connect(saveFileBtn, &PushButton::clicked, [this, fileResult]() {
            const auto path = DialogUtils::saveFile(this, "保存文件");
            fileResult->setText("文件结果: " + (path.isEmpty() ? QString("<空>") : path));
        });
        QObject::connect(openDirBtn, &PushButton::clicked, [this, dirResult]() {
            const auto path = DialogUtils::openDirectory(this, "选择目录");
            dirResult->setText("目录结果: " + (path.isEmpty() ? QString("<空>") : path));
        });
        QObject::connect(pickColorBtn, &PushButton::clicked, [this, colorResult]() {
            const auto c = DialogUtils::pickColor(this, QColor(), "选择颜色");
            colorResult->setText(QString("颜色结果: %1").arg(c.name()));
            colorResult->setStyleSheet(QString("background:%1; padding:6px;").arg(c.name()));
        });
        QObject::connect(pickFontBtn, &PushButton::clicked, [this, fontResult]() {
            bool ok = false;
            const auto f = DialogUtils::pickFont(this, fontResult->font(), &ok, "选择字体");
            if (ok) {
                fontResult->setText(QString("字体结果: %1, %2pt").arg(f.family()).arg(f.pointSize()));
                fontResult->setFont(f);
            }
        });
        QObject::connect(msgInfoBtn, &PushButton::clicked, [this]() {
            DialogUtils::messageInfo(this, "信息", "这是一个信息提示");
        });
        QObject::connect(msgWarnBtn, &PushButton::clicked, [this]() {
            DialogUtils::messageWarning(this, "警告", "这是一个警告提示");
        });
        QObject::connect(msgErrorBtn, &PushButton::clicked, [this]() {
            DialogUtils::messageError(this, "错误", "这是一个错误提示");
        });
        QObject::connect(msgSuccessBtn, &PushButton::clicked, [this]() {
            DialogUtils::messageSuccess(this, "成功", "这是一个成功提示");
        });
        QObject::connect(askQuestionBtn, &PushButton::clicked, [this, inputResult]() {
            const bool yes = DialogUtils::askQuestion(this, "询问", "是否继续操作？");
            inputResult->setText(QString("输入结果: 询问 = %1").arg(yes ? "是" : "否"));
        });
        QObject::connect(inputTextBtn, &PushButton::clicked, [this, inputResult]() {
            const auto t = DialogUtils::inputText(this, "输入文本", "请输入内容:");
            inputResult->setText("输入结果: 文本 = " + t);
        });
        QObject::connect(inputIntBtn, &PushButton::clicked, [this, inputResult]() {
            const int v = DialogUtils::inputInt(this, "输入整数", "请输入整数:", 5, 0, 100, 1);
            inputResult->setText(QString("输入结果: 整数 = %1").arg(v));
        });
        QObject::connect(inputDoubleBtn, &PushButton::clicked, [this, inputResult]() {
            const double v = DialogUtils::inputDouble(this, "输入小数", "请输入小数:", 1.2, 0.0, 10.0, 2);
            inputResult->setText(QString("输入结果: 小数 = %1").arg(v));
        });
    }
    dialogsGroup->setMinimumWidth(360);
    dialogsGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* rightPanel = new StyledWidget(content);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->addWidget(dialogsGroup);
    rightLayout->addStretch();

    leftPanel->setLayout(leftLayout);
    rightPanel->setLayout(rightLayout);
    contentLayout->addWidget(leftPanel, 0, 0, Qt::AlignTop);
    contentLayout->addWidget(rightPanel, 0, 1, Qt::AlignTop);

    content->setLayout(contentLayout);
    scrollArea->setWidget(content);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    setLayout(pageLayout);
}
