#include "PagedTablePage.h"

#include <basewidgets/Containers/GroupBox.h>
#include <basewidgets/Views/AdvancedTable.h>
#include <basewidgets/Views/TableView.h>

#include <QVBoxLayout>
#include <QStandardItemModel>

using namespace basewidgets;

PagedTablePage::PagedTablePage(QWidget* parent)
    : StyledWidget(parent)
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(10);

    auto* gb = new GroupBox("", this);
    auto* vbox = new QVBoxLayout(gb);
    vbox->setContentsMargins(10, 10, 10, 10);
    vbox->setSpacing(8);

    // 源模型与数据
    auto* source = new QStandardItemModel(0, 6, this);
    QStringList headers = {"ID", "名称", "类别", "价格", "库存", "更新时间"};
    source->setHorizontalHeaderLabels(headers);
    for (int i = 0; i < 200; ++i) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(1000 + i));
        row << new QStandardItem(QString("商品%1").arg(i));
        row << new QStandardItem(QStringList{"食品", "服饰", "数码", "家居"}[i % 4]);
        row << new QStandardItem(QString::number((i % 50) + 1));
        row << new QStandardItem(QString::number((i * 3) % 200));
        row << new QStandardItem(QString("2025-10-%1").arg((i % 30) + 1, 2, 10, QChar('0')));
        source->appendRow(row);
    }
    // 复合控件：由库提供 AdvancedTable
    auto* adv = new basewidgets::AdvancedTable(gb);
    adv->setSourceModel(source);
    adv->pager()->setPageSize(20);
    adv->view()->sortByColumn(0, Qt::AscendingOrder);

    vbox->addWidget(adv);
    outer->addWidget(gb);
}