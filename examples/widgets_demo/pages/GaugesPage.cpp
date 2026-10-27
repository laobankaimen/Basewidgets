// 仪表接入示例：数值、量程、单位、阈值文字和目标值相互独立；不用 QSS。
// 所有数字都是演示输入。自动数据只用于观察重绘，绝不由控件自己轮询业务数据。
#include "GaugesPage.h"
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/Slider.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Gauges/CircularGauge.h>
#include <basewidgets/Gauges/CompassGauge.h>
#include <basewidgets/Gauges/AttitudeMeter.h>
#include <QTimer>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QShowEvent>
#include <QHideEvent>
#include <QtMath>
using namespace basewidgets;

GaugesPage::GaugesPage(QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(12,12,12,12); outer->setSpacing(10);
    auto* introduction=new Label("数据优先 · 阈值与目标分层 · 明暗主题一致",this);
    introduction->setWordWrap(true); outer->addWidget(introduction);
    m_animate=new CheckBox("自动演示数据（离开本页自动暂停）",this); m_animate->setObjectName("GaugeDemoAnimate"); outer->addWidget(m_animate);
    auto* scroll=new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame); outer->addWidget(scroll,1);
    auto* content=new StyledWidget; auto* grid=new ResponsiveGridLayout(content,300,2);
    auto card=[&](const QString& heading,const QString& description) {
        auto* group=new GroupBox(heading,content); grid->addWidget(group);
        auto* layout=new QVBoxLayout(group); layout->setContentsMargins(12,18,12,12); layout->setSpacing(8);
        auto* help=new Label(description,group); help->setWordWrap(true); layout->addWidget(help);
        return layout;
    };
    auto addGauge=[](QVBoxLayout* layout,QWidget* gauge) {
        gauge->setMinimumHeight(280); gauge->setMaximumHeight(340); layout->addWidget(gauge);
    };
    auto addSlider=[](QVBoxLayout* layout,int low,int high,int value) {
        auto* slider=new Slider(Qt::Horizontal); slider->setRange(low,high); slider->setValue(value); layout->addWidget(slider); return slider;
    };
    auto* completionLayout=card("完成量 / 目标","完成率采用弧形和大号读数，不把正常进度误标成红色告警。");
    auto* completion=new CircularGauge;
    completion->useCompletionRatePreset("批次完成率","%"); completion->setValue(68); completion->setTargetValue(80);
    completion->setObjectName("DemoCompletionGauge"); addGauge(completionLayout,completion);
    auto* completionInput=addSlider(completionLayout,0,100,68);
    connect(completionInput,&QSlider::valueChanged,completion,[completion](int value) { completion->setValue(value); });

    auto* temperatureLayout=card("监控 / 阈值","外圈保留阈值区间；内圈显示当前量。告警状态同时显示文字。");
    auto* temperature=new CircularGauge;
    temperature->setTitle("设备温度"); temperature->setUnit("°C"); temperature->setRange(-20,120);
    temperature->setLabelPrecision(1); temperature->setMajorTickStep(20); temperature->setValue(76); temperature->setTargetValue(65);
    auto applyTemperatureRanges=[temperature] {
        auto* t=ThemeTokens::instance();
        temperature->setRanges({{-20,70,t->success(),5,"正常"},{70,90,t->warning(),5,"注意：接近上限"},{90,120,t->danger(),5,"过热"}});
    };
    applyTemperatureRanges(); connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,temperature,applyTemperatureRanges);
    addGauge(temperatureLayout,temperature); auto* temperatureInput=addSlider(temperatureLayout,-20,120,76);
    connect(temperatureInput,&QSlider::valueChanged,temperature,[temperature](int value) { temperature->setValue(value); });

    auto* speedLayout=card("测量 / 指针","速度、压力等读数量使用统一指针模式，不再维护多套速度表。");
    auto* speed=new CircularGauge; speed->setDisplayMode(CircularGauge::DisplayMode::Needle);
    speed->setTitle("电机转速"); speed->setUnit("rpm"); speed->setRange(0,8000); speed->setTickCount(4);
    speed->setRanges({{0,6000,ThemeTokens::instance()->success(),5,"工作区间"},{6000,7200,ThemeTokens::instance()->warning(),5,"高转速"},{7200,8000,ThemeTokens::instance()->danger(),5,"超限"}});
    speed->setTargetValue(6000); speed->setValue(3200); addGauge(speedLayout,speed);
    auto* speedInput=addSlider(speedLayout,0,8000,3200); connect(speedInput,&QSlider::valueChanged,speed,[speed](int value) { speed->setValue(value); });

    auto* headingLayout=card("方向 / 航向","北 0°，东 90°，顺时针增加；指针和数字使用同一约定。");
    auto* compass=new CompassGauge; compass->setHeading(72); addGauge(headingLayout,compass);
    auto* headingInput=addSlider(headingLayout,0,359,72); connect(headingInput,&QSlider::valueChanged,compass,[compass](int value) { compass->setHeading(value); });

    auto* attitudeLayout=card("姿态 / 俯仰与横滚","天空/地面按姿态移动，飞行器符号保持固定；数值始终保留单位。");
    auto* attitude=new AttitudeMeter; attitude->setAttitude(8,18); addGauge(attitudeLayout,attitude);
    auto* pitch=addSlider(attitudeLayout,-45,45,8); pitch->setToolTip("俯仰角：正值抬头");
    auto* roll=addSlider(attitudeLayout,-90,90,18); roll->setToolTip("横滚角：正值向右倾侧");
    connect(pitch,&QSlider::valueChanged,attitude,[attitude](int value) { attitude->setAttitude(value,attitude->roll()); });
    connect(roll,&QSlider::valueChanged,attitude,[attitude](int value) { attitude->setAttitude(attitude->pitch(),value); });

    auto* missingLayout=card("无数据 / 禁用","无数据不会伪装成 0；禁用控件保留读数，用低对比度表示不可操作。");
    auto* missing=new CircularGauge; missing->setTitle("传感器压力"); missing->setUnit("bar"); missing->setRange(0,16); missing->setValue(6.4); missing->setLabelPrecision(1); missing->setValueAvailable(false);
    addGauge(missingLayout,missing);
    auto* available=new CheckBox("数据可用"); missingLayout->addWidget(available);
    auto* enabled=new CheckBox("启用控件"); enabled->setChecked(true); missingLayout->addWidget(enabled);
    connect(available,&QCheckBox::toggled,missing,&CircularGauge::setValueAvailable);
    connect(enabled,&QCheckBox::toggled,missing,&QWidget::setEnabled);

    scroll->setWidget(content);
    m_timer=new QTimer(this); m_timer->setObjectName("GaugeDemoTimer"); m_timer->setInterval(80);
    connect(m_animate,&QCheckBox::toggled,this,[this](bool on) { if (on && isVisible()) m_timer->start(); else m_timer->stop(); });
    connect(m_timer,&QTimer::timeout,this,[completionInput,temperatureInput,speedInput,headingInput,pitch,roll,t=0.]() mutable {
        t+=.08;
        completionInput->setValue(qRound(50+45*qSin(t*.3)));
        temperatureInput->setValue(qRound(65+35*qSin(t*.5)));
        speedInput->setValue(qRound(4000+3500*qSin(t*.4)));
        headingInput->setValue(int(t*12)%360);
        pitch->setValue(qRound(18*qSin(t*.7))); roll->setValue(qRound(35*qSin(t*.5)));
    });
}
void GaugesPage::showEvent(QShowEvent* event) { StyledWidget::showEvent(event); if (m_animate->isChecked()) m_timer->start(); }
void GaugesPage::hideEvent(QHideEvent* event) { m_timer->stop(); StyledWidget::hideEvent(event); }
