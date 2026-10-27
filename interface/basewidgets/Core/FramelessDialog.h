/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_FRAMELESSDIALOG_H
#define BASEWIDGETS_FRAMELESSDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QDialogButtonBox>
#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Core/BaseWidgetInterface.h>
#include <QWidget>
#include <QVBoxLayout>
class QShowEvent;

namespace basewidgets {
class CustomTitleBar;

/** 可组合对话框外壳；独立浮层底色和细边框，标准页脚按钮默认纯文字。
 * 内容区与可选 OK/Cancel 页脚分开；不强制改变消息正文中显式设置的图标。
 * 保留 QDialog 激活/模态语义；窗口拖动优先由 Qt 的平台插件交给系统处理。
 */
class ACSS_UI_EXPORT FramelessDialog : public QDialog , public BaseWidgetInterface
{
    Q_OBJECT
public:
    /** popDialog=true 创建按钮页脚；parent 遵循 Qt 所有权规则。 */
    explicit FramelessDialog(bool popDialog = true,QWidget* parent = nullptr);
    explicit FramelessDialog(QWidget* parent);
    ~FramelessDialog() override;
    /** 借用标题栏，可独立配置展示标题、图标、对齐方式与扩展操作。 */
    CustomTitleBar* titleBar() const;

    /** 同步系统窗口标题和自定义标题栏文字。 */
    void setTitle(const QString& title) override;
    /** 接管内容，替换时延迟删除旧内容；nullptr 清空，不重复创建布局。 */
    void setContentWidget(QWidget* w) override;
    /** 控制标题栏最小化/最大化按钮、双击和右下角 size grip。 */
    void setResizeable(bool canResize = true);
    /** 配置已有页脚；不带页脚的对话框忽略此设置，okHighLight 表示主操作强调。 */
    void setCustomDialogBtn(bool showOK = true,bool showCancel = true,bool okHighLight = true);
    QWidget* contentWidget() const { return m_contentWidget; }
    void setFramelessWindowState(FramelessWindowState state) override;
    void reject() override;

protected:
#ifdef Q_OS_WIN
    //windows下使用窗口管理器处理拖动和缩放
    bool nativeEvent(const QByteArray &eventType, void *message, NativeEventResult *result) override;
#endif
    void showEvent(QShowEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void changeEvent(QEvent* e) override;
private:
    void initUI() override;
    void updateSurfacePalette();
signals:
    /** OK 按钮被触发；由业务决定验证后 accept()，不会自动关闭。 */
    void okButtonClicked();
    /** Cancel 按钮被触发；业务可连接 reject()。 */
    void cancelButtonClicked();
    void onDialogClosing();
private:
    QWidget* m_titleBar = nullptr;
    QWidget* m_contentHost = nullptr;
    QPointer<QWidget> m_contentWidget;
    QVBoxLayout* mainLayout = nullptr;
    QDialogButtonBox* btnBox = nullptr;
    bool isPopDialog = false;
    bool m_centeredOnce = false;
};

} // namespace basewidgets

#endif // BASEWIDGETS_FRAMELESSDIALOG_H
