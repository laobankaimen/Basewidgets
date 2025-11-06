#include "FormPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Inputs/SpinBox.h>
#include <QtAdvancedUI/Inputs/TextEdit.h>
#include <QtAdvancedUI/Inputs/PlainTextEdit.h>
#include <QtAdvancedUI/Inputs/MultiSelectComboBox.h>
#include <QtAdvancedUI/Containers/GroupBox.h>

#include <QGridLayout>
#include <QVBoxLayout>

using namespace QtAdvancedUI;

FormPage::FormPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* formGroup = new GroupBox("表单", this);
    auto* formLayout = new QGridLayout(formGroup);
    // 列伸缩：标签列不扩展，输入列按比例扩展
    formLayout->setColumnStretch(0, 0);
    formLayout->setColumnStretch(1, 1);

    int fr = 0;
    auto* nameLabel = new Label("名称：", formGroup);
    auto* nameEdit = new LineEdit(formGroup);
    nameEdit->setPlaceholderText("请输入内容...");
    formLayout->addWidget(nameLabel, fr, 0);
    formLayout->addWidget(nameEdit, fr, 1);
    fr++;

    auto* spinLabel = new Label("数量：", formGroup);
    auto* spin = new SpinBox(formGroup);
    spin->setRange(0, 100);
    spin->setValue(10);
    formLayout->addWidget(spinLabel, fr, 0);
    formLayout->addWidget(spin, fr, 1);
    fr++;

    auto* textLabel = new Label("备注：", formGroup);
    auto* textEdit = new TextEdit(formGroup);
    formLayout->addWidget(textLabel, fr, 0);
    formLayout->addWidget(textEdit, fr, 1);
    fr++;

    // 多选下拉
    auto* multiLabel = new Label("标签：", formGroup);
    auto* multiSelect = new MultiSelectComboBox(formGroup);
    multiSelect->setOptions({"后台", "前端", "移动端", "AI", "测试", "DevOps"});
    multiSelect->setSelectedOptions({"前端", "AI"});
    formLayout->addWidget(multiLabel, fr, 0);
    formLayout->addWidget(multiSelect, fr, 1);
    fr++;

    auto* plainLabel = new Label("日志：", formGroup);
    auto* plainEdit = new PlainTextEdit(formGroup);
    formLayout->addWidget(plainLabel, fr, 0);
    formLayout->addWidget(plainEdit, fr, 1);

    formGroup->setLayout(formLayout);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->addWidget(formGroup);
    // 页面底部加入弹簧，避免表单控件被垂直方向拉伸过长
    pageLayout->addStretch();
    setLayout(pageLayout);
}