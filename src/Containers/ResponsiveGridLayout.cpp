#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <QWidget>

namespace basewidgets {
ResponsiveGridLayout::ResponsiveGridLayout(QWidget* parent, int minimumColumnWidth, int maximumColumns)
    : QLayout(parent), m_columnWidth(qMax(1, minimumColumnWidth)), m_maxColumns(qMax(1, maximumColumns))
{
    setContentsMargins(12,12,12,12);
    setSpacing(12);
}
ResponsiveGridLayout::~ResponsiveGridLayout()
{
    while (auto* item = takeAt(0)) delete item;
}
void ResponsiveGridLayout::addItem(QLayoutItem* item) { m_items.append(item); invalidate(); }
int ResponsiveGridLayout::count() const { return m_items.size(); }
QLayoutItem* ResponsiveGridLayout::itemAt(int index) const { return m_items.value(index); }
QLayoutItem* ResponsiveGridLayout::takeAt(int index)
{
    if (index < 0 || index >= m_items.size()) return nullptr;
    auto* item = m_items.takeAt(index); invalidate(); return item;
}
QSize ResponsiveGridLayout::minimumSize() const
{
    QSize result(m_columnWidth,0);
    for (auto* item : m_items) if (!item->isEmpty()) result = result.expandedTo(item->minimumSize());
    const QMargins m = contentsMargins();
    return result + QSize(m.left()+m.right(),m.top()+m.bottom());
}
QSize ResponsiveGridLayout::sizeHint() const
{
    const int columns = qMax(1, qMin(m_maxColumns, count()));
    const int width = minimumSize().width()*columns + qMax(0, spacing())*(columns-1);
    return QSize(width, heightForWidth(width));
}
int ResponsiveGridLayout::heightForWidth(int width) const { return arrange(QRect(0,0,width,0),false); }
void ResponsiveGridLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    arrange(rect,true);
}
int ResponsiveGridLayout::arrange(const QRect& rect, bool apply) const
{
    const QMargins m = contentsMargins();
    const int gap = qMax(0, spacing());
    const int available = qMax(0, rect.width()-m.left()-m.right());
    QList<QLayoutItem*> visible;
    int required = m_columnWidth;
    for (auto* item : m_items) {
        if (item->isEmpty()) continue;
        visible.append(item);
        required = qMax(required,item->minimumSize().width());
    }
    if (visible.isEmpty()) return m.top()+m.bottom();
    const int columns = qMin(visible.size(), qBound(1,(available+gap)/(required+gap),m_maxColumns));
    const int cellWidth = qMax(0,(available-gap*(columns-1))/columns);
    int y = rect.top()+m.top();
    for (int start=0; start<visible.size(); start+=columns) {
        int rowHeight = 0;
        for (int col=0; col<columns && start+col<visible.size(); ++col) {
            auto* item = visible[start+col];
            const int hint = item->hasHeightForWidth() ? item->heightForWidth(cellWidth) : item->sizeHint().height();
            rowHeight = qMax(rowHeight,qMax(hint,item->minimumSize().height()));
        }
        if (apply) {
            for (int col=0; col<columns && start+col<visible.size(); ++col) {
                const int logicalX = col*(cellWidth+gap);
                const int x = parentWidget() && parentWidget()->layoutDirection()==Qt::RightToLeft
                    ? available-logicalX-cellWidth : logicalX;
                visible[start+col]->setGeometry(QRect(rect.left()+m.left()+x,y,cellWidth,rowHeight));
            }
        }
        y += rowHeight+gap;
    }
    return y-gap-rect.top()+m.bottom();
}
}
