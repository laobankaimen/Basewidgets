#include "ViewsPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Views/ListWidget.h>
#include <QtAdvancedUI/Views/TreeWidget.h>
#include <QtAdvancedUI/Views/TableWidget.h>
#include <QtAdvancedUI/Views/ListView.h>
#include <QtAdvancedUI/Views/TreeView.h>
#include <QtAdvancedUI/Views/TableView.h>
#include <QtAdvancedUI/Containers/GroupBox.h>
#include <QtAdvancedUI/Containers/ScrollArea.h>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QSizePolicy>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QRegExp>

using namespace QtAdvancedUI;

ViewsPage::ViewsPage(QWidget* parent)
    : StyledWidget(parent)
{
    // 顶层采用滚动区域，避免界面放大后控件被强行拉伸
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    auto* scrollArea = new ScrollArea(this);
    auto* content = new StyledWidget(scrollArea);
    auto* grid = new QGridLayout(content);
    grid->setContentsMargins(8, 8, 8, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    // ListWidget
    auto* listWidget = new ListWidget(content);
    listWidget->addItems({"一", "二", "三"});
    listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    listWidget->setMinimumHeight(180);

    // TreeWidget
    auto* treeWidget = new TreeWidget(content);
    treeWidget->setHeaderLabels({"列1", "列2"});
    auto* root = new QTreeWidgetItem(treeWidget, QStringList{"Root", ""});
    new QTreeWidgetItem(root, QStringList{"Child A", "Value"});
    new QTreeWidgetItem(root, QStringList{"Child B", "Value"});
    treeWidget->expandAll();
    treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    treeWidget->setMinimumHeight(200);

    // TableWidget
    auto* tableWidget = new TableWidget(content);
    tableWidget->setRowCount(3);
    tableWidget->setColumnCount(3);
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            tableWidget->setItem(r, c, new QTableWidgetItem(QString("%1,%2").arg(r).arg(c)));
        }
    }
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tableWidget->setMinimumHeight(200);

    // ListView
    auto* listView = new ListView(content);
    auto* listModel = new QStringListModel(QStringList{ "项 A", "项 B", "项 C" }, this);
    listView->setModel(listModel);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    listView->setMinimumHeight(180);

    // TreeView + model + proxy
    auto* treeView = new TreeView(content);
    auto* treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels({ "名称", "值" });
    auto* rootItem = new QStandardItem("Root A");
    rootItem->appendRow({ new QStandardItem("Child A"), new QStandardItem("42") });
    rootItem->appendRow({ new QStandardItem("Child B"), new QStandardItem("73") });
    treeModel->appendRow(rootItem);
    auto* treeProxy = new QSortFilterProxyModel(this);
    treeProxy->setSourceModel(treeModel);
    treeProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    treeProxy->setFilterKeyColumn(-1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    treeProxy->setRecursiveFilteringEnabled(true);
#endif
    treeView->setModel(treeProxy);
    treeView->setSortingEnabled(true);
    treeView->expandAll();
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(treeView, &TreeView::customContextMenuRequested, [treeView](const QPoint& pos){
        QMenu menu(treeView);
        menu.addAction("展开全部", [treeView](){ treeView->expandAll(); });
        menu.addAction("折叠全部", [treeView](){ treeView->collapseAll(); });
        menu.exec(treeView->viewport()->mapToGlobal(pos));
    });
    treeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    treeView->setMinimumHeight(220);

    // TableView + model + proxy
    auto* tableView = new TableView(content);
    auto* tvModel = new QStandardItemModel(3, 3, this);
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            tvModel->setItem(r, c, new QStandardItem(QString("%1,%2").arg(r).arg(c)));
        }
    }
    auto* tableProxy = new QSortFilterProxyModel(this);
    tableProxy->setSourceModel(tvModel);
    tableProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    tableProxy->setFilterKeyColumn(-1);
    tableView->setModel(tableProxy);
    tableView->setSortingEnabled(true);
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(tableView, &TableView::customContextMenuRequested, [tableView, tableProxy, tvModel](const QPoint& pos){
        QMenu menu(tableView);
        QAction* copyAct = menu.addAction("复制单元格");
        QAction* delRowAct = menu.addAction("删除行");
        QAction* chosen = menu.exec(tableView->viewport()->mapToGlobal(pos));
        if (chosen == copyAct) {
            QModelIndex idx = tableView->currentIndex();
            if (idx.isValid()) {
                QModelIndex srcIdx = tableProxy->mapToSource(idx);
                QApplication::clipboard()->setText(tvModel->data(srcIdx).toString());
            }
        } else if (chosen == delRowAct) {
            QModelIndex idx = tableView->currentIndex();
            if (idx.isValid()) {
                int srcRow = tableProxy->mapToSource(idx).row();
                tvModel->removeRow(srcRow);
            }
        }
    });
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tableView->setMinimumHeight(240);

    // selection sync with another tableView
    auto* tableView2 = new TableView(content);
    tableView2->setModel(tableProxy);
    auto* selModel = new QItemSelectionModel(tableProxy, this);
    tableView->setSelectionModel(selModel);
    tableView2->setSelectionModel(selModel);
    tableView2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tableView2->setMinimumHeight(240);

    // 顶部筛选条：跨两列
    auto* filterLabel = new Label("筛选：", content);
    auto* filterEdit = new LineEdit(content);
    filterEdit->setPlaceholderText("输入关键字以过滤树/表...");
    QObject::connect(filterEdit, &LineEdit::textChanged, [treeProxy, tableProxy](const QString& text){
        QRegExp rx(text, Qt::CaseInsensitive, QRegExp::FixedString);
        treeProxy->setFilterRegExp(rx);
        tableProxy->setFilterRegExp(rx);
    });
    grid->addWidget(filterLabel, 0, 0);
    grid->addWidget(filterEdit, 0, 1);

    // 左右两列分区，减少纵向堆叠
    auto* gbListWidget = new GroupBox("ListWidget：", content);
    auto* v1 = new QVBoxLayout(gbListWidget);
    v1->addWidget(listWidget);
    gbListWidget->setLayout(v1);

    auto* gbTreeWidget = new GroupBox("TreeWidget：", content);
    auto* v2 = new QVBoxLayout(gbTreeWidget);
    v2->addWidget(treeWidget);
    gbTreeWidget->setLayout(v2);

    auto* gbTableWidget = new GroupBox("TableWidget：", content);
    auto* v3 = new QVBoxLayout(gbTableWidget);
    v3->addWidget(tableWidget);
    gbTableWidget->setLayout(v3);

    auto* gbListView = new GroupBox("ListView：", content);
    auto* v4 = new QVBoxLayout(gbListView);
    v4->addWidget(listView);
    gbListView->setLayout(v4);

    auto* gbTreeView = new GroupBox("TreeView：", content);
    auto* v5 = new QVBoxLayout(gbTreeView);
    v5->addWidget(treeView);
    gbTreeView->setLayout(v5);

    auto* gbTableView = new GroupBox("TableView：", content);
    auto* v6 = new QVBoxLayout(gbTableView);
    v6->addWidget(tableView);
    gbTableView->setLayout(v6);

    auto* gbTableView2 = new GroupBox("TableView2（选择同步）：", content);
    auto* v7 = new QVBoxLayout(gbTableView2);
    v7->addWidget(tableView2);
    gbTableView2->setLayout(v7);

    // 布局到两列网格
    int row = 1;
    grid->addWidget(gbListWidget,   row, 0);
    grid->addWidget(gbTreeWidget,   row, 1);
    ++row;
    grid->addWidget(gbTableWidget,  row, 0);
    grid->addWidget(gbListView,     row, 1);
    ++row;
    grid->addWidget(gbTreeView,     row, 0);
    grid->addWidget(gbTableView,    row, 1);
    ++row;
    grid->addWidget(gbTableView2,   row, 0, 1, 2);

    content->setLayout(grid);
    scrollArea->setWidget(content);
    outerLayout->addWidget(scrollArea);
    outerLayout->addStretch();
    setLayout(outerLayout);
}