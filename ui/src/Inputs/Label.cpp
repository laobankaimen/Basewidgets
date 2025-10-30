#include <QtAdvancedUI/Inputs/Label.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

using namespace acss;

namespace QtAdvancedUI {

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
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &Label::onStylesheetChanged);
    applyStylesheet();
}

void Label::onStylesheetChanged()
{
    applyStylesheet();
}

void Label::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

} // namespace QtAdvancedUI