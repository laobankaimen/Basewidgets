#pragma once
#include <QIcon>
#include <basewidgets/Core/StyledWidget.h>
class QLabel;
namespace basewidgets {
class PushButton;
/** 空列表/无搜索结果/待连接时的占位面板。默认自绘资料托盘图形，可设置业务图标。
 * 标题/说明使用可换行纯文本；无动画、不创建数据、不自动弹窗。
 * 主动作和次动作仅发信号，业务负责加载/重试/清除筛选。所有 API 在 GUI 线程调用。
 */
class ACSS_UI_EXPORT EmptyState : public StyledWidget {
    Q_OBJECT
public:
    explicit EmptyState(QWidget* parent=nullptr);
    void setTitle(const QString& title);
    QString title() const;
    void setDescription(const QString& description);
    QString description() const;
    /** 空图标恢复内置矢量图形；QIcon 在实际 DPI 目标绘制。 */
    void setIcon(const QIcon& icon);
    QIcon icon() const;
    /** 空字符串隐藏对应动作；两个按钮分行排列，避免窄窗口挤压。 */
    void setActionText(const QString& text);
    QString actionText() const;
    void setSecondaryActionText(const QString& text);
    QString secondaryActionText() const;
signals:
    void actionRequested();
    void secondaryActionRequested();
private:
    void updateColors();
    QLabel *m_title, *m_description;
    QWidget* m_art;
    PushButton *m_action, *m_secondary;
};
}
