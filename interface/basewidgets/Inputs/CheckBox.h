/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_CHECKBOX_H
#define BASEWIDGETS_CHECKBOX_H

#include <QCheckBox>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 三态复选框包装；setCheckState(Qt::PartiallyChecked) 表示半选，交互与无障碍行为继承 QCheckBox。 */
class ACSS_UI_EXPORT CheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit CheckBox(QWidget* parent = nullptr);
    explicit CheckBox(const QString& text, QWidget* parent = nullptr);
    explicit CheckBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~CheckBox() override;

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

#endif // BASEWIDGETS_CHECKBOX_H

