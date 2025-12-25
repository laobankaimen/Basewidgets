/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <QtAdvancedUI/Charts/StyledPlot.h>
#include <QPalette>

namespace QtAdvancedUI {

StyledPlot::StyledPlot(QWidget* parent)
    : QCustomPlot(parent) {
    
    // 转发基类信号
    connect(this, static_cast<void(QCustomPlot::*)(QMouseEvent*)>(&QCustomPlot::mouseDoubleClick),
            this, &StyledPlot::mouseDoubleClick);
    connect(this, static_cast<void(QCustomPlot::*)(QMouseEvent*)>(&QCustomPlot::mousePress),
            this, &StyledPlot::mousePress);
    connect(this, static_cast<void(QCustomPlot::*)(QMouseEvent*)>(&QCustomPlot::mouseRelease),
            this, &StyledPlot::mouseRelease);
    connect(this, static_cast<void(QCustomPlot::*)(QMouseEvent*)>(&QCustomPlot::mouseMove),
            this, &StyledPlot::mouseMove);
    connect(this, static_cast<void(QCustomPlot::*)(QWheelEvent*)>(&QCustomPlot::mouseWheel),
            this, &StyledPlot::mouseWheel);

    axisRect()->setupFullAxesBox(true);
    configureDefaultInteractions();
    applyTheme();

    auto* mgr = StyleManagerAdapter::instance();
    if (mgr) {
        connect(mgr, &acss::QtAdvancedStylesheet::stylesheetChanged,
                this, &StyledPlot::onStylesheetChanged);
    }
}

void StyledPlot::configureDefaultInteractions() {
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    if (axisRect()) {
        axisRect()->setRangeZoomAxes(xAxis, yAxis);
    }
    setCursor(Qt::CrossCursor);
}

void StyledPlot::applyTheme() {
    auto* mgr = StyleManagerAdapter::instance();
    QPalette pal = qApp->palette();

    const bool dark = mgr ? mgr->isCurrentThemeDark() : false;
    QColor viewportBg = dark ? QColor("#141414") : pal.color(QPalette::Window);
    QColor canvasBg   = dark ? QColor("#222222") : QColor(Qt::white);

    setBackground(QBrush(viewportBg));
    axisRect()->setBackground(QBrush(canvasBg));

    QColor axisColor = pal.color(QPalette::Text);
    QPen axisPen(axisColor);
    xAxis->setBasePen(axisPen);
    yAxis->setBasePen(axisPen);
    xAxis->setTickPen(axisPen);
    yAxis->setTickPen(axisPen);
    xAxis->setSubTickPen(axisPen);
    yAxis->setSubTickPen(axisPen);
    xAxis->setLabelColor(axisColor);
    yAxis->setLabelColor(axisColor);
    xAxis->setTickLabelColor(axisColor);
    yAxis->setTickLabelColor(axisColor);

    if (legend) {
        // 同步图例文本颜色
        legend->setTextColor(axisColor);
        // 同步图例背景与边框，使其与主题匹配
        // 暗色主题使用画布色并稍加透明，亮色主题使用窗口色并稍加透明
        QColor legendBg = canvasBg;
        legendBg.setAlpha(200);
        legend->setBrush(QBrush(legendBg));
        // 使用较弱的边框以融入主题
        QPen borderPen(axisColor);
        borderPen.setColor(axisColor);
        borderPen.setStyle(Qt::SolidLine);
        borderPen.setWidth(1);
        borderPen.setColor(dark ? QColor(255,255,255,60) : QColor(0,0,0,40));
        legend->setBorderPen(borderPen);
    }
}

void StyledPlot::onStylesheetChanged() {
    applyTheme();
    replot();
}

void StyledPlot::replot(QCustomPlot::RefreshPriority priority) {
    QCustomPlot::replot(priority);
}

} // namespace QtAdvancedUI

