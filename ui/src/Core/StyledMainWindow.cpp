#include <QtAdvancedUI/Core/StyledMainWindow.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

StyledMainWindow::StyledMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

StyledMainWindow::StyledMainWindow(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

StyledMainWindow::~StyledMainWindow() = default;

void StyledMainWindow::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void StyledMainWindow::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void StyledMainWindow::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void StyledMainWindow::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI