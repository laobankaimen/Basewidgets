#ifndef QTADVANCEDUI_PAGEDPROXYMODEL_H
#define QTADVANCEDUI_PAGEDPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QtAdvancedUI/QtAdvancedUIExport.h>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT PagedProxyModel : public QAbstractProxyModel {
    Q_OBJECT
public:
    explicit PagedProxyModel(QObject* parent = nullptr);

    void setSourceModel(QAbstractItemModel* source) override;

    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setPageSize(int size);
    void setCurrentPage(int page);
    int pageSize() const { return m_pageSize; }
    int currentPage() const { return m_currentPage; }
    int totalRows() const;
    int pageCount() const;

signals:
    void pageChanged(int current, int total);

private:
    void invalidateAndReset();
    void resetPage();

private:
    int m_pageSize = 20;
    int m_currentPage = 0;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_PAGEDPROXYMODEL_H