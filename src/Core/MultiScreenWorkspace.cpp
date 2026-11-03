#include <basewidgets/Core/MultiScreenWorkspace.h>
#include <basewidgets/Core/WindowPlacement.h>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QCryptographicHash>
namespace basewidgets {
MultiScreenWorkspace::MultiScreenWorkspace(ScreenWindow* primary,QObject* controllerParent)
    : QObject(controllerParent?controllerParent:primary),m_primary(primary),m_primaryScreen(primary?WindowPlacement::screenFor(primary):nullptr)
{
    if(primary) {
        connect(primary,&ScreenWindow::closeAccepted,this,[this] {
            m_stopped=true; for(auto& entry:m_entries)if(entry.window)entry.window->hide();
        });
        connect(primary,&QObject::destroyed,this,[this] {
            m_stopped=true; for(auto& entry:m_entries)if(entry.window)entry.window->hide();
        });
    }
    for(auto* screen:QGuiApplication::screens())observeScreen(screen);
    connect(qGuiApp,&QGuiApplication::screenAdded,this,[this](QScreen* screen) { observeScreen(screen); scheduleSync(); });
    connect(qGuiApp,&QGuiApplication::screenRemoved,this,[this](QScreen* removed) {
        // Hide before Qt relocates windows to another screen. Keep wanted=true for reconnect.
        const QString removedKey=screenKey(removed);
        const auto ids=m_entries.keys();
        for(const auto& id:ids)if(m_entries.contains(id)&&m_entries[id].screenKey==removedKey) {
            if(m_entries[id].window)m_entries[id].window->hide();
            if(!m_entries[id].suspended) { m_entries[id].suspended=true; emit secondaryAvailabilityChanged(id,false); }
        }
        if(m_primaryScreen==removed)m_primaryScreen=nullptr;
        scheduleSync();
    });
}
MultiScreenWorkspace::~MultiScreenWorkspace()
{
    m_destroying=true;
    for(auto& entry:m_entries)if(entry.window) { disconnect(entry.window,nullptr,this,nullptr); delete entry.window; }
}
QString MultiScreenWorkspace::screenKey(const QScreen* screen)
{
    if(!screen)return {};
    const QString identity=screen->manufacturer()+"|"+screen->model()+"|"+screen->serialNumber()+"|"+screen->name();
    return QString::fromLatin1(QCryptographicHash::hash(identity.toUtf8(),QCryptographicHash::Sha256).toHex().left(24));
}
QList<int> MultiScreenWorkspace::secondaryScreenIndexes(int count,int primary)
{
    QList<int> indexes;
    if(count<=0||primary<0||primary>=count)return indexes;
    for(int i=0;i<count;++i)if(i!=primary)indexes.append(i);
    return indexes;
}
QScreen* MultiScreenWorkspace::primaryScreen() const { return m_primaryScreen; }
bool MultiScreenWorkspace::setPrimaryScreen(QScreen* screen)
{
    if(!m_primary||!QGuiApplication::screens().contains(screen))return false;
    const bool changed=m_primaryScreen!=screen; m_primaryScreen=screen;
    const auto result=WindowPlacement::moveToScreen(m_primary,screen);
    if(result==WindowPlacement::MoveResult::CompositorManaged)emit compositorPlacementRequested("primary",screen);
    if(changed)emit primaryScreenChanged(screen);
    synchronizeScreens(); return true;
}
ScreenWindow* MultiScreenWorkspace::secondaryWindow(const QString& id) const { return m_entries.value(id).window; }
QStringList MultiScreenWorkspace::secondaryIds() const { return m_entries.keys(); }
QScreen* MultiScreenWorkspace::targetScreen(const QString& id) const
{
    const QString key=m_entries.value(id).screenKey;
    for(auto* screen:QGuiApplication::screens())if(screenKey(screen)==key)return screen;
    return nullptr;
}
bool MultiScreenWorkspace::isSuspended(const QString& id) const { return m_entries.contains(id)&&m_entries.value(id).suspended; }
ScreenWindow* MultiScreenWorkspace::createSecondary(const QString& id,const QString& title)
{
    if(m_entries.contains(id)&&m_entries[id].window)return m_entries[id].window;
    auto* window=new ScreenWindow(ScreenWindow::Role::Secondary);
    window->setObjectName("SecondaryScreen-"+id); window->setTitle(title.isEmpty()?tr("子屏工作区"):title);
    window->resize(1000,720);
    // Workspace owns placement. Generic recovery would stack unplugged windows onto primary.
    WindowPlacement::watch(window)->setRecoveryEnabled(false);
    m_entries[id].window=window;
    connect(window,&ScreenWindow::primaryWindowRequested,this,&MultiScreenWorkspace::activatePrimary);
    connect(window,&ScreenWindow::closeAccepted,this,[this,id] { if(m_entries.contains(id))m_entries[id].wanted=false; });
    connect(window,&QObject::destroyed,this,[this,id] { if(!m_destroying)m_entries.remove(id); });
    QPointer<ScreenWindow> guard=window;
    emit windowCreated(id,window);
    return guard;
}
ScreenWindow* MultiScreenWorkspace::openSecondary(const QString& id,QScreen* screen,const QString& title)
{
    if(!m_primary || m_stopped || id.trimmed().isEmpty() || !QGuiApplication::screens().contains(screen))return nullptr;
    QPointer<ScreenWindow> window=createSecondary(id,title); if(!window)return nullptr;
    auto& entry=m_entries[id]; entry.screenKey=screenKey(screen); entry.wanted=true;
    entry.allowPrimary=screen==m_primaryScreen;
    if(!title.isEmpty())window->setTitle(title);
    placeSecondary(id,screen,true); synchronizeScreens(); return window;
}
void MultiScreenWorkspace::placeSecondary(const QString& id,QScreen* screen,bool force)
{
    if(!m_entries.contains(id))return;
    auto& entry=m_entries[id]; if(!entry.window||!entry.wanted||!screen)return;
    const bool wasSuspended=entry.suspended; entry.suspended=false;
    const QPointer<ScreenWindow> window=entry.window;
    if(force || wasSuspended || !window->isVisible()) {
        const auto result=WindowPlacement::moveToScreen(window,screen);
        if(result==WindowPlacement::MoveResult::CompositorManaged)emit compositorPlacementRequested(id,screen);
        if(!window || !m_entries.contains(id) || !m_entries[id].wanted || m_stopped)return;
        window->show();
    }
    if(wasSuspended)emit secondaryAvailabilityChanged(id,true);
}
void MultiScreenWorkspace::closeSecondary(const QString& id) { if(auto* window=secondaryWindow(id))window->close(); }
void MultiScreenWorkspace::setAutomaticSecondaryWindows(bool enabled)
{
    m_automatic=enabled;
    if(!enabled)for(auto& entry:m_entries)if(entry.automatic&&entry.window)entry.window->hide();
    synchronizeScreens();
}
void MultiScreenWorkspace::showAllSecondaryScreens()
{
    for(auto& entry:m_entries)if(entry.automatic)entry.wanted=true;
    setAutomaticSecondaryWindows(true);
}
void MultiScreenWorkspace::observeScreen(QScreen* screen)
{
    connect(screen,&QScreen::availableGeometryChanged,this,[this] { m_reposition=true; scheduleSync(); });
}
void MultiScreenWorkspace::scheduleSync()
{
    if(m_syncPending || m_stopped)return;
    m_syncPending=true; QTimer::singleShot(0,this,[this] { m_syncPending=false; synchronizeScreens(); });
}
void MultiScreenWorkspace::synchronizeScreens()
{
    if(m_syncing||m_stopped||!m_primary)return;
    m_syncing=true;
    const auto screens=QGuiApplication::screens();
    const bool reposition=m_reposition; m_reposition=false;
    if(!screens.contains(m_primaryScreen)) {
        m_primaryScreen=QGuiApplication::primaryScreen();
        if(m_primaryScreen) {
            const auto result=WindowPlacement::moveToScreen(m_primary,m_primaryScreen);
            if(result==WindowPlacement::MoveResult::CompositorManaged)emit compositorPlacementRequested("primary",m_primaryScreen);
            emit primaryScreenChanged(m_primaryScreen);
        }
    }
    if(m_automatic)for(int index:secondaryScreenIndexes(screens.size(),screens.indexOf(m_primaryScreen))) {
        auto* screen=screens[index]; const QString key=screenKey(screen),id="screen-"+key;
        // An explicitly assigned business window already occupies this screen.
        bool assigned=false;
        for(auto it=m_entries.cbegin();it!=m_entries.cend();++it)if(!it->automatic && it->screenKey==key && it->wanted) { assigned=true; break; }
        if(assigned)continue;
        if(!m_entries.contains(id) && createSecondary(id,tr("子屏 %1 · %2").arg(index+1).arg(screen->name()))) {
            m_entries[id].screenKey=key; m_entries[id].automatic=true;
        }
    }
    const auto ids=m_entries.keys();
    for(const auto& id:ids) {
        if(!m_entries.contains(id))continue;
        auto* screen=targetScreen(id);
        const auto entry=m_entries[id];
        bool assigned=false;
        if(entry.automatic)for(auto it=m_entries.cbegin();it!=m_entries.cend();++it)
            if(!it->automatic && it->wanted && it->screenKey==entry.screenKey) { assigned=true; break; }
        const bool available=screen && (screen!=m_primaryScreen || entry.allowPrimary) && !(entry.automatic && assigned);
        if(!available) {
            if(entry.window)entry.window->hide();
            if(!entry.suspended) { m_entries[id].suspended=true; emit secondaryAvailabilityChanged(id,false); }
        } else {
            if(entry.suspended) { m_entries[id].suspended=false; emit secondaryAvailabilityChanged(id,true); }
            if(m_entries.contains(id)&&entry.wanted && (!entry.automatic || m_automatic))placeSecondary(id,screen,reposition||entry.suspended);
        }
    }
    m_syncing=false;
}
void MultiScreenWorkspace::activatePrimary()
{
    if(!m_primary||m_stopped)return;
    if(m_primary->isMinimized())m_primary->showNormal(); else m_primary->show();
    m_primary->raise(); m_primary->activateWindow();
}
}
