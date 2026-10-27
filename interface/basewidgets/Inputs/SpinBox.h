/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_SPINBOX_H
#define BASEWIDGETS_SPINBOX_H

#include <QSpinBox>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 整数编辑器包装；范围、步长、前后缀和验证接口继承 QSpinBox。 */
class ACSS_UI_EXPORT SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget* parent = nullptr);
    explicit SpinBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~SpinBox() override;

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

#endif // BASEWIDGETS_SPINBOX_H

