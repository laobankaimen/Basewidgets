#include "DialogsPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Inputs/FilePicker.h>
#include <QtAdvancedUI/Inputs/ColorPicker.h>
#include <QtAdvancedUI/Inputs/FontPicker.h>
#include <QtAdvancedUI/Dialogs/DialogUtils.h>

#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace QtAdvancedUI;

DialogsPage::DialogsPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* filePicker = new FilePicker(this);
    filePicker->setTitle("选择文件");
    filePicker->setFilter("所有文件 (*.*);;文本文件 (*.txt);;图片 (*.png *.jpg)");
    filePicker->setMode(FilePicker::OpenFile);
    layout->addWidget(new Label("文件选择器：", this));
    layout->addWidget(filePicker);

    layout->addWidget(new Label("颜色选择器：", this));
    auto* colorPicker = new ColorPicker(this);
    layout->addWidget(colorPicker);

    layout->addWidget(new Label("字体选择器：", this));
    auto* fontPicker = new FontPicker(this);
    layout->addWidget(fontPicker);

    auto* dlgButtonsRow = new QHBoxLayout();
    auto* btnInfo = new PushButton("信息", this);
    auto* btnWarn = new PushButton("警告", this);
    auto* btnError = new PushButton("错误", this);
    auto* btnAsk = new PushButton("提问", this);
    auto* btnText = new PushButton("输入文本", this);
    auto* btnInt = new PushButton("输入整数", this);
    auto* btnDouble = new PushButton("输入小数", this);
    dlgButtonsRow->addWidget(btnInfo);
    dlgButtonsRow->addWidget(btnWarn);
    dlgButtonsRow->addWidget(btnError);
    dlgButtonsRow->addWidget(btnAsk);
    dlgButtonsRow->addWidget(btnText);
    dlgButtonsRow->addWidget(btnInt);
    dlgButtonsRow->addWidget(btnDouble);
    layout->addLayout(dlgButtonsRow);

    QWidget* topWindow = this->window();

    QObject::connect(btnInfo, &PushButton::clicked, [topWindow]() {
        DialogUtils::messageInfo(topWindow, "信息", "这是一条信息提示");
    });
    QObject::connect(btnWarn, &PushButton::clicked, [topWindow]() {
        DialogUtils::messageWarning(topWindow, "警告", "这是一条警告提示");
    });
    QObject::connect(btnError, &PushButton::clicked, [topWindow]() {
        DialogUtils::messageError(topWindow, "错误", "这是一条错误提示");
    });
    QObject::connect(btnAsk, &PushButton::clicked, [topWindow]() {
        bool ok = DialogUtils::askQuestion(topWindow, "提问", "要继续吗？");
        Q_UNUSED(ok);
    });

    QObject::connect(btnText, &PushButton::clicked, [topWindow]() {
        QString text = DialogUtils::inputText(topWindow, "输入文本", "请输入内容：", "默认值");
        Q_UNUSED(text);
    });
    QObject::connect(btnInt, &PushButton::clicked, [topWindow]() {
        int val = DialogUtils::inputInt(topWindow, "输入整数", "请输入数字：", 5, 0, 100, 1);
        Q_UNUSED(val);
    });
    QObject::connect(btnDouble, &PushButton::clicked, [topWindow]() {
        double val = DialogUtils::inputDouble(topWindow, "输入小数", "请输入浮点数：", 3.14, -100.0, 100.0, 2);
        Q_UNUSED(val);
    });

    setLayout(layout);
}