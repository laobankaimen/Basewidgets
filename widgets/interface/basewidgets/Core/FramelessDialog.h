/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#ifndef BASEWIDGETS_FRAMELESSDIALOG_H
#define BASEWIDGETS_FRAMELESSDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <basewidgets/QtAdvancedUIExport.h>
#include <basewidgets/Core/BaseWidgetInterface.h>
#include <QWidget>
#include <QVBoxLayout>

namespace basewidgets {

class ACSS_UI_EXPORT FramelessDialog : public QDialog , public BaseWidgetInterface
{
    Q_OBJECT
public:
    explicit FramelessDialog(bool popDialog = true,QWidget* parent = nullptr);
    ~FramelessDialog() override;

    void setTitle(const QString& title) override;
    void setContentWidget(QWidget* w) override;
    void setResizeable(bool canResize = true);
    void setCustomDialogBtn(bool showOK = true,bool showCancel = true,bool okHighLight = true);
    QWidget* contentWidget() const { return m_contentHost; }
    void setFramelessWindowState(FramelessWindowState state) override;
    void reject() override;

protected:
#ifdef Q_OS_WIN
    //windows下使用窗口管理器处理拖动和缩放
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
private:
    void initUI() override;
signals:
    void okButtonClicked();
    void cancelButtonClicked();
    void onDialogClosing();
private:
    QWidget* m_titleBar = nullptr;
    QWidget* m_contentHost = nullptr;
    QVBoxLayout* mainLayout = nullptr;
    QDialogButtonBox* btnBox = nullptr;
    int m_titlebarHeight = 40;
    bool isPopDialog = false;
};

} // namespace basewidgets

#endif // BASEWIDGETS_FRAMELESSDIALOG_H

