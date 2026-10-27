// 输入控件示例：只演示公开 API，不注入样式表。
// 主程序先 initialize()；以下控件自动使用已安装的 NativeStyle 和明暗调色板。
#include "InputsPage.h"
#include <QFormLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/BusyIndicator.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Inputs/DatePicker.h>
#include <basewidgets/Inputs/DateRangePicker.h>
#include <basewidgets/Inputs/DateTimePicker.h>
#include <basewidgets/Inputs/ElidedLabel.h>
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

InputsPage::InputsPage(QWidget* parent) : StyledWidget(parent)
{
    auto* scroll = new ScrollArea(this);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    auto* content = new StyledWidget;
    // 320 是卡片的期望最小逻辑宽度，空间不足时自动换成单列；无需监听 resizeEvent。
    auto* cards = new ResponsiveGridLayout(content,320,2);
    auto makeCard = [&](const QString& title) {
        auto* card = new GroupBox(title,content);
        card->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        cards->addWidget(card);
        return card;
    };
    auto* buttons = makeCard("按钮与操作");
    auto* buttonLayout = new QGridLayout(buttons);
    buttonLayout->setColumnStretch(0,1);
    buttonLayout->setColumnStretch(1,1);
    auto* normal = new PushButton("普通按钮",buttons);
    auto* selected = new PushButton("选中按钮",buttons); selected->setCheckable(true); selected->setChecked(true);
    auto* primary = new PushButton("主要操作",buttons); primary->setAppearance(ButtonAppearance::Primary);
    auto* disabled = new PushButton("禁用按钮",buttons); disabled->setEnabled(false);
    // 外观层级与选中语义分别配置：Primary 是动作优先级，setChecked 是持续状态。
    auto* subtle = new PushButton("了解更多",buttons); subtle->setAppearance(ButtonAppearance::Subtle);
    auto* outline = new PushButton("显式描边",buttons); outline->setAppearance(ButtonAppearance::Outline);
    auto* danger = new PushButton("删除示例",buttons); danger->setAppearance(ButtonAppearance::Danger);
    danger->setToolTip("仅演示危险按钮外观，不执行删除操作");
    auto* tool = new ToolButton(buttons); tool->setText("工具按钮"); tool->setAppearance(ButtonAppearance::Subtle);
    // 图集采用等宽示例，避免 QToolButton 的默认 Fixed 策略压缩右列。
    // 实际业务页可按内容选择尺寸；视觉层级由 appearance 表达，不依赖按钮宽度。
    QAbstractButton* examples[] = {normal, selected, primary, disabled, subtle, outline, danger, tool};
    for (QAbstractButton* button : examples) {
        button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    }
    buttonLayout->addWidget(normal,0,0); buttonLayout->addWidget(selected,0,1);
    buttonLayout->addWidget(primary,1,0); buttonLayout->addWidget(disabled,1,1);
    buttonLayout->addWidget(subtle,2,0); buttonLayout->addWidget(outline,2,1);
    buttonLayout->addWidget(danger,3,0); buttonLayout->addWidget(tool,3,1);

    auto* editors = makeCard("编辑与数值");
    auto* form = new QFormLayout(editors); form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    auto* line = new LineEdit(editors); line->setPlaceholderText("输入文本…");
    auto* spin = new SpinBox(editors); spin->setRange(0,100); spin->setValue(50);
    auto* slider = new Slider(Qt::Horizontal,editors); slider->setRange(0,100); slider->setValue(50);
    slider->setTickPosition(QSlider::TicksBelow); slider->setTickInterval(20);
    // Qt setter 在数值未变化时不会重复发信号，双向绑定不会造成递归更新。
    connect(slider,&QSlider::valueChanged,spin,&QSpinBox::setValue);
    connect(spin,QOverload<int>::of(&QSpinBox::valueChanged),slider,&QSlider::setValue);
    form->addRow("输入框",line); form->addRow("数值",spin); form->addRow("滑块",slider);

    auto* choices = makeCard("选择与开关");
    auto* choiceLayout = new QGridLayout(choices);
    auto* checked = new CheckBox("选中",choices); checked->setChecked(true);
    // 半选不是 checked 的另一种颜色，必须使用三态 API。
    auto* partial = new CheckBox("半选",choices); partial->setCheckState(Qt::PartiallyChecked);
    auto* radioA = new RadioButton("选项 A",choices); radioA->setChecked(true);
    auto* radioB = new RadioButton("选项 B",choices);
    auto* toggle = new ToggleSwitch(choices);
    auto* combo = new ComboBox(choices); combo->addItems({"选项 A","选项 B","选项 C"});
    choiceLayout->addWidget(checked,0,0); choiceLayout->addWidget(partial,0,1);
    choiceLayout->addWidget(radioA,1,0); choiceLayout->addWidget(radioB,1,1);
    choiceLayout->addWidget(new Label("开关",choices),2,0); choiceLayout->addWidget(toggle,2,1);
    choiceLayout->addWidget(new Label("下拉选项",choices),3,0); choiceLayout->addWidget(combo,3,1);

    auto* dates = makeCard("日期与时间");
    auto* dateLayout = new QFormLayout(dates);
    dateLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    dateLayout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    dateLayout->addRow("日期",new DatePicker(dates));
    dateLayout->addRow("时间",new TimePicker(dates));
    dateLayout->addRow("日期时间",new DateTimePicker(dates));
    dateLayout->addRow("日期范围",new DateRangePicker(dates));

    auto* progress = makeCard("进度与状态");
    auto* progressLayout = new QVBoxLayout(progress);
    auto* value = new ProgressBar(progress); value->setValue(50);
    connect(slider,&QSlider::valueChanged,value,&QProgressBar::setValue);
    // range(0,0) 启动 Qt 管理的 busy 动画；BusyIndicator 在页面隐藏时自动暂停计时。
    auto* busy = new ProgressBar(progress); busy->setRange(0,0);
    auto* spinner = new BusyIndicator(progress); spinner->setFixedSize(28,28);
    progressLayout->addWidget(new Label("跟随数值 / 滑块",progress)); progressLayout->addWidget(value);
    progressLayout->addWidget(new Label("不确定进度",progress)); progressLayout->addWidget(busy);
    progressLayout->addWidget(spinner);

    auto* labels = makeCard("长文本与自适应布局");
    auto* labelLayout = new QVBoxLayout(labels);
    auto* title = new ElidedLabel("这是一段较长的单行标题：缩小窗口时自动省略，完整文字保留在提示中。",labels);
    title->setToolTip(title->text());
    auto* description = new Label("拖动窗口边缘：卡片会从两列切换为一列。字体增大时，按钮和输入框随字体尺寸调整。",labels);
    description->setWordWrap(true);
    labelLayout->addWidget(title); labelLayout->addWidget(description); labelLayout->addStretch();

    scroll->setWidget(content);
    auto* layout = new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0); layout->addWidget(scroll);
}
