#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QApplication>
#include <QPointer>
#include <cmath>

namespace basewidgets {
namespace {
// Selection backgrounds may be bright in a dark theme. Derive their text
// independently from normal text and from the darker primary-button surface.
QColor selectionText(const QColor& background)
{
    auto luminance = [](const QColor& color) {
        auto linear = [](double value) {
            return value <= .04045 ? value / 12.92 : std::pow((value + .055) / 1.055, 2.4);
        };
        return .2126 * linear(color.redF()) + .7152 * linear(color.greenF()) + .0722 * linear(color.blueF());
    };
    const QColor darkInk("#202327"), lightInk("#F1F4F8");
    const double backgroundLuminance = luminance(background);
    auto contrast = [&](const QColor& ink) {
        const double inkLuminance = luminance(ink);
        return (qMax(backgroundLuminance, inkLuminance) + .05)
            / (qMin(backgroundLuminance, inkLuminance) + .05);
    };
    return contrast(darkInk) >= contrast(lightInk) ? darkInk : lightInk;
}
} // namespace

ThemeTokens* ThemeTokens::instance()
{
    static QPointer<ThemeTokens> instance;
    if (!instance) instance = new ThemeTokens();
    return instance;
}

// Do not call the lazy style manager here: initialize() also loads the tokens.
ThemeTokens::ThemeTokens() : QObject(qApp),
    m_bg0("#202327"), m_bg1("#292D32"), m_bg2("#30353B"), m_bgCtrl("#363C43"),
    m_bgHover("#3E454E"), m_bgPressed("#484F59"), m_textPrimary("#D1D5DC"),
    m_textSecondary("#ABB3BE"), m_textPlaceholder("#97A1AE"), m_textDisabled("#78818D"), m_border("#555F6B"),
    m_borderLight("#404852"), m_primary("#6CA6E8"), m_danger("#E47175"),
    m_warning("#D6AD64"), m_success("#86BD9A"),
    m_selection(m_primary), m_selectionText(selectionText(m_selection)), m_buttonText("#F1F4F8"),
    m_primaryButton(m_primary), m_dangerButton(m_danger), m_plotCursor(m_warning), m_switchKnob(m_buttonText),
    m_seriesColors{m_primary, m_warning, m_success, QColor("#B99ADC"), QColor("#DA99B7"), QColor("#75BDB7")}
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
    m_bg0 = color("pageBackgroundColor", "#202327", "#E9EDF2");
    m_bg1 = color("overlayBackgroundColor", "#292D32", "#F7F8FA");
    m_bg2 = color("baseFillColor", "#30353B", "#E2E7ED");
    m_bgCtrl = color("darkerFillColor", "#363C43", "#E1E6EC");
    m_bgHover = color("darkFillColor", "#3E454E", "#D9E1EA");
    m_bgPressed = color("pressedFillColor", "#484F59", "#CBD5E1");
    m_textPrimary = color("primaryTextColor", "#D1D5DC", "#363F4B");
    m_textSecondary = color("regularTextColor", "#ABB3BE", "#536170");
    m_textPlaceholder = color("placeholderTextColor", "#97A1AE", "#606D7D");
    m_textDisabled = color("disabledTextColor", "#78818D", "#8A95A3");
    m_border = color("darkerBorderColor", "#555F6B", "#A6B1BF");
    m_borderLight = color("lightBorderColor", "#404852", "#C9D1DC");
    m_primary = color("primaryColor", "#6CA6E8", "#326CAA");
    m_danger = color("danger", "#E47175", "#B4474D");
    m_warning = color("warning", "#D6AD64", "#8B620E");
    m_success = color("success", "#86BD9A", "#31774E");
    // Optional roles inherit from the loaded theme, not from a fixed blue palette.
    auto optionalColor = [&](const QString& key, const QColor& fallback) {
        QColor value = mgr->themeColor(key);
        if (!value.isValid()) value = QColor(mgr->themeVariableValue(key));
        return value.isValid() ? value : fallback;
    };
    m_selection = optionalColor("selectionColor", m_primary);
    m_selectionText = optionalColor("selectionTextColor", selectionText(m_selection));
    m_buttonText = optionalColor("buttonTextColor", QColor("#F1F4F8"));
    m_primaryButton = optionalColor("primaryButtonColor", m_primary);
    m_dangerButton = optionalColor("dangerButtonColor", m_danger);
    m_plotCursor = optionalColor("plotCursorColor", m_warning);
    m_switchKnob = optionalColor("switchKnobColor", m_buttonText);
    const QVector<QColor> seriesDefaults{m_primary, m_warning, m_success,
        QColor(dark ? "#B99ADC" : "#8053AB"), QColor(dark ? "#DA99B7" : "#A84874"),
        QColor(dark ? "#75BDB7" : "#287D78")};
    m_seriesColors.clear();
    for (int i = 0; i < seriesDefaults.size(); ++i)
        m_seriesColors.append(optionalColor(QString("seriesColor%1").arg(i + 1), seriesDefaults[i]));
    emit tokensChanged();
}

QColor ThemeTokens::seriesColor(int index) const
{
    return index < 0 || m_seriesColors.isEmpty() ? QColor() : m_seriesColors[index % m_seriesColors.size()];
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
        result.setColor(group, QPalette::Highlight, disabled ? m_textDisabled : m_selection);
        result.setColor(group, QPalette::HighlightedText, disabled ? selectionText(m_textDisabled) : m_selectionText);
        result.setColor(group, QPalette::ToolTipBase, m_bg1);
        result.setColor(group, QPalette::ToolTipText, text);
        result.setColor(group, QPalette::Link, m_primary);
        result.setColor(group, QPalette::LinkVisited, m_primary);
        result.setColor(group, QPalette::BrightText, m_buttonText);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        result.setColor(group, QPalette::PlaceholderText, disabled ? m_textDisabled : m_textPlaceholder);
#endif
    }
    return result;
}
} // namespace basewidgets
