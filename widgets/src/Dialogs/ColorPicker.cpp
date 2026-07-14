/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#include "ColorPicker.h"

#include <basewidgets/Core/StyleManagerAdapter.h>

#include <QColor>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QButtonGroup>
#include <QPainter>
#include <QIcon>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QToolButton>
#include <QScreen>
#include <QCursor>
#include <QStyleOptionSlider>
#include <QWindow>
#include <QApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

using namespace acss;

namespace basewidgets
{

// 2D Color Area: Saturation (X) vs Value (Y) for a fixed Hue
class ColorArea : public QWidget
{
    Q_OBJECT
public:
    explicit ColorArea(QWidget* parent = nullptr) : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumHeight(200);
        setMouseTracking(true);
    }

    void setHue(int h)
    {
        h_ = qBound(0, h, 359);
        regenerate();
        update();
    }

    void setSV(int s, int v)
    {
        s_ = qBound(0, s, 255);
        v_ = qBound(0, v, 255);
        update();
    }

signals:
    void svChanged(int s, int v);

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        if (img_.isNull() || img_.size() != size())
            regenerate();
        
        p.drawImage(rect(), img_);

        // Draw indicator
        QPoint pt = mapFromSV(s_, v_);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::black, 2));
        p.drawEllipse(pt, 6, 6);
        p.setPen(QPen(Qt::white, 2));
        p.drawEllipse(pt, 8, 8);
    }

    void resizeEvent(QResizeEvent*) override
    {
        regenerate();
    }

    void mousePressEvent(QMouseEvent* e) override { applyPoint(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (e->buttons() & Qt::LeftButton)
            applyPoint(e->pos());
    }

private:
    void regenerate()
    {
        if (width() <= 0 || height() <= 0) return;
        
        QImage sv(size(), QImage::Format_ARGB32_Premultiplied);
        
        // 1. Fill with Hue color
        QColor hueColor;
        hueColor.setHsv(h_, 255, 255);
        sv.fill(hueColor);

        QPainter p(&sv);
        // 2. Saturation Gradient: White (Left) -> Transparent (Right)
        sv.fill(Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        
        QLinearGradient satGrad(0, 0, width(), 0);
        satGrad.setColorAt(0, Qt::white);
        satGrad.setColorAt(1, hueColor);
        p.fillRect(sv.rect(), satGrad);
        
        // 3. Value Gradient: Transparent (Top) -> Black (Bottom)
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        QLinearGradient valGrad(0, 0, 0, height());
        valGrad.setColorAt(0, Qt::transparent);
        valGrad.setColorAt(1, Qt::black);
        p.fillRect(sv.rect(), valGrad);
        
        p.end();
        img_ = sv;
    }

    QPoint mapFromSV(int s, int v) const
    {
        int x = s * width() / 255;
        int y = (255 - v) * height() / 255;
        return QPoint(x, y);
    }

    void applyPoint(const QPoint& pt)
    {
        int x = qBound(0, pt.x(), width() - 1);
        int y = qBound(0, pt.y(), height() - 1);
        
        int s = x * 255 / width();
        int v = 255 - (y * 255 / height());
        
        if (s != s_ || v != v_) {
            s_ = s;
            v_ = v;
            emit svChanged(s_, v_);
            update();
        }
    }

    int h_ = 0;
    int s_ = 0;
    int v_ = 255;
    QImage img_;
};

// Hue Slider with Rainbow Gradient
class HueSlider : public QSlider
{
    Q_OBJECT
public:
    explicit HueSlider(QWidget* parent = nullptr) : QSlider(Qt::Horizontal, parent)
    {
        setRange(0, 359);
    }

protected:
    void paintEvent(QPaintEvent* ev) override
    {
        QPainter p(this);
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
        QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

        // Draw Rainbow Groove
        QLinearGradient grad(groove.left(), 0, groove.right(), 0);
        for (int i = 0; i <= 360; i += 60) {
            QColor c;
            c.setHsv(i % 360, 255, 255);
            grad.setColorAt(i / 360.0, c);
        }
        grad.setColorAt(1.0, QColor::fromHsv(0, 255, 255)); // Wrap around

        p.setRenderHint(QPainter::Antialiasing);

        // Draw rounded groove
        QPainterPath path;
        path.addRoundedRect(groove, 4, 4);
        p.fillPath(path, grad);

        // Draw Handle
        p.setBrush(Qt::white);
        p.setPen(QPen(QColor(0xCCCCCC), 1));
        
        int hSize = handle.width();
        QRect customHandle = handle;
        //不是绝对的正中心，需要微调
        QPoint ellipseCenter = customHandle.center()+QPoint(1,1);

        // Draw handle shadow/border
        p.setBrush(Qt::white);
        p.setPen(QPen(Qt::lightGray, 1));
        p.drawEllipse(ellipseCenter, hSize/2, hSize/2);
        
        // Inner blue dot
        QColor hueColor;
        hueColor.setHsv(value(), 255, 255);
        p.setBrush(hueColor);
        p.setPen(Qt::NoPen);
        p.drawEllipse(ellipseCenter, hSize/4, hSize/4);
    }
};

// Screen Picker Logic
class ScreenPicker : public QObject
{
    Q_OBJECT
public:
    explicit ScreenPicker(QObject* parent = nullptr) : QObject(parent)
    {
        m_timer = new QTimer(this);
        m_timer->setInterval(50); // Check every 50ms
        connect(m_timer, &QTimer::timeout, this, &ScreenPicker::checkMouse);
    }

    void start()
    {
#ifdef Q_OS_WIN
        m_isPicking = true;
        m_timer->start();
#else
        // Linux/Mac fallback: Use Qt's cursor tracking (simplified)
        // For strict system API requirements on Linux, X11/XCB calls would be needed,
        // but this provides a functional fallback using Qt's abstraction.
        m_isPicking = true;
        m_timer->start();
#endif
    }

    void stop()
    {
        m_isPicking = false;
        m_timer->stop();
    }

signals:
    void colorPicked(const QColor& c);

private slots:
    void checkMouse()
    {
        if (!m_isPicking) return;

#ifdef Q_OS_WIN
        // Windows System API Implementation
        
        // 1. Check for cancellation keys (Esc, Right Click)
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000 || GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
            stop();
            return;
        }

        // 2. Get Cursor Position
        POINT pt;
        if (GetCursorPos(&pt)) {
            // 3. Get Color at Cursor
            HDC hdc = GetDC(NULL);
            if (hdc) {
                COLORREF color = GetPixel(hdc, pt.x, pt.y);
                ReleaseDC(NULL, hdc);
                
                // Optional: Emit a "preview" signal if we wanted to show live color
                // emit colorPreview(QColor(GetRValue(color), GetGValue(color), GetBValue(color)));

                // 4. Check for Click (Left Button)
                // Use 0x8000 to check if currently down. 
                // Note: This might pass the click through to the underlying window.
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    QColor c(GetRValue(color), GetGValue(color), GetBValue(color));
                    emit colorPicked(c);
                    stop();
                    
                    // Consume the click? Hard without hooks.
                    // We accept that it might click the window below.
                }
            }
        }
#else
        // Linux/Generic Qt Implementation
        // Uses Qt to poll global cursor position and grab pixel
        
        // Check for Esc key (requires window focus usually, so this is a limitation of polling)
        // But we can check mouse buttons via Qt::GuiApplication if supported
        
        QPoint pos = QCursor::pos();
        QScreen* screen = QGuiApplication::screenAt(pos);
        if (!screen) screen = QGuiApplication::primaryScreen();
        
        if (screen) {
            // Grab 1x1 pixel
            QPixmap px = screen->grabWindow(0, pos.x(), pos.y(), 1, 1);
            if (!px.isNull()) {
                QColor c = px.toImage().pixelColor(0, 0);
                
                // Detect click - In generic Qt without a window grab, it's hard to detect global clicks.
                // We'll rely on a "hover to pick, click to confirm" strategy?
                // Or simply: On Linux, we might MUST use the invisible window approach.
                // But user asked for System API. 
                // Since we are on Windows env now, we provide this placeholder.
                
                if (QGuiApplication::mouseButtons() & Qt::LeftButton) {
                    emit colorPicked(c);
                    stop();
                }
            }
        }
#endif
    }

private:
    QTimer* m_timer;
    bool m_isPicking = false;
};

// Custom Color Button for Recent/Presets
class ColorButton : public QAbstractButton
{
public:
    explicit ColorButton(const QColor& c, int size, QWidget* parent = nullptr) 
        : QAbstractButton(parent), m_color(c)
    {
        setCheckable(true);
        setFixedSize(size, size);
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Margin for border (2px)
        QRect r = rect().adjusted(3, 3, -3, -3);

        // Fill Color
        p.setBrush(m_color);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(r, 2, 2);

        // Draw Border
        if (isChecked()) {
            // Selected: White inner border + Blue/Theme outer border?
            // Or just a strong double border.
            // Inner white
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::white, 2));
            p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 2, 2);
            
            // Outer highlight (blue-ish or contrast)
            p.setPen(QPen(QColor(0x21, 0x96, 0xF3), 2)); // Material Blue
            p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);
        }
        else if (underMouse()) {
            // Hover: White border
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::white, 2));
            p.drawRoundedRect(r, 2, 2);
            
            // Outer gray ring
            p.setPen(QPen(QColor(100, 100, 100, 50), 1));
            p.drawRoundedRect(rect().adjusted(0,0,-1,-1), 3, 3);
        } else {
            // Normal: Light gray border
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(0xCC, 0xCC, 0xCC), 1));
            p.drawRoundedRect(r, 2, 2);
        }
    }
    
    void enterEvent(QEvent* e) override {
        update();
        QAbstractButton::enterEvent(e);
    }
    
    void leaveEvent(QEvent* e) override {
        update();
        QAbstractButton::leaveEvent(e);
    }

private:
    QColor m_color;
};

#include "ColorPicker.moc"

QVector<QColor> ColorPickerWidget::s_customColors;

ColorPickerWidget::ColorPickerWidget(QWidget* parent, const QColor& initial)
    : QWidget(parent), m_color(initial)
{
    // Initialize custom colors if empty
    if (s_customColors.isEmpty()) {
        s_customColors = {
            QColor("#F44336"), QColor("#E91E63"), QColor("#9C27B0"), QColor("#673AB7"),
            QColor("#3F51B5"), QColor("#2196F3"), QColor("#03A9F4"), QColor("#00BCD4"),
            QColor("#009688"), QColor("#4CAF50"), QColor("#8BC34A"), QColor("#CDDC39"),
            QColor("#FFEB3B"), QColor("#FFC107"), QColor("#FF9800"), QColor("#FF5722")
        };
    }
    
    m_colorGroup = new QButtonGroup(this);
    m_colorGroup->setExclusive(true);

    setupUi(initial);
}

QColor ColorPickerWidget::currentColor() const
{
    return m_color;
}

void ColorPickerWidget::setupUi(const QColor& initial)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);

    setFixedWidth(600); // Try a fixed width to ensure compactness

    // 1. Color Area (S-V)
    m_svArea = new ColorArea(this);
    mainLayout->addWidget(m_svArea);

    // 2. Hue Slider
    m_hueSlider = new HueSlider(this);
    mainLayout->addWidget(m_hueSlider);

    // 3. Mode & Inputs
    m_inputContainer = new QWidget(this);
    auto* inputLayout = new QHBoxLayout(m_inputContainer);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItems({"RGB", "HEX"});
    m_modeCombo->setFixedWidth(80);
    inputLayout->addWidget(m_modeCombo);

    // RGB Inputs - Fix stretch
    m_rgbWidget = new QWidget(this);
    auto* rgbLayout = new QHBoxLayout(m_rgbWidget);
    rgbLayout->setContentsMargins(0, 0, 0, 0);
    rgbLayout->setSpacing(5);
    
    auto createSpin = [&](QSpinBox** sp) {
        *sp = new QSpinBox(this);
        (*sp)->setRange(0, 255);
        (*sp)->setAlignment(Qt::AlignCenter);
        (*sp)->setProperty("hideButton", QVariant(true));
        (*sp)->setButtonSymbols(QAbstractSpinBox::NoButtons);
        (*sp)->setFixedWidth(80);
        rgbLayout->addWidget(*sp);
    };
    createSpin(&m_rSpin);
    createSpin(&m_gSpin);
    createSpin(&m_bSpin);

    rgbLayout->addStretch(); 
    
    inputLayout->addWidget(m_rgbWidget);

    // Hex Input - Fix stretch
    m_hexWidget = new QWidget(this);
    auto* hexLayout = new QHBoxLayout(m_hexWidget);
    hexLayout->setContentsMargins(0, 0, 0, 0);
    m_hexEdit = new QLineEdit(this);
    m_hexEdit->setAlignment(Qt::AlignCenter);
    m_hexEdit->setFixedWidth(100); // Constrain width
    hexLayout->addWidget(m_hexEdit);
    hexLayout->addStretch();
    
    inputLayout->addWidget(m_hexWidget);
    m_hexWidget->setVisible(false);

    inputLayout->addStretch(); // Push everything to left, except "Pick" button

    // Screen Pick Button
    QPushButton* screenPickBtn = new QPushButton(this);
    //screenPickBtn->setIcon(QIcon::fromTheme("color-picker"));
    screenPickBtn->setText(tr("取色"));
    screenPickBtn->setFixedWidth(60);
    inputLayout->addWidget(screenPickBtn);

    mainLayout->addWidget(m_inputContainer);

    // 4. Recent Colors - Fix Vertical Alignment
    auto* recentLblRow = new QHBoxLayout();
    auto* recentLbl = new QLabel(tr("最近使用颜色"), this);
    recentLblRow->addWidget(recentLbl);
    recentLblRow->addStretch();
    
    auto* btnAdd = new QToolButton(this);
    btnAdd->setToolTip(tr("添加当前颜色"));
    btnAdd->setAutoRaise(true);
    // Make button larger and text larger
    btnAdd->setFixedSize(30, 30);
    btnAdd->setIcon(QIcon("bwicon:buttons/square_plus.svg"));
    btnAdd->setProperty("iconOnly", QVariant(true));
    recentLblRow->addWidget(btnAdd);
    
    // Align text and button vertically center
    recentLblRow->setAlignment(Qt::AlignVCenter);
    mainLayout->addLayout(recentLblRow);

    m_recentColorsWidget = new QWidget(this);
    m_recentLayout = new QHBoxLayout(m_recentColorsWidget);
    m_recentLayout->setContentsMargins(0, 0, 0, 0);
    m_recentLayout->setSpacing(4);
    // Align Center to center the color blocks
    m_recentLayout->setAlignment(Qt::AlignCenter); 
    mainLayout->addWidget(m_recentColorsWidget);

    rebuildRecentColors();

    // 5. Preset Colors - Fix Vertical Alignment
    auto* presetLbl = new QLabel(tr("系统预设颜色"), this);
    mainLayout->addWidget(presetLbl); // Default alignment usually fine, but can ensure
    
    auto* presetWidget = new QWidget(this);
    auto* presetLayout = new QVBoxLayout(presetWidget);
    presetLayout->setContentsMargins(0, 0, 0, 0);
    presetLayout->setSpacing(2);
    // Align Center to center the grid
    presetLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop); 
    createPresetColors(presetWidget, presetLayout);
    mainLayout->addWidget(presetWidget);
    

    // Connections
    connect(m_hueSlider, &QSlider::valueChanged, this, &ColorPickerWidget::updateFromHue);
    connect(m_svArea, &ColorArea::svChanged, this, &ColorPickerWidget::updateFromSV);
    
    connect(m_rSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerWidget::updateFromRGB);
    connect(m_gSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerWidget::updateFromRGB);
    connect(m_bSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorPickerWidget::updateFromRGB);
    
    connect(m_hexEdit, &QLineEdit::editingFinished, this, &ColorPickerWidget::updateFromHex);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index){
        m_rgbWidget->setVisible(index == 0);
        m_hexWidget->setVisible(index == 1);
        updateInputs();
    });

    connect(btnAdd, &QToolButton::clicked, [this](){
        addRecentColor(m_color);
    });

    // Screen pick logic - System API Based
    connect(screenPickBtn, &QPushButton::clicked, [this](){
        auto* picker = new ScreenPicker(this);
        connect(picker, &ScreenPicker::colorPicked, this, [this, picker](const QColor& c){
             m_color = c;
             m_h = c.hue(); if(m_h < 0) m_h = 0;
             m_s = c.saturation();
             m_v = c.value();
             
             QSignalBlocker b1(m_hueSlider), b2(m_svArea);
             m_hueSlider->setValue(m_h);
             m_svArea->setHue(m_h);
             m_svArea->setSV(m_s, m_v);
             
             updateInputs();
             emit colorChanged(m_color);
             
             picker->deleteLater();
             // Restore cursor
             QApplication::restoreOverrideCursor();
        });
        
        // Change cursor to crosshair globally
        QApplication::setOverrideCursor(Qt::CrossCursor);
        picker->start();
    });

    // Initial State
    m_h = initial.hue(); if(m_h < 0) m_h = 0;
    m_s = initial.saturation();
    m_v = initial.value();
    
    m_hueSlider->setValue(m_h);
    m_svArea->setHue(m_h);
    m_svArea->setSV(m_s, m_v);
    
    updateInputs();
}

void ColorPickerWidget::updateFromHue(int h)
{
    m_h = h;
    m_svArea->setHue(h);
    m_color.setHsv(m_h, m_s, m_v);
    
    // Clear selection from presets
    if (m_colorGroup) {
        if (QAbstractButton* b = m_colorGroup->checkedButton()) {
            m_colorGroup->setExclusive(false);
            b->setChecked(false);
            m_colorGroup->setExclusive(true);
        }
    }

    updateInputs();
    emit colorChanged(m_color);
}

void ColorPickerWidget::updateFromSV(int s, int v)
{
    m_s = s;
    m_v = v;
    m_color.setHsv(m_h, m_s, m_v);
    
    // Clear selection
    if (m_colorGroup) {
        if (QAbstractButton* b = m_colorGroup->checkedButton()) {
            m_colorGroup->setExclusive(false);
            b->setChecked(false);
            m_colorGroup->setExclusive(true);
        }
    }

    updateInputs();
    emit colorChanged(m_color);
}

void ColorPickerWidget::updateFromRGB()
{
    QColor c(m_rSpin->value(), m_gSpin->value(), m_bSpin->value());
    if (c.isValid()) {
        m_color = c;
        m_h = c.hue(); if(m_h < 0) m_h = 0;
        m_s = c.saturation();
        m_v = c.value();
        
        // Block signals to prevent loop
        QSignalBlocker b1(m_hueSlider), b2(m_svArea);
        m_hueSlider->setValue(m_h);
        m_svArea->setHue(m_h);
        m_svArea->setSV(m_s, m_v);
        
        // Clear selection
        if (m_colorGroup) {
            if (QAbstractButton* b = m_colorGroup->checkedButton()) {
                m_colorGroup->setExclusive(false);
                b->setChecked(false);
                m_colorGroup->setExclusive(true);
            }
        }
        
        if (m_modeCombo->currentIndex() == 1) {
             m_hexEdit->setText(m_color.name().toUpper());
        }
        
        emit colorChanged(m_color);
    }
}

void ColorPickerWidget::updateFromHex()
{
    QColor c(m_hexEdit->text());
    if (c.isValid()) {
        m_color = c;
        m_h = c.hue(); if(m_h < 0) m_h = 0;
        m_s = c.saturation();
        m_v = c.value();
        
        QSignalBlocker b1(m_hueSlider), b2(m_svArea);
        m_hueSlider->setValue(m_h);
        m_svArea->setHue(m_h);
        m_svArea->setSV(m_s, m_v);
        
        // Clear selection
        if (m_colorGroup) {
            if (QAbstractButton* b = m_colorGroup->checkedButton()) {
                m_colorGroup->setExclusive(false);
                b->setChecked(false);
                m_colorGroup->setExclusive(true);
            }
        }
        
        // Update RGB
        QSignalBlocker b3(m_rSpin), b4(m_gSpin), b5(m_bSpin);
        m_rSpin->setValue(c.red());
        m_gSpin->setValue(c.green());
        m_bSpin->setValue(c.blue());
        
        emit colorChanged(m_color);
    }
}

void ColorPickerWidget::updateInputs()
{
    QSignalBlocker b1(m_rSpin), b2(m_gSpin), b3(m_bSpin), b4(m_hexEdit);
    m_rSpin->setValue(m_color.red());
    m_gSpin->setValue(m_color.green());
    m_bSpin->setValue(m_color.blue());
    m_hexEdit->setText(m_color.name().toUpper());
}

void ColorPickerWidget::addRecentColor(const QColor& c)
{
    // Check duplicates
    if (s_customColors.contains(c)) return;
    
    s_customColors.prepend(c);
    if (s_customColors.size() > 10) s_customColors.removeLast(); // Limit to 10
    
    rebuildRecentColors();
}

void ColorPickerWidget::rebuildRecentColors()
{
    // Clear layout
    QLayoutItem* item;
    while ((item = m_recentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // 30px size for better visibility
    int btnSize = 30;
    
    for (const QColor& c : s_customColors) {
        ColorButton* btn = new ColorButton(c, btnSize, this);
        m_colorGroup->addButton(btn); // Add to group for mutual exclusion
        
        connect(btn, &ColorButton::clicked, [this, c](){
             m_color = c;
             m_h = c.hue(); if(m_h < 0) m_h = 0;
             m_s = c.saturation();
             m_v = c.value();
             
             QSignalBlocker b1(m_hueSlider), b2(m_svArea);
             m_hueSlider->setValue(m_h);
             m_svArea->setHue(m_h);
             m_svArea->setSV(m_s, m_v);
             
             updateInputs();
             emit colorChanged(m_color);
        });
        
        m_recentLayout->addWidget(btn);
    }

    m_recentLayout->addStretch(); 
}

void ColorPickerWidget::createPresetColors(QWidget* parent, QVBoxLayout* layout)
{
    // Grid of colors
    QList<QColor> presets = {
        Qt::black, Qt::darkGray, Qt::gray, Qt::lightGray, Qt::white,
        Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta, Qt::yellow,
        Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow,
        QColor("#F44336"), QColor("#E91E63"), QColor("#9C27B0"), QColor("#673AB7"), QColor("#3F51B5"),
        QColor("#2196F3"), QColor("#03A9F4"), QColor("#00BCD4"), QColor("#009688"), QColor("#4CAF50"),
        QColor("#8BC34A"), QColor("#CDDC39"), QColor("#FFEB3B"), QColor("#FFC107"), QColor("#FF9800")
    };
    
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(6); // Increased spacing
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Ensure grid itself is aligned
    
    int row = 0, col = 0;
    int maxCols = 16; // 16 columns * (30+6) = ~576px, fits in 600px nicely
    int btnSize = 30;

    for (const QColor& c : presets) {
        ColorButton* btn = new ColorButton(c, btnSize, parent);
        m_colorGroup->addButton(btn); // Add to group
        
        connect(btn, &ColorButton::clicked, [this, c](){
             m_color = c;
             m_h = c.hue(); if(m_h < 0) m_h = 0;
             m_s = c.saturation();
             m_v = c.value();
             
             QSignalBlocker b1(m_hueSlider), b2(m_svArea);
             m_hueSlider->setValue(m_h);
             m_svArea->setHue(m_h);
             m_svArea->setSV(m_s, m_v);
             
             updateInputs();
             emit colorChanged(m_color);
        });
        
        grid->addWidget(btn, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
    
    layout->addLayout(grid);
}

QWidget* ColorPicker::makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut)
{
    auto* w = new ColorPickerWidget(parent, initial);
    if (currentOut) *currentOut = w->currentColor();
    QObject::connect(w, &ColorPickerWidget::colorChanged, [currentOut](const QColor& c){
        if (currentOut) *currentOut = c;
    });
    return w;
}

} // namespace basewidgets
