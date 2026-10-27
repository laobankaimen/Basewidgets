//
// Created by zhoutao@zkhx.com on 2026/1/9.
//

#ifndef BASEWIDGETS_BASEWINDOWINTERFACE_H
#define BASEWIDGETS_BASEWINDOWINTERFACE_H

#include <QWidget>

namespace basewidgets {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using NativeEventResult = qintptr;
#else
using NativeEventResult = long;
#endif
enum FramelessWindowState {Maximized,Minimized,FullScreen,Normal};
class BaseWidgetInterface {
public:
    explicit BaseWidgetInterface() = default;

    virtual ~BaseWidgetInterface() = default;

    virtual void setTitle(const QString& title) = 0;
    virtual void setContentWidget(QWidget* w) = 0;
    virtual void setFramelessWindowState(FramelessWindowState state) = 0;
private:
    virtual void initUI() = 0;
};
}
#endif //BASEWIDGETS_BASEWINDOWINTERFACE_H
