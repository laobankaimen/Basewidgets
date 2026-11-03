/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/ToggleSwitch.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QPainter>
#include <QMouseEvent>
#include <basewidgets/Core/ThemeTokens.h>

using namespace acss;

namespace basewidgets {

ToggleSwitch::ToggleSwitch(QWidget* parent)
    : QAbstractButton(parent)
{
    initSwitch();
    //主题切换
    connect(ThemeTokens::instance(), &ThemeTokens::tokensChanged, this, [this] {
        updateColors();
        update();
    });
}

ToggleSwitch::~ToggleSwitch() = default;

QSize ToggleSwitch::sizeHint() const
{
    return QSize(m_w, m_h);
}

void ToggleSwitch::updateColors() {
    StyleManagerAdapter::instance();
    auto* tokens = ThemeTokens::instance();
    setOnColor(tokens->primaryButton());
    setOffColor(tokens->bgHover());
    setKnobColor(tokens->switchKnob());
}

void ToggleSwitch::setAnimationDuration(int ms)
{
    m_anim->setDuration(qMax(60, ms));
}

void ToggleSwitch::setOffset(qreal v)
{
    m_offset = v;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF track(0, 0, m_w, m_h);
    qreal radius = m_h / 2.0;
    QColor trackColor = isEnabled() ? (isChecked() ? m_on : m_off) : ThemeTokens::instance()->borderLight();
    p.setPen(Qt::NoPen);
    p.setBrush(trackColor);
    p.drawRoundedRect(track, radius, radius);

    qreal knobR = m_h/2.0 - 3;
    qreal leftX = knobR + 3;
    qreal rightX = m_w - knobR - 3;
    const qreal offset = layoutDirection() == Qt::RightToLeft ? 1.0-m_offset : m_offset;
    qreal x = leftX + (rightX - leftX) * offset;
    QPointF center(x, m_h/2.0);
    p.setBrush(isEnabled() ? m_knob : ThemeTokens::instance()->textDisabled());
    p.drawEllipse(center, knobR, knobR);
    if (hasFocus()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(ThemeTokens::instance()->primary(), 1));
        p.drawRoundedRect(track.adjusted(.5, .5, -.5, -.5), radius, radius);
    }
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent* e)
{
    // 交由基类处理切换逻辑（checkable 按钮会自动切换并发射信号）
    QAbstractButton::mouseReleaseEvent(e);
}

void ToggleSwitch::animateTo(bool checked)
{
    m_anim->stop();
    m_anim->setStartValue(m_offset);
    m_anim->setEndValue(checked ? 1.0 : 0.0);
    m_anim->start();
}

void ToggleSwitch::initSwitch() {
    updateColors();
    setCheckable(true);
    setOffset(isChecked() ? 1.0 : 0.0);
    setCursor(Qt::PointingHandCursor);
    m_anim = new QPropertyAnimation(this, "offset");
    m_anim->setDuration(120);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, &QAbstractButton::toggled, this, &ToggleSwitch::animateTo);
}
} // namespace basewidgets

