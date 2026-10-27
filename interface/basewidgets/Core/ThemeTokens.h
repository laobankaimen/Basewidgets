/*
 * Author: zhoutao@zkhx.com
 * Date: 2026-10-26
 */
#ifndef BASEWIDGETS_THEMETOKENS_H
#define BASEWIDGETS_THEMETOKENS_H

#include <QColor>
#include <QObject>
#include <QPalette>
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
    QColor bg0() const { return m_bg0; }            // 最深背景（窗口）
    QColor bg1() const { return m_bg1; }            // 弹出层/菜单
    QColor bg2() const { return m_bg2; }            // 表面/面板
    QColor bgCtrl() const { return m_bgCtrl; }      // 控件背景
    QColor bgHover() const { return m_bgHover; }
    QColor bgPressed() const { return m_bgPressed; }

    // 文本
    QColor textPrimary() const { return m_textPrimary; }
    QColor textSecondary() const { return m_textSecondary; }
    QColor textDisabled() const { return m_textDisabled; }

    // 边框
    QColor border() const { return m_border; }
    QColor borderLight() const { return m_borderLight; }

    // 语义色
    QColor primary() const { return m_primary; }
    QColor danger() const { return m_danger; }
    QColor warning() const { return m_warning; }
    QColor success() const { return m_success; }

    // 从 StyleManagerAdapter 重新加载色板；若未初始化则回退到内置暗色值。
    void reload();
    /** 返回 Active/Inactive/Disabled 三组完整调色板；不直接修改 QApplication。 */
    QPalette palette() const;

signals:
    /** reload 完成后发出；纯 QPainter 控件可连接 update()。 */
    void tokensChanged();

private:
    ThemeTokens();

    QColor m_bg0;
    QColor m_bg1;
    QColor m_bg2;
    QColor m_bgCtrl;
    QColor m_bgHover;
    QColor m_bgPressed;
    QColor m_textPrimary;
    QColor m_textSecondary;
    QColor m_textDisabled;
    QColor m_border;
    QColor m_borderLight;
    QColor m_primary;
    QColor m_danger;
    QColor m_warning;
    QColor m_success;
};

} // namespace basewidgets

#endif // BASEWIDGETS_THEMETOKENS_H
