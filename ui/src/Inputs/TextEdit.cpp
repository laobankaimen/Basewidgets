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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &TextEdit::onStylesheetChanged);
    applyStylesheet();
}

void TextEdit::onStylesheetChanged()
{
    applyStylesheet();
}

void TextEdit::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI