/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TABWIDGET_H
#define BASEWIDGETS_TABWIDGET_H

#include <QTabWidget>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 标签页容器；页面归容器持有，关闭/移除页面后的销毁由调用方按 Qt 约定处理。 */
class ACSS_UI_EXPORT TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget* parent = nullptr);
    explicit TabWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TabWidget() override;

    /** 标签 hover 底色默认关闭；仅影响鼠标外观，不影响选中、点击和键盘焦点。 */
    void setHoverEffectEnabled(bool enabled);
    bool hoverEffectEnabled() const;
    /** 页面内容外框的 hover 强调默认关闭，与标签 hover 独立；不影响子输入框和键盘焦点。 */
    void setContentHoverEffectEnabled(bool enabled);
    bool contentHoverEffectEnabled() const;

    /** 兼容管理器入口；不转移所有权。全应用主题请通过 StyleManagerAdapter 设置。 */
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

#endif // BASEWIDGETS_TABWIDGET_H

