#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QVBoxLayout;
namespace basewidgets {
class ToolButton;
/** 可折叠分组：标题可点击或按 Space/Enter，内容折叠时保留对象和输入值。
 * 无高度动画/轮询，适合高级参数、连接详情和筛选条件；GUI 线程使用。
 */
class ACSS_UI_EXPORT CollapsibleSection : public StyledWidget {
    Q_OBJECT
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)
public:
    explicit CollapsibleSection(const QString& title=QString(),QWidget* parent=nullptr);
    void setTitle(const QString& title); ///< 长标题单行省略，tooltip/无障碍名称保留完整文字。
    QString title() const;
    bool isExpanded() const { return m_expanded; } ///< 默认展开。
    QWidget* contentWidget() const { return m_content; } ///< 借用；子控件归此容器持有。
    QVBoxLayout* contentLayout() const { return m_layout; } ///< 借用；addWidget/addLayout，勿替换/删除。
public slots:
    void setExpanded(bool expanded); ///< 幂等；折叠前将内容内焦点移回标题，隐藏内容不销毁。
signals:
    void expandedChanged(bool expanded);
protected:
    void changeEvent(QEvent*) override;
private:
    void updateArrow();
    ToolButton* m_header=nullptr;
    QWidget* m_content=nullptr;
    QVBoxLayout* m_layout=nullptr;
    bool m_expanded=true;
};
}
