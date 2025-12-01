#ifndef QTADVANCEDUI_FRAMELESSDIALOG_H
#define QTADVANCEDUI_FRAMELESSDIALOG_H

#include <QDialog>
#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QWidget>
#include <QVBoxLayout>

namespace QtAdvancedUI {

class ACSS_UI_EXPORT FramelessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FramelessDialog(QWidget* parent = nullptr);
    ~FramelessDialog() override;

    void setTitle(const QString& title);
    void setContentWidget(QWidget* w);
    QWidget* contentWidget() const { return m_contentHost; }

protected:
#ifdef Q_OS_WIN
    //windows下使用窗口管理器处理拖动和缩放
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
private:
    void initUI();

private:
    QWidget* m_titleBar = nullptr;
    QWidget* m_contentHost = nullptr;
    QVBoxLayout* mainLayout = nullptr;
    int m_titlebarHeight = 30;
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_FRAMELESSDIALOG_H
