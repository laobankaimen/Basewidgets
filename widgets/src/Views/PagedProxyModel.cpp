/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Views/PagedProxyModel.h>
#include <QAbstractItemModel>
#include <QtMath>

namespace basewidgets {

PagedProxyModel::PagedProxyModel(QObject* parent)
    : QAbstractProxyModel(parent)
{
}

void PagedProxyModel::setSourceModel(QAbstractItemModel* source)
{
    if (auto* oldSource = sourceModel()) {
        disconnect(oldSource, nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(source);
    m_currentPage = 0;
    if (!source) {
        invalidateAndReset();
        return;
    }

    connect(source, &QAbstractItemModel::modelReset, this, &PagedProxyModel::resetPage, Qt::UniqueConnection);
    connect(source, &QAbstractItemModel::layoutChanged, this, &PagedProxyModel::invalidateAndReset, Qt::UniqueConnection);
    connect(source, &QAbstractItemModel::dataChanged, this, &PagedProxyModel::invalidateAndReset, Qt::UniqueConnection);
    connect(source, &QAbstractItemModel::rowsInserted, this, &PagedProxyModel::invalidateAndReset, Qt::UniqueConnection);
    connect(source, &QAbstractItemModel::rowsRemoved, this, &PagedProxyModel::invalidateAndReset, Qt::UniqueConnection);
    invalidateAndReset();
}

QModelIndex PagedProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid() || !sourceModel()) return QModelIndex();
    int srcRow = m_currentPage * m_pageSize + proxyIndex.row();
    return sourceModel()->index(srcRow, proxyIndex.column(), QModelIndex());
}

QModelIndex PagedProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceIndex.isValid() || !sourceModel()) return QModelIndex();
    int base = m_currentPage * m_pageSize;
    int row = sourceIndex.row() - base;
    if (row < 0 || row >= m_pageSize) return QModelIndex();
    return index(row, sourceIndex.column());
}

QModelIndex PagedProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid()) return QModelIndex();
    return createIndex(row, column);
}

QModelIndex PagedProxyModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

int PagedProxyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !sourceModel()) return 0;
    int total = sourceModel()->rowCount();
    int base = m_currentPage * m_pageSize;
    int remaining = total - base;
    if (remaining <= 0) return 0;
    return qBound(0, remaining, m_pageSize);
}

int PagedProxyModel::columnCount(const QModelIndex& parent) const
{
    return sourceModel() ? sourceModel()->columnCount(parent) : 0;
}

QVariant PagedProxyModel::data(const QModelIndex& index, int role) const
{
    return sourceModel() ? sourceModel()->data(mapToSource(index), role) : QVariant();
}

bool PagedProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return sourceModel() ? sourceModel()->setData(mapToSource(index), value, role) : false;
}

Qt::ItemFlags PagedProxyModel::flags(const QModelIndex& index) const
{
    return sourceModel() ? sourceModel()->flags(mapToSource(index)) : Qt::NoItemFlags;
}

QVariant PagedProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return sourceModel() ? sourceModel()->headerData(section, orientation, role) : QVariant();
}

void PagedProxyModel::sort(int column, Qt::SortOrder order)
{
    if (sourceModel()) sourceModel()->sort(column, order);
}

void PagedProxyModel::setPageSize(int size)
{
    if (size <= 0 || m_pageSize == size) return;
    m_pageSize = size;
    m_currentPage = 0;
    invalidateAndReset();
}

void PagedProxyModel::setCurrentPage(int page)
{
    int maxPage = qMax(0, pageCount() - 1);
    int newPage = qBound(0, page, maxPage);
    if (m_currentPage == newPage) return;
    m_currentPage = newPage;
    invalidateAndReset();
}

int PagedProxyModel::totalRows() const
{
    return sourceModel() ? sourceModel()->rowCount() : 0;
}

int PagedProxyModel::pageCount() const
{
    return m_pageSize > 0 ? qCeil(totalRows() / double(m_pageSize)) : 0;
}

void PagedProxyModel::invalidateAndReset()
{
    beginResetModel();
    endResetModel();
    emit pageChanged(m_currentPage, pageCount());
}

void PagedProxyModel::resetPage()
{
    m_currentPage = 0;
    invalidateAndReset();
}

} // namespace basewidgets
