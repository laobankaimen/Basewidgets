#include <QtAdvancedUI/Inputs/LineEdit.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

LineEdit::LineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

LineEdit::LineEdit(const QString& text, QWidget* parent)
    : QLineEdit(text, parent)
{
    setStyleManager(StyleManagerAdapter::instance());
}

LineEdit::LineEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QLineEdit(parent)
{
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

LineEdit::~LineEdit() = default;

void LineEdit::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void LineEdit::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void LineEdit::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void LineEdit::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
