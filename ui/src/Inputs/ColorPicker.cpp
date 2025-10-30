#include <QtAdvancedUI/Inputs/ColorPicker.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>
#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QHBoxLayout>
#include <QPalette>

using namespace acss;

namespace QtAdvancedUI {

ColorPicker::ColorPicker(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(StyleManagerAdapter::instance());

    m_button = new PushButton(this);
    m_button->setText("选择颜色...");
    m_preview = new QLabel(this);
    m_preview->setFixedSize(32, 20);
    m_preview->setFrameShape(QFrame::Box);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_button);
    lay->addWidget(m_preview);
    setLayout(lay);

    connect(m_button, &PushButton::clicked, this, &ColorPicker::chooseColor);
}

ColorPicker::ColorPicker(acss::QtAdvancedStylesheet* manager, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleManager(manager ? manager : StyleManagerAdapter::instance());

    m_button = new PushButton(this);
    m_button->setText("选择颜色...");
    m_preview = new QLabel(this);
    m_preview->setFixedSize(32, 20);
    m_preview->setFrameShape(QFrame::Box);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_button);
    lay->addWidget(m_preview);
    setLayout(lay);

    connect(m_button, &PushButton::clicked, this, &ColorPicker::chooseColor);
}

ColorPicker::~ColorPicker() = default;

void ColorPicker::setStyleManager(acss::QtAdvancedStylesheet* manager)
{
    if (m_manager == manager) return;
    m_manager = manager;
    attachToManager();
}

void ColorPicker::attachToManager()
{
    if (!m_manager) return;
    connect(m_manager, &QtAdvancedStylesheet::stylesheetChanged,
            this, &ColorPicker::onStylesheetChanged);
    applyStylesheet();
}

void ColorPicker::onStylesheetChanged()
{
    applyStylesheet();
}

void ColorPicker::applyStylesheet()
{
    if (!m_manager) return;
    this->setStyleSheet(m_manager->styleSheet());
}

void ColorPicker::setColor(const QColor& c)
{
    if (m_color == c) return;
    m_color = c;
    updatePreview();
    emit colorChanged(m_color);
}

void ColorPicker::updatePreview()
{
    QPalette pal = m_preview->palette();
    pal.setColor(QPalette::Window, m_color);
    m_preview->setAutoFillBackground(true);
    m_preview->setPalette(pal);
}

void ColorPicker::chooseColor()
{
    QColor chosen = DialogUtils::pickColor(this, m_color);
    if (chosen.isValid()) {
        setColor(chosen);
    }
}

} // namespace QtAdvancedUI