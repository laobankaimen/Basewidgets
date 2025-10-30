#include <QtAdvancedUI/Inputs/PlainTextEdit.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

PlainTextEdit::PlainTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());
}

PlainTextEdit::PlainTextEdit(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QPlainTextEdit(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());
}

PlainTextEdit::~PlainTextEdit() = default;

void PlainTextEdit::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void PlainTextEdit::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &PlainTextEdit::onStylesheetChanged);
    applyStylesheet();
}

void PlainTextEdit::onStylesheetChanged()
{
    applyStylesheet();
}

void PlainTextEdit::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI