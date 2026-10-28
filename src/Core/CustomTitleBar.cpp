#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/BaseWidgetInterface.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Inputs/NotificationBell.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/ElidedLabel.h>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QTimer>
#include <QWindow>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QSignalBlocker>
#include <QTextBoundaryFinder>
#include <QIconEngine>
#include <cmath>
namespace basewidgets {
namespace {
// Compact title actions use their glyph as the menu affordance (gear/avatar/ellipsis).
// Keep QToolButton's native keyboard/menu behavior; suppress only the redundant arrow
// for InstantPopup icon-only buttons so it cannot consume the whole icon area.
class TitleToolButton : public ToolButton {
public:
    explicit TitleToolButton(QWidget* parent) : ToolButton(parent) {}
protected:
    void paintEvent(QPaintEvent*) override {
        QStylePainter painter(this); QStyleOptionToolButton option; initStyleOption(&option);
        if(toolButtonStyle()==Qt::ToolButtonIconOnly && popupMode()==QToolButton::InstantPopup) {
            option.features &= ~(QStyleOptionToolButton::HasMenu|QStyleOptionToolButton::MenuButtonPopup);
            option.subControls &= ~QStyle::SC_ToolButtonMenu;
        }
        painter.drawComplexControl(QStyle::CC_ToolButton,option);
    }
};
// Paint the QIcon directly into logical bounds. QIcon::pixmap(size) may already
// return a high-DPI image; multiplying size and then relabeling its DPR can crop it.
class TitleIconLabel : public QLabel {
public:
    explicit TitleIconLabel(QWidget* parent) : QLabel(parent) {}
    QIcon value;
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        value.paint(&painter,rect(),Qt::AlignCenter,isEnabled()?QIcon::Normal:QIcon::Disabled);
    }
};
QIcon titleIcon(const QString& name)
{
    return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/common/icons/titlebar/"+name+".svg");
}
// Retain the original avatar pixels/name, render only at the requested final size.
class AvatarIconEngine : public QIconEngine {
public:
    AvatarIconEngine(const QString& name,const QPixmap& image,const QFont& font) : m_name(name),m_image(image),m_font(font) {}
    QIconEngine* clone() const override { return new AvatarIconEngine(m_name,m_image,m_font); }
    void paint(QPainter* painter,const QRect& rectangle,QIcon::Mode mode,QIcon::State) override {
        painter->save(); painter->setRenderHint(QPainter::Antialiasing); painter->setRenderHint(QPainter::SmoothPixmapTransform);
        const int side=qMin(rectangle.width(),rectangle.height());
        const QRectF target(rectangle.center().x()-side/2.+.5,rectangle.center().y()-side/2.+.5,side,side);
        QPainterPath clip; clip.addEllipse(target.adjusted(.6,.6,-.6,-.6)); painter->setClipPath(clip,Qt::IntersectClip);
        auto* tokens=ThemeTokens::instance();
        if(mode==QIcon::Disabled)painter->setOpacity(.45);
        painter->fillRect(target,tokens->primary());
        if(!m_image.isNull()) {
            const int sourceSide=qMin(m_image.width(),m_image.height());
            painter->drawPixmap(target,m_image,QRectF((m_image.width()-sourceSide)/2.,(m_image.height()-sourceSide)/2.,sourceSide,sourceSide));
        } else {
            QTextBoundaryFinder boundary(QTextBoundaryFinder::Grapheme,m_name); const int end=boundary.toNextBoundary();
            QFont font=m_font; font.setPixelSize(qMax(8,qRound(side*.48))); font.setBold(false); painter->setFont(font);
            painter->setPen(tokens->palette().color(QPalette::HighlightedText));
            painter->drawText(target,Qt::AlignCenter,m_name.left(end>0?end:1));
        }
        painter->restore();
    }
    QPixmap pixmap(const QSize& size,QIcon::Mode mode,QIcon::State state) override {
        QPixmap result(size); result.fill(Qt::transparent); QPainter painter(&result); paint(&painter,QRect(QPoint(),size),mode,state); return result;
    }
private:
    QString m_name; QPixmap m_image; QFont m_font;
};
QIcon avatarIcon(const QString& name,const QPixmap& image,const QFont& font)
{
    return name.isEmpty()&&image.isNull()?titleIcon("user"):QIcon(new AvatarIconEngine(name,image,font));
}
}
CustomTitleBar::CustomTitleBar(QWidget* parent) : QWidget(parent)
{
    setProperty("title",true); setAttribute(Qt::WA_StyledBackground,true); setAutoFillBackground(true); setMouseTracking(true);
    initUI(); setMinimumWidth(220); updateHeight();
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&CustomTitleBar::updateIcons);
    updateIcons();
}
CustomTitleBar::~CustomTitleBar()
{
    if(m_window) { m_window->removeEventFilter(this); disconnect(m_window,nullptr,this,nullptr); }
    for(const auto& item:m_actions)if(item.action)disconnect(item.action,nullptr,this,nullptr);
}
void CustomTitleBar::attachToWindow(QWidget* window)
{
    if(m_window) { m_window->removeEventFilter(this); disconnect(m_window,nullptr,this,nullptr); }
    m_window=window;
    if(window) {
        connect(window,&QWidget::windowTitleChanged,this,[this] { syncIdentity(); });
        connect(window,&QWidget::windowIconChanged,this,[this] { syncIdentity(); });
        window->installEventFilter(this);
    }
    syncIdentity(); updateForWindowState();
}
QString CustomTitleBar::title() const
{
    if(m_titleOverride)return m_explicitTitle;
    if(m_window && !m_window->windowTitle().isEmpty())return m_window->windowTitle();
    const auto display=QGuiApplication::applicationDisplayName();
    return display.isEmpty()?QCoreApplication::applicationName():display;
}
QIcon CustomTitleBar::icon() const
{
    if(m_iconOverride)return m_explicitIcon;
    if(m_profile!=Profile::MainWindow)return {};
    if(m_window && !m_window->windowIcon().isNull())return m_window->windowIcon();
    if(!QApplication::windowIcon().isNull())return QApplication::windowIcon();
    return StyleManagerAdapter::instance()->loadThemeAwareSvgIcon(":/styles/themes/logo/logo.svg");
}
void CustomTitleBar::setTitle(const QString& text) { m_titleOverride=true; m_explicitTitle=text; syncIdentity(); }
void CustomTitleBar::setIcon(const QIcon& value) { m_iconOverride=true; m_explicitIcon=value; syncIdentity(); }
void CustomTitleBar::resetTitle() { m_titleOverride=false; syncIdentity(); }
void CustomTitleBar::resetIcon() { m_iconOverride=false; syncIdentity(); }
void CustomTitleBar::setTitleVisible(bool visible) { m_titleVisible=visible; layoutChildren(); }
void CustomTitleBar::setIconVisible(bool visible) { m_iconVisible=visible; layoutChildren(); }
void CustomTitleBar::setTitleAlignment(TitleAlignment alignment) { m_alignment=alignment; layoutChildren(); }
void CustomTitleBar::setIconPlacement(IconPlacement placement) { m_iconPlacement=placement; layoutChildren(); }
void CustomTitleBar::setDecoration(Decoration decoration) { m_decoration=decoration; update(); }
void CustomTitleBar::syncIdentity()
{
    const QString text=title(); m_titleLabel->setText(text); m_titleLabel->setToolTip(text);
    static_cast<TitleIconLabel*>(m_iconLabel)->value=icon(); m_iconLabel->update(); layoutChildren();
}
void CustomTitleBar::setProfile(Profile profile)
{
    m_profile=profile;
    m_showMinimize=m_showMaximize=profile!=Profile::Dialog; m_showClose=true;
    m_doubleClickToggleEnabled=profile!=Profile::Dialog;
    if(profile!=Profile::MainWindow) { m_searchPopup->hide(); m_moreMenu->hide(); }
    updateHeight(); syncIdentity(); updateForWindowState();
}
void CustomTitleBar::updateHeight() { setFixedHeight(qMax(m_profile==Profile::MainWindow?48:40,fontMetrics().height()+20)); }
void CustomTitleBar::setSearchVisible(bool visible) { m_searchVisible=visible; if(!visible)m_searchPopup->hide(); layoutChildren(); }
void CustomTitleBar::setSettingsVisible(bool visible) { m_settingsVisible=visible; layoutChildren(); }
void CustomTitleBar::setUserVisible(bool visible) { m_userVisible=visible; layoutChildren(); }
void CustomTitleBar::setNotificationsVisible(bool visible) { m_notificationsVisible=visible; layoutChildren(); }
void CustomTitleBar::setSearchShortcut(const QKeySequence& shortcut)
{
    m_searchShortcut->setKey(shortcut);
    m_searchButton->setToolTip(shortcut.isEmpty()?tr("搜索"):tr("搜索 (%1)").arg(shortcut.toString(QKeySequence::NativeText)));
    m_searchButton->setAccessibleName(m_searchButton->toolTip()); layoutChildren();
}
void CustomTitleBar::setSearchPlaceholderText(const QString& text) { m_searchEdit->setPlaceholderText(text); m_popupSearchEdit->setPlaceholderText(text); }
void CustomTitleBar::setSearchText(const QString& text) { syncSearch(text); }
QString CustomTitleBar::searchText() const { return m_searchEdit->text(); }
void CustomTitleBar::syncSearch(const QString& text)
{
    const bool changed=m_searchEdit->text()!=text || m_popupSearchEdit->text()!=text;
    { const QSignalBlocker a(m_searchEdit),b(m_popupSearchEdit); m_searchEdit->setText(text); m_popupSearchEdit->setText(text); }
    if(changed)emit searchTextChanged(text);
}
void CustomTitleBar::setUserInfo(const QString& name,const QPixmap& avatar,const QString& detail)
{
    m_userName=name.trimmed(); m_avatar=avatar; m_userDetail=detail;
    const QString tooltip=(m_userName.isEmpty()?tr("用户"):m_userName)+(detail.isEmpty()?QString():"\n"+detail);
    m_user->setToolTip(tooltip); m_user->setAccessibleName(tooltip); m_user->setAccessibleDescription(detail); updateIcons();
}
void CustomTitleBar::updateIcons()
{
    // Local Window role also supplies the restore glyph's occlusion background.
    QPalette colors=ThemeTokens::instance()->palette();
    colors.setColor(QPalette::Window,ThemeTokens::instance()->titleBarBackground());
    setPalette(colors);
    m_settings->setIcon(titleIcon("settings")); m_searchButton->setIcon(titleIcon("search")); m_more->setIcon(titleIcon("more"));
    m_user->setIcon(avatarIcon(m_userName,m_avatar,font()));
    syncIdentity(); updateForWindowState(); update();
}
void CustomTitleBar::updateForWindowState()
{
    const bool maximized=m_window && (m_window->isMaximized() || isFullScreen());
    m_btnMax->setProperty("bwWindowControl",maximized?"restore":"maximize");
    m_btnMax->setToolTip(maximized?tr("还原"):tr("最大化")); m_btnMax->setAccessibleName(m_btnMax->toolTip());
    layoutChildren();
}
void CustomTitleBar::initUI()
{
    m_iconLabel=new TitleIconLabel(this); m_iconLabel->setObjectName("TitleIcon"); m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel=new ElidedLabel(this); m_titleLabel->setObjectName("TitleLabel"); m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setProperty("title",true);
    auto button=[this](const QString& name,const QString& tooltip) {
        auto* result=new TitleToolButton(this); result->setObjectName(name); result->setToolTip(tooltip); result->setAccessibleName(tooltip);
        result->setAutoRaise(true); result->setAppearance(ButtonAppearance::Subtle); result->setIconSize(QSize(18,18));
        result->setToolButtonStyle(Qt::ToolButtonIconOnly); result->setFocusPolicy(Qt::StrongFocus); return result;
    };
    m_btnMin=button("TitleBarMinButton",tr("最小化")); m_btnMin->setProperty("bwWindowControl","minimize");
    m_btnMax=button("TitleBarMaxButton",tr("最大化")); m_btnMax->setProperty("bwWindowControl","maximize");
    m_btnClose=button("TitleBarCloseButton",tr("关闭")); m_btnClose->setProperty("bwWindowControl","close"); m_btnClose->setAppearance(ButtonAppearance::Danger);
    m_settings=button("TitleBarSettingsButton",tr("设置")); m_settings->setPopupMode(QToolButton::InstantPopup);
    m_user=button("TitleBarUserButton",tr("用户")); m_user->setPopupMode(QToolButton::InstantPopup); m_user->setIconSize(QSize(22,22));
    m_notifications=new NotificationBell(this); m_notifications->setObjectName("TitleBarNotificationButton"); m_notifications->installEventFilter(this);
    m_notificationsOverflow=new QAction(tr("消息中心"),this);
    connect(m_notifications,&QToolButton::clicked,this,&CustomTitleBar::notificationsRequested);
    connect(m_notificationsOverflow,&QAction::triggered,this,&CustomTitleBar::notificationsRequested);
    connect(m_notifications,&NotificationBell::unreadCountChanged,this,[this](int) { layoutChildren(); });
    m_searchButton=button("TitleBarSearchButton",tr("搜索 (Ctrl+K)"));
    m_more=button("TitleBarMoreButton",tr("更多操作")); m_moreMenu=new QMenu(m_more); m_more->setMenu(m_moreMenu); m_more->setPopupMode(QToolButton::InstantPopup);
    m_searchEdit=new LineEdit(this); m_searchEdit->setObjectName("TitleBarSearchEdit"); m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setAccessibleName(tr("全局搜索"));
    m_searchPopup=new QFrame(this,Qt::Popup); m_searchPopup->setObjectName("TitleBarSearchPopup"); m_searchPopup->setFrameShape(QFrame::StyledPanel);
    auto* layout=new QVBoxLayout(m_searchPopup); layout->setContentsMargins(10,10,10,10);
    m_popupSearchEdit=new LineEdit(m_searchPopup); m_popupSearchEdit->setObjectName("TitleBarPopupSearchEdit"); m_popupSearchEdit->setClearButtonEnabled(true);
    m_popupSearchEdit->setAccessibleName(tr("全局搜索")); layout->addWidget(m_popupSearchEdit);
    setSearchPlaceholderText(tr("搜索数据 / 任务"));
    m_settingsOverflow=new QAction(tr("设置"),this); m_userOverflow=new QAction(tr("用户"),this); m_searchOverflow=new QAction(tr("搜索"),this);
    connect(m_settingsOverflow,&QAction::triggered,this,[this] { if(!m_settings->menu())emit settingsRequested(); });
    connect(m_userOverflow,&QAction::triggered,this,[this] { if(!m_user->menu())emit userRequested(); });
    connect(m_searchOverflow,&QAction::triggered,this,&CustomTitleBar::focusSearch);
    m_searchShortcut=new QShortcut(QKeySequence("Ctrl+K"),this);
    connect(m_searchShortcut,&QShortcut::activated,this,&CustomTitleBar::focusSearch);
    connect(m_searchButton,&QToolButton::clicked,this,&CustomTitleBar::focusSearch);
    connect(m_searchEdit,&QLineEdit::textChanged,this,&CustomTitleBar::syncSearch);
    connect(m_popupSearchEdit,&QLineEdit::textChanged,this,&CustomTitleBar::syncSearch);
    connect(m_searchEdit,&QLineEdit::returnPressed,this,[this] { emit searchSubmitted(searchText()); });
    connect(m_popupSearchEdit,&QLineEdit::returnPressed,this,[this] { m_searchPopup->hide(); m_searchButton->setFocus(); emit searchSubmitted(searchText()); });
    connect(m_settings,&QToolButton::clicked,this,&CustomTitleBar::settingsRequested);
    connect(m_user,&QToolButton::clicked,this,&CustomTitleBar::userRequested);
    connect(m_btnMin,&QToolButton::clicked,this,[this] {
        if(auto* window=dynamic_cast<BaseWidgetInterface*>(m_window.data()))window->setFramelessWindowState(Minimized);
        else if(m_window)m_window->showMinimized();
    });
    connect(m_btnMax,&QToolButton::clicked,this,[this] { toggleMaximizeRestore(); });
    connect(m_btnClose,&QToolButton::clicked,this,[this] { if(m_window)m_window->close(); });
}
void CustomTitleBar::addTitleAction(QAction* action)
{
    addTitleAction(action,ActionPosition::Right);
}
void CustomTitleBar::addTitleAction(QAction* action,ActionPosition position)
{
    if(!action)return;
    for(auto& item:m_actions)if(item.action==action) {
        item.position=position; layoutChildren(); return;
    }
    auto* button=new TitleToolButton(this); button->setAutoRaise(true); button->setAppearance(ButtonAppearance::Subtle);
    button->setFocusPolicy(Qt::StrongFocus); button->setDefaultAction(action); button->setIconSize(QSize(18,18));
    button->setObjectName("TitleBarCustomAction"); m_actions.append({action,button,position});
    connect(action,&QAction::changed,this,[this] { layoutChildren(); });
    connect(action,&QObject::destroyed,this,[this] {
        for(int i=m_actions.size()-1;i>=0;--i)if(!m_actions[i].action) {
            if(m_actions[i].button) { m_actions[i].button->hide(); m_actions[i].button->deleteLater(); } m_actions.removeAt(i);
        }
        layoutChildren();
    });
    layoutChildren();
}
void CustomTitleBar::removeTitleAction(QAction* action)
{
    for(int i=0;i<m_actions.size();++i)if(m_actions[i].action==action) {
        disconnect(action,nullptr,this,nullptr); m_moreMenu->removeAction(action);
        if(m_actions[i].button) { m_actions[i].button->hide(); m_actions[i].button->deleteLater(); }
        m_actions.removeAt(i); layoutChildren(); return;
    }
}
void CustomTitleBar::layoutChildren()
{
    if(!m_btnClose || m_layoutActive)return;
    m_layoutActive=true;
    const bool extended=m_profile==Profile::MainWindow;
    const int h=height(),buttonWidth=44,spacing=4,top=6,actionHeight=qMax(24,h-12);
    int right=width();
    auto placeRight=[&](QWidget* widget,int w,bool visible,bool caption=false) {
        widget->setVisible(visible);
        if(!visible)return;
        right-=w; widget->setGeometry(right,caption?0:top,w,caption?h:actionHeight); if(!caption)right-=spacing;
    };
    placeRight(m_btnClose,buttonWidth,m_showClose,true);
    placeRight(m_btnMax,buttonWidth,m_showMaximize&&!isFullScreen(),true);
    placeRight(m_btnMin,buttonWidth,m_showMinimize&&!isFullScreen(),true);
    right-=8;
    const bool user=extended&&m_userVisible,settings=extended&&m_settingsVisible,search=extended&&m_searchVisible;
    const bool notifications=extended&&m_notificationsVisible;
    const bool overflowNotifications=notifications&&width()<600;
    m_notificationsOverflow->setText(m_notifications->unreadCount()?tr("消息中心（%1 条未读）").arg(m_notifications->unreadCount()):tr("消息中心"));
    m_notificationsOverflow->setEnabled(m_notifications->isEnabled());
    const bool centered=m_alignment==TitleAlignment::Center;
    const bool compact=width()<900 || centered;
    const bool overflowSettings=settings && (width()<500 || (centered&&width()<800));
    const bool overflowUser=user && (width()<360 || (centered&&width()<700));
    const bool overflowSearch=search && (width()<300 || (centered&&width()<500));
    m_settingsOverflow->setMenu(m_settings->menu()); m_settingsOverflow->setEnabled(m_settings->isEnabled());
    m_userOverflow->setMenu(m_user->menu()); m_userOverflow->setEnabled(m_user->isEnabled());
    m_userOverflow->setText(m_userName.isEmpty()?tr("用户"):tr("用户：%1").arg(m_userName));
    m_user->setText(m_userName); m_user->setToolButtonStyle(compact?Qt::ToolButtonIconOnly:Qt::ToolButtonTextBesideIcon);
    const int userWidth=compact?38:qMin(180,qMax(38,m_user->sizeHint().width()));
    placeRight(m_user,userWidth,user&&!overflowUser);
    placeRight(m_settings,36,settings&&!overflowSettings);
    placeRight(m_notifications,40,notifications&&!overflowNotifications);
    // Reserve identity/drag space and an overflow entry before allocating actions.
    // Left actions remain visible with a centered title whenever they fit; right
    // actions keep the existing compact behavior for global search/user controls.
    const bool withTitle=m_iconPlacement==IconPlacement::WithTitle;
    const bool wantsIcon=m_iconVisible&&!icon().isNull();
    int left=(!withTitle&&wantsIcon)?44:14;
    QVector<int> leftActions,visibleActions; QList<QAction*> overflow;
    int actionBudget=qMax(0,right-170-(search?40:0));
    int leftBudget=centered?qMax(0,width()/2-left-140):actionBudget;
    if(overflowSettings)overflow.append(m_settingsOverflow);
    if(overflowUser)overflow.append(m_userOverflow);
    if(overflowSearch)overflow.append(m_searchOverflow);
    if(overflowNotifications)overflow.append(m_notificationsOverflow);
    for(auto side:{ActionPosition::Left,ActionPosition::Right})for(int i=0;i<m_actions.size();++i) {
        auto& item=m_actions[i];
        if(!item.action || !item.button || item.position!=side || !extended || !item.action->isVisible())continue;
        item.button->setPopupMode(item.action->menu()?QToolButton::InstantPopup:QToolButton::DelayedPopup);
        const bool hasIcon=!item.action->icon().isNull();
        item.button->setToolButtonStyle(hasIcon?Qt::ToolButtonIconOnly:Qt::ToolButtonTextOnly);
        item.button->setToolTip(item.action->toolTip().isEmpty()?item.action->text():item.action->toolTip());
        const int w=hasIcon?36:qBound(36,item.button->sizeHint().width(),160);
        const bool isLeft=side==ActionPosition::Left;
        const bool fits=isLeft?(width()>=500&&leftBudget>=w+spacing):!compact;
        if(fits && actionBudget>=w+40) {
            (isLeft?leftActions:visibleActions).append(i); actionBudget-=w+spacing;
            if(isLeft)leftBudget-=w+spacing;
            item.button->setProperty("titleActionWidth",w);
        } else overflow.append(item.action);
    }
    // Keep visible buttons mounted during QAction::changed (for example a check
    // toggle). Hiding/re-showing the focused button would lose keyboard focus.
    for(int i=0;i<m_actions.size();++i)if(m_actions[i].button)
        m_actions[i].button->setVisible(leftActions.contains(i)||visibleActions.contains(i));
    for(int i:leftActions) {
        auto* button=m_actions[i].button.data(); const int w=button->property("titleActionWidth").toInt();
        button->setGeometry(left,top,w,actionHeight); button->show(); left+=w+spacing;
    }
    if(!leftActions.isEmpty())left+=6;
    const auto oldActions=m_moreMenu->actions();
    if(oldActions!=overflow) {
        for(auto* action:oldActions)m_moreMenu->removeAction(action);
        for(auto* action:overflow)m_moreMenu->addAction(action);
    }
    placeRight(m_more,36,!overflow.isEmpty());
    for(auto it=visibleActions.crbegin();it!=visibleActions.crend();++it) {
        auto* button=m_actions[*it].button.data(); placeRight(button,button->property("titleActionWidth").toInt(),true);
    }
    const bool inlineSearch=search&&!compact&&right-left>=386;
    placeRight(m_searchEdit,qMin(220,qMax(160,right-left-286)),inlineSearch);
    placeRight(m_searchButton,36,search&&!inlineSearch&&!overflowSearch);
    if(!search)m_searchPopup->hide();
    m_searchShortcut->setEnabled(search&&!m_searchShortcut->key().isEmpty());
    right=qMax(left,right-8);
    if(centered) {
        const int symmetric=qMax(left,width()-right);
        if(width()-2*symmetric>=72) { left=symmetric; right=width()-symmetric; }
        // Extremely narrow: use the remaining safe area, with elision if needed.
    }
    bool showIcon=wantsIcon;
    if(withTitle) {
        const int available=qMax(0,right-left);
        showIcon=wantsIcon&&available>=20;
        const bool hasText=m_titleVisible&&!title().isEmpty();
        const int iconWidth=showIcon?(hasText?28:20):0;
        const int textWidth=hasText?qMin(qMax(0,available-iconWidth),m_titleLabel->fontMetrics().horizontalAdvance(title())+2):0;
        const int groupWidth=qMin(available,iconWidth+textWidth);
        const int x=centered?left+(available-groupWidth)/2:left;
        m_iconLabel->setGeometry(x,(h-20)/2,20,20);
        m_titleLabel->setGeometry(x+qMin(iconWidth,groupWidth),0,textWidth,h);
        m_titleLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_iconLabel->setGeometry(14,(h-20)/2,20,20);
        m_titleLabel->setAlignment((centered?Qt::AlignHCenter:Qt::AlignLeft)|Qt::AlignVCenter);
        m_titleLabel->setGeometry(left,0,qMax(0,right-left),h);
    }
    m_iconLabel->setVisible(showIcon);
    m_titleLabel->setVisible(m_titleVisible);
    m_layoutActive=false;
}
void CustomTitleBar::focusSearch()
{
    if(!m_searchVisible || m_profile!=Profile::MainWindow || !isVisible() || !isEnabled())return;
    if(!m_searchEdit->isHidden()) { m_searchEdit->setFocus(Qt::ShortcutFocusReason); m_searchEdit->selectAll(); return; }
    QScreen* target=m_window?m_window->screen():screen();
    const QRect available=target?target->availableGeometry():QRect(mapToGlobal(QPoint()),QSize(360,100));
    m_searchPopup->resize(qMin(360,available.width()),m_popupSearchEdit->sizeHint().height()+20);
    QWidget* anchor=m_searchButton->isHidden()?static_cast<QWidget*>(m_more):static_cast<QWidget*>(m_searchButton);
    QPoint pos=anchor->mapToGlobal(QPoint(anchor->width()-m_searchPopup->width(),anchor->height()+4));
    pos.setX(qBound(available.left(),pos.x(),qMax(available.left(),available.right()-m_searchPopup->width()+1)));
    pos.setY(qBound(available.top(),pos.y(),qMax(available.top(),available.bottom()-m_searchPopup->height()+1)));
    m_searchPopup->move(pos); m_searchPopup->show(); m_popupSearchEdit->setFocus(Qt::ShortcutFocusReason); m_popupSearchEdit->selectAll();
}
bool CustomTitleBar::isDraggableAt(const QPoint& position) const
{
    return rect().contains(position) && childAt(position)==nullptr;
}
void CustomTitleBar::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); layoutChildren(); }
void CustomTitleBar::paintEvent(QPaintEvent*)
{
    auto* t=ThemeTokens::instance(); QPainter painter(this);
    QLinearGradient gradient(0,0,width(),0); gradient.setColorAt(0,t->titleBarBackground()); gradient.setColorAt(1,t->titleBarGradientEnd());
    painter.fillRect(rect(),gradient);
    painter.setPen(t->borderLight()); painter.drawLine(0,height()-1,width(),height()-1);
    if(m_decoration!=Decoration::Technical || m_profile!=Profile::MainWindow)return;
    painter.setRenderHint(QPainter::Antialiasing);
    // Decorations stay in the bottom six pixels; never intercept input or reduce
    // the title's hit area. Narrow windows use a single accent rule.
    QColor accent=t->titleBarAccent(); accent.setAlpha(170); painter.setPen(QPen(accent,1));
    if(width()<700) { painter.drawLine(14,height()-2,width()-14,height()-2); return; }
    const qreal center=width()/2.; const qreal half=qBound(75.,m_titleLabel->width()/2.+36.,170.);
    QPainterPath edge; edge.moveTo(0,height()-6); edge.lineTo(center-half-18,height()-6);
    edge.lineTo(center-half,height()-1); edge.lineTo(center+half,height()-1);
    edge.lineTo(center+half+18,height()-6); edge.lineTo(width(),height()-6); painter.drawPath(edge);
    for(int side:{-1,1}) {
        const qreal x=center+side*(half+24);
        painter.fillRect(QRectF(x-2,height()-7,4,3),accent);
        for(int i=0;i<5;++i) { const qreal offset=x+side*(16+i*7); painter.drawLine(QPointF(offset,height()-8),QPointF(offset+side*3,height()-12)); }
    }
}
bool CustomTitleBar::eventFilter(QObject* watched,QEvent* event)
{
    if(watched==m_notifications&&event->type()==QEvent::EnabledChange)layoutChildren();
    if(watched==m_window) {
        if(event->type()==QEvent::WindowStateChange)updateForWindowState();
        else if(event->type()==QEvent::ScreenChangeInternal)updateIcons();
    }
    return QWidget::eventFilter(watched,event);
}
bool CustomTitleBar::event(QEvent* event)
{
    // Compositors can consume the release during native move. Do not leave the
    // fallback drag active after a grab is cancelled, the window deactivates or hides.
    if (event->type()==QEvent::UngrabMouse || event->type()==QEvent::WindowDeactivate
        || event->type()==QEvent::Hide) {
        m_dragging=false;
        if (event->type()!=QEvent::UngrabMouse && QWidget::mouseGrabber()==this) releaseMouse();
    }
    const bool result=QWidget::event(event);
    if(event->type()==QEvent::FontChange && m_btnClose) { updateHeight(); updateIcons(); }
    if(event->type()==QEvent::LayoutRequest && m_btnClose)layoutChildren();
    return result;
}

#ifndef Q_OS_WIN
void CustomTitleBar::mousePressEvent(QMouseEvent* e)
{
    if (!m_window || !isDraggableAt(e->pos())) {
        QWidget::mousePressEvent(e);
        return;
    }
    if (m_window->windowState() == Qt::WindowMaximized || isFullScreen()) {
        QWidget::mousePressEvent(e);
        return;
    }
    if (e->button() == Qt::LeftButton) {
        m_dragging=false;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        if (QWindow* handle = m_window->windowHandle()) {
            if (handle->startSystemMove()) {
                // The compositor owns the drag. Do not retain an explicit widget grab.
                if (QWidget::mouseGrabber()==this) releaseMouse();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                if (QGuiApplication::platformName().startsWith("wayland")) {
                    // Qt5 Widgets keeps an implicit pressed receiver until a release
                    // passes through QWidgetWindow. Wayland may consume the real release
                    // during xdg_toplevel.move, otherwise later hover stays on the title.
                    // Send one Qt-only release AFTER the initiating press finishes. It
                    // cannot activate a button: this handoff starts on title-bar blank space.
                    const QPointer<QWindow> window=handle;
                    const QPoint local=m_window->mapFromGlobal(e->globalPos());
                    const QPoint global=e->globalPos();
                    const Qt::MouseButtons remaining=e->buttons() & ~Qt::LeftButton;
                    const Qt::KeyboardModifiers modifiers=e->modifiers();
                    QTimer::singleShot(0,this,[window,local,global,remaining,modifiers] {
                        if (!window || !window->isVisible()) return;
                        QMouseEvent release(QEvent::MouseButtonRelease,local,local,global,
                                            Qt::LeftButton,remaining,modifiers,Qt::MouseEventSynthesizedByApplication);
                        QCoreApplication::sendEvent(window,&release);
                    });
                }
#endif
                e->accept();
                return;
            }
        }
#endif
        // QWidget::move is not a supported fallback on Wayland and can leave a
        // misleading drag state when the compositor rejected an input serial.
        if (QGuiApplication::platformName().startsWith("wayland")) {
            e->ignore();
            return;
        }
        m_dragging = true;
        m_dragStartPos = e->globalPos() - m_window->frameGeometry().topLeft();
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_window) {
        QWidget::mouseMoveEvent(e);
        return;
    }
    if (m_dragging && !(e->buttons() & Qt::LeftButton)) {
        m_dragging=false; // missed release: next mouse motion recovers instead of dragging forever
        if (QWidget::mouseGrabber()==this) releaseMouse();
    }
    if (m_dragging) {
        m_window->move(e->globalPos() - m_dragStartPos);
        e->accept();
        return;
    }
    QWidget::mouseMoveEvent(e);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_window) {
        QWidget::mouseReleaseEvent(e);
        return;
    }
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
        if (QWidget::mouseGrabber()==this) releaseMouse();
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_window) {
        QWidget::mouseDoubleClickEvent(e);
        return;
    }
    if (m_doubleClickToggleEnabled && isDraggableAt(e->pos()) && e->button() == Qt::LeftButton) {
        toggleMaximizeRestore(true);
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}
#endif
// 简化toggleMaximizeRestore，移除状态记录逻辑
void CustomTitleBar::toggleMaximizeRestore(bool isDoubleClick)
{
    if (!m_window) return;
    if(isDoubleClick && isFullScreen()){
        return;//全屏双击不生效
    }
    FramelessWindowState states;
    if (m_window->windowState() == Qt::WindowMaximized || isFullScreen()) {
        states = Normal;
    }
    else {
        states = Maximized;//默认全部最大化，全屏没有动画效果
    }
    auto *window = dynamic_cast<BaseWidgetInterface *>(m_window.data());
    if (window) window->setFramelessWindowState(states);
    else if (m_window) {
        if (states == Maximized) m_window->showMaximized();
        else m_window->showNormal();
    }

}

bool CustomTitleBar::isFullScreen()
{
    return m_window && m_window->isFullScreen();
}

void CustomTitleBar::setButtonsVisible(bool showMinimize, bool showMaximize, bool showClose)
{
    m_showMinimize = showMinimize;
    m_showMaximize = showMaximize;
    m_showClose = showClose;
    layoutChildren();
}

void CustomTitleBar::setDoubleClickToToggleEnabled(bool enabled)
{
    m_doubleClickToggleEnabled = enabled;
}

} // namespace basewidgets
