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
    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);
    void updateForWindowState();
    // 仅用于对话框等场景：按钮显隐与双击开关
    void setButtonsVisible(bool showMinimize, bool showMaximize, bool showClose);
    void setDoubleClickToToggleEnabled(bool enabled);
    void updateIcons();
#ifdef Q_OS_LINUX
protected:
    //Linux下拖动需要处理
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
    QWidget* m_window = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    ToolButton* m_btnMin = nullptr;
    ToolButton* m_btnMax = nullptr;
    ToolButton* m_btnClose = nullptr;

    bool m_doubleClickToggleEnabled = true;

    // 移除所有拖动和状态记录相关的成员变量
    bool m_dragging = false;
    QPoint m_dragStartPos;

    QRect normalGeometry;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_CUSTOMTITLEBAR_H
