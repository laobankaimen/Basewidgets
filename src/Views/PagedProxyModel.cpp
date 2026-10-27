#include <basewidgets/Views/PagedProxyModel.h>
#include <QSet>
#include <algorithm>
namespace basewidgets {
PagedProxyModel::PagedProxyModel(QObject* parent) : QAbstractProxyModel(parent) {}
PagedProxyModel::~PagedProxyModel() { for(const auto& c:m_connections) disconnect(c); }
QModelIndex PagedProxyModel::canonicalIndex(const QModelIndex& input) const
{
    if(!input.isValid()||input.parent().isValid()) return {};
    QModelIndex index=input.sibling(input.row(),0);
    for(int depth=0;depth<64;++depth) {
        auto* proxy=qobject_cast<const QAbstractProxyModel*>(index.model());
        if(!proxy) return index;
        index=proxy->mapToSource(index); if(!index.isValid()) return {};
    }
    return {};
}
QModelIndex PagedProxyModel::resolvePin(const QPersistentModelIndex& pin) const
{
    if(!pin.isValid()||!sourceModel()) return {};
    QVector<const QAbstractProxyModel*> chain;
    const QAbstractItemModel* model=sourceModel();
    while(model && model!=pin.model() && chain.size()<64) {
        auto* proxy=qobject_cast<const QAbstractProxyModel*>(model); if(!proxy) return {};
        chain.append(proxy); model=proxy->sourceModel();
    }
    if(model!=pin.model()) return {};
    QModelIndex index=pin;
    for(auto it=chain.crbegin();it!=chain.crend();++it) { index=(*it)->mapFromSource(index); if(!index.isValid()) return {}; }
    return index.parent().isValid()?QModelIndex():index;
}
void PagedProxyModel::setSourceModel(QAbstractItemModel* source)
{
    if(source==sourceModel()) return;
    const QAbstractItemModel* model=source;
    for(int depth=0;model && depth<64;++depth) {
        if(model==this) { qWarning("PagedProxyModel: cyclic source rejected"); return; }
        auto* proxy=qobject_cast<const QAbstractProxyModel*>(model); if(!proxy) break; model=proxy->sourceModel();
    }
    beginResetModel();
    for(const auto& c:m_connections) disconnect(c); m_connections.clear();
    m_resetDepth=0; m_topPins.clear(); m_bottomPins.clear(); m_currentPage=0;
    QAbstractProxyModel::setSourceModel(source);
    if(source) {
        auto before=[this] { beginSourceChange(); }; auto after=[this] { endSourceChange(); };
        m_connections << connect(source,&QAbstractItemModel::modelAboutToBeReset,this,before)
            << connect(source,&QAbstractItemModel::modelReset,this,after)
            << connect(source,&QAbstractItemModel::layoutAboutToBeChanged,this,before)
            << connect(source,&QAbstractItemModel::layoutChanged,this,after)
            << connect(source,&QAbstractItemModel::rowsAboutToBeInserted,this,before)
            << connect(source,&QAbstractItemModel::rowsInserted,this,after)
            << connect(source,&QAbstractItemModel::rowsAboutToBeRemoved,this,before)
            << connect(source,&QAbstractItemModel::rowsRemoved,this,after)
            << connect(source,&QAbstractItemModel::rowsAboutToBeMoved,this,before)
            << connect(source,&QAbstractItemModel::rowsMoved,this,after)
            << connect(source,&QAbstractItemModel::columnsAboutToBeInserted,this,before)
            << connect(source,&QAbstractItemModel::columnsInserted,this,after)
            << connect(source,&QAbstractItemModel::columnsAboutToBeRemoved,this,before)
            << connect(source,&QAbstractItemModel::columnsRemoved,this,after)
            << connect(source,&QAbstractItemModel::columnsAboutToBeMoved,this,before)
            << connect(source,&QAbstractItemModel::columnsMoved,this,after)
            << connect(source,&QAbstractItemModel::dataChanged,this,&PagedProxyModel::forwardData)
            << connect(source,&QAbstractItemModel::headerDataChanged,this,[this](Qt::Orientation orientation,int first,int last) {
                if(orientation==Qt::Horizontal) emit headerDataChanged(orientation,first,last);
                else if(!m_rows.isEmpty()) emit headerDataChanged(orientation,0,m_rows.size()-1);
            })
            << connect(source,&QObject::destroyed,this,[this] {
                beginResetModel(); m_topPins.clear(); m_bottomPins.clear(); m_top.clear(); m_bottom.clear(); m_normal.clear(); m_rows.clear(); m_reverse.clear(); m_currentPage=0;
                endResetModel(); emit pageChanged(0,0);
            });
    }
    rebuildRows(); endResetModel(); emit pageChanged(m_currentPage,pageCount());
}
void PagedProxyModel::beginSourceChange() { if(m_resetDepth++==0) beginResetModel(); }
void PagedProxyModel::endSourceChange()
{
    if(m_resetDepth==0) beginResetModel(); else if(--m_resetDepth>0) return;
    rebuildRows(); endResetModel(); emit pageChanged(m_currentPage,pageCount());
}
void PagedProxyModel::rebuildRows()
{
    m_top.clear(); m_bottom.clear(); m_normal.clear(); m_rows.clear();
    const int count=sourceModel()?sourceModel()->rowCount():0;
    m_reverse=QVector<int>(count,-1); QSet<int> pinned;
    auto collect=[&](const QVector<QPersistentModelIndex>& pins,QVector<int>& output) {
        for(const auto& pin:pins) {
            const auto index=resolvePin(pin);
            if(index.isValid() && index.row()>=0 && index.row()<count && !pinned.contains(index.row())) { pinned.insert(index.row()); output.append(index.row()); }
        }
    };
    collect(m_topPins,m_top); collect(m_bottomPins,m_bottom);
    for(int row=0;row<count;++row) if(!pinned.contains(row)) m_normal.append(row);
    m_currentPage=qBound(0,m_currentPage,qMax(0,pageCount()-1)); rebuildPage();
}
void PagedProxyModel::rebuildPage()
{
    for(int row:m_rows) if(row>=0 && row<m_reverse.size()) m_reverse[row]=-1;
    m_rows=m_top;
    const qint64 begin=m_paginated?qint64(m_currentPage)*m_pageSize:0;
    const qint64 end=m_paginated?qMin(qint64(m_normal.size()),begin+m_pageSize):m_normal.size();
    for(qint64 row=begin;row<end;++row) m_rows.append(m_normal[int(row)]);
    m_rows+=m_bottom;
    for(int row=0;row<m_rows.size();++row) m_reverse[m_rows[row]]=row;
}
bool PagedProxyModel::setPinnedIndexes(const QModelIndexList& top,const QModelIndexList& bottom)
{
    if(top.size()+bottom.size()>64) return false;
    QVector<QPersistentModelIndex> a,b; QSet<QPersistentModelIndex> seen;
    auto collect=[&](const QModelIndexList& input,QVector<QPersistentModelIndex>& output) {
        for(const auto& index:input) {
            const QPersistentModelIndex canonical=canonicalIndex(index);
            if(!canonical.isValid()||seen.contains(canonical)) return false;
            // Resolve ancestry even if filtering currently hides this row.
            const QAbstractItemModel* source=sourceModel(); int depth=0;
            while(source && source!=canonical.model() && depth++<64) { auto* proxy=qobject_cast<const QAbstractProxyModel*>(source); source=proxy?proxy->sourceModel():nullptr; }
            if(source!=canonical.model()) return false;
            seen.insert(canonical); output.append(canonical);
        }
        return true;
    };
    if(!collect(top,a)||!collect(bottom,b)) return false;
    if(a==m_topPins && b==m_bottomPins) return true;
    beginResetModel(); m_topPins=a; m_bottomPins=b; rebuildRows(); endResetModel(); emit pageChanged(m_currentPage,pageCount()); return true;
}
QModelIndex PagedProxyModel::mapToSource(const QModelIndex& index) const
{
    if(!index.isValid()||index.model()!=this||!sourceModel()||index.row()<0||index.row()>=m_rows.size()||index.column()<0||index.column()>=columnCount()) return {};
    return sourceModel()->index(m_rows[index.row()],index.column());
}
QModelIndex PagedProxyModel::mapFromSource(const QModelIndex& source) const
{
    if(!source.isValid()||source.model()!=sourceModel()||source.parent().isValid()||source.row()<0||source.row()>=m_reverse.size()) return {};
    return index(m_reverse[source.row()],source.column());
}
QModelIndex PagedProxyModel::index(int row,int column,const QModelIndex& parent) const
{
    return !parent.isValid() && row>=0 && row<rowCount() && column>=0 && column<columnCount() ? createIndex(row,column) : QModelIndex();
}
int PagedProxyModel::rowCount(const QModelIndex& parent) const { return parent.isValid()?0:m_rows.size(); }
int PagedProxyModel::columnCount(const QModelIndex& parent) const { return !parent.isValid()&&sourceModel()?sourceModel()->columnCount():0; }
QVariant PagedProxyModel::data(const QModelIndex& index,int role) const
{
    const auto mapped=mapToSource(index); if(!mapped.isValid()) return {};
    if(role==PinPositionRole) return index.row()<m_top.size()?1:index.row()>=m_rows.size()-m_bottom.size()?-1:0;
    return sourceModel()->data(mapped,role);
}
bool PagedProxyModel::setData(const QModelIndex& index,const QVariant& value,int role)
{
    const auto mapped=mapToSource(index); return role!=PinPositionRole&&mapped.isValid()&&sourceModel()->setData(mapped,value,role);
}
Qt::ItemFlags PagedProxyModel::flags(const QModelIndex& index) const { const auto mapped=mapToSource(index); return mapped.isValid()?sourceModel()->flags(mapped):Qt::NoItemFlags; }
QVariant PagedProxyModel::headerData(int section,Qt::Orientation orientation,int role) const
{
    if(!sourceModel()||section<0) return {};
    if(orientation==Qt::Horizontal) return section<columnCount()?sourceModel()->headerData(section,orientation,role):QVariant();
    if(section>=m_rows.size()) return {};
    const QVariant value=sourceModel()->headerData(m_rows[section],orientation,role);
    if(role==Qt::DisplayRole) {
        const int position=data(index(section,0),PinPositionRole).toInt();
        if(position) return (position>0?QString::fromUtf8("↑ "):QString::fromUtf8("↓ "))+value.toString();
    }
    return value;
}
void PagedProxyModel::forwardData(const QModelIndex& first,const QModelIndex& last,const QVector<int>& roles)
{
    if(m_resetDepth||first.parent().isValid()||last.parent().isValid()) return;
    int begin=-1;
    for(int row=0;row<=m_rows.size();++row) {
        const bool affected=row<m_rows.size()&&m_rows[row]>=first.row()&&m_rows[row]<=last.row();
        if(affected && begin<0) begin=row;
        if(!affected && begin>=0) { emit dataChanged(index(begin,first.column()),index(row-1,last.column()),roles); begin=-1; }
    }
}
void PagedProxyModel::sort(int column,Qt::SortOrder order) { if(sourceModel()) sourceModel()->sort(column,order); }
void PagedProxyModel::setPageSize(int size)
{
    if(size<=0||m_pageSize==size) return;
    beginResetModel(); m_pageSize=size; m_currentPage=0; rebuildPage(); endResetModel(); emit pageChanged(m_currentPage,pageCount());
}
void PagedProxyModel::setCurrentPage(int page)
{
    const int target=qBound(0,page,qMax(0,pageCount()-1)); if(target==m_currentPage) return;
    beginResetModel(); m_currentPage=target; rebuildPage(); endResetModel(); emit pageChanged(m_currentPage,pageCount());
}
void PagedProxyModel::setPaginationEnabled(bool enabled)
{
    if(m_paginated==enabled) return;
    beginResetModel(); m_paginated=enabled; m_currentPage=0; rebuildPage(); endResetModel(); emit pageChanged(0,pageCount());
}
int PagedProxyModel::totalRows() const { return sourceModel()?sourceModel()->rowCount():0; }
int PagedProxyModel::pageCount() const
{
    if(m_normal.isEmpty()) return pinnedRowCount()>0?1:0;
    return m_paginated?int((qint64(m_normal.size())+m_pageSize-1)/m_pageSize):1;
}
}
