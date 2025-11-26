#ifndef QTADVANCEDUI_FRAMELESSDIALOG_H
#define QTADVANCEDUI_FRAMELESSDIALOG_H

#include <QDialog>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QWidget>
#include <QLabel>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT FramelessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget* parent = nullptr);
    ~FramelessDialog() override;

    void setTitle(const QString& title);
    QString title() const;

    void setContentWidget(QWidget* w);
    QWidget* contentWidget() const { return m_contentHost; }

    // 圆角与阴影开关
    void setCornerRadius(int radiusPx);
    int cornerRadius() const { return m_cornerRadius; }
    // 便捷开关：启用/关闭圆角（关闭即半径为 0；启用恢复上次非零半径）
    void setRoundedCornersEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void changeEvent(QEvent* e) override;
#ifdef Q_OS_WIN
    //windows下使用窗口管理器处理拖动和缩放
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
private:
    void initUI();

private:
    QWidget* m_titleBar = nullptr;
    QLabel* m_titleLabel = nullptr;
    QWidget* m_contentHost = nullptr;

    // 圆角
    int m_cornerRadius = 0;
    int m_lastCornerRadiusNonZero = 12; // 记录最近一次非零圆角，便于关闭后再次启用恢复
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSDIALOG_H
