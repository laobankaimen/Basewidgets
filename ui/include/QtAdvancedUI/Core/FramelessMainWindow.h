#ifndef QTADVANCEDUI_FRAMELESSMAINWINDOW_H
#define QTADVANCEDUI_FRAMELESSMAINWINDOW_H

#include <QtAdvancedUI/Core/StyledMainWindow.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QPoint>

class QMenuBar;
class QVBoxLayout;

namespace QtAdvancedUI {

class CustomTitleBar;

// 无边框主窗口，集成自定义标题栏，并支持菜单栏复合（标题栏+菜单）
class ACSS_UI_EXPORT FramelessMainWindow : public StyledMainWindow
{
    Q_OBJECT
public:
    explicit FramelessMainWindow(QWidget* parent = nullptr);
    explicit FramelessMainWindow(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~FramelessMainWindow() override;

    // 覆盖设置菜单方法，将菜单置于标题栏下方的复合区域
    void setMenuBar(QMenuBar* menuBar);

    // 角拖拽缩放开关与边距
    void setResizableEdgesEnabled(bool enabled);
    bool resizableEdgesEnabled() const { return m_resizeEnabled; }
    void setResizeMargin(int px);
    int resizeMargin() const { return m_resizeMargin; }

    // 圆角与阴影开关
    void setCornerRadius(int radiusPx);
    int cornerRadius() const { return m_cornerRadius; }
    // 便捷开关：启用/关闭圆角（关闭即半径为 0；启用恢复上次非零半径）
    void setRoundedCornersEnabled(bool enabled);
    void setDropShadowEnabled(bool enabled);
    bool dropShadowEnabled() const { return m_dropShadowEnabled; }


protected:
    void showEvent(QShowEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void changeEvent(QEvent* e) override;

private:
    void initUI();

private:
    CustomTitleBar* m_titleBar = nullptr;
    QWidget* m_menuComposite = nullptr; // 复合区域：标题栏 + 菜单栏
    QVBoxLayout* m_menuCompositeLayout = nullptr;
    QMenuBar* m_menuBarRef = nullptr;

    // 角拖拽缩放
    bool m_resizeEnabled = false;
    int m_resizeMargin = 6;
    enum ResizeRegion { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeRegion m_resizeRegion = None;
    bool m_resizing = false;
    QRect m_startGeo;
    QPoint m_dragStartPos;

    // 圆角与阴影
    int m_cornerRadius = 0;
    int m_lastCornerRadiusNonZero = 12; // 记录最近一次非零圆角，便于关闭后再次启用恢复
    bool m_dropShadowEnabled = false;
    QColor m_backgroundColor;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSMAINWINDOW_H