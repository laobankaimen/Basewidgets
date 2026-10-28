#pragma once
#include <QFrame>
#include <QPointer>
#include <basewidgets/QtAdvancedUIExport.h>
namespace basewidgets {
class NotificationPanel;
class NotificationModel;
/** 消息面板的非模态弹出包装。Qt::Popup 支持点击外部关闭；Escape 关闭。
 * popup(anchor) 按锚点所在屏幕的 availableGeometry 放置，边缘自动避让。
 * 模型借用、面板由弹窗持有；所有 API 在 GUI 线程调用。
 */
class ACSS_UI_EXPORT NotificationPopup : public QFrame {
    Q_OBJECT
public:
    explicit NotificationPopup(QWidget* parent=nullptr);
    NotificationPanel* panel() const { return m_panel; }
    void setModel(NotificationModel* model);
    void popup(QWidget* anchor); ///< 空/隐藏锚点忽略；复用本实例，关闭不删除。
protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    NotificationPanel* m_panel=nullptr;
    QPointer<QWidget> m_anchor;
};
}
