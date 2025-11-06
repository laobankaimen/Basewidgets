#ifndef QTADVANCEDUI_TREETABLE_H
#define QTADVANCEDUI_TREETABLE_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QtAdvancedStylesheet.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT TreeTable : public QTreeView
{
    Q_OBJECT
public:
    enum Aggregation { None, Sum, Avg, Count };

    explicit TreeTable(QWidget* parent = nullptr);
    explicit TreeTable(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TreeTable() override;

    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

    // 虚拟化（性能优化）：统一行高、禁用动画
    void setVirtualizationEnabled(bool enabled);
    bool virtualizationEnabled() const { return m_virtualization; }

    // 列汇总策略：为不同列设置聚合类型
    void setColumnAggregation(int column, Aggregation agg);
    Aggregation columnAggregation(int column) const;
    void clearColumnAggregations();

    // 分组汇总行：在每个父节点末尾新增独立“汇总”子行
    void setGroupSummaryEnabled(bool enabled);
    bool groupSummaryEnabled() const { return m_groupSummary; }
    void updateSummaries();

protected:
    void applyStylesheet();

private:
    void attachToManager();
    double aggregateChildren(const QModelIndex& parent, int column, Aggregation agg, int& count) const;
    void removeExistingSummaryRows();
    void appendSummaryRow(QStandardItemModel* m, QStandardItem* parentItem, int column, Aggregation agg);

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
    bool m_virtualization = true;
    bool m_groupSummary = true;
    QHash<int, Aggregation> m_columnAggs;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_TREETABLE_H