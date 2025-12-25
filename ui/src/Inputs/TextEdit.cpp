#include <QtAdvancedUI/Inputs/TextEdit.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

TextEdit::TextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

TextEdit::TextEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QTextEdit(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

TextEdit::~TextEdit() = default;

void TextEdit::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void TextEdit::attachToManager()
{
    if (!m_manager) return;
    // 组件级不再连接样式变更信号；应用级统一管理
}

void TextEdit::onStylesheetChanged()
{
    // 统一在应用级进行样式更新；此处不再单独处理
}

void TextEdit::applyStylesheet()
{
    // 清空组件级样式应用，改为应用级统一管理
}

} // namespace QtAdvancedUI
