/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_COMBOBOX_H
#define BASEWIDGETS_COMBOBOX_H

#include <QComboBox>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 下拉框包装；编辑、弹出列表和模型接口沿用 QComboBox，外观由全应用主题管理。 */
class ACSS_UI_EXPORT ComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit ComboBox(QWidget* parent = nullptr);
    explicit ComboBox(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ComboBox() override;

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

#endif // BASEWIDGETS_COMBOBOX_H

