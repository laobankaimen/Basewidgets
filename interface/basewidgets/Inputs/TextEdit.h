/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_TEXTEDIT_H
#define BASEWIDGETS_TEXTEDIT_H

#include <QTextEdit>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 富文本编辑器包装；使用 setHtml/setPlainText；光标与文档归 Qt 管理。 */
class ACSS_UI_EXPORT TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget* parent = nullptr);
    explicit TextEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~TextEdit() override;

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

#endif // BASEWIDGETS_TEXTEDIT_H

