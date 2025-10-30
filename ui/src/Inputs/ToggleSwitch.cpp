#include <QtAdvancedUI/Inputs/ToggleSwitch.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QPainter>
#include <QMouseEvent>

using namespace acss;

namespace QtAdvancedUI {

ToggleSwitch::ToggleSwitch(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    m_anim = new QPropertyAnimation(this, "offset");
    m_anim->setDuration(120);
    setStyleManager(StyleManagerAdapter::instance());
}

ToggleSwitch::ToggleSwitch(const QString& text, QWidget* parent)
    : QAbstractButton(parent)
{
    setText(text);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    m_anim = new QPropertyAnimation(this, "offset");
    m_anim->setDuration(120);
    setStyleManager(StyleManagerAdapter::instance());
}

ToggleSwitch::ToggleSwitch(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    m_anim = new QPropertyAnimation(this, "offset");
    m_anim->setDuration(120);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

ToggleSwitch::~ToggleSwitch() = default;

QSize ToggleSwitch::sizeHint() const
{
    return QSize(m_w, m_h);
}

void ToggleSwitch::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    applyStylesheet();
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

void ToggleSwitch::applyStylesheet()
{
    if (m_manager) setStyleSheet(m_manager->styleSheet());
}

void ToggleSwitch::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRectF track(0, 0, m_w, m_h);
    qreal radius = m_h / 2.0;
    QColor trackColor = isChecked() ? m_on : m_off;
    p.setPen(Qt::NoPen);
    p.setBrush(trackColor);
    p.drawRoundedRect(track, radius, radius);

    qreal knobR = m_h/2.0 - 3;
    qreal leftX = knobR + 3;
    qreal rightX = m_w - knobR - 3;
    qreal x = leftX + (rightX - leftX) * (isChecked() ? m_offset : (1.0 - m_offset));
    QPointF center(x, m_h/2.0);
    p.setBrush(m_knob);
    p.drawEllipse(center, knobR, knobR);
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent* e)
{
    // 交由基类处理切换逻辑（checkable 按钮会自动切换并发射信号）
    QAbstractButton::mouseReleaseEvent(e);
    // 根据当前状态启动动画，不再手动二次切换状态以避免复原
    animateTo(isChecked());
}

void ToggleSwitch::animateTo(bool checked)
{
    m_anim->stop();
    m_anim->setStartValue(checked ? 0.0 : 1.0);
    m_anim->setEndValue(checked ? 1.0 : 0.0);
    m_anim->start();
}

} // namespace QtAdvancedUI