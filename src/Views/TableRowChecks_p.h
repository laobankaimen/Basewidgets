// Private UI helpers: checkboxes live in the row header, so business column numbers,
// delegates, sorting, source CheckStateRole and CSV contents remain unchanged.
#pragma once
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStyleOptionHeader>
#include <QStyleOptionButton>
#include <functional>
namespace basewidgets {
class CheckedRowsHeader : public QHeaderView {
public:
    explicit CheckedRowsHeader(QWidget* parent) : QHeaderView(Qt::Vertical,parent)
    {
        setSectionsClickable(true);
        setToolTip(tr("勾选记录；勾选跨页保留。也可聚焦单元格后按 Ctrl+Space。"));
    }
    bool checksVisible=true;
    std::function<bool(int)> isChecked;
    std::function<bool(int)> isCheckable;
    std::function<void(int)> toggle;
    int checkWidth() const { return style()->pixelMetric(QStyle::PM_IndicatorWidth,nullptr,this)+12; }
    QSize sizeHint() const override {
        auto size=QHeaderView::sizeHint(); if(checksVisible)size.rwidth()+=checkWidth(); return size;
    }
protected:
    void paintSection(QPainter* painter,const QRect& rectangle,int row) const override
    {
        if(!checksVisible) { QHeaderView::paintSection(painter,rectangle,row); return; }
        painter->save();
        // Paint the native row label separately, preserving pin arrows and native states.
        QRect label=rectangle; label.setLeft(label.left()+checkWidth());
        QStyleOptionHeader background; initStyleOption(&background); background.rect=rectangle; background.text.clear();
        painter->save();
        style()->drawControl(QStyle::CE_Header,&background,painter,this);
        painter->restore();
        painter->save();
        QHeaderView::paintSection(painter,label,row);
        painter->restore(); // QHeaderView narrows the clip to its label rectangle.
        QStyleOptionButton option; option.initFrom(this);
        const bool enabled=isEnabled() && isCheckable && isCheckable(row);
        option.state.setFlag(QStyle::State_Enabled,enabled);
        option.state &= ~(QStyle::State_On|QStyle::State_Off|QStyle::State_NoChange|QStyle::State_HasFocus);
        option.state |= isChecked && isChecked(row)?QStyle::State_On:QStyle::State_Off;
        const int w=style()->pixelMetric(QStyle::PM_IndicatorWidth,&option,this);
        const int h=style()->pixelMetric(QStyle::PM_IndicatorHeight,&option,this);
        option.rect=QRect(rectangle.left()+6,rectangle.center().y()-h/2,w,h);
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox,&option,painter,this);
        painter->restore();
    }
    void mousePressEvent(QMouseEvent* event) override
    {
        const int row=logicalIndexAt(event->pos());
        if(checksVisible && event->button()==Qt::LeftButton && event->pos().x()<checkWidth() && row>=0) {
            m_pressed=row; m_pressedIndex=model()->index(row,0); event->accept(); return;
        }
        QHeaderView::mousePressEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if(m_pressed>=0) {
            const int pressed=m_pressed; m_pressed=-1;
            const bool sameRecord=m_pressedIndex.isValid() && m_pressedIndex.row()==pressed;
            m_pressedIndex=QPersistentModelIndex();
            if(sameRecord && checksVisible && event->button()==Qt::LeftButton && event->pos().x()<checkWidth()
               && logicalIndexAt(event->pos())==pressed && isCheckable && isCheckable(pressed) && toggle)toggle(pressed);
            event->accept(); return;
        }
        QHeaderView::mouseReleaseEvent(event);
    }
private:
    int m_pressed=-1;
    QPersistentModelIndex m_pressedIndex;
};
class RowCheckTableView : public TableView {
public:
    explicit RowCheckTableView(QWidget* parent) : TableView(parent)
    {
        rowChecks=new CheckedRowsHeader(this); setVerticalHeader(rowChecks);
        allCheck=new CheckBox(this); allCheck->setObjectName("AdvancedTableCheckAll");
        allCheck->setAccessibleName(tr("勾选当前页全部记录"));
        allCheck->setToolTip(tr("全选 / 取消当前页；其他页面的勾选保留"));
        allCheck->setFocusPolicy(Qt::StrongFocus);
        setCornerButtonEnabled(false);
    }
    CheckedRowsHeader* rowChecks=nullptr;
    CheckBox* allCheck=nullptr;
    void setChecksVisible(bool visible) {
        rowChecks->checksVisible=visible; setCornerButtonEnabled(!visible);
        rowChecks->updateGeometry(); rowChecks->viewport()->update(); updateGeometries();
    }
    void refreshChecks() { rowChecks->viewport()->update(); updateGeometries(); }
protected:
    void updateGeometries() override {
        TableView::updateGeometries();
        if(!allCheck || !rowChecks)return;
        allCheck->setVisible(rowChecks->checksVisible && !verticalHeader()->isHidden() && !horizontalHeader()->isHidden());
        const QSize size=allCheck->sizeHint();
        allCheck->setGeometry(verticalHeader()->geometry().left()+6,
            horizontalHeader()->geometry().top()+(horizontalHeader()->height()-size.height())/2,size.width(),size.height());
        allCheck->raise();
    }
    void keyPressEvent(QKeyEvent* event) override {
        if(rowChecks->checksVisible && event->key()==Qt::Key_Space && event->modifiers()==Qt::ControlModifier
           && currentIndex().isValid() && rowChecks->isCheckable && rowChecks->isCheckable(currentIndex().row())) {
            if(rowChecks->toggle)rowChecks->toggle(currentIndex().row());
            event->accept(); return;
        }
        TableView::keyPressEvent(event);
    }
};
}
