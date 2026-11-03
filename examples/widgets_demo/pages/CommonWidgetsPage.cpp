#include <basewidgets/Containers/LoginWidget.h>
// 常用组件组合示例：通过公开信号连接业务，控件本身不执行导入/筛选/删除。
#include "CommonWidgetsPage.h"
#include "../DemoAbout.h"
#include <basewidgets/Views/NotificationModel.h>
#include <basewidgets/Inputs/NotificationBell.h>
#include <basewidgets/Containers/NotificationPanel.h>
#include <basewidgets/Dialogs/NotificationPopup.h>
#include <basewidgets/Dialogs/AboutDialog.h>
#include <basewidgets/Inputs/SegmentedControl.h>
#include <basewidgets/Inputs/FilterChip.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/StatusBadge.h>
#include <basewidgets/Inputs/SearchEdit.h>
#include <basewidgets/Inputs/CopyField.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Inputs/SpinBox.h>
#include <basewidgets/Containers/CollapsibleSection.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Containers/MessageBanner.h>
#include <basewidgets/Containers/EmptyState.h>
#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFormLayout>
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Views/ListView.h>
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Views/TreeTable.h>

using namespace basewidgets;
namespace {
// 小控件按实际业务接线：不访问磁盘/设备，不在输入事件里执行耗时查询。
void addUtilityExamples(ResponsiveGridLayout* cards,QWidget* content)
{
    auto card=[&](const QString& title) {
        auto* box=new GroupBox(title,content); auto* layout=new QVBoxLayout(box);
        layout->setContentsMargins(8,6,8,8); layout->setSpacing(8); cards->addWidget(box); return layout;
    };
    auto* loginLayout=card("登录页面 · 接线示例");
    auto* login=new LoginWidget(content); loginLayout->addWidget(login);
    QObject::connect(login,&LoginWidget::authenticationRequested,login,[login](const QString&,const QString&) {
        login->clearPassword(); login->setError("此处为界面演示，尚未连接认证服务。");
    });
    auto* lookup=card("搜索与复制");
    auto* search=new SearchEdit(content); search->setObjectName("CommonDemoSearch"); search->setPlaceholderText("筛选下方任务列表，如 A");
    search->setDebounceInterval(300); lookup->addWidget(search);
    auto* result=new Label("输入后自动筛选；Enter 立即提交。",content); result->setObjectName("CommonDemoSearchResult"); result->setWordWrap(true); lookup->addWidget(result);
    auto* copy=new CopyField("REC-2026-1028-001",content); copy->setObjectName("CommonDemoCopy"); copy->textEdit()->setAccessibleName("示例录音编号"); lookup->addWidget(copy);
    auto* copied=new Label("录音编号可选择，也可一键复制。",content); copied->setObjectName("CommonDemoCopyResult"); copied->setWordWrap(true); lookup->addWidget(copied); lookup->addStretch();
    QObject::connect(copy,&CopyField::copied,copied,[copied](const QString&) { copied->setText("录音编号已复制到剪贴板。"); });
    // 列表在下面的分组创建，按 QObject 名称定位接线，模型归视图持有。
    QObject::connect(search,&SearchEdit::searchRequested,content,[content,result](const QString& text) {
        auto* view=content->findChild<ListView*>("CommonDemoList"); if(!view)return;
        auto* proxy=qobject_cast<QSortFilterProxyModel*>(view->model());
        if(!proxy) {
            proxy=new QSortFilterProxyModel(view); proxy->setSourceModel(view->model());
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive); proxy->setFilterKeyColumn(0); view->setModel(proxy);
        }
        proxy->setFilterFixedString(text);
        result->setText(text.isEmpty()?QString("已显示全部 %1 项任务").arg(proxy->rowCount()):QString("匹配 %1 项任务").arg(proxy->rowCount()));
    });

    auto* states=card("状态徽标");
    auto* host=new QWidget(content); auto* grid=new ResponsiveGridLayout(host,110,2); grid->setContentsMargins(0,0,0,0); grid->setSpacing(6);
    const QStringList labels{"采集在线","已核验","等待连接","校验失败"};
    const QList<FeedbackTone> tones{FeedbackTone::Info,FeedbackTone::Success,FeedbackTone::Warning,FeedbackTone::Danger};
    for(int i=0;i<labels.size();++i) { auto* badge=new StatusBadge(labels[i],host); badge->setTone(tones[i]); grid->addWidget(badge); }
    states->addWidget(host);
    auto* connection=new StatusBadge("设备在线",content); connection->setObjectName("CommonDemoConnectionStatus"); connection->setTone(FeedbackTone::Success); states->addWidget(connection);
    auto* status=new ComboBox(content); status->setObjectName("CommonDemoConnectionChoice"); status->addItems({"设备在线","连接中","设备离线"}); status->setAccessibleName("模拟设备状态"); states->addWidget(status); states->addStretch();
    QObject::connect(status,QOverload<int>::of(&QComboBox::currentIndexChanged),connection,[connection,status](int index) {
        connection->setText(status->itemText(index)); connection->setTone(index==0?FeedbackTone::Success:index==1?FeedbackTone::Warning:FeedbackTone::Neutral);
    });

    auto* advanced=card("参数收纳");
    auto* section=new CollapsibleSection("高级采集参数",content); section->setObjectName("CommonDemoAdvancedSection");
    auto* form=new QFormLayout; form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    auto* overlap=new SpinBox(section->contentWidget()); overlap->setObjectName("CommonDemoOverlap"); overlap->setRange(0,90); overlap->setValue(50); overlap->setSuffix(" %"); overlap->setAccessibleName("帧重叠比例");
    auto* window=new ComboBox(section->contentWidget()); window->addItems({"Hann","矩形窗"}); window->setAccessibleName("分析窗函数");
    form->addRow("重叠",overlap); form->addRow("窗函数",window); section->contentLayout()->addLayout(form); advanced->addWidget(section);
    auto* details=new CollapsibleSection("连接详情",content); details->setObjectName("CommonDemoConnectionSection");
    auto* address=new CopyField("127.0.0.1:9000",details->contentWidget()); address->textEdit()->setAccessibleName("演示端点地址"); details->contentLayout()->addWidget(address); details->setExpanded(false); advanced->addWidget(details);
    auto* hint=new Label("折叠保留参数；Space / Enter 展开。",content); hint->setWordWrap(true); advanced->addWidget(hint); advanced->addStretch();
}
void addMessageAndAboutExamples(ResponsiveGridLayout* cards,QWidget* content)
{
    auto* messages=new GroupBox("消息铃铛与面板",content); auto* layout=new QVBoxLayout(messages); layout->setContentsMargins(8,6,8,8); layout->setSpacing(8);
    auto* model=new NotificationModel(content); model->setObjectName("CommonDemoNotifications"); model->setMaximumCount(200);
    // 稳定 ID 避免重复推送造成重复条目；时间为明确的模拟快照，不接入真实服务。
    model->addMessage({"import-001","录音导入完成","已导入 12 个录音文件，可到数据目录查看。",QDateTime::fromString("2026-10-28T09:30:00+08:00",Qt::ISODate),FeedbackTone::Success,false,"查看录音"});
    model->addMessage({"storage-001","存储容量提醒","实时存储集群使用率达到预警阈值，请检查归档策略。",QDateTime::fromString("2026-10-28T09:45:00+08:00",Qt::ISODate),FeedbackTone::Warning,false,"查看存储"});
    auto* top=new QHBoxLayout; auto* bell=new NotificationBell(messages); bell->setObjectName("CommonDemoBell"); bell->setModel(model); top->addWidget(bell);
    auto* summary=new Label(messages); summary->setObjectName("CommonDemoNotificationCount"); summary->setWordWrap(true); top->addWidget(summary,1); layout->addLayout(top);
    auto refresh=[summary,model] { summary->setText(QString("%1 条消息 · %2 条未读").arg(model->rowCount()).arg(model->unreadCount())); };
    refresh(); QObject::connect(model,&NotificationModel::unreadCountChanged,summary,[refresh](int) { refresh(); }); QObject::connect(model,&NotificationModel::countChanged,summary,[refresh](int) { refresh(); });
    auto* popup=new NotificationPopup(messages); popup->setObjectName("CommonDemoNotificationPopup"); popup->setModel(model);
    QObject::connect(bell,&QToolButton::clicked,popup,[popup,bell] { if(popup->isVisible())popup->hide(); else popup->popup(bell); });
    auto* add=new PushButton("模拟收到消息",messages); add->setObjectName("CommonDemoAddMessage"); layout->addWidget(add);
    QObject::connect(add,&QPushButton::clicked,model,[model,count=0]() mutable {
        ++count; model->addMessage({QString("demo-%1").arg(count),QString("分析任务 %1 已完成").arg(count),"任务结果已就绪。这里可以连接应用自己的任务列表或消息服务。",QDateTime::currentDateTime(),FeedbackTone::Info,false,"查看任务"});
    });
    auto* read=new PushButton("全部标为已读",messages); read->setAppearance(ButtonAppearance::Subtle); layout->addWidget(read); QObject::connect(read,&QPushButton::clicked,model,&NotificationModel::markAllRead);
    auto* hint=new Label("打开铃铛查看详情；标题栏与此处共享未读状态。",messages); hint->setWordWrap(true); layout->addWidget(hint); layout->addStretch();
    QObject::connect(popup->panel(),&NotificationPanel::actionRequested,popup,[popup,hint](const QString& id) { hint->setText("业务动作："+id+"（此处接入页面跳转）"); popup->hide(); }); cards->addWidget(messages);

    auto* about=new GroupBox("可配置关于页面",content); auto* form=new QVBoxLayout(about); form->setContentsMargins(8,6,8,8); form->setSpacing(8);
    auto* name=new LineEdit("水声数据管理中心",about); name->setObjectName("CommonDemoProductName"); name->setAccessibleName("产品名称"); form->addWidget(name);
    auto* version=new LineEdit("2.0.0-demo",about); version->setObjectName("CommonDemoProductVersion"); version->setAccessibleName("产品版本"); form->addWidget(version);
    auto* preview=new PushButton("预览关于窗口",about); preview->setObjectName("CommonDemoAboutPreview"); form->addWidget(preview);
    auto* description=new Label("修改名称和版本再预览；Logo、说明、版权、许可和链接均可配置。",about); description->setWordWrap(true); form->addWidget(description); form->addStretch();
    QObject::connect(preview,&QPushButton::clicked,about,[about,name,version] {
        auto* dialog=new AboutDialog(about); dialog->setObjectName("CommonDemoAboutDialog"); dialog->setAttribute(Qt::WA_DeleteOnClose);
        auto info=demoAboutInfo(); info.productName=name->text(); info.version=version->text(); dialog->setInfo(info);
        QObject::connect(dialog->aboutWidget(),&AboutWidget::linkActivated,dialog,[dialog](const QString& id,const QUrl&) {
            if(id=="licenses")if(auto* license=dialog->findChild<CollapsibleSection*>("AboutLicense"))license->setExpanded(true);
            if(id=="guide")dialog->setTitle("使用指南 · 项目 docs 目录提供完整接入说明");
        }); dialog->open();
    }); cards->addWidget(about);
}
// 视图示例与其他常用组件共用页面布局，不再建立独立的整页滚动区。
// 各模型以视图为 QObject 父对象，页面销毁时自动释放；选择/排序沿用 Qt API。
void addViewExamples(ResponsiveGridLayout* cards,QWidget* content)
{
    auto* navigation=new GroupBox("导航与列表",content); navigation->setObjectName("CommonNavigationCard");
    auto* navLayout=new QVBoxLayout(navigation); navLayout->setContentsMargins(8,6,8,8); navLayout->setSpacing(6);
    auto* row=new QHBoxLayout; row->setSpacing(8);
    auto* nav=new NavigationList(navigation); nav->setObjectName("CommonDemoNavigation");
    nav->addButton("首页"); nav->addButton("设置"); nav->addButton("关于");
    nav->insertButton("个人中心",1); nav->insertButton("消息",2,10); nav->addButton("帮助",11); nav->setCurrentButton(0);
    nav->setMinimumWidth(100); nav->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);
    auto* list=new ListView(navigation); list->setObjectName("CommonDemoList"); list->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);
    auto* listModel=new QStandardItemModel(list);
    for(const auto& text:{"任务 A","任务 B","任务 C"})listModel->appendRow(new QStandardItem(text));
    list->setModel(listModel); row->addWidget(nav,1); row->addWidget(list,1); navLayout->addLayout(row,1);
    auto* current=new Label("首页 · ID 0",navigation); current->setObjectName("CommonDemoNavigationSelection"); current->setWordWrap(true); navLayout->addWidget(current);
    QObject::connect(nav,&QListWidget::currentItemChanged,current,[nav,current](QListWidgetItem* item,QListWidgetItem*) {
        current->setText(item?QString("%1 · ID %2").arg(item->text()).arg(nav->getCurrentButton()):QString("未选择导航"));
    });
    cards->addWidget(navigation);

    auto* tableCard=new GroupBox("基础表格",content); tableCard->setObjectName("CommonTableCard");
    auto* tableLayout=new QVBoxLayout(tableCard); tableLayout->setContentsMargins(8,6,8,8);
    auto* table=new TableView(tableCard); table->setObjectName("CommonDemoTable"); table->setMinimumWidth(0);
    auto* tableModel=new QStandardItemModel(table); tableModel->setHorizontalHeaderLabels({"产品","区域","销量"});
    tableModel->appendRow({new QStandardItem("A"),new QStandardItem("华北"),new QStandardItem("120")});
    tableModel->appendRow({new QStandardItem("B"),new QStandardItem("华东"),new QStandardItem("98")});
    tableModel->appendRow({new QStandardItem("C"),new QStandardItem("华南"),new QStandardItem("150")});
    table->setModel(tableModel); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(qMax(26,table->fontMetrics().height()+8));
    tableLayout->addWidget(table,0,Qt::AlignTop); tableLayout->addStretch(); cards->addWidget(tableCard);

    auto* treeCard=new GroupBox("树形表格 · 分组汇总",content); treeCard->setObjectName("CommonTreeCard");
    auto* treeLayout=new QVBoxLayout(treeCard); treeLayout->setContentsMargins(8,6,8,8);
    auto* tree=new TreeTable(treeCard); tree->setObjectName("CommonDemoTree"); tree->setMinimumWidth(0);
    auto* treeModel=new QStandardItemModel(tree); treeModel->setHorizontalHeaderLabels({"类别","项目","数值"});
    auto* sales=new QStandardItem("销售"); auto* research=new QStandardItem("研发");
    treeModel->appendRow({sales,new QStandardItem,new QStandardItem}); treeModel->appendRow({research,new QStandardItem,new QStandardItem});
    const QStringList quarters{"Q1","Q2","Q3","Q4"},values{"120.5","98.0","150.3","110.0"};
    for(int i=0;i<quarters.size();++i)sales->appendRow({new QStandardItem(quarters[i]),new QStandardItem,new QStandardItem(values[i])});
    const QStringList projects{"平台","算法","客户端"},costs{"80.0","132.7","64.2"};
    for(int i=0;i<projects.size();++i)research->appendRow({new QStandardItem(projects[i]),new QStandardItem,new QStandardItem(costs[i])});
    tree->setModel(treeModel); tree->clearColumnAggregations(); tree->setColumnAggregation(2,TreeTable::Sum);
    tree->expandAll(); tree->header()->setSectionResizeMode(QHeaderView::Stretch); treeLayout->addWidget(tree,0,Qt::AlignTop); treeLayout->addStretch(); cards->addWidget(treeCard);
    // 保留可读字号，限制示例视口的初始高度；更多项目在视图内部滚动。
    for(auto* view:QList<QAbstractItemView*>{nav,list,table,tree}) {
        view->setMinimumHeight(140); view->setMaximumHeight(qMax(156,view->fontMetrics().height()*7+12)); view->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);
    }
    for(auto* card:{navigation,tableCard,treeCard})card->setMinimumHeight(220);
}
}
CommonWidgetsPage::CommonWidgetsPage(QWidget* parent) : StyledWidget(parent)
{
    auto* scroll=new ScrollArea(this); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    scroll->setObjectName("CommonDemoScroll");
    auto* content=new StyledWidget; auto* layout=new QVBoxLayout(content); layout->setContentsMargins(10,10,10,10); layout->setSpacing(8);
    auto* cards=new ResponsiveGridLayout(nullptr,300,3); cards->setContentsMargins(0,0,0,0); cards->setSpacing(8); layout->addLayout(cards);
    addUtilityExamples(cards,content);
    addViewExamples(cards,content);
    auto* feedback=new GroupBox("消息与反馈",content); auto* feedbackLayout=new QVBoxLayout(feedback);
    feedbackLayout->setContentsMargins(8,6,8,8); feedbackLayout->setSpacing(8);

    auto* banner=new MessageBanner(content); banner->setObjectName("CommonDemoBanner");
    banner->setTone(FeedbackTone::Info); banner->setTitle("数据目录已连接");
    banner->setMessage("示例数据已就绪，可调整视图与筛选条件。");
    banner->setActionText("查看接线说明"); feedbackLayout->addWidget(banner); feedbackLayout->addStretch();
    connect(banner,&MessageBanner::actionRequested,this,[banner] {
        banner->setMessage("分段切换使用 currentChanged；筛选标签使用 toggled 和 removeRequested。所有状态由业务层控制。");
    });

    auto* filters=new GroupBox("视图与筛选",content); auto* left=new QVBoxLayout(filters); left->setContentsMargins(8,6,8,8); left->setSpacing(8);
    auto* segments=new SegmentedControl(filters); segments->setObjectName("CommonDemoSegments"); segments->setAccessibleName("录音视图");
    segments->addTab("全部录音"); segments->setTabData(0,"all");
    segments->addTab("待核验"); segments->setTabData(1,"pending");
    segments->addTab("已归档"); segments->setTabData(2,"archived");
    left->addWidget(segments);
    auto* hint=new Label("Space 切换条件，× / Delete 移除。",filters); hint->setWordWrap(true); left->addWidget(hint);
    auto* chipHost=new QWidget(filters); auto* chipLayout=new ResponsiveGridLayout(chipHost,120,3); chipLayout->setContentsMargins(0,0,0,0); chipLayout->setSpacing(6);
    QList<FilterChip*> chips;
    for(const QString& text:{QString("东海站点"),QString("WAV 格式"),QString("已核验")}) {
        auto* chip=new FilterChip(text,chipHost); chip->setRemovable(true); chip->setChecked(true); chip->setToolTip(text);
        chipLayout->addWidget(chip); chips.append(chip);
    }
    auto refreshSupportingText=[hint] {
        auto* tokens=ThemeTokens::instance();
        for(auto* label:{hint}) {
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
    updateSummary(); cards->addWidget(filters); cards->addWidget(feedback);

    auto* emptyCard=new GroupBox("空状态与下一步",content); auto* right=new QVBoxLayout(emptyCard); right->setContentsMargins(4,4,4,4);
    auto* empty=new EmptyState(emptyCard); empty->setObjectName("CommonDemoEmptyState");
    empty->setTitle("没有符合条件的录音"); empty->setDescription("清除筛选，或选择其他目录。");
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
    cards->addWidget(emptyCard); addMessageAndAboutExamples(cards,content); layout->addStretch(); scroll->setWidget(content);
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(0,0,0,0); outer->addWidget(scroll);
}
