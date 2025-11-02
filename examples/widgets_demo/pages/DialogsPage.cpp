#include "DialogsPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QDir>
#include <QFrame>
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

    // 文件选择示例：直接调用 DialogUtils
    layout->addWidget(new Label("文件选择器：", this));
    {
        auto* row = new QHBoxLayout();
        auto* btnOpen = new PushButton("打开文件", this);
        auto* btnOpenMulti = new PushButton("打开多个", this);
        auto* btnSave = new PushButton("保存文件", this);
        auto* btnDir = new PushButton("选择文件夹", this);
        auto* result = new Label("结果: ", this);
        result->setMinimumWidth(300);
        row->addWidget(btnOpen);
        row->addWidget(btnOpenMulti);
        row->addWidget(btnSave);
        row->addWidget(btnDir);
        row->addWidget(result, 1);
        layout->addLayout(row);

        QObject::connect(btnOpen, &PushButton::clicked, [this, result]() {
            const QString f = DialogUtils::openFile(this->window(), "选择文件", QDir::homePath(), "所有文件 (*.*);;文本文件 (*.txt);;图片 (*.png *.jpg)");
            if (!f.isEmpty()) result->setText("结果: " + f);
        });
        QObject::connect(btnOpenMulti, &PushButton::clicked, [this, result]() {
            const QStringList fs = DialogUtils::openFiles(this->window(), "选择多个文件", QDir::homePath(), "所有文件 (*.*);;文本文件 (*.txt);;图片 (*.png *.jpg)");
            if (!fs.isEmpty()) result->setText("结果: " + fs.join(", "));
        });
        QObject::connect(btnSave, &PushButton::clicked, [this, result]() {
            const QString f = DialogUtils::saveFile(this->window(), "保存文件", QDir::homePath(), "文本文件 (*.txt);;图片 (*.png *.jpg)");
            if (!f.isEmpty()) result->setText("结果: " + f);
        });
        QObject::connect(btnDir, &PushButton::clicked, [this, result]() {
            const QString d = DialogUtils::openDirectory(this->window(), "选择文件夹", QDir::homePath());
            if (!d.isEmpty()) result->setText("结果: " + d);
        });
    }

    // 颜色选择示例：直接调用 DialogUtils
    layout->addWidget(new Label("颜色选择器：", this));
    {
        auto* row = new QHBoxLayout();
        auto* btnColor = new PushButton("选择颜色...", this);
        auto* preview = new Label(" ", this);
        preview->setFixedSize(64, 24);
        preview->setFrameShape(QFrame::Box);
        row->addWidget(btnColor);
        row->addWidget(preview);
        row->addStretch();
        layout->addLayout(row);

        QColor lastColor = QColor(66, 133, 244);
        auto setPreview = [&](const QColor& c){
            QPalette pal = preview->palette();
            pal.setColor(QPalette::Window, c);
            preview->setAutoFillBackground(true);
            preview->setPalette(pal);
        };
        setPreview(lastColor);
        QObject::connect(btnColor, &PushButton::clicked, [this, lastColor, setPreview]() mutable {
            QColor c = DialogUtils::pickColor(this->window(), lastColor, "选择颜色");
            if (c.isValid()) { lastColor = c; setPreview(c); }
        });
    }

    // 字体选择示例：直接调用 DialogUtils
    layout->addWidget(new Label("字体选择器：", this));
    {
        auto* row = new QHBoxLayout();
        auto* btnFont = new PushButton("选择字体...", this);
        auto* preview = new Label("示例文本 AaBbCc 123", this);
        preview->setMinimumHeight(40);
        row->addWidget(preview, 1);
        row->addWidget(btnFont);
        layout->addLayout(row);

        static QFont lastFont = QFont("Microsoft YaHei", 12);
        preview->setFont(lastFont);
        QObject::connect(btnFont, &PushButton::clicked, [this, preview]() {
            bool ok = false;
            QFont f = DialogUtils::pickFont(this->window(), preview->font(), &ok, "选择字体");
            if (ok) preview->setFont(f);
        });
    }

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