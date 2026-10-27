#ifndef BASEWIDGETS_RESPONSIVEGRIDLAYOUT_H
#define BASEWIDGETS_RESPONSIVEGRIDLAYOUT_H
#include <QLayout>
#include <QList>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {
/** 等宽卡片布局：按宽度减少列数，支持 RTL、隐藏项及 heightForWidth。
 * addWidget 的控件所有权归父容器；布局仅负责 QLayoutItem 的生命周期。
 */
class ACSS_UI_EXPORT ResponsiveGridLayout : public QLayout
{
public:
    /** minimumColumnWidth 为逻辑像素期望值；maximumColumns 至少为 1。
     * 单项自身 minimumSize 更大时以其为准，因此过窄窗口应外包 QScrollArea。
     */
    explicit ResponsiveGridLayout(QWidget* parent = nullptr, int minimumColumnWidth = 320, int maximumColumns = 2);
    ~ResponsiveGridLayout() override;
    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    /** 移除并返还布局项，调用方负责删除该项；不删除项中的 QWidget。 */
    QLayoutItem* takeAt(int index) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override;
    Qt::Orientations expandingDirections() const override { return Qt::Horizontal; }
    void setGeometry(const QRect& rect) override;
private:
    int arrange(const QRect& rect, bool apply) const;
    QList<QLayoutItem*> m_items;
    int m_columnWidth;
    int m_maxColumns;
};
}
#endif
