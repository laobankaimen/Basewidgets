#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

StyledWidget::StyledWidget(QWidget* parent)
    : QWidget(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

StyledWidget::StyledWidget(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QWidget(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

StyledWidget::~StyledWidget() = default;

void StyledWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void StyledWidget::attachToManager()
{
    if (!m_manager) return;
    // 连接信号，当样式更新时刷新本控件样式
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &StyledWidget::onStylesheetChanged);
    applyStylesheet();
}

void StyledWidget::onStylesheetChanged()
{
    applyStylesheet();
}

void StyledWidget::applyStylesheet()
{
    if (!m_manager) return;
    // 简单应用全局样式表；复杂组件可在派生类中覆盖
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI