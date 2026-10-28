#pragma once
#include <QTabBar>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {
/** 紧凑的分段视图切换器；仅管理选择状态，不创建或切换业务页面。
 * 沿用 QTabBar 的 addTab/setTabData/currentIndex/currentChanged、禁用项、
 * 左右键、助记符、RTL 和滚动按钮。可连接 QStackedWidget::setCurrentIndex。
 * 设计用于顶部水平、不可关闭的短选项；默认无鼠标悬停底色。
 * 所有调用均在 GUI 线程，业务应设置 accessibleName 说明选择的用途。
 */
class ACSS_UI_EXPORT SegmentedControl : public QTabBar {
    Q_OBJECT
public:
    explicit SegmentedControl(QWidget* parent=nullptr);
protected:
    void paintEvent(QPaintEvent* event) override;
    QSize tabSizeHint(int index) const override;
    QSize minimumTabSizeHint(int index) const override;
};
}
