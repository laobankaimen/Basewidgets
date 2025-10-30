#ifndef QTADVANCEDUI_FRAMELESSDIALOG_H
#define QTADVANCEDUI_FRAMELESSDIALOG_H

#include <QtAdvancedUI/Core/StyledDialog.h>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QWidget>
#include <QLabel>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT FramelessDialog : public StyledDialog
{
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget* parent = nullptr);
    explicit FramelessDialog(acss::QtAdvancedStylesheet* manager, QWidget* parent = nullptr);
    ~FramelessDialog() override;

    void setTitle(const QString& title);
    QString title() const;

    void setContentWidget(QWidget* w);
    QWidget* contentWidget() const { return m_contentHost; }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initUI();

private:
    QWidget* m_titleBar = nullptr;
    QLabel* m_titleLabel = nullptr;
    QWidget* m_contentHost = nullptr;
    QWidget* m_closeBtn = nullptr; // Use ToolButton but keep opaque to avoid include cycles

    bool m_dragging = false;
    QPoint m_dragStartPos;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSDIALOG_H