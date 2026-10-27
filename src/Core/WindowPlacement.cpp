#include <basewidgets/Core/WindowPlacement.h>
#include <QWidget>
#include <QWindow>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>
#include <limits>
namespace basewidgets {
namespace {
bool compositorPositionsWindows() { return QGuiApplication::platformName().startsWith("wayland"); }
QVector<QRect> availableScreens()
{
    QVector<QRect> result; for(auto* screen:QGuiApplication::screens())result.append(screen->availableGeometry()); return result;
}
QRect fit(const QRect& rect,const QRect& available)
{
    if(!rect.isValid()||!available.isValid())return rect;
    const QSize size=rect.size().boundedTo(available.size());
    return QRect(QPoint(qBound(available.left(),rect.x(),available.right()-size.width()+1),
                        qBound(available.top(),rect.y(),available.bottom()-size.height()+1)),size);
}
void place(QWidget* window,QScreen* screen,const QRect& rect)
{
    if(!window||!screen||!rect.isValid())return;
    if(!window->windowHandle())window->winId();
    if(window->windowHandle() && window->windowHandle()->screen()!=screen)window->windowHandle()->setScreen(screen);
    const QMargins margins=window->windowHandle()?window->windowHandle()->frameMargins():QMargins();
    const QRect available=screen->availableGeometry();
    // QWidget::geometry excludes the native frame, but top-level move() uses the frame origin.
    const QSize frameExtra(margins.left()+margins.right(),margins.top()+margins.bottom());
    const QSize usable=(available.size()-frameExtra).expandedTo(QSize(1,1));
    const QSize size=rect.size().boundedTo(usable).expandedTo(window->minimumSize()).boundedTo(window->maximumSize());
    window->resize(size);
    if(!compositorPositionsWindows()) {
        const QSize frameSize=size+frameExtra;
        const QPoint origin=rect.topLeft()-QPoint(margins.left(),margins.top());
        window->move(qBound(available.left(),origin.x(),qMax(available.left(),available.right()-frameSize.width()+1)),
                     qBound(available.top(),origin.y(),qMax(available.top(),available.bottom()-frameSize.height()+1)));
    }
}
QJsonArray packRect(const QRect& rect) { return {rect.x(),rect.y(),rect.width(),rect.height()}; }
bool unpackRect(const QJsonValue& value,QRect& rect)
{
    if(!value.isArray()||value.toArray().size()!=4)return false;
    const auto a=value.toArray(); int v[4];
    for(int i=0;i<4;++i) {
        const double d=a[i].toDouble(std::numeric_limits<double>::quiet_NaN());
        // Bounds protect QRect arithmetic against corrupt settings; far beyond real desktops.
        if(!std::isfinite(d)||d!=std::floor(d)||std::abs(d)>1000000)return false;
        v[i]=int(d);
    }
    rect=QRect(v[0],v[1],v[2],v[3]); return rect.isValid();
}
}
int WindowPlacement::bestScreen(const QRect& rectangle,const QVector<QRect>& available,int fallback)
{
    int best=-1; qint64 area=0;
    for(int i=0;i<available.size();++i) {
        const QRect intersection=rectangle.intersected(available[i]);
        const qint64 overlap=intersection.isValid()?qint64(intersection.width())*intersection.height():0;
        if(overlap>area) { area=overlap; best=i; }
    }
    if(best>=0)return best;
    if(fallback>=0&&fallback<available.size()&&available[fallback].isValid())return fallback;
    double distance=std::numeric_limits<double>::infinity();
    for(int i=0;i<available.size();++i)if(available[i].isValid()) {
        const auto screen=available[i]; const auto point=rectangle.center();
        const double dx=double(point.x())-qBound(screen.left(),point.x(),screen.right());
        const double dy=double(point.y())-qBound(screen.top(),point.y(),screen.bottom());
        const double candidate=dx*dx+dy*dy;
        if(candidate<distance) { distance=candidate; best=i; }
    }
    return best;
}
QRect WindowPlacement::fitToScreens(const QRect& rectangle,const QVector<QRect>& available,int fallback)
{
    const int index=bestScreen(rectangle,available,fallback); return index<0?rectangle:fit(rectangle,available[index]);
}
WindowPlacement* WindowPlacement::watch(QWidget* window)
{
    if(!window||!window->isWindow())return nullptr;
    if(auto* existing=window->findChild<WindowPlacement*>(QString(),Qt::FindDirectChildrenOnly))return existing;
    return new WindowPlacement(window);
}
WindowPlacement::WindowPlacement(QWidget* window) : QObject(window),m_window(window)
{
    window->installEventFilter(this);
    for(auto* screen:QGuiApplication::screens())observe(screen);
    connect(qGuiApp,&QGuiApplication::screenAdded,this,[this](QScreen* screen) { observe(screen); scheduleRecovery(); });
    connect(qGuiApp,&QGuiApplication::screenRemoved,this,[this] { scheduleRecovery(); });
}
void WindowPlacement::observe(QScreen* screen)
{
    connect(screen,&QScreen::availableGeometryChanged,this,[this] { scheduleRecovery(); });
    connect(screen,&QScreen::geometryChanged,this,[this] { scheduleRecovery(); });
}
void WindowPlacement::scheduleRecovery()
{
    if(!m_enabled||m_pending)return;
    m_pending=true;
    // Coalesce topology signals; Qt first reassigns windows whose QScreen disappeared.
    QTimer::singleShot(0,this,[this] { m_pending=false; if(m_enabled&&m_window&&m_window->isVisible())ensureVisible(m_window); });
}
bool WindowPlacement::eventFilter(QObject* watched,QEvent* event)
{
    if(watched==m_window && (event->type()==QEvent::Show || event->type()==QEvent::WindowStateChange))scheduleRecovery();
    return QObject::eventFilter(watched,event);
}
void WindowPlacement::ensureVisible(QWidget* window)
{
    if(!window||!window->isWindow()||window->isMaximized()||window->isFullScreen()||window->isMinimized())return;
    const auto screens=QGuiApplication::screens(); const auto available=availableScreens();
    const QRect rectangle=window->geometry();
    const int index=bestScreen(rectangle,available,screens.indexOf(window->screen()));
    if(index<0)return;
    // On Wayland only clamp size. Calling setScreen can recreate a native window unnecessarily.
    const QRect fitted=fit(rectangle,available[index]);
    if(compositorPositionsWindows()) { window->resize(fitted.size().expandedTo(window->minimumSize())); return; }
    if(fitted!=rectangle || !available[index].contains(window->frameGeometry()))place(window,screens[index],fitted);
}
WindowPlacement::MoveResult WindowPlacement::moveToScreen(QWidget* window,QScreen* screen)
{
    if(!window||!window->isWindow()||!QGuiApplication::screens().contains(screen))return MoveResult::InvalidTarget;
    const auto state=window->windowState() & (Qt::WindowMaximized|Qt::WindowFullScreen);
    const QRect available=screen->availableGeometry();
    const QSize size=(window->normalGeometry().isValid()?window->normalGeometry().size():window->size()).boundedTo(available.size());
    // Clear states without showNormal(): a hidden auxiliary window must remain hidden.
    window->setWindowState(Qt::WindowNoState);
    place(window,screen,QRect(available.center()-QPoint(size.width()/2,size.height()/2),size));
    window->setWindowState(state);
    return compositorPositionsWindows()?MoveResult::CompositorManaged:MoveResult::Applied;
}
void WindowPlacement::centerOnParent(QWidget* window)
{
    if(!window||!window->isWindow())return;
    QWidget* parent=window->parentWidget()?window->parentWidget()->window():nullptr;
    const auto screens=QGuiApplication::screens(); if(screens.isEmpty())return;
    const auto available=availableScreens();
    const int index=bestScreen(parent?parent->frameGeometry():window->geometry(),available,screens.indexOf(parent?parent->screen():window->screen()));
    if(index<0)return;
    const auto reference=parent?parent->frameGeometry():available[index];
    const QSize size=window->size().boundedTo(available[index].size());
    place(window,screens[index],fit(QRect(reference.center()-QPoint(size.width()/2,size.height()/2),size),available[index]));
}
QByteArray WindowPlacement::save(const QWidget* window)
{
    if(!window||!window->isWindow())return {};
    const QRect normal=window->normalGeometry().isValid()?window->normalGeometry():window->geometry();
    const auto screens=QGuiApplication::screens();
    const int index=bestScreen(normal,availableScreens(),screens.indexOf(window->screen()));
    QScreen* screen=index>=0?screens[index]:nullptr;
    QJsonObject object{{"version",1},{"normal",packRect(normal)},{"state",window->isFullScreen()?2:window->isMaximized()?1:0}};
    if(screen) {
        object.insert("screenName",screen->name()); object.insert("serial",screen->serialNumber());
        object.insert("manufacturer",screen->manufacturer()); object.insert("model",screen->model());
        object.insert("available",packRect(screen->availableGeometry()));
    }
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}
bool WindowPlacement::restore(QWidget* window,const QByteArray& data)
{
    if(!window||!window->isWindow()||data.size()>16384)return false;
    QJsonParseError error; const auto document=QJsonDocument::fromJson(data,&error);
    if(error.error!=QJsonParseError::NoError||!document.isObject())return false;
    const auto object=document.object(); QRect normal,savedAvailable;
    const int state=object.value("state").toInt(-1);
    if(object.value("version").toInt()!=1||!unpackRect(object.value("normal"),normal)||state<0||state>2)return false;
    const auto screens=QGuiApplication::screens(); if(screens.isEmpty())return false;
    const auto available=availableScreens(); int match=-1;
    const QString serial=object.value("serial").toString();
    // Unique monitor identity first; name fallback is needed when drivers omit EDID serials.
    for(int i=0;i<screens.size();++i) {
        const bool same=!serial.isEmpty()
            ? screens[i]->serialNumber()==serial && screens[i]->manufacturer()==object.value("manufacturer").toString() && screens[i]->model()==object.value("model").toString()
            : screens[i]->name()==object.value("screenName").toString();
        if(same) { if(match>=0) { match=-2; break; } match=i; }
    }
    if(match>=0 && unpackRect(object.value("available"),savedAvailable))normal.translate(available[match].topLeft()-savedAvailable.topLeft());
    const int target=match>=0?match:bestScreen(normal,available,screens.indexOf(QGuiApplication::primaryScreen()));
    if(target<0)return false;
    window->setWindowState(Qt::WindowNoState);
    place(window,screens[target],fit(normal,available[target]));
    window->setWindowState(state==2?Qt::WindowFullScreen:state==1?Qt::WindowMaximized:Qt::WindowNoState);
    return true;
}
}
