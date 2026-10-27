/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_RADIOBUTTON_H
#define BASEWIDGETS_RADIOBUTTON_H

#include <QRadioButton>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 单选按钮包装；同一父容器中默认自动互斥，跨容器分组请使用 QButtonGroup。 */
class ACSS_UI_EXPORT RadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit RadioButton(QWidget* parent = nullptr);
    explicit RadioButton(const QString& text, QWidget* parent = nullptr);
    explicit RadioButton(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~RadioButton() override;

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

#endif // BASEWIDGETS_RADIOBUTTON_H

