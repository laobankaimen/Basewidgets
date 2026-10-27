/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_PUSHBUTTON_H
#define BASEWIDGETS_PUSHBUTTON_H

#include <QPushButton>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Core/ButtonAppearance.h>

namespace basewidgets {

/** 标准操作按钮。保留 QPushButton 的快捷键、默认按钮、菜单及选中语义。 */
class ACSS_UI_EXPORT PushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButton(QWidget* parent = nullptr);
    explicit PushButton(const QString& text, QWidget* parent = nullptr);
    explicit PushButton(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~PushButton() override;

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

#endif // BASEWIDGETS_PUSHBUTTON_H

