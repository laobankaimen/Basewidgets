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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &LineEdit::onStylesheetChanged);
    applyStylesheet();
}

void LineEdit::onStylesheetChanged()
{
    applyStylesheet();
}

void LineEdit::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI