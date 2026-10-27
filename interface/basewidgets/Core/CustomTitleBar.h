/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_CUSTOMTITLEBAR_H
#define BASEWIDGETS_CUSTOMTITLEBAR_H

#include <basewidgets/QtAdvancedUIExport.h>
#include <QWidget>
#include <QPoint>
#include <QPointer>

class QLabel;
class QMenuBar;

namespace basewidgets {

class ToolButton;

// 一个可复用的自定义标题栏控件，提供最小化、最大化/还原、关闭按钮，并支持拖动与贴边停靠
class ACSS_UI_EXPORT CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTitleBar(QWidget* parent = nullptr);
    ~CustomTitleBar() override;

    /** 绑定被控制窗口；不接管所有权，重绑定会断开旧窗口的同步连接。 */
    void attachToWindow(QWidget* window);
    QWidget* attachedWindow() const { return m_window.data(); }

public slots:
    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);
    void updateForWindowState();
    // 仅用于对话框等场景：按钮显隐与双击开关
    /** 保存按钮显隐偏好；后续状态更新不会重新显示被调用方隐藏的按钮。 */
    void setButtonsVisible(bool showMinimize, bool showMaximize, bool showClose);
    /** 禁用后双击不切换最大化；对话框通常关闭此行为。 */
    void setDoubleClickToToggleEnabled(bool enabled);
    void updateIcons();
protected:
    bool event(QEvent* event) override;
#ifndef Q_OS_WIN
    // 非 Windows 平台优先使用 QWindow 系统拖动；Windows 由原生标题栏命中处理。
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
#endif
private:
    void initUI();
    void toggleMaximizeRestore(bool isDoubleClick = false);
    bool isFullScreen();

private:
    QPointer<QWidget> m_window;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    ToolButton* m_btnMin = nullptr;
    ToolButton* m_btnMax = nullptr;
    ToolButton* m_btnClose = nullptr;

    bool m_doubleClickToggleEnabled = true;
    bool m_showMinimize = true;
    bool m_showMaximize = true;
    bool m_showClose = true;

    // 仅手动回退拖动使用；系统拖动绝不留下此状态。
    bool m_dragging = false;
    QPoint m_dragStartPos;

};

} // namespace basewidgets

#endif // BASEWIDGETS_CUSTOMTITLEBAR_H

