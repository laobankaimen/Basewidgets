// 常用组件组合示例：通过公开信号连接业务，控件本身不执行导入/筛选/删除。
#include "CommonWidgetsPage.h"
#include <basewidgets/Inputs/SegmentedControl.h>
#include <basewidgets/Inputs/FilterChip.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Containers/MessageBanner.h>
#include <basewidgets/Containers/EmptyState.h>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <QVBoxLayout>

using namespace basewidgets;
CommonWidgetsPage::CommonWidgetsPage(QWidget* parent) : StyledWidget(parent)
{
    auto* scroll=new ScrollArea(this); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    auto* content=new StyledWidget; auto* layout=new QVBoxLayout(content); layout->setContentsMargins(18,18,18,18); layout->setSpacing(12);
    auto* title=new Label("筛选与反馈",content); QFont heading; heading.setBold(true); heading.setPointSize(15); title->setFont(heading);
    layout->addWidget(title);
    auto* intro=new Label("紧凑导航、可移除条件与就地反馈，适合录音检索、文件管理和任务页面。",content); intro->setWordWrap(true); layout->addWidget(intro);

    auto* banner=new MessageBanner(content); banner->setObjectName("CommonDemoBanner");
    banner->setTone(FeedbackTone::Info); banner->setTitle("数据目录已连接");
    banner->setMessage("示例数据已就绪，可调整视图与筛选条件。");
    banner->setActionText("查看示例说明"); layout->addWidget(banner);
    connect(banner,&MessageBanner::actionRequested,this,[banner] {
        banner->setMessage("分段切换使用 currentChanged；筛选标签使用 toggled 和 removeRequested。所有状态由业务层控制。");
    });

    auto* cards=new ResponsiveGridLayout(nullptr,320,2); cards->setContentsMargins(0,0,0,0); layout->addLayout(cards);
    auto* filters=new GroupBox("视图与筛选",content); auto* left=new QVBoxLayout(filters); left->setSpacing(14);
    auto* segments=new SegmentedControl(filters); segments->setObjectName("CommonDemoSegments"); segments->setAccessibleName("录音视图");
    segments->addTab("全部录音"); segments->setTabData(0,"all");
    segments->addTab("待核验"); segments->setTabData(1,"pending");
    segments->addTab("已归档"); segments->setTabData(2,"archived");
    left->addWidget(segments);
    auto* hint=new Label("勾选启用条件；点 × 移除。聚焦标签后可用 Space 切换、Delete 移除。",filters); hint->setWordWrap(true); left->addWidget(hint);
    auto* chipHost=new QWidget(filters); auto* chipLayout=new ResponsiveGridLayout(chipHost,140,3); chipLayout->setContentsMargins(0,0,0,0);
    QList<FilterChip*> chips;
    for(const QString& text:{QString("东海站点"),QString("WAV 格式"),QString("已核验")}) {
        auto* chip=new FilterChip(text,chipHost); chip->setRemovable(true); chip->setChecked(true); chip->setToolTip(text);
        chipLayout->addWidget(chip); chips.append(chip);
    }
    auto refreshSupportingText=[intro,hint] {
        auto* tokens=ThemeTokens::instance();
        for(auto* label:{intro,hint}) {
            auto palette=label->palette(); palette.setColor(QPalette::WindowText,tokens->textSecondary());
            palette.setColor(QPalette::Disabled,QPalette::WindowText,tokens->textDisabled()); label->setPalette(palette);
        }
    };
    refreshSupportingText(); connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,refreshSupportingText);
    chips.last()->setTone(FeedbackTone::Success); left->addWidget(chipHost);
    auto* summary=new Label(filters); summary->setObjectName("CommonDemoSummary"); summary->setWordWrap(true); left->addWidget(summary); left->addStretch();
    auto updateSummary=[segments,chips,summary] {
        int active=0; for(auto* chip:chips)if(!chip->isHidden()&&chip->isChecked())++active;
        summary->setText(QString("当前视图：%1\n已启用 %2 项筛选条件").arg(segments->tabText(segments->currentIndex())).arg(active));
    };
    connect(segments,&QTabBar::currentChanged,this,[updateSummary](int) { updateSummary(); });
    for(auto* chip:chips) {
        connect(chip,&QAbstractButton::toggled,this,[updateSummary](bool) { updateSummary(); });
        // 控件只请求移除。示例隐藏并保留对象，方便恢复；真实应用应同步模型。
        connect(chip,&FilterChip::removeRequested,this,[chip,segments,updateSummary] { chip->hide(); segments->setFocus(Qt::OtherFocusReason); updateSummary(); });
    }
    auto* reset=new PushButton("恢复示例",filters); reset->setAppearance(ButtonAppearance::Subtle); left->addWidget(reset,0,Qt::AlignLeading);
    connect(reset,&QPushButton::clicked,this,[chips,banner,segments,updateSummary] {
        for(auto* chip:chips) { chip->show(); chip->setChecked(true); }
        segments->setCurrentIndex(0); banner->setTone(FeedbackTone::Info); banner->setTitle("数据目录已连接");
        banner->setMessage("可以继续调整筛选条件，所有数据均为合成示例。"); banner->show(); updateSummary();
    });
    updateSummary(); cards->addWidget(filters);

    auto* emptyCard=new GroupBox("空状态与下一步",content); auto* right=new QVBoxLayout(emptyCard);
    auto* empty=new EmptyState(emptyCard); empty->setObjectName("CommonDemoEmptyState");
    empty->setTitle("没有符合条件的录音"); empty->setDescription("请清除筛选条件，或选择其他数据目录。");
    empty->setActionText("清除筛选"); empty->setSecondaryActionText("选择数据目录"); right->addWidget(empty);
    connect(empty,&EmptyState::actionRequested,this,[chips,banner,updateSummary] {
        for(auto* chip:chips)chip->setChecked(false);
        banner->setTone(FeedbackTone::Success); banner->setTitle("已清除筛选条件");
        banner->setMessage("示例中的所有条件已取消选中；实际应用在此重新查询录音列表。"); banner->show(); updateSummary();
    });
    connect(empty,&EmptyState::secondaryActionRequested,this,[banner] {
        banner->setTone(FeedbackTone::Warning); banner->setTitle("尚未选择数据目录");
        banner->setMessage("此处接入业务的目录选择与扫描逻辑；示例不访问或导入实际文件。"); banner->show();
    });
    cards->addWidget(emptyCard); layout->addStretch(); scroll->setWidget(content);
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->addWidget(scroll);
}
