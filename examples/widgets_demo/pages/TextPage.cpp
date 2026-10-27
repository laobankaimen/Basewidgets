// 文本与容器示例：QTextEdit 保留富文本能力，QPlainTextEdit 用于纯文本。
// 卡片采用 ResponsiveGridLayout，长单行文字采用 ElidedLabel；二者均不需要 QSS。
#include "TextPage.h"
#include <QVBoxLayout>
#include <basewidgets/Containers/Frame.h>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/ElidedLabel.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/PlainTextEdit.h>
#include <basewidgets/Inputs/TextEdit.h>
using namespace basewidgets;
TextPage::TextPage(QWidget* parent) : StyledWidget(parent)
{
    auto* scroll = new ScrollArea(this); scroll->setFrameShape(QFrame::NoFrame);
    auto* content = new StyledWidget;
    auto* cards = new ResponsiveGridLayout(content,300,2);
    auto makeCard = [&](const QString& title) {
        auto* group = new GroupBox(title,content); cards->addWidget(group); return new QVBoxLayout(group);
    };
    auto* richLayout = makeCard("富文本编辑");
    auto* rich = new TextEdit; rich->setHtml("<b>富文本编辑器</b><p>保留选择、复制、输入法和文档排版。</p>");
    rich->setMinimumHeight(200); richLayout->addWidget(rich);
    auto* plainLayout = makeCard("纯文本编辑");
    auto* plain = new PlainTextEdit; plain->setPlainText("纯文本编辑器\n适合日志和代码编辑");
    plain->setMinimumHeight(200); plainLayout->addWidget(plain);
    auto* frameLayout = makeCard("Frame 容器");
    auto* frame = new Frame; auto* body = new QVBoxLayout(frame);
    body->addWidget(new Label("容器边框由 NativeStyle 绘制"));
    auto* description = new Label("缩小窗口后，编辑器和容器会自动换行，不依赖固定宽度。");
    description->setWordWrap(true); body->addWidget(description); frameLayout->addWidget(frame);
    auto* titleLayout = makeCard("单行省略标题");
    auto* title = new ElidedLabel("这是一个可复用的 ElidedLabel，用于标题栏、卡片标题及受限宽度的文本。");
    title->setToolTip(title->text()); titleLayout->addWidget(title); titleLayout->addStretch();
    scroll->setWidget(content); auto* layout = new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0); layout->addWidget(scroll);
}
