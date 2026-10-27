#pragma once
#include <basewidgets/Core/FramelessWidgetWindow.h>
namespace basewidgets {
/** 多屏业务窗口。Primary 用于全局导航/账户/设置，Secondary 用于某个业务视图。
 * 两者都是真正的顶层窗口，不把子屏做成 QDialog 或主窗口里的子控件。
 * 默认内容为空，应用用 setContentWidget 注入各自页面，可共享同一业务模型。
 */
class ACSS_UI_EXPORT ScreenWindow : public FramelessWidgetWindow {
    Q_OBJECT
public:
    enum class Role { Primary, Secondary };
    Q_ENUM(Role)
    explicit ScreenWindow(Role role=Role::Primary,QWidget* parent=nullptr);
    Role role() const { return m_role; }
    /** 设置当前窗口的状态提示；主/子屏分别维护，不自动复制主屏连接或进度状态。 */
    void setContextText(const QString& text);
signals:
    /** 子屏状态栏“返回主屏”入口，由工作区管理器激活主窗口。 */
    void primaryWindowRequested();
    /** closeEvent 被接受后发送；关闭子屏只隐藏，管理器保留实例供重新打开。 */
    void closeAccepted();
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    Role m_role;
};
}
