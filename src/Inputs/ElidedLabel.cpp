#include <basewidgets/Inputs/ElidedLabel.h>
#include <QPainter>
#include <QStyle>
namespace basewidgets {
ElidedLabel::ElidedLabel(QWidget* parent) : ElidedLabel(QString(),parent) {}
ElidedLabel::ElidedLabel(const QString& text, QWidget* parent) : QLabel(text,parent)
{
    setTextFormat(Qt::PlainText);
    setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
}
QSize ElidedLabel::minimumSizeHint() const { return QSize(0,fontMetrics().height()+2*margin()); }
void ElidedLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    const QRect area = contentsRect().adjusted(margin(),margin(),-margin(),-margin());
    const QString visible = fontMetrics().elidedText(text(),Qt::ElideRight,qMax(0,area.width()));
    style()->drawItemText(&painter,area,alignment()|Qt::TextSingleLine,palette(),isEnabled(),visible,foregroundRole());
}
}
