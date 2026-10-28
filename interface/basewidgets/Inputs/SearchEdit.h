#pragma once
#include <basewidgets/Inputs/LineEdit.h>
class QTimer;
class QAction;
namespace basewidgets {
/** 搜索输入框：图标、清除按钮、合并输入请求；不包含检索或网络逻辑。
 * textChanged（包括 setText/clear）在停止输入 300 ms 后发 searchRequested。
 * Return/点击搜索图标立即提交，并取消待发请求；空字符串表示清空筛选。
 * 输入法预编辑不提交；有 validator 时仅提交可接受值。GUI 线程调用。
 */
class ACSS_UI_EXPORT SearchEdit : public LineEdit {
    Q_OBJECT
public:
    explicit SearchEdit(QWidget* parent=nullptr);
    /** 0..60000 ms，0 为下一事件循环合并；失败保留旧值，成功重排待发请求。 */
    bool setDebounceInterval(int milliseconds);
    int debounceInterval() const;
public slots:
    void requestSearch(); ///< 立即提交当前原文（不隐式 trim）；禁用/校验失败不提交。
    void cancelPendingSearch(); ///< 只取消待发请求，不清除文字或撤销业务已经发起的检索。
signals:
    void searchRequested(const QString& text);
protected:
    void keyPressEvent(QKeyEvent*) override; ///< Escape 清空可编辑内容，沿用同一延迟提交规则。
    void changeEvent(QEvent*) override;
private:
    void updateIcon();
    QTimer* m_timer=nullptr;
    QAction* m_searchAction=nullptr;
};
}
