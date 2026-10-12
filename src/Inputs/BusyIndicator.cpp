/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/BusyIndicator.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QPainter>
#include <QTimer>
#include <QtMath>

namespace basewidgets {

BusyIndicator::BusyIndicator(QWidget* parent)
    : StyledWidget(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &BusyIndicator::onTick);
    setMinimumSize(24, 24);
    setColor(StyleManagerAdapter::instance()->themeColor("primaryColor"));
    start();
}

BusyIndicator::~BusyIndicator() = default;

void BusyIndicator::setInterval(int ms)
{
    m_interval = qMax(10, ms);
    if (m_timer) m_timer->setInterval(m_interval);
}

void BusyIndicator::start()
{
    m_running = true;
    m_timer->start(m_interval);
}

void BusyIndicator::stop()
{
    m_running = false;
    m_timer->stop();
}

void BusyIndicator::onTick()
{
    m_step = (m_step + 1) % m_segments;
    update();
}

void BusyIndicator::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int size = qMin(w, h);
    const QPointF center(w / 2.0, h / 2.0);
    const qreal radius = size / 2.0 - 2;
    const int count = m_segments;

    for (int i = 0; i < count; ++i) {
        const qreal angle = (360.0 / count) * i;
        const qreal rad = qDegreesToRadians(angle);
        const qreal dx = qCos(rad) * (radius - 4);
        const qreal dy = qSin(rad) * (radius - 4);
        const qreal dotR = size / 12.0;

        int dist = (i - m_step + count) % count;
        qreal alpha = 1.0 - (dist / qreal(count));
        QColor c = m_color;
        c.setAlphaF(0.2 + 0.8 * alpha);

        p.setBrush(c);
        p.setPen(Qt::NoPen);
        p.drawEllipse(center + QPointF(dx, dy), dotR, dotR);
    }
}

} // namespace basewidgets

