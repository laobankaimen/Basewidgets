#include "ToolBoxPage.h"

#include <QtAdvancedUI/Containers/ToolBox.h>
#include <QtAdvancedUI/Inputs/Label.h>

#include <QVBoxLayout>

using namespace QtAdvancedUI;

ToolBoxPage::ToolBoxPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* toolbox = new ToolBox(this);

    auto* pageA = new StyledWidget(toolbox);
    auto* layA = new QVBoxLayout(pageA);
    layA->addWidget(new Label("这是工具箱页面 A", pageA));
    pageA->setLayout(layA);

    auto* pageB = new StyledWidget(toolbox);
    auto* layB = new QVBoxLayout(pageB);
    layB->addWidget(new Label("这是工具箱页面 B", pageB));
    pageB->setLayout(layB);

    toolbox->addItem(pageA, "页面 A");
    toolbox->addItem(pageB, "页面 B");

    layout->addWidget(toolbox);
    // 页面底部加入弹簧，避免工具箱控件纵向被过度拉伸
    layout->addStretch();
    setLayout(layout);
}