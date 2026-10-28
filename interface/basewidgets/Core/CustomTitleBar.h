#ifndef BASEWIDGETS_CUSTOMTITLEBAR_H
#define BASEWIDGETS_CUSTOMTITLEBAR_H
#include <basewidgets/QtAdvancedUIExport.h>
#include <QWidget>
#include <QPointer>
#include <QIcon>
#include <QPixmap>
#include <QVector>
#include <QKeySequence>
class QLabel;
class QAction;
class QLineEdit;
class QFrame;
class QMenu;
class QShortcut;
namespace basewidgets {
class ToolButton;
class NotificationBell;
/** 独立标题栏：主窗口可启用搜索/设置/头像/业务操作，弹窗默认只有标题和关闭。
 * 仅提供 UI 与信号，不执行搜索、登录或设置业务；所有接口在 GUI 线程调用。
 * 空白和标题可拖动，交互控件不参与拖动；绘制使用 NativeStyle/ThemeTokens，无 QSS。
 */
class ACSS_UI_EXPORT CustomTitleBar : public QWidget {
    Q_OBJECT
public:
    enum class Profile { MainWindow, Dialog, SecondaryWindow };
    Q_ENUM(Profile)
    enum class TitleAlignment { Left, Center };
    Q_ENUM(TitleAlignment)
    /** Leading：图标固定在左端；WithTitle：图标紧随标题组成整体，支持整体居中。 */
    enum class IconPlacement { Leading, WithTitle };
    Q_ENUM(IconPlacement)
    /** Technical 为主窗口添加静态折线/短线装饰；默认 Plain，简洁 profile 忽略装饰。 */
    enum class Decoration { Plain, Technical };
    Q_ENUM(Decoration)
    void setDecoration(Decoration decoration);
    Decoration decoration() const { return m_decoration; }
    /** 物理左/右侧，不随文本的 RTL 方向换边。窗口控制始终位于最右侧。 */
    enum class ActionPosition { Left, Right };
    Q_ENUM(ActionPosition)
    explicit CustomTitleBar(QWidget* parent=nullptr);
    ~CustomTitleBar() override;
    /** 不接管窗口；重绑定断开旧窗口同步，显式标题/图标覆盖保留。nullptr 恢复应用默认。 */
    void attachToWindow(QWidget* window);
    QWidget* attachedWindow() const { return m_window; }
    /** MainWindow 默认 48 逻辑像素，Dialog 默认 40，字体变大时自动增加。
     * SecondaryWindow 为 40 像素、无全局扩展区但保留最小化/最大化；Dialog 仅关闭。
     * 切换会重设窗口按钮和双击偏好；简洁 profile 隐藏扩展区但保留配置。 */
    void setProfile(Profile profile);
    Profile profile() const { return m_profile; }
    /** Center 优先以整个窗口中心对齐；极窄时退让到剩余标题区域，避免覆盖窗口按钮。 */
    void setTitleAlignment(TitleAlignment alignment);
    TitleAlignment titleAlignment() const { return m_alignment; }
    /** 默认 Leading，兼容已有标题布局；WithTitle 配合 Center 可复现居中的品牌标识。 */
    void setIconPlacement(IconPlacement placement);
    IconPlacement iconPlacement() const { return m_iconPlacement; }
    QString title() const;
    QIcon icon() const;
    /** 恢复跟随窗口/应用标题；与图标默认设置相互独立。 */
    void resetTitle();
    /** 主窗口默认跟随窗口/应用图标，均为空使用内嵌图标；弹窗默认无图标。 */
    void resetIcon();
    void setTitleVisible(bool visible);
    void setIconVisible(bool visible);
    /** 内置扩展项默认关闭，按需开启。窄窗口将搜索框折叠为搜索按钮。 */
    void setSearchVisible(bool visible);
    void setSettingsVisible(bool visible);
    void setUserVisible(bool visible);
    /** 主窗口消息入口默认关闭；窄窗口移入“更多”，简洁 profile 隐藏。
     * notificationButton()->setModel 可同步未读数，点击只发 notificationsRequested。 */
    void setNotificationsVisible(bool visible);
    NotificationBell* notificationButton() const { return m_notifications; } ///< 借用，不要删除/另设固定尺寸。
    void setSearchPlaceholderText(const QString& placeholder);
    /** 默认 Ctrl+K；空快捷键禁用，避免与业务自身的全局搜索命令冲突。 */
    void setSearchShortcut(const QKeySequence& shortcut);
    void setSearchText(const QString& text);
    QString searchText() const;
    /** 显示圆形头像；空头像用姓名首字形，空姓名用访客图标。detail 用于提示/无障碍描述。
     * 不联网下载头像；传入业务已加载的 QPixmap，不接管外部对象。 */
    void setUserInfo(const QString& name,const QPixmap& avatar=QPixmap(),const QString& detail=QString());
    QString userName() const { return m_userName; }
    /** 借用按钮，可设置 QMenu/tooltip 等；菜单所有权按 Qt 规则由调用者管理。
     * 不给这些按钮设置固定尺寸，标题栏会根据宽度重新布局。 */
    ToolButton* settingsButton() const { return m_settings; }
    ToolButton* userButton() const { return m_user; }
    /** 借用内嵌输入框，用于 placeholder/completer 等；弹出搜索通过同一文本 API/信号同步。 */
    QLineEdit* searchEdit() const { return m_searchEdit; }
    /** QAction 仍归调用者持有，销毁时自动移除。保留顺序，空间不足进入更多菜单。
     * QAction 可带图标、菜单、checked/enabled/visible；文本始终作为 tooltip/菜单标签。 */
    void addTitleAction(QAction* action);
    /** 指定操作位置，单参数版本等同于 Right。左侧操作位于标题之前。
     * 同侧按添加顺序显示；重复传入已有动作只调整位置，不新建入口。
     * 窄窗口将两侧放不下的动作收纳到右侧“更多”菜单；简洁 profile 暂时隐藏。
     * QAction 的所有权不变，可直接复用到菜单/工具栏。 */
    void addTitleAction(QAction* action, ActionPosition position);
    void removeTitleAction(QAction* action);
    /** Windows 命中与非 Windows 拖动共用判断；标题栏坐标，true 表示可拖动空白。 */
    bool isDraggableAt(const QPoint& position) const;
    bool doubleClickToToggleEnabled() const { return m_doubleClickToggleEnabled; }
public slots:
    /** 显式覆盖标题栏展示，不修改操作系统窗口标题；空字符串明确隐藏标题文字。 */
    void setTitle(const QString& title);
    /** 显式覆盖标题栏图标；空图标明确不显示。切换主题不会覆盖用户设置。 */
    void setIcon(const QIcon& icon);
    /** Ctrl+K 也可触发；仅在本主窗口搜索开启时有效。窄宽度使用浮层输入。 */
    void focusSearch();
    void updateForWindowState();
    void setButtonsVisible(bool showMinimize,bool showMaximize,bool showClose);
    void setDoubleClickToToggleEnabled(bool enabled);
    void updateIcons();
signals:
    void searchTextChanged(const QString& text);
    void searchSubmitted(const QString& text);
    /** 未给对应按钮绑定菜单时触发；有菜单时按 QToolButton 规则直接打开菜单。 */
    void settingsRequested();
    void userRequested();
    void notificationsRequested();
protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched,QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
#ifndef Q_OS_WIN
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
#endif
private:
    void initUI();
    void layoutChildren();
    void syncIdentity();
    void syncSearch(const QString& text);
    void updateHeight();
    void toggleMaximizeRestore(bool isDoubleClick=false);
    bool isFullScreen();
    QPointer<QWidget> m_window;
    QLabel *m_iconLabel=nullptr,*m_titleLabel=nullptr;
    ToolButton *m_btnMin=nullptr,*m_btnMax=nullptr,*m_btnClose=nullptr;
    ToolButton *m_settings=nullptr,*m_user=nullptr,*m_searchButton=nullptr,*m_more=nullptr;
    NotificationBell* m_notifications=nullptr;
    QAction* m_notificationsOverflow=nullptr;
    bool m_notificationsVisible=false;
    QLineEdit *m_searchEdit=nullptr,*m_popupSearchEdit=nullptr;
    QFrame* m_searchPopup=nullptr;
    QMenu* m_moreMenu=nullptr;
    QAction *m_settingsOverflow=nullptr,*m_userOverflow=nullptr,*m_searchOverflow=nullptr;
    QShortcut* m_searchShortcut=nullptr;
    struct TitleAction { QPointer<QAction> action; QPointer<ToolButton> button; ActionPosition position; };
    QVector<TitleAction> m_actions;
    Profile m_profile=Profile::MainWindow;
    TitleAlignment m_alignment=TitleAlignment::Left;
    IconPlacement m_iconPlacement=IconPlacement::Leading;
    Decoration m_decoration=Decoration::Plain;
    bool m_titleOverride=false,m_iconOverride=false,m_titleVisible=true,m_iconVisible=true;
    bool m_searchVisible=false,m_settingsVisible=false,m_userVisible=false,m_layoutActive=false;
    bool m_doubleClickToggleEnabled=true,m_showMinimize=true,m_showMaximize=true,m_showClose=true;
    QString m_explicitTitle,m_userName,m_userDetail;
    QIcon m_explicitIcon;
    QPixmap m_avatar;
    bool m_dragging=false;
    QPoint m_dragStartPos;
};
}
#endif
