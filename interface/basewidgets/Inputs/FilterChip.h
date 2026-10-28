#pragma once
#include <QAbstractButton>
#include <basewidgets/Core/FeedbackTone.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {
/** 可选中/移除的筛选标签，默认 checkable=true、checked=false、removable=false。
 * clicked/toggled 沿用 QAbstractButton；Space/Enter 切换，Delete 请求移除。
 * 移除只发信号，不自动隐藏/删除，不触发 toggled；业务负责同步筛选模型。
 * 长文字绘制时省略，text()/无障碍名称保留全文；移除按钮可独立 Tab 聚焦。
 */
class ACSS_UI_EXPORT FilterChip : public QAbstractButton {
    Q_OBJECT
public:
    explicit FilterChip(const QString& text=QString(), QWidget* parent=nullptr);
    void setRemovable(bool removable);
    bool isRemovable() const { return m_removable; }
    void setTone(FeedbackTone tone);
    FeedbackTone tone() const { return m_tone; }
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void removeRequested();
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void changeEvent(QEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    bool hitButton(const QPoint& position) const override;
private:
    void layoutRemoveButton();
    QAbstractButton* m_remove;
    bool m_removable=false;
    FeedbackTone m_tone=FeedbackTone::Info;
};
}
