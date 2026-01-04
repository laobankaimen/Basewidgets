/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Views/TableView.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <QHeaderView>

using namespace acss;

namespace basewidgets {

TableView::TableView(QWidget* parent)
    : QTableView(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    if (auto* vh = verticalHeader()) {
        vh->setAttribute(Qt::WA_StyledBackground, true);
        vh->setAutoFillBackground(true);
    }
    if (auto* hh = horizontalHeader()) {
        hh->setAttribute(Qt::WA_StyledBackground, true);
        hh->setAutoFillBackground(true);
    }
    setStyleManager(StyleManagerAdapter::instance());
    setAlternatingRowColors(true);//交替行颜色
}

TableView::~TableView() = default;

void TableView::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TableView::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void TableView::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void TableView::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

