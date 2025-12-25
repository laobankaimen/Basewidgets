#include <QtAdvancedUI/Inputs/RadioButton.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

RadioButton::RadioButton(QWidget* parent)
    : QRadioButton(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

RadioButton::RadioButton(const QString& text, QWidget* parent)
    : QRadioButton(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

RadioButton::RadioButton(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QRadioButton(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

RadioButton::~RadioButton() = default;

void RadioButton::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void RadioButton::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void RadioButton::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void RadioButton::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
