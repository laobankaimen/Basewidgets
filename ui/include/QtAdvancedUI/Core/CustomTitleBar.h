#ifndef QTADVANCEDUI_CUSTOMTITLEBAR_H
#define QTADVANCEDUI_CUSTOMTITLEBAR_H

#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QWidget>
#include <QPoint>

class QLabel;
class QMenuBar;

namespace QtAdvancedUI {

class ToolButton;

// 一个可复用的自定义标题栏控件，提供最小化、最大化/还原、关闭按钮，并支持拖动与贴边停靠
class ACSS_UI_EXPORT CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTitleBar(QWidget* parent = nullptr);
    ~CustomTitleBar() override;

    void attachToWindow(QWidget* window);
    QWidget* attachedWindow() const { return m_window; }

public slots:
    void setCornerRadius(int radiusPx);//废弃
    int cornerRadius() const { return m_cornerRadius; }
    void setTitle(const QString& title);
    void updateForWindowState();
    // 仅用于对话框等场景：按钮显隐与双击开关
    void setButtonsVisible(bool showMinimize, bool showMaximize, bool showClose);
    void setDoubleClickToToggleEnabled(bool enabled);

protected:
    // 移除所有拖动相关的事件处理，让framelesswindow处理
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
    void initUI();
    // 移除贴边停靠逻辑，让framelesswindow处理
    void snapDockIfNeeded(const QPoint& globalReleasePos);
    void toggleMaximizeRestore(bool isDoubleClick = false);

private:
    QWidget* m_window = nullptr;
    QLabel* m_titleLabel = nullptr;
    ToolButton* m_btnMin = nullptr;
    ToolButton* m_btnMax = nullptr;
    ToolButton* m_btnClose = nullptr;

    bool m_doubleClickToggleEnabled = true;

    // 移除所有拖动和状态记录相关的成员变量
    bool m_dragging = false;
    QPoint m_dragStartPos;

    int m_cornerRadius = 0;

    QRect normalGeometry;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_CUSTOMTITLEBAR_H
