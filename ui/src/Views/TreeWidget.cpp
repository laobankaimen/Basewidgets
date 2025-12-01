#include <QtAdvancedUI/Views/TreeWidget.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TreeWidget::TreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TreeWidget::~TreeWidget() = default;

void TreeWidget::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TreeWidget::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TreeWidget::onStylesheetChanged);
}

void TreeWidget::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void TreeWidget::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI