// 一个 AdvancedTable 覆盖基础/无分页/分页展示；源模型、委托和业务菜单由应用提供。
// 本页全部为合成记录。数量字段存数值，不把数值写成字符串后再做字典排序。
#include "PagedTablePage.h"
#include <basewidgets/Views/AdvancedTable.h>
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Inputs/ComboBox.h>
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Inputs/PushButton.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QPersistentModelIndex>
using namespace basewidgets;
PagedTablePage::PagedTablePage(QWidget* parent) : StyledWidget(parent)
{
    auto* outer=new QVBoxLayout(this); outer->setContentsMargins(8,8,8,8); outer->setSpacing(8);
    auto* introduction=new Label("录音文件管理（模拟） · 左侧勾选可跨页保留，左上角全选当前页；点击列头排序。",this);
    introduction->setWordWrap(true); outer->addWidget(introduction);
    auto* row=new QHBoxLayout;
    auto* mode=new ComboBox(this); mode->addItems({"基础表格","筛选 / 不分页","筛选 / 分页"}); mode->setCurrentIndex(2);
    row->addWidget(new Label("模式",this)); row->addWidget(mode);
    auto* top=new PushButton("固定选中",this); auto* unpin=new PushButton("取消固定",this); auto* clear=new PushButton("清空固定",this);
    row->addWidget(top); row->addWidget(unpin); row->addWidget(clear); row->addStretch(); outer->addLayout(row);
    auto* feedback=new Label("↑ 前两条记录固定顶部；排序不改变固定顺序，分页时每页保留。过滤仍对它们生效。",this);
    feedback->setWordWrap(true); outer->addWidget(feedback);
    auto* source=new QStandardItemModel(200,6,this);
    source->setHorizontalHeaderLabels({"编号","录音文件","测站","大小 (MiB)","时长 (s)","索引状态"});
    for(int i=0;i<source->rowCount();++i) {
        source->setData(source->index(i,0),1000+i);
        source->setData(source->index(i,1),QString("station_%1_%2.wav").arg(i%4+1).arg(i,4,10,QChar('0')));
        source->setData(source->index(i,2),QString("测站 %1").arg(i%4+1));
        source->setData(source->index(i,3),64+i%50*8);
        source->setData(source->index(i,4),30+i%12*10);
        source->setData(source->index(i,5),i%13==0?"待重试":"已索引");
    }
    auto* table=new AdvancedTable(this); table->setObjectName("RecordingAdvancedTable");
    table->setSourceModel(source); table->setPageSize(20);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 参数是源模型记录的行号，不是排序后/当前页的行号。可同时固定顶部和底部。
    table->setPinnedRows({0,1}); outer->addWidget(table,1);
    auto* checkedRow=new QHBoxLayout;
    auto* checkedCount=new Label("已勾选 0 条（跨页）",this);
    auto* clearChecks=new PushButton("清空勾选",this);
    checkedRow->addWidget(checkedCount); checkedRow->addWidget(clearChecks); checkedRow->addStretch(); outer->addLayout(checkedRow);
    // 勾选与行高亮独立：批量业务用 checkedSourceRows()，返回原始模型记录，不受翻页影响。
    connect(table,&AdvancedTable::checkedRowsChanged,checkedCount,[table,checkedCount] {
        checkedCount->setText(QString("已勾选 %1 条（跨页）").arg(table->checkedSourceRows().size()));
    });
    connect(clearChecks,&QPushButton::clicked,table,&AdvancedTable::clearCheckedRows);
    connect(mode,QOverload<int>::of(&QComboBox::currentIndexChanged),table,[table](int index) {
        table->setPresentationMode(static_cast<AdvancedTable::PresentationMode>(index));
    });
    connect(top,&QPushButton::clicked,table,[table,feedback] {
        if(!table->pinRows(table->selectedSourceRows()))feedback->setText("固定失败：最多固定 64 条记录。");
    });
    connect(unpin,&QPushButton::clicked,table,[table] { table->unpinRows(table->selectedSourceRows()); });
    connect(clear,&QPushButton::clicked,table,&AdvancedTable::clearPinnedRows);
    connect(table,&AdvancedTable::filterRejected,feedback,[feedback](const QString& error) { feedback->setText("筛选表达式无效，保留上次结果："+error); });
    // 菜单信号传回 sourceModel() 的索引。QPersistentModelIndex 可防止菜单打开期间删行后误操作。
    connect(table,&AdvancedTable::prepareContextMenu,table,[table,feedback](QMenu* menu,const QModelIndexList& rows) {
        menu->addSeparator(); auto* action=menu->addAction("业务扩展：查看录音详情"); action->setEnabled(rows.size()==1);
        const QPersistentModelIndex index=rows.size()==1?rows.first():QModelIndex();
        connect(action,&QAction::triggered,table,[index,feedback] {
            if(index.isValid())feedback->setText(QString("录音 %1 · %2（此处可接入业务详情页）").arg(index.data().toString(),index.sibling(index.row(),1).data().toString()));
        });
    });
}
