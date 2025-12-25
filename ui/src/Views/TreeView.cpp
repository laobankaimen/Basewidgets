#include <QtAdvancedUI/Views/TreeView.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TreeView::~TreeView() = default;

void TreeView::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TreeView::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void TreeView::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void TreeView::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
