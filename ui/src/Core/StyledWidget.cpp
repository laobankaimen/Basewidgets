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
    // 组件级不再连接样式变更信号；应用级统一管理
}

void StyledWidget::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void StyledWidget::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
