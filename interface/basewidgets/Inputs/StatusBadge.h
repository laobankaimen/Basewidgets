#pragma once
#include <QWidget>
#include <basewidgets/Core/FeedbackTone.h>
#include <basewidgets/QtAdvancedUIExport.h>
namespace basewidgets {
/** 只读状态徽标：轻底色、语义圆点与文字，不参与焦点或鼠标操作。
 * 适合设备在线/核验结果/任务状态；状态必须用文字说明，不仅靠颜色。
 * GUI 线程使用，长文字省略但 text()/tooltip/无障碍名称保留原文。
 */
class ACSS_UI_EXPORT StatusBadge : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool dotVisible READ isDotVisible WRITE setDotVisible)
public:
    explicit StatusBadge(const QString& text=QString(),QWidget* parent=nullptr);
    void setText(const QString& text);
    QString text() const { return m_text; }
    void setTone(FeedbackTone tone); ///< 默认 Neutral；切换主题自动同步实际颜色。
    FeedbackTone tone() const { return m_tone; }
    void setDotVisible(bool visible); ///< 默认显示圆点；文字始终保留。
    bool isDotVisible() const { return m_dotVisible; }
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
protected:
    void paintEvent(QPaintEvent*) override;
    void changeEvent(QEvent*) override;
private:
    QString m_text;
    FeedbackTone m_tone=FeedbackTone::Neutral;
    bool m_dotVisible=true;
};
}
