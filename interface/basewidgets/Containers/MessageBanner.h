#pragma once
#include <basewidgets/Core/StyledWidget.h>
#include <basewidgets/Core/FeedbackTone.h>
class QLabel;
class QAbstractButton;
namespace basewidgets {
class PushButton;
/** 页面内非模态提示，可用于导入结果、离线状态和告警；所有文字按纯文本处理。
 * 标题/正文自动换行，动作独立成行以适应窄窗口，无定时器、不抢焦点。
 * 默认 Info、可关闭。关闭后 hide() 并发 dismissed；可用 show() 再次显示。
 * actionRequested 仅通知业务，不自动关闭或执行动作。所有 API 在 GUI 线程调用。
 */
class ACSS_UI_EXPORT MessageBanner : public StyledWidget {
    Q_OBJECT
public:
    explicit MessageBanner(QWidget* parent=nullptr);
    void setTitle(const QString& title);
    QString title() const;
    void setMessage(const QString& message);
    QString message() const;
    void setTone(FeedbackTone tone);
    FeedbackTone tone() const { return m_tone; }
    /** 空字符串隐藏动作；动作按钮支持键盘操作。 */
    void setActionText(const QString& text);
    QString actionText() const;
    void setDismissible(bool dismissible);
    bool isDismissible() const { return m_dismissible; }
public slots:
    /** 仅在可关闭且未隐藏时生效，重复关闭不重复发 dismissed。 */
    void dismiss();
signals:
    void actionRequested();
    void dismissed();
protected:
    void paintEvent(QPaintEvent*) override;
    void changeEvent(QEvent*) override;
private:
    void updateColors();
    QLabel *m_title, *m_message;
    QWidget* m_symbol;
    PushButton* m_action;
    QAbstractButton* m_close;
    FeedbackTone m_tone=FeedbackTone::Info;
    bool m_dismissible=true;
};
}
