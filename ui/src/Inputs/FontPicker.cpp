#include <QtAdvancedUI/Inputs/FontPicker.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QHBoxLayout>

using namespace acss;

namespace QtAdvancedUI {

FontPicker::FontPicker(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());

    m_preview = new QLabel(this);
    m_preview->setText("示例文本 AaBbCc 123");
    m_button = new PushButton(this);
    m_button->setText("选择字体...");

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_preview, 1);
    lay->addWidget(m_button);
    setLayout(lay);

    connect(m_button, &PushButton::clicked, this, &FontPicker::chooseFont);
}

FontPicker::FontPicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());

    m_preview = new QLabel(this);
    m_preview->setText("示例文本 AaBbCc 123");
    m_button = new PushButton(this);
    m_button->setText("选择字体...");

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_preview, 1);
    lay->addWidget(m_button);
    setLayout(lay);

    connect(m_button, &PushButton::clicked, this, &FontPicker::chooseFont);
}

FontPicker::~FontPicker() = default;

void FontPicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void FontPicker::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &FontPicker::onStylesheetChanged);
    applyStylesheet();
}

void FontPicker::onStylesheetChanged()
{
    applyStylesheet();
}

void FontPicker::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

void FontPicker::setFontValue(const QFont& f)
{
    if (m_font == f) return;
    m_font = f;
    updatePreview();
    emit fontChanged(m_font);
}

void FontPicker::updatePreview()
{
    m_preview->setFont(m_font);
}

void FontPicker::chooseFont()
{
    bool ok = false;
    QFont chosen = DialogUtils::pickFont(this, m_font, &ok);
    if (ok) {
        setFontValue(chosen);
    }
}

} // namespace QtAdvancedUI