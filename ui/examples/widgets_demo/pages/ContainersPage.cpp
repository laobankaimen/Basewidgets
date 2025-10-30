#include "ContainersPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/Slider.h>
#include <QtAdvancedUI/Inputs/ProgressBar.h>
#include <QtAdvancedUI/Containers/Frame.h>
#include <QtAdvancedUI/Containers/ScrollArea.h>

#include <QGridLayout>
#include <QVBoxLayout>

using namespace QtAdvancedUI;

ContainersPage::ContainersPage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* panel = new Frame(this);
    auto* panelLayout = new QGridLayout(panel);

    auto* sliderLabel = new Label("滑块：50", panel);
    auto* slider = new Slider(Qt::Horizontal, panel);
    slider->setRange(0, 100);
    slider->setValue(50);
    QObject::connect(slider, &Slider::valueChanged, [sliderLabel](int v){
        sliderLabel->setText(QString("滑块：%1").arg(v));
    });

    panelLayout->addWidget(sliderLabel, 0, 0);
    panelLayout->addWidget(slider, 0, 1);

    auto* progress = new ProgressBar(panel);
    progress->setRange(0, 100);
    progress->setValue(30);
    panelLayout->addWidget(progress, 1, 0, 1, 2);
    panel->setLayout(panelLayout);

    auto* scrollArea = new ScrollArea(this);
    auto* scrollContent = new StyledWidget(scrollArea);
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    for (int i = 1; i <= 15; ++i) {
        scrollLayout->addWidget(new Label(QString("项目 %1").arg(i), scrollContent));
    }
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->addWidget(panel);
    pageLayout->addWidget(scrollArea);
    setLayout(pageLayout);
}