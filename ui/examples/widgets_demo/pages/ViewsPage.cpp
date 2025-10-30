#include "ViewsPage.h"

#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Views/ListWidget.h>
#include <QtAdvancedUI/Views/TreeWidget.h>
#include <QtAdvancedUI/Views/TableWidget.h>
#include <QtAdvancedUI/Views/ListView.h>
#include <QtAdvancedUI/Views/TreeView.h>
#include <QtAdvancedUI/Views/TableView.h>

#include <QVBoxLayout>
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
    auto* layout = new QVBoxLayout(this);

    // ListWidget
    auto* listWidget = new ListWidget(this);
    listWidget->addItems({"一", "二", "三"});

    // TreeWidget
    auto* treeWidget = new TreeWidget(this);
    treeWidget->setHeaderLabels({"列1", "列2"});
    auto* root = new QTreeWidgetItem(treeWidget, QStringList{"Root", ""});
    new QTreeWidgetItem(root, QStringList{"Child A", "Value"});
    new QTreeWidgetItem(root, QStringList{"Child B", "Value"});
    treeWidget->expandAll();

    // TableWidget
    auto* tableWidget = new TableWidget(this);
    tableWidget->setRowCount(3);
    tableWidget->setColumnCount(3);
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            tableWidget->setItem(r, c, new QTableWidgetItem(QString("%1,%2").arg(r).arg(c)));
        }
    }

    // ListView
    auto* listView = new ListView(this);
    auto* listModel = new QStringListModel(QStringList{ "项 A", "项 B", "项 C" }, this);
    listView->setModel(listModel);

    // TreeView + model + proxy
    auto* treeView = new TreeView(this);
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

    // TableView + model + proxy
    auto* tableView = new TableView(this);
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

    // selection sync with another tableView
    auto* tableView2 = new TableView(this);
    tableView2->setModel(tableProxy);
    auto* selModel = new QItemSelectionModel(tableProxy, this);
    tableView->setSelectionModel(selModel);
    tableView2->setSelectionModel(selModel);

    layout->addWidget(new Label("筛选：", this));
    auto* filterEdit = new LineEdit(this);
    filterEdit->setPlaceholderText("输入关键字以过滤树/表...");
    QObject::connect(filterEdit, &LineEdit::textChanged, [treeProxy, tableProxy](const QString& text){
        QRegExp rx(text, Qt::CaseInsensitive, QRegExp::FixedString);
        treeProxy->setFilterRegExp(rx);
        tableProxy->setFilterRegExp(rx);
    });
    layout->addWidget(filterEdit);

    layout->addWidget(new Label("ListWidget：", this));
    layout->addWidget(listWidget);
    layout->addWidget(new Label("TreeWidget：", this));
    layout->addWidget(treeWidget);
    layout->addWidget(new Label("TableWidget：", this));
    layout->addWidget(tableWidget);
    layout->addWidget(new Label("ListView：", this));
    layout->addWidget(listView);
    layout->addWidget(new Label("TreeView：", this));
    layout->addWidget(treeView);
    layout->addWidget(new Label("TableView：", this));
    layout->addWidget(tableView);
    layout->addWidget(new Label("TableView2（选择同步）：", this));
    layout->addWidget(tableView2);

    setLayout(layout);
}