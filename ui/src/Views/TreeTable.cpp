/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QtAdvancedUI/Views/TreeTable.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QStandardItemModel>

using namespace acss;

namespace QtAdvancedUI {

TreeTable::TreeTable(QWidget* parent)
    : QTreeView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setVirtualizationEnabled(true);
    setRootIsDecorated(true);
    setItemsExpandable(true);
    setExpandsOnDoubleClick(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TreeTable::~TreeTable() = default;

void TreeTable::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TreeTable::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void TreeTable::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void TreeTable::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

void TreeTable::setVirtualizationEnabled(bool enabled)
{
    m_virtualization = enabled;
    setUniformRowHeights(enabled);
    setAnimated(!enabled);
}

void TreeTable::setColumnAggregation(int column, Aggregation agg)
{
    if (agg == NoneR) {
        m_columnAggs.remove(column);
    } else {
        m_columnAggs.insert(column, agg);
    }
    updateSummaries();
}

TreeTable::Aggregation TreeTable::columnAggregation(int column) const
{
    return m_columnAggs.value(column, NoneR);
}

void TreeTable::clearColumnAggregations()
{
    m_columnAggs.clear();
    updateSummaries();
}

double TreeTable::aggregateChildren(const QModelIndex& parent, int column, Aggregation agg, int& count) const
{
    if (!model()) return 0.0;
    double sum = 0.0;
    count = 0;
    const int rows = model()->rowCount(parent);
    for (int r = 0; r < rows; ++r) {
        QModelIndex childIndex = model()->index(r, column, parent);
        bool ok = false;
        // 直接取显示角色并尝试转换为 double
        double v = model()->data(childIndex, Qt::DisplayRole).toString().toDouble(&ok);
        if (!ok) {
            // 尝试以数值角色获取
            v = model()->data(childIndex, Qt::EditRole).toDouble(&ok);
        }
        if (ok) {
            sum += v;
            ++count;
        }
        // 递归子级
        int subCount = 0;
        double subSum = aggregateChildren(model()->index(r, 0, parent), column, agg, subCount);
        sum += subSum;
        count += subCount;
    }
    return sum;
}

void TreeTable::updateSummaries()
{
    if (!model() || m_columnAggs.isEmpty()) return;
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return;

    removeExistingSummaryRows();

    const int topRows = m->rowCount(QModelIndex());
    for (int r = 0; r < topRows; ++r) {
        QStandardItem* parentItem = m->item(r, 0);
        if (!parentItem) continue;
        if (m_groupSummary) {
            for (auto it = m_columnAggs.constBegin(); it != m_columnAggs.constEnd(); ++it) {
                appendSummaryRow(m, parentItem, it.key(), it.value());
            }
        } else {
            // 写入父行对应列（兼容旧模式）
            for (auto it = m_columnAggs.constBegin(); it != m_columnAggs.constEnd(); ++it) {
                int count = 0;
                double sum = aggregateChildren(m->indexFromItem(parentItem), it.key(), it.value(), count);
                double value = (it.value() == Avg) ? (count > 0 ? sum / count : 0.0) : (it.value() == Count ? count : sum);
                QString label = it.value() == Sum ? tr("合计") : it.value() == Avg ? tr("平均") : tr("数量");
                QModelIndex writeIdx = m->index(parentItem->row(), it.key(), QModelIndex());
                m->setData(writeIdx, QString("%1: %2").arg(label).arg(value, 0, 'f', 2), Qt::DisplayRole);
            }
        }
    }
}

void TreeTable::setGroupSummaryEnabled(bool enabled)
{
    m_groupSummary = enabled;
    updateSummaries();
}

void TreeTable::removeExistingSummaryRows()
{
    auto* m = qobject_cast<QStandardItemModel*>(model());
    if (!m) return;
    const int topRows = m->rowCount(QModelIndex());
    for (int r = 0; r < topRows; ++r) {
        QStandardItem* parentItem = m->item(r, 0);
        if (!parentItem) continue;
        for (int i = parentItem->rowCount() - 1; i >= 0; --i) {
            QStandardItem* child0 = parentItem->child(i, 0);
            if (!child0) continue;
            if (child0->data(Qt::UserRole + 1).toBool()) {
                parentItem->removeRow(i);
            }
        }
    }
}

void TreeTable::appendSummaryRow(QStandardItemModel* m, QStandardItem* parentItem, int column, Aggregation agg)
{
    int count = 0;
    double sum = aggregateChildren(m->indexFromItem(parentItem), column, agg, count);
    double value = (agg == Avg) ? (count > 0 ? sum / count : 0.0) : (agg == Count ? count : sum);
    QString label = agg == Sum ? tr("合计") : agg == Avg ? tr("平均") : tr("数量");

    QList<QStandardItem*> row;
    auto* nameItem = new QStandardItem(QString("%1").arg(label));
    nameItem->setData(true, Qt::UserRole + 1); // 标记汇总行
    nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    row << nameItem;
    int cols = m->columnCount();
    for (int c = 1; c < cols; ++c) {
        row << new QStandardItem();
    }
    row[column]->setText(QString::number(value, 'f', 2));
    row[column]->setData(true, Qt::UserRole + 1);
    parentItem->appendRow(row);
}

} // namespace QtAdvancedUI

