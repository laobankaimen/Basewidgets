/*
 * Author: zhoutao@zkhx.com
 * Date: 2026-10-26
 */
#ifndef BASEWIDGETS_THEMETOKENS_H
#define BASEWIDGETS_THEMETOKENS_H

#include <QColor>
#include <QObject>
#include <QPalette>
#include <QVector>
#include <basewidgets/QtAdvancedUIExport.h>

namespace basewidgets {

// 原生绘制色板令牌。脱离 QSS 后由本单例统一提供颜色，
// 响应 StyleManagerAdapter 的主题切换并通知各控件重绘。
class ACSS_UI_EXPORT ThemeTokens : public QObject
{
    Q_OBJECT
public:
    /** GUI 线程单例，由 QApplication 持有，不要 delete；颜色在主题切换后统一更新。 */
    static ThemeTokens* instance();

    // 背景层
    QColor bg0() const { return m_bg0; }            // 窗口/页面背景（明暗主题均适用）
    QColor bg1() const { return m_bg1; }            // 弹出层/菜单
    QColor bg2() const { return m_bg2; }            // 表面/面板
    QColor bgCtrl() const { return m_bgCtrl; }      // 控件背景
    QColor bgHover() const { return m_bgHover; }
    QColor bgPressed() const { return m_bgPressed; }

    /** 标题栏独立底色；XML 的 titleBarBackgroundColor，缺省沿用页面背景。 */
    QColor titleBarBackground() const { return m_titleBarBackground; }

    /** 标题栏渐变终点与装饰线；未配置时为标题底色和主题强调色。 */
    QColor titleBarGradientEnd() const { return m_titleBarGradientEnd; }
    QColor titleBarAccent() const { return m_titleBarAccent; }

    // 文本
    QColor textPrimary() const { return m_textPrimary; }
    /** 辅助说明、单位与刻度；不要用禁用色承载仍需阅读的信息。 */
    QColor textSecondary() const { return m_textSecondary; }
    /** 可编辑输入框的占位文字；独立于不可操作控件的禁用文字色。 */
    QColor textPlaceholder() const { return m_textPlaceholder; }
    QColor textDisabled() const { return m_textDisabled; }

    // 边框
    QColor border() const { return m_border; }
    QColor borderLight() const { return m_borderLight; }

    // 语义色
    QColor primary() const { return m_primary; }
    QColor danger() const { return m_danger; }
    QColor warning() const { return m_warning; }
    QColor success() const { return m_success; }

    /** 主题默认图表序列色，index >= 0 时循环取六色；负数返回无效颜色。
     * 仅用于未指定业务颜色的曲线/容量分段，显式 QColor 优先。
     */
    QColor seriesColor(int index) const;
    /** A/B 和测向游标色，与告警语义分开配置。 */
    QColor plotCursor() const { return m_plotCursor; }
    /** 实心按钮的原始背景；NativeStyle 会按文字色进一步确保可读性。 */
    QColor primaryButton() const { return m_primaryButton; }
    QColor dangerButton() const { return m_dangerButton; }
    /** 开关圆点默认色；业务调用 setKnobColor 仍可覆盖。 */
    QColor switchKnob() const { return m_switchKnob; }

    // 从 StyleManagerAdapter 重新加载色板；若未初始化则回退到内置暗色值。
    void reload();
    /** 返回 Active/Inactive/Disabled 三组完整调色板；不直接修改 QApplication。
     * HighlightedText 默认按选中背景取深/浅文字；可由 selectionTextColor 覆盖。
     * BrightText 来自 buttonTextColor，用于经过对比度调整的实心按钮。
     */
    QPalette palette() const;

signals:
    /** reload 完成后发出；纯 QPainter 控件可连接 update()。 */
    void tokensChanged();

private:
    ThemeTokens();

    QColor m_titleBarBackground, m_titleBarGradientEnd, m_titleBarAccent;
    QColor m_bg0;
    QColor m_bg1;
    QColor m_bg2;
    QColor m_bgCtrl;
    QColor m_bgHover;
    QColor m_bgPressed;
    QColor m_textPrimary;
    QColor m_textSecondary;
    QColor m_textPlaceholder;
    QColor m_textDisabled;
    QColor m_border;
    QColor m_borderLight;
    QColor m_primary;
    QColor m_danger;
    QColor m_warning;
    QColor m_success;
    QColor m_selection, m_selectionText, m_buttonText;
    QColor m_primaryButton, m_dangerButton, m_plotCursor, m_switchKnob;
    QVector<QColor> m_seriesColors;
};

} // namespace basewidgets

#endif // BASEWIDGETS_THEMETOKENS_H
