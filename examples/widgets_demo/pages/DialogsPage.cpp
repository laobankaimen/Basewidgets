// 对话框示例：DialogUtils 管理弹窗生命周期；每次调用均以当前页面作为父窗口。
// 取消结果通过空值/ok 标志区分；颜色预览通过 QPalette 更新。
#include "DialogsPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QMenu>
#include <QVBoxLayout>

#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Dialogs/DialogUtils.h>
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Inputs/PushButton.h>

using namespace basewidgets;

DialogsPage::DialogsPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* scrollArea = new ScrollArea(this);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto* content = new StyledWidget(scrollArea);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop);

    auto* simpleTitle=new PushButton("简洁弹窗 · 居中标题",content);
    layout->addWidget(simpleTitle);
    connect(simpleTitle,&QPushButton::clicked,this,[this] {
        auto* dialog=new FramelessDialog(this); dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setTitle("录音详情");
        // 弹窗默认 Dialog profile：无搜索/头像/设置，只有关闭；也支持独立图标和对齐。
        dialog->titleBar()->setTitleAlignment(CustomTitleBar::TitleAlignment::Center);
        dialog->setContentWidget(new Label("演示简洁弹窗标题栏。标题位置与主窗口分别配置。",dialog));
        connect(dialog,&FramelessDialog::okButtonClicked,dialog,&QDialog::accept);
        connect(dialog,&FramelessDialog::cancelButtonClicked,dialog,&QDialog::reject);
        dialog->resize(520,260); dialog->open();
    });

    auto* dialogsGroup = new GroupBox("文件、消息与输入对话框", content);
    auto* dialogsLayout = new QGridLayout(dialogsGroup);
    dialogsLayout->setHorizontalSpacing(10);
    dialogsLayout->setVerticalSpacing(20);
    dialogsLayout->setContentsMargins(6, 6, 6, 6);
    dialogsLayout->setColumnStretch(0, 1);
    dialogsLayout->setColumnStretch(1, 1);

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

    for (auto* b : {openFileBtn, openFilesBtn, saveFileBtn, openDirBtn,
                    pickColorBtn, pickFontBtn,
                    msgInfoBtn, msgWarnBtn, msgErrorBtn, msgSuccessBtn, askQuestionBtn,
                    inputTextBtn, inputIntBtn, inputDoubleBtn}) {
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

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
        if (!c.isValid()) return;
        colorResult->setText(QString("颜色结果: %1").arg(c.name()));
        QPalette palette = colorResult->palette();
        palette.setColor(QPalette::Window, c);
        palette.setColor(QPalette::WindowText, c.lightness() > 128 ? Qt::black : Qt::white);
        colorResult->setPalette(palette);
        colorResult->setAutoFillBackground(true);
        colorResult->setMargin(6);
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

    // 可见的实机验证入口：系统合成器负责拖动，检查拖动结束后 hover/click/键盘是否仍有效。
    auto* dragDemo=new PushButton("拖动与悬停验证",content);
    layout->addWidget(dragDemo);
    connect(dragDemo,&QPushButton::clicked,this,[this] {
        FramelessDialog dialog(true,this);
        dialog.setTitle("对话框交互验证");
        auto* body=new QWidget;
        auto* bodyLayout=new QVBoxLayout(body);
        auto* hint=new Label("拖动标题栏并松开鼠标，再移动到按钮上：底色应变化，点击应更新计数；Tab 可移动焦点。",body);
        hint->setWordWrap(true);
        auto* probe=new PushButton("悬停 / 点击此按钮",body);
        auto* count=new Label("点击次数：0",body);
        bodyLayout->addWidget(hint); bodyLayout->addWidget(probe); bodyLayout->addWidget(count);
        connect(probe,&QPushButton::clicked,count,[count,n=0]() mutable { count->setText(QString("点击次数：%1").arg(++n)); });
        dialog.setContentWidget(body);
        connect(&dialog,&FramelessDialog::okButtonClicked,&dialog,&QDialog::accept);
        connect(&dialog,&FramelessDialog::cancelButtonClicked,&dialog,&QDialog::reject);
        dialog.exec();
    });
    layout->addWidget(dialogsGroup);

    scrollArea->setWidget(content);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);

}

