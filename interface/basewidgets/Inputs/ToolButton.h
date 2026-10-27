/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TOOLBUTTON_H
#define BASEWIDGETS_TOOLBUTTON_H

#include <QToolButton>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Core/ButtonAppearance.h>

namespace basewidgets {

/** 工具按钮。保留图文排列、autoRaise、箭头和分离菜单交互。 */
class ACSS_UI_EXPORT ToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget* parent = nullptr);
    explicit ToolButton(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ToolButton() override;

    /** 设置视觉层级；Neutral 为默认值。setChecked() 仍用于表达持续的选中状态。 */
    void setAppearance(ButtonAppearance appearance);
    /** 返回显式层级；兼容 bwPrimary 属性不改变此返回值。 */
    ButtonAppearance appearance() const;

    /** 兼容主题管理器入口；管理器不由按钮持有，全应用颜色由 StyleManagerAdapter 同步。 */
    void setStyleManager(acss::QtAdvancedStylesheet* manager);
    acss::QtAdvancedStylesheet* styleManager() const { return m_manager; }

protected:
    void applyStylesheet();

private:
    void attachToManager();

private slots:
    void onStylesheetChanged();

private:
    acss::QtAdvancedStylesheet* m_manager = nullptr;
};

} // namespace basewidgets

#endif // BASEWIDGETS_TOOLBUTTON_H

