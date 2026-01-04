/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
#include <basewidgets/Inputs/Label.h>
#include <basewidgets/Core/StyleManagerAdapter.h>

using namespace acss;

namespace basewidgets {

Label::Label(QWidget* parent)
    : QLabel(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

Label::Label(const QString& text, QWidget* parent)
    : QLabel(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

Label::Label(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QLabel(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

Label::~Label() = default;

void Label::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void Label::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void Label::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void Label::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace basewidgets

