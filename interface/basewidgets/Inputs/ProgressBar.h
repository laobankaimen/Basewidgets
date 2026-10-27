/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_PROGRESSBAR_H
#define BASEWIDGETS_PROGRESSBAR_H

#include <QProgressBar>
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

/** 进度条包装；setRange(0,0) 为忙碌状态，支持方向、反向、RTL 和 Qt 文本格式。 */
class ACSS_UI_EXPORT ProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget* parent = nullptr);
    explicit ProgressBar(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~ProgressBar() override;

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

#endif // BASEWIDGETS_PROGRESSBAR_H

