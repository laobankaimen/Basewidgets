#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QLabel;
class QVBoxLayout;
namespace basewidgets {
/** 仪表盘面板：轻量渐变背景、标题分隔线，无整框 hover 效果。
 * 向 contentLayout() 添加控件，所有权按 Qt 布局规则转交；纯文本标题。
 * GUI 线程使用，色彩跟随主题，适用于统计卡和图表分区。
 */
class ACSS_UI_EXPORT DashboardPanel : public StyledWidget {
    Q_OBJECT
public:
    explicit DashboardPanel(const QString& title=QString(),QWidget* parent=nullptr);
    void setTitle(const QString& title); ///< 空标题隐藏标题行及分隔线。
    QString title() const;
    QVBoxLayout* contentLayout() const { return m_content; } ///< 借用指针，不要替换/删除。
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QLabel* m_title=nullptr;
    QVBoxLayout* m_content=nullptr;
};
}
