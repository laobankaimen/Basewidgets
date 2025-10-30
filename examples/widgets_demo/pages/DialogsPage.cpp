#include "DialogsPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Inputs/FilePicker.h>
#include <QtAdvancedUI/Inputs/ColorPicker.h>
#include <QtAdvancedUI/Inputs/FontPicker.h>
#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QtAdvancedUI/Inputs/ToolButton.h>
#include <QtAdvancedUI/Containers/GroupBox.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>

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
    // 在按钮行末尾加入弹簧，避免按钮被横向拉伸
    dlgButtonsRow->addStretch();
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

    // 弹出菜单示例：增加工具按钮菜单、即时菜单与右键菜单
    auto* popupBox = new GroupBox("弹出菜单示例：", this);
    auto* popupRow = new QHBoxLayout(popupBox);
    // 1) 带菜单按钮（MenuButtonPopup）
    auto* tbMenu = new ToolButton(popupBox);
    tbMenu->setText("主菜单");
    tbMenu->setPopupMode(QToolButton::MenuButtonPopup);
    auto* menu1 = new QMenu(tbMenu);
    menu1->addAction("新建");
    menu1->addAction("打开");
    menu1->addAction("保存");
    tbMenu->setMenu(menu1);
    popupRow->addWidget(tbMenu);

    // 2) 即时弹出菜单（InstantPopup），包含子菜单与禁用项
    auto* tbMore = new ToolButton(popupBox);
    tbMore->setText("更多");
    tbMore->setPopupMode(QToolButton::InstantPopup);
    auto* menu2 = new QMenu(tbMore);
    auto* themeSub = menu2->addMenu("主题");
    themeSub->addAction("深色");
    themeSub->addAction("浅色");
    menu2->addSeparator();
    QAction* disabledAct = menu2->addAction("Menu disabled");
    disabledAct->setEnabled(false);
    tbMore->setMenu(menu2);
    popupRow->addWidget(tbMore);

    // 3) 普通按钮点击在其下方弹出菜单
    auto* btnPopup = new PushButton("按钮弹出菜单", popupBox);
    auto* menu3 = new QMenu(btnPopup);
    menu3->addAction("动作 A");
    menu3->addAction("动作 B");
    QObject::connect(btnPopup, &PushButton::clicked, [btnPopup, menu3]() {
        QPoint p = btnPopup->mapToGlobal(QPoint(0, btnPopup->height()));
        menu3->exec(p);
    });
    popupRow->addWidget(btnPopup);
    popupRow->addStretch();
    popupBox->setLayout(popupRow);
    layout->addWidget(popupBox);

    // 4) 右键菜单示例区域
    auto* ctxLabel = new Label("在此区域右键打开菜单", this);
    ctxLabel->setAlignment(Qt::AlignCenter);
    ctxLabel->setMinimumHeight(120);
    ctxLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ctxLabel, &Label::customContextMenuRequested, [ctxLabel](const QPoint& pos){
        QMenu m(ctxLabel);
        m.addAction("复制", []{});
        m.addAction("粘贴", []{});
        m.addSeparator();
        m.addAction("更多...", []{});
        m.exec(ctxLabel->mapToGlobal(pos));
    });
    layout->addWidget(ctxLabel);

    // 页面底部加入弹簧，将多余空间分配给弹簧以避免控件过度拉伸
    layout->addStretch();
    setLayout(layout);
}