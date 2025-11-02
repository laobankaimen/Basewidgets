#include "DashboardPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/PushButton.h>
#include <QtAdvancedUI/Inputs/RadioButton.h>
#include <QtAdvancedUI/Inputs/CheckBox.h>
#include <QtAdvancedUI/Inputs/Slider.h>
#include <QtAdvancedUI/Inputs/ComboBox.h>
#include <QtAdvancedUI/Inputs/ToggleSwitch.h>
#include <QtAdvancedUI/Views/ListView.h>
#include <QtAdvancedUI/Views/CircularGauge.h>
#include <QtAdvancedUI/Containers/GroupBox.h>
#include <QtAdvancedUI/Containers/Frame.h>

#include <QSplitter>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QStyle>
#include <QApplication>
#include <QTimer>

using namespace QtAdvancedUI;

DashboardPage::DashboardPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* root = new QSplitter(Qt::Horizontal, this);
    root->setChildrenCollapsible(false);

    // 左侧竖向导航条（近似截图中的左栏）
    auto* leftNav = new StyledWidget(root);
    auto* leftLayout = new QVBoxLayout(leftNav);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(6);
    auto* logoBtn = new PushButton("", leftNav);
    logoBtn->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    logoBtn->setFixedSize(36, 36);
    leftLayout->addWidget(logoBtn, 0, Qt::AlignLeft);
    leftLayout->addSpacing(8);
    leftLayout->addWidget(new Label("PAGE", leftNav));
    leftLayout->addWidget(new Label("PAGE", leftNav));
    leftLayout->addWidget(new Label("PAGE", leftNav));
    leftLayout->addStretch();
    leftNav->setLayout(leftLayout);
    leftNav->setMinimumWidth(90);

    // 中央区：网格分组 - Buttons / Radio / Check / Inputs 等
    auto* center = new StyledWidget(root);
    auto* grid = new QGridLayout(center);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(10);

    // Buttons 组
    auto* gbButtons = new GroupBox("BUTTONS", center);
    auto* vButtons = new QVBoxLayout(gbButtons);
    auto* hRow1 = new QHBoxLayout();
    auto* btnPush = new PushButton("PUSHBUTTON", gbButtons);
    auto* btnChecked = new PushButton("CHECKED", gbButtons); btnChecked->setCheckable(true); btnChecked->setChecked(true);
    auto* btnDisabled = new PushButton("DISABLED", gbButtons); btnDisabled->setEnabled(false);
    hRow1->addWidget(btnPush);
    hRow1->addWidget(btnChecked);
    hRow1->addWidget(btnDisabled);
    vButtons->addLayout(hRow1);
    auto* hRow2 = new QHBoxLayout();
    auto* btnDisableChecked = new PushButton("DISABLE CHECKED", gbButtons);
    QObject::connect(btnDisableChecked, &PushButton::clicked, [btnChecked]{ btnChecked->setEnabled(false); });
    hRow2->addWidget(btnDisableChecked);
    hRow2->addStretch();
    vButtons->addLayout(hRow2);
    auto* hRow3 = new QHBoxLayout();
    auto* btnFlat = new PushButton("FLAT", gbButtons); btnFlat->setFlat(true);
    auto* btnFlatCheck = new PushButton("FLAT CHECKABLE", gbButtons); btnFlatCheck->setCheckable(true); btnFlatCheck->setFlat(true);
    hRow3->addWidget(btnFlat);
    hRow3->addWidget(btnFlatCheck);
    hRow3->addStretch();
    vButtons->addLayout(hRow3);
    gbButtons->setLayout(vButtons);

    // Radio 组
    auto* gbRadio = new GroupBox("RADIO", center);
    auto* vRadio = new QVBoxLayout(gbRadio);
    vRadio->addWidget(new RadioButton("RadioButton", gbRadio));
    vRadio->addWidget(new RadioButton("RadioButton", gbRadio));
    auto* rbDisabled = new RadioButton("RadioButton", gbRadio); rbDisabled->setEnabled(false);
    vRadio->addWidget(rbDisabled);
    gbRadio->setLayout(vRadio);

    // Check 组
    auto* gbCheck = new GroupBox("CHECK", center);
    auto* vCheck = new QVBoxLayout(gbCheck);
    vCheck->addWidget(new CheckBox("CheckBox", gbCheck));
    vCheck->addWidget(new CheckBox("CheckBox", gbCheck));
    auto* cbDisabled = new CheckBox("CheckBox", gbCheck); cbDisabled->setEnabled(false);
    vCheck->addWidget(cbDisabled);
    gbCheck->setLayout(vCheck);

    // Inputs 组（演示下拉与滑块）
    auto* gbInputs = new GroupBox("INPUTS", center);
    auto* vInputs = new QVBoxLayout(gbInputs);
    auto* combo = new ComboBox(gbInputs);
    combo->addItem("Material theme");
    combo->addItem("Material dark");
    combo->addItem("Material light");
    vInputs->addWidget(combo);
    auto* hSlider = new QHBoxLayout();
    auto* sliderLabel = new Label("50", gbInputs);
    auto* slider = new Slider(Qt::Horizontal, gbInputs); slider->setRange(0,100); slider->setValue(50);
    QObject::connect(slider, &Slider::valueChanged, [sliderLabel](int v){ sliderLabel->setText(QString::number(v)); });
    hSlider->addWidget(sliderLabel);
    hSlider->addWidget(slider, 1);
    vInputs->addLayout(hSlider);
    gbInputs->setLayout(vInputs);

    // Material Tree（改为 ListView 展示）
    auto* gbTree = new GroupBox("MATERIAL TREE", center);
    auto* vTree = new QVBoxLayout(gbTree);
    auto* matListView = new ListView(gbTree);
    auto* matListModel = new QStringListModel(QStringList{
        "Tree #1", "Tree #2", "Subitem #4", "Subitem #5"
    }, gbTree);
    matListView->setModel(matListModel);
    vTree->addWidget(matListView);
    gbTree->setLayout(vTree);

    // 垂直滑块演示
    auto* gbVSlider = new GroupBox("SLIDER", center);
    auto* vVSlider = new QVBoxLayout(gbVSlider);
    auto* vsl = new Slider(Qt::Vertical, gbVSlider); vsl->setRange(0,100); vsl->setValue(50);
    vVSlider->addWidget(vsl, 1, Qt::AlignHCenter);
    gbVSlider->setLayout(vVSlider);

    // 将分组放入网格（近似布局）
    grid->addWidget(gbButtons, 0, 0);
    grid->addWidget(gbInputs, 0, 1);
    grid->addWidget(gbRadio, 1, 0);
    grid->addWidget(gbCheck, 1, 1);
    grid->addWidget(gbTree, 2, 0);
    grid->addWidget(gbVSlider, 2, 1);

    // Switch 开关演示
    auto* gbSwitch = new GroupBox("SWITCH", center);
    auto* vSwitch = new QVBoxLayout(gbSwitch);
    auto* sw = new ToggleSwitch("启用", gbSwitch);
    auto* swState = new Label("OFF", gbSwitch);
    QObject::connect(sw, &ToggleSwitch::toggled, [swState](bool on){ swState->setText(on ? "ON" : "OFF"); });
    vSwitch->addWidget(sw);
    vSwitch->addWidget(swState);
    gbSwitch->setLayout(vSwitch);


    // CircularGauge 圆形仪表盘演示
    auto* gbDashboard = new GroupBox("GAUGE", center);
    auto* vDashboard = new QVBoxLayout(gbDashboard);
    auto* gauge = new CircularGauge(gbDashboard);
    gauge->setRange(0, 100);
    gauge->setUnit("%");
    gauge->setTitle("CPU 使用率");
    gauge->setValue(37);
    auto* hGaugeCtrl = new QHBoxLayout();
    auto* gaugeSlider = new Slider(Qt::Horizontal, gbDashboard); gaugeSlider->setRange(0,100); gaugeSlider->setValue(37);
    QObject::connect(gaugeSlider, &Slider::valueChanged, [gauge](int v){ gauge->setValue(v); });
    hGaugeCtrl->addWidget(new Label("调整", gbDashboard));
    hGaugeCtrl->addWidget(gaugeSlider, 1);
    vDashboard->addWidget(gauge);
    vDashboard->addLayout(hGaugeCtrl);
    gbDashboard->setLayout(vDashboard);

    grid->addWidget(gbSwitch, 3, 0);
    grid->addWidget(gbDashboard, 3, 1);
    center->setLayout(grid);

    // 右侧 Dock 区域：Top Dock 列表 + 文本说明
    auto* right = new StyledWidget(root);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(8, 8, 8, 8);
    rightLayout->setSpacing(10);
    auto* gbTopDock = new GroupBox("TOP DOCK", right);
    auto* vTopDock = new QVBoxLayout(gbTopDock);
    auto* topDockView = new ListView(gbTopDock);
    auto* topDockModel = new QStandardItemModel(gbTopDock);
    for (int i = 0; i < 8; ++i) {
        auto* item = new QStandardItem(QString("New Item"));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(i % 2 == 0 ? Qt::Checked : Qt::Unchecked);
        topDockModel->appendRow(item);
    }
    topDockView->setModel(topDockModel);
    vTopDock->addWidget(topDockView);
    gbTopDock->setLayout(vTopDock);
    auto* gbRightDoc = new GroupBox("RIGHT DOC", right);
    auto* vRightDoc = new QVBoxLayout(gbRightDoc);
    auto* text = new QTextEdit(gbRightDoc);
    text->setPlainText("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam.");
    vRightDoc->addWidget(text);
    gbRightDoc->setLayout(vRightDoc);
    rightLayout->addWidget(gbTopDock);
    rightLayout->addWidget(gbRightDoc, 1);
    right->setLayout(rightLayout);
    right->setMinimumWidth(260);

    root->addWidget(leftNav);
    root->addWidget(center);
    root->addWidget(right);
    root->setStretchFactor(0, 0);
    root->setStretchFactor(1, 1);
    root->setStretchFactor(2, 0);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(root);
    setLayout(pageLayout);
}