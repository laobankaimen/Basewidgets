/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_LABEL_H
#define BASEWIDGETS_LABEL_H

#include <QLabel>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 文本标签包装；富文本、换行与选择沿用 QLabel；单行省略请使用 ElidedLabel。 */
class ACSS_UI_EXPORT Label : public QLabel
{
    Q_OBJECT
public:
    explicit Label(QWidget* parent = nullptr);
    explicit Label(const QString& text, QWidget* parent = nullptr);
    explicit Label(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~Label() override;

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

#endif // BASEWIDGETS_LABEL_H

