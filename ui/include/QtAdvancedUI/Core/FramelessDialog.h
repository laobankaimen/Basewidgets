#ifndef QTADVANCEDUI_FRAMELESSDIALOG_H
#define QTADVANCEDUI_FRAMELESSDIALOG_H

#include <QtAdvancedUI/Core/StyledDialog.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QWidget>
#include <QLabel>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT FramelessDialog : public StyledDialog
{
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget* parent = nullptr);
    explicit FramelessDialog(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~FramelessDialog() override;

    void setTitle(const QString& title);
    QString title() const;

    void setContentWidget(QWidget* w);
    QWidget* contentWidget() const { return m_contentHost; }

    // 边角拖拽缩放开关与边距
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
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void changeEvent(QEvent* e) override;

private:
    void initUI();

private:
    QWidget* m_titleBar = nullptr;
    QLabel* m_titleLabel = nullptr;
    QWidget* m_contentHost = nullptr;
    QWidget* m_closeBtn = nullptr; // Use ToolButton but keep opaque to avoid include cycles

    bool m_dragging = false;
    QPoint m_dragStartPos;

    // 角拖拽缩放
    bool m_resizeEnabled = false;
    int m_resizeMargin = 6;
    enum ResizeRegion { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeRegion m_resizeRegion = None;
    bool m_resizing = false;
    QRect m_startGeo;

    // 圆角与阴影
    int m_cornerRadius = 0;
    int m_lastCornerRadiusNonZero = 12; // 记录最近一次非零圆角，便于关闭后再次启用恢复
    bool m_dropShadowEnabled = false;
    QColor m_backgroundColor;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSDIALOG_H