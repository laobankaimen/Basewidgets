#include "ViewsPage.h"

#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QVBoxLayout>

#include <QStandardItemModel>

#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Containers/ScrollArea.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Views/ListView.h>
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Views/TreeTable.h>

using namespace basewidgets;

ViewsPage::ViewsPage(QWidget* parent)
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

    auto* navGroup = new GroupBox("", content);
    {
        auto* navLayout = new QHBoxLayout(navGroup);
        navLayout->setContentsMargins(4, 4, 4, 4);
        navLayout->setSpacing(8);

        auto* navList = new NavigationList(navGroup);
        navList->addButton("首页");
        navList->addButton("设置");
        navList->addButton("关于");
        navList->insertButton("个人中心", 1);
        navList->insertButton("消息", 2, 10);
        navList->addButton("帮助", 11);
        navList->setCurrentButton(0);

        auto* navCurrentLabel = new Label("当前选中: 首页 (ID: 0)", navGroup);
        navCurrentLabel->setWordWrap(true);

        navLayout->addWidget(navList);
        navLayout->addWidget(navCurrentLabel);
        navGroup->setLayout(navLayout);

        QObject::connect(navList, &NavigationList::currentItemChanged,
                         [navList, navCurrentLabel](QListWidgetItem* current, QListWidgetItem* /*previous*/){
            if (current) {
                const int id = navList->getCurrentButton();
                navCurrentLabel->setText(QString("当前选中: %1 (ID: %2)").arg(current->text()).arg(id));
            }
        });
    }
    layout->addWidget(navGroup);

    auto* viewGroup = new GroupBox("", content);
    {
        auto* viewLayout = new QGridLayout(viewGroup);

        auto* listView = new ListView(viewGroup);
        auto* tableView = new TableView(viewGroup);
        listView->setMinimumHeight(160);
        tableView->setMinimumHeight(200);
        auto* treeTable = new TreeTable(viewGroup);

        {
            auto* listModel = new QStandardItemModel(listView);
            listModel->setColumnCount(1);
            listModel->appendRow(new QStandardItem("任务 A"));
            listModel->appendRow(new QStandardItem("任务 B"));
            listModel->appendRow(new QStandardItem("任务 C"));
            listView->setModel(listModel);
        }
        {
            auto* tableModel = new QStandardItemModel(tableView);
            tableModel->setColumnCount(3);
            tableModel->setHorizontalHeaderLabels({"产品", "区域", "销量"});
            tableModel->appendRow(QList<QStandardItem*>{new QStandardItem("A"), new QStandardItem("华北"), new QStandardItem("120")});
            tableModel->appendRow(QList<QStandardItem*>{new QStandardItem("B"), new QStandardItem("华东"), new QStandardItem("98")});
            tableModel->appendRow(QList<QStandardItem*>{new QStandardItem("C"), new QStandardItem("华南"), new QStandardItem("150")});
            tableView->setModel(tableModel);
            tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        }
        {
            auto* tmodel = new QStandardItemModel(treeTable);
            tmodel->setColumnCount(3);
            tmodel->setHorizontalHeaderLabels({"类别", "项目", "数值"});
            auto* sales = new QStandardItem("销售");
            auto* rnd = new QStandardItem("研发");
            tmodel->appendRow(QList<QStandardItem*>{sales, new QStandardItem(), new QStandardItem()});
            tmodel->appendRow(QList<QStandardItem*>{rnd, new QStandardItem(), new QStandardItem()});
            sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q1"), new QStandardItem(), new QStandardItem("120.5")});
            sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q2"), new QStandardItem(), new QStandardItem("98.0")});
            sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q3"), new QStandardItem(), new QStandardItem("150.3")});
            sales->appendRow(QList<QStandardItem*>{new QStandardItem("Q4"), new QStandardItem(), new QStandardItem("110.0")});
            rnd->appendRow(QList<QStandardItem*>{new QStandardItem("平台"), new QStandardItem(), new QStandardItem("80.0")});
            rnd->appendRow(QList<QStandardItem*>{new QStandardItem("算法"), new QStandardItem(), new QStandardItem("132.7")});
            rnd->appendRow(QList<QStandardItem*>{new QStandardItem("客户端"), new QStandardItem(), new QStandardItem("64.2")});
            treeTable->setModel(tmodel);
            treeTable->clearColumnAggregations();
            treeTable->setColumnAggregation(2, TreeTable::Sum);
            treeTable->expandAll();
            treeTable->header()->setSectionResizeMode(QHeaderView::Stretch);
        }

        viewLayout->addWidget(new Label("列表视图:", viewGroup), 0, 0);
        viewLayout->addWidget(listView, 1, 0);
        viewLayout->addWidget(new Label("表格视图:", viewGroup), 2, 0);
        viewLayout->addWidget(tableView, 3, 0);
        viewLayout->addWidget(new Label("树形表格:", viewGroup), 4, 0);
        viewLayout->addWidget(treeTable, 5, 0);
        viewLayout->setColumnStretch(0, 1);
        viewGroup->setLayout(viewLayout);
    }
    layout->addWidget(viewGroup);

    content->setLayout(layout);
    scrollArea->setWidget(content);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(scrollArea);
    setLayout(pageLayout);
}

