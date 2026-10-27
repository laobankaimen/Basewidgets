/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_LINEEDIT_H
#define BASEWIDGETS_LINEEDIT_H

#include <QLineEdit>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 单行编辑器包装；保留验证器、回显模式、选择与输入法接口。 */
class ACSS_UI_EXPORT LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget* parent = nullptr);
    explicit LineEdit(const QString& text, QWidget* parent = nullptr);
    explicit LineEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~LineEdit() override;

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

#endif // BASEWIDGETS_LINEEDIT_H

