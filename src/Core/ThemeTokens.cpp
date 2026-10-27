#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QApplication>
#include <QPointer>

namespace basewidgets {

ThemeTokens* ThemeTokens::instance()
{
    static QPointer<ThemeTokens> instance;
    if (!instance) instance = new ThemeTokens();
    return instance;
}

// Do not call the lazy style manager here: initialize() also loads the tokens.
ThemeTokens::ThemeTokens() : QObject(qApp),
    m_bg0("#0A0A0A"), m_bg1("#1D1E1F"), m_bg2("#303030"), m_bgCtrl("#424243"),
    m_bgHover("#39393A"), m_bgPressed("#58585B"), m_textPrimary("#E5EAF3"),
    m_textSecondary("#CFD3DC"), m_textDisabled("#6C6E72"), m_border("#58585B"),
    m_borderLight("#414243"), m_primary("#409EFF"), m_danger("#F56C6C"),
    m_warning("#E6A23C"), m_success("#67C23A")
{
}

void ThemeTokens::reload()
{
    auto* mgr = StyleManagerAdapter::instance();
    const bool dark = mgr->currentTheme().isEmpty() || mgr->isCurrentThemeDark();
    auto color = [&](const QString& key, const char* darkFallback, const char* lightFallback) {
        QColor c = mgr->themeColor(key);
        if (!c.isValid()) c = QColor(mgr->themeVariableValue(key));
        return c.isValid() ? c : QColor(dark ? darkFallback : lightFallback);
    };
    m_bg0 = color("pageBackgroundColor", "#0A0A0A", "#F2F3F5");
    m_bg1 = color("overlayBackgroundColor", "#1D1E1F", "#FFFFFF");
    m_bg2 = color("baseFillColor", "#303030", "#F0F2F5");
    m_bgCtrl = color("darkerFillColor", "#424243", "#E6E8EB");
    m_bgHover = color("darkFillColor", "#39393A", "#EBEDF0");
    m_bgPressed = color("darkerBorderColor", "#58585B", "#CDD0D6");
    m_textPrimary = color("primaryTextColor", "#E5EAF3", "#303133");
    m_textSecondary = color("regularTextColor", "#CFD3DC", "#606266");
    m_textDisabled = color("disabledTextColor", "#6C6E72", "#C0C4CC");
    m_border = color("darkerBorderColor", "#58585B", "#CDD0D6");
    m_borderLight = color("lightBorderColor", "#414243", "#E4E7ED");
    m_primary = color("primaryColor", "#409EFF", "#409EFF");
    m_danger = color("danger", "#F56C6C", "#F56C6C");
    m_warning = color("warning", "#E6A23C", "#E6A23C");
    m_success = color("success", "#67C23A", "#67C23A");
    emit tokensChanged();
}

QPalette ThemeTokens::palette() const
{
    QPalette result;
    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        const bool disabled = group == QPalette::Disabled;
        const QColor text = disabled ? m_textDisabled : m_textPrimary;
        result.setColor(group, QPalette::Window, m_bg0);
        result.setColor(group, QPalette::WindowText, text);
        result.setColor(group, QPalette::Base, m_bg1);
        result.setColor(group, QPalette::AlternateBase, m_bg2);
        result.setColor(group, QPalette::Text, text);
        result.setColor(group, QPalette::Button, disabled ? m_bg2 : m_bgCtrl);
        result.setColor(group, QPalette::ButtonText, text);
        result.setColor(group, QPalette::Light, m_bgHover);
        result.setColor(group, QPalette::Midlight, m_borderLight);
        result.setColor(group, QPalette::Mid, disabled ? m_borderLight : m_border);
        result.setColor(group, QPalette::Dark, m_bgPressed);
        result.setColor(group, QPalette::Shadow, m_bg0);
        result.setColor(group, QPalette::Highlight, disabled ? m_textDisabled : m_primary);
        result.setColor(group, QPalette::HighlightedText, Qt::white);
        result.setColor(group, QPalette::ToolTipBase, m_bg1);
        result.setColor(group, QPalette::ToolTipText, text);
        result.setColor(group, QPalette::Link, m_primary);
        result.setColor(group, QPalette::LinkVisited, m_primary);
        result.setColor(group, QPalette::BrightText, Qt::white);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        result.setColor(group, QPalette::PlaceholderText, m_textDisabled);
#endif
    }
    return result;
}
} // namespace basewidgets
