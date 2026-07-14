#include "TextPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>

#include <basewidgets/Containers/Frame.h>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/PlainTextEdit.h>
#include <basewidgets/Inputs/TextEdit.h>

using namespace basewidgets;

TextPage::TextPage(QWidget* parent)
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

    auto* textGroup = new GroupBox("", content);
    {
        auto* textLayout = new QGridLayout(textGroup);
        auto* textEdit = new TextEdit(textGroup);
        textEdit->setPlainText("这是一个富文本编辑器\n支持多种格式");
        textEdit->setMinimumHeight(160);
        auto* plainTextEdit = new PlainTextEdit(textGroup);
        plainTextEdit->setPlainText("这是一个纯文本编辑器\n适合代码编辑");
        plainTextEdit->setMinimumHeight(160);
        textLayout->addWidget(new Label("富文本编辑器:", textGroup), 0, 0);
        textLayout->addWidget(textEdit, 1, 0);
        textLayout->addWidget(new Label("纯文本编辑器:", textGroup), 0, 1);
        textLayout->addWidget(plainTextEdit, 1, 1);
        textGroup->setLayout(textLayout);
    }
    layout->addWidget(textGroup);

    auto* frameGroup = new GroupBox("", content);
    {
        auto* dialogLayout = new QVBoxLayout(frameGroup);
        auto* frameExample = new Frame(frameGroup);
        frameExample->setMinimumHeight(80);
        auto* frameLayout = new QVBoxLayout(frameExample);
        frameLayout->addWidget(new Label("这是一个 Frame 容器", frameExample));
        frameLayout->addWidget(new Label("可以包含其他控件", frameExample));
        frameExample->setLayout(frameLayout);
        dialogLayout->addWidget(new Label("Frame 容器示例:", frameGroup));
        dialogLayout->addWidget(frameExample);
        frameGroup->setLayout(dialogLayout);
    }
    layout->addWidget(frameGroup);

    content->setLayout(layout);
    scrollArea->setWidget(content);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    setLayout(pageLayout);
}

