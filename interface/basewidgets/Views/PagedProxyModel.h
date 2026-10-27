#ifndef BASEWIDGETS_PAGEDPROXYMODEL_H
#define BASEWIDGETS_PAGEDPROXYMODEL_H
#include <QAbstractProxyModel>
#include <QPersistentModelIndex>
#include <QVector>
#include <basewidgets/QtAdvancedUIExport.h>
namespace basewidgets {
/** 平面表格的可选分页/固定行投影，不拥有源模型。
 * pageSize 表示每页普通行数量，固定顶部/底部记录额外加入每页，顺序由调用方给出。
 * 固定记录遵守上游筛选；使用原始模型的持久索引，因此排序/插入不会固定错记录。
 * 纯 dataChanged 转发，不整表 reset；结构改变时页码自动夹取。
 */
class ACSS_UI_EXPORT PagedProxyModel : public QAbstractProxyModel {
    Q_OBJECT
public:
    enum { PinPositionRole=Qt::UserRole+0x421 }; ///< 1=顶部，-1=底部，0=普通；不改源数据。
    explicit PagedProxyModel(QObject* parent=nullptr);
    ~PagedProxyModel() override;
    void setSourceModel(QAbstractItemModel* source) override;
    QModelIndex mapToSource(const QModelIndex& index) const override;
    QModelIndex mapFromSource(const QModelIndex& index) const override;
    QModelIndex index(int row,int column,const QModelIndex& parent=QModelIndex()) const override;
    QModelIndex parent(const QModelIndex&) const override { return QModelIndex(); }
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant data(const QModelIndex& index,int role=Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index,const QVariant& value,int role=Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section,Qt::Orientation orientation,int role=Qt::DisplayRole) const override;
    void sort(int column,Qt::SortOrder order=Qt::AscendingOrder) override;
    void setPageSize(int size);
    void setCurrentPage(int page);
    void setPaginationEnabled(bool enabled);
    bool paginationEnabled() const { return m_paginated; }
    /** 接受源模型或其上游模型索引，规范为 column 0；最多 64 条，不接收重复记录。 */
    bool setPinnedIndexes(const QModelIndexList& top,const QModelIndexList& bottom);
    int pageSize() const { return m_pageSize; }
    int currentPage() const { return m_currentPage; }
    int totalRows() const;
    int normalRowCount() const { return m_normal.size(); }
    int pinnedRowCount() const { return m_top.size()+m_bottom.size(); }
    int pageCount() const;
signals:
    void pageChanged(int current,int total);
private:
    QModelIndex resolvePin(const QPersistentModelIndex& pin) const;
    QModelIndex canonicalIndex(const QModelIndex& index) const;
    void rebuildRows();
    void rebuildPage();
    void beginSourceChange();
    void endSourceChange();
    void forwardData(const QModelIndex& first,const QModelIndex& last,const QVector<int>& roles);
    QVector<QMetaObject::Connection> m_connections;
    QVector<QPersistentModelIndex> m_topPins,m_bottomPins;
    QVector<int> m_top,m_bottom,m_normal,m_rows,m_reverse;
    int m_pageSize=20,m_currentPage=0,m_resetDepth=0;
    bool m_paginated=true;
};
}
#endif
