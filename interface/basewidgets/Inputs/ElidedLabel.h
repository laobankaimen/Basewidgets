#ifndef BASEWIDGETS_ELIDEDLABEL_H
#define BASEWIDGETS_ELIDEDLABEL_H
#include <QLabel>
#include <basewidgets/QtAdvancedUIExport.h>
namespace basewidgets {
/** 受限宽度的单行纯文本标签：只在绘制时省略，text() 始终保留完整内容。
 * 不支持富文本/换行。需要完整悬停提示时由调用方 setToolTip(text())。
 */
class ACSS_UI_EXPORT ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget* parent = nullptr);
    explicit ElidedLabel(const QString& text, QWidget* parent = nullptr);
    QSize minimumSizeHint() const override;
protected:
    void paintEvent(QPaintEvent* event) override;
};
}
#endif
