/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_PLAINTEXTEDIT_H
#define BASEWIDGETS_PLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 纯文本编辑器包装；使用 setPlainText/appendPlainText 管理纯文本内容。 */
class ACSS_UI_EXPORT PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit PlainTextEdit(QWidget* parent = nullptr);
    explicit PlainTextEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~PlainTextEdit() override;

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

#endif // BASEWIDGETS_PLAINTEXTEDIT_H

