#pragma once
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Containers/AboutWidget.h>
namespace basewidgets {
/** 通用关于弹窗，复用 AboutWidget 与简洁标题栏。可 open() 非阻塞展示。
 * 底部关闭按钮、标题栏 ×、Escape 和窗口 close() 均可关闭；点击窗口外不自动关闭。
 * 不自动检查升级或打开外部链接。parent 按 Qt 规则管理所有权。
 */
class ACSS_UI_EXPORT AboutDialog : public FramelessDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent=nullptr);
    bool setInfo(const AboutInfo& info); ///< 验证失败保持已有页面及窗口标题。
    bool setConfiguration(const QJsonObject& configuration); ///< 与 AboutWidget 同一 JSON 协议，成功后同步标题。
    AboutWidget* aboutWidget() const { return m_about; } ///< 借用，可连接链接/复制信号或隐藏复制按钮。
public slots:
    /** 保留 QDialog 取消语义（Rejected），不沿用无页脚基类的关闭拦截。 */
    void reject() override;
private:
    AboutWidget* m_about=nullptr;
};
}
