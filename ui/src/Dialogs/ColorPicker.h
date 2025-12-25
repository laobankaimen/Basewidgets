/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#pragma once

#include <QWidget>
#include <QColor>
#include <QVBoxLayout>
#include <QVector>

class QSlider;
class QSpinBox;
class QLineEdit;
class QLabel;
class QCheckBox;
class QHBoxLayout;
class QComboBox;
class QToolButton;
class QButtonGroup;

namespace QtAdvancedUI {

class ColorArea;
class HueSlider;

class ColorPickerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPickerWidget(QWidget* parent = nullptr, const QColor& initial = Qt::white);
    QColor currentColor() const;

signals:
    void colorChanged(const QColor& c);

private:
    void setupUi(const QColor& initial);
    void updateFromSV(int s, int v);
    void updateFromHue(int h);
    void updateFromRGB();
    void updateFromHex();

    QColor m_color;

    // UI elements
    // New Design Elements
    ColorArea* m_svArea = nullptr; // Saturation-Value Area
    HueSlider* m_hueSlider = nullptr; // Hue Slider
    QComboBox* m_modeCombo = nullptr;
    QWidget* m_inputContainer = nullptr;
    
    // RGB Inputs
    QWidget* m_rgbWidget = nullptr;
    QSpinBox *m_rSpin = nullptr, *m_gSpin = nullptr, *m_bSpin = nullptr;
    
    // Hex Input
    QWidget* m_hexWidget = nullptr;
    QLineEdit* m_hexEdit = nullptr;
    
    // Recent & Preset
    QWidget* m_recentColorsWidget = nullptr;
    QHBoxLayout* m_recentLayout = nullptr;
    QButtonGroup* m_colorGroup = nullptr;
    
    // Internal state
    int m_h = 0, m_s = 0, m_v = 255;
    
    void updateInputs();
    void addRecentColor(const QColor& c);
    void rebuildRecentColors();
    void createPresetColors(QWidget* parent, QVBoxLayout* layout);

    // Custom colors
    static QVector<QColor> s_customColors;
};

class ColorPicker
{
public:
    static QWidget* makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut);
};

} // namespace QtAdvancedUI
