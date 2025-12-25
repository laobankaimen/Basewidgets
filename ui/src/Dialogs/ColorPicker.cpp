#include "ColorPicker.h"

#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include <QCheckBox>
#include <QColor>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>

using namespace acss;

namespace QtAdvancedUI
{

// 简易 2D 颜色选择区域：水平为色相 H(0..359)，垂直为饱和度 S(0..255)
class ColorArea : public QWidget
{
    Q_OBJECT
public:
    explicit ColorArea(QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedSize(200, 160);
        setMouseTracking(true);
        regenerate();
    }
    void setHS(int h, int s)
    {
        h_ = qBound(0, h, 359);
        s_ = qBound(0, s, 255);
        update();
    }
signals:
    void hsChanged(int h, int s);

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.drawImage(rect(), img_);
        // 绘制选择指示器
        QPoint pt = mapFromHS(h_, s_);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::black, 2));
        p.drawEllipse(pt, 6, 6);
        p.setPen(QPen(Qt::white, 2));
        p.drawEllipse(pt, 8, 8);
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
        img_ = QImage(size(), QImage::Format_RGB32);
        for (int y = 0; y < img_.height(); ++y) {
            for (int x = 0; x < img_.width(); ++x) {
                int h = x * 360 / img_.width();
                int s = y * 255 / img_.height();
                QColor c;
                c.setHsv(h, s, 255);
                img_.setPixelColor(x, y, c);
            }
        }
    }
    QPoint mapFromHS(int h, int s) const
    {
        int x = h * width() / 360;
        int y = s * height() / 255;
        return QPoint(x, y);
    }
    void applyPoint(const QPoint& pt)
    {
        int x = qBound(0, pt.x(), width() - 1);
        int y = qBound(0, pt.y(), height() - 1);
        int h = x * 360 / width();
        int s = y * 255 / height();
        if (h != h_ || s != s_) {
            h_ = h;
            s_ = s;
            emit hsChanged(h_, s_);
            update();
        }
    }
    int h_ = 0;
    int s_ = 0;
    QImage img_;
};

#include "ColorPicker.moc"

QWidget* ColorPicker::makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut)
{
    QWidget* body = new QWidget(parent);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);

    QLabel* preview = new QLabel(body);
    preview->setFixedSize(64, 32);
    preview->setFrameShape(QFrame::Box);

    auto setPreview = [&](const QColor& c) {
        QPalette pal = preview->palette();
        pal.setColor(QPalette::Window, c);
        preview->setAutoFillBackground(true);
        preview->setPalette(pal);
    };

    auto* form = new QWidget(body);
    auto* grid = new QGridLayout(form);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(4);

    auto addRow = [&](int row, const QString& name, int value, QSlider** outSl, QSpinBox** outSp) {
        QLabel* lbl = new QLabel(name, form);
        QSlider* sl = new QSlider(Qt::Horizontal, form);
        sl->setRange(0, 255);
        sl->setValue(value);
        QSpinBox* sp = new QSpinBox(form);
        sp->setRange(0, 255);
        sp->setValue(value);
        QObject::connect(sl, &QSlider::valueChanged, sp, &QSpinBox::setValue);
        grid->addWidget(lbl, row, 0);
        grid->addWidget(sl, row, 1);
        grid->addWidget(sp, row, 2);
        *outSl = sl;
        *outSp = sp;
    };

    QSlider *slR, *slG, *slB;
    QSpinBox *spR, *spG, *spB;
    addRow(0, QObject::tr("红"), initial.red(), &slR, &spR);
    addRow(1, QObject::tr("绿"), initial.green(), &slG, &spG);
    addRow(2, QObject::tr("蓝"), initial.blue(), &slB, &spB);

    QLineEdit* hexEdit = new QLineEdit(form);
    hexEdit->setPlaceholderText("#RRGGBB");
    auto updateHexFromRGB = [&]() {
        QColor c(spR->value(), spG->value(), spB->value());
        hexEdit->setText('#' + c.name(QColor::HexRgb).mid(1));
        setPreview(c);
        if (currentOut)
            *currentOut = c;
    };
    auto updateRGBFromHex = [&]() {
        QString s = hexEdit->text().trimmed();
        if (s.startsWith('#'))
            s = s.mid(1);
        if (s.size() == 6) {
            QColor c('#' + s);
            if (c.isValid()) {
                slR->setValue(c.red());
                spR->setValue(c.red());
                slG->setValue(c.green());
                spG->setValue(c.green());
                slB->setValue(c.blue());
                spB->setValue(c.blue());
                setPreview(c);
                if (currentOut)
                    *currentOut = c;
            }
        }
    };
    QObject::connect(spR, QOverload<int>::of(&QSpinBox::valueChanged), [&](int) { updateHexFromRGB(); });
    QObject::connect(spG, QOverload<int>::of(&QSpinBox::valueChanged), [&](int) { updateHexFromRGB(); });
    QObject::connect(spB, QOverload<int>::of(&QSpinBox::valueChanged), [&](int) { updateHexFromRGB(); });
    QObject::connect(hexEdit, &QLineEdit::textChanged, [&](const QString&) { updateRGBFromHex(); });

    grid->addWidget(new QLabel(QObject::tr("十六进制"), form), 3, 0);
    grid->addWidget(hexEdit, 3, 1, 1, 2);
    form->setLayout(grid);

    // 屏幕取色器按钮
    QPushButton* screenPick = new QPushButton(QObject::tr("屏幕取色器"), body);
    v->addWidget(preview, 0, Qt::AlignLeft);
    v->addWidget(screenPick, 0, Qt::AlignLeft);
    v->addWidget(form);
    // 自定义色板
    {
        auto* paletteRow = new QHBoxLayout();
        QList<QColor> paletteColors = {QColor("#000000"),
                                     QColor("#FFFFFF"),
                                     QColor("#FF0000"),
                                     QColor("#00FF00"),
                                     QColor("#0000FF"),
                                     QColor("#FFFF00"),
                                     QColor("#FF00FF"),
                                     QColor("#00FFFF"),
                                     QColor("#FFA500"),
                                     QColor("#808080")};
        for (const QColor& pc : paletteColors) {
            QPushButton* b = new QPushButton(" ", body);
            b->setFixedSize(20, 20);
            
            QPixmap pix(18, 18);
            pix.fill(pc);
            QPainter p(&pix);
            p.setPen(QColor(0x999999));
            p.drawRect(0, 0, 17, 17);
            b->setIcon(QIcon(pix));
            b->setIconSize(QSize(18, 18));
            b->setFlat(true); // Make it flat to show icon clearly without button borders interfering too much? 
            // Actually, standard push button with icon is fine, but we want it to look like a color swatch.
            // Let's just set the icon. The border logic "border:1px solid #999" was in CSS.
            // I'll draw the border in the pixmap.

            QObject::connect(b, &QPushButton::clicked, [&, pc]() {
                slR->setValue(pc.red());
                spR->setValue(pc.red());
                slG->setValue(pc.green());
                spG->setValue(pc.green());
                slB->setValue(pc.blue());
                spB->setValue(pc.blue());
                updateHexFromRGB();
            });
            paletteRow->addWidget(b);
        }
        paletteRow->addStretch();
        v->addLayout(paletteRow);
    }
    // 扩展：模式切换 RGB/HSV/HSL + 交互式区域 + Alpha
    {
        auto* modeRow = new QHBoxLayout();
        QLabel* modeLbl = new QLabel(QObject::tr("模式"), body);
        QComboBox* modeBox = new QComboBox(body);
        modeBox->addItems({QObject::tr("RGB"), QObject::tr("HSV"), QObject::tr("HSL")});
        modeRow->addWidget(modeLbl);
        modeRow->addWidget(modeBox);
        modeRow->addStretch();
        v->addLayout(modeRow);

        QWidget* formHSV = new QWidget(body);
        auto* gridHSV = new QGridLayout(formHSV);
        gridHSV->setContentsMargins(0, 0, 0, 0);
        gridHSV->setHorizontalSpacing(6);
        gridHSV->setVerticalSpacing(4);

        // 顶部 2D 颜色区域（H/S），右侧明度滑条（V）
        auto* topRow = new QHBoxLayout();
        ColorArea* area = new ColorArea(formHSV);
        QSlider* slV = new QSlider(Qt::Vertical, formHSV);
        slV->setRange(0, 255);
        slV->setValue(initial.value());
        topRow->addWidget(area);
        topRow->addWidget(slV);
        gridHSV->addLayout(topRow, 0, 0, 1, 3);

        // 数值行：H / S / V
        auto addHSVRow =
            [&](int row, const QString& name, int min, int max, int value, QSlider** outSl, QSpinBox** outSp) {
                QLabel* lbl = new QLabel(name, formHSV);
                QSlider* sl = new QSlider(Qt::Horizontal, formHSV);
                sl->setRange(min, max);
                sl->setValue(value);
                QSpinBox* sp = new QSpinBox(formHSV);
                sp->setRange(min, max);
                sp->setValue(value);
                QObject::connect(sl, &QSlider::valueChanged, sp, &QSpinBox::setValue);
                gridHSV->addWidget(lbl, row, 0);
                gridHSV->addWidget(sl, row, 1);
                gridHSV->addWidget(sp, row, 2);
                *outSl = sl;
                *outSp = sp;
            };
        int initH = initial.hue();
        if (initH < 0)
            initH = 0;
        int initS = initial.saturation();
        QSlider *slH, *slS;
        QSpinBox *spH, *spS, *spV;
        addHSVRow(1, QObject::tr("色相"), 0, 359, initH, &slH, &spH);
        addHSVRow(2, QObject::tr("饱和度"), 0, 255, initS, &slS, &spS);
        // 明度滑条已在右侧，值同步到 SpinBox
        spV = new QSpinBox(formHSV);
        spV->setRange(0, 255);
        spV->setValue(slV->value());
        gridHSV->addWidget(new QLabel(QObject::tr("明度"), formHSV), 3, 0);
        gridHSV->addWidget(new QWidget(formHSV), 3, 1);
        gridHSV->addWidget(spV, 3, 2);

        // Alpha 通道（可选）
        QCheckBox* useAlpha = new QCheckBox(QObject::tr("使用 Alpha"), body);
        QSlider* slA = new QSlider(Qt::Horizontal, body);
        slA->setRange(0, 255);
        slA->setValue(initial.alpha());
        QSpinBox* spA = new QSpinBox(body);
        spA->setRange(0, 255);
        spA->setValue(initial.alpha());
        QObject::connect(slA, &QSlider::valueChanged, spA, &QSpinBox::setValue);
        auto* alphaRow = new QHBoxLayout();
        alphaRow->addWidget(useAlpha);
        alphaRow->addWidget(slA, 1);
        alphaRow->addWidget(spA);
        v->addLayout(alphaRow);

        auto applyHSV = [&]() {
            QColor c;
            c.setHsv(spH->value(), spS->value(), spV->value());
            if (useAlpha->isChecked())
                c.setAlpha(spA->value());
            slR->setValue(c.red());
            spR->setValue(c.red());
            slG->setValue(c.green());
            spG->setValue(c.green());
            slB->setValue(c.blue());
            spB->setValue(c.blue());
            hexEdit->setText('#' + c.name(QColor::HexRgb).mid(1));
            setPreview(c);
            if (currentOut)
                *currentOut = c;
        };
        area->setHS(initH, initS);
        QObject::connect(spH, QOverload<int>::of(&QSpinBox::valueChanged), [&](int) {
            area->setHS(spH->value(), spS->value());
            applyHSV();
        });
        QObject::connect(spS, QOverload<int>::of(&QSpinBox::valueChanged), [&](int) {
            area->setHS(spH->value(), spS->value());
            applyHSV();
        });
        QObject::connect(spV, QOverload<int>::of(&QSpinBox::valueChanged), [&](int) { applyHSV(); });
        QObject::connect(slV, &QSlider::valueChanged, spV, &QSpinBox::setValue);
        QObject::connect(area, &ColorArea::hsChanged, [&](int h, int s) {
            spH->setValue(h);
            spS->setValue(s);
            applyHSV();
        });

        formHSV->setLayout(gridHSV);
        v->addWidget(formHSV);
        formHSV->setVisible(false);

        // HSL 形态
        QWidget* formHSL = new QWidget(body);
        auto* gridHSL = new QGridLayout(formHSL);
        gridHSL->setContentsMargins(0, 0, 0, 0);
        gridHSL->setHorizontalSpacing(6);
        gridHSL->setVerticalSpacing(4);
        auto* topRowHSL = new QHBoxLayout();
        ColorArea* areaHSL = new ColorArea(formHSL);
        QSlider* slL = new QSlider(Qt::Vertical, formHSL);
        slL->setRange(0, 255);
        slL->setValue(initial.lightness());
        topRowHSL->addWidget(areaHSL);
        topRowHSL->addWidget(slL);
        gridHSL->addLayout(topRowHSL, 0, 0, 1, 3);

        auto addHSLRow = [&](int row, const QString& name, int min, int max, int value, QSlider** outSl, QSpinBox** outSp) {
            QLabel* lbl = new QLabel(name, formHSL);
            QSlider* sl = new QSlider(Qt::Horizontal, formHSL);
            sl->setRange(min, max);
            sl->setValue(value);
            QSpinBox* sp = new QSpinBox(formHSL);
            sp->setRange(min, max);
            sp->setValue(value);
            QObject::connect(sl, &QSlider::valueChanged, sp, &QSpinBox::setValue);
            gridHSL->addWidget(lbl, row, 0);
            gridHSL->addWidget(sl, row, 1);
            gridHSL->addWidget(sp, row, 2);
            *outSl = sl;
            *outSp = sp;
        };
        int initHHSL = initial.hue(); if (initHHSL < 0) initHHSL = 0;
        int initSHSL = initial.hslSaturation();
        QSlider *slHHSL, *slSHSL;
        QSpinBox *spHHSL, *spSHSL, *spL;
        addHSLRow(1, QObject::tr("色相"), 0, 359, initHHSL, &slHHSL, &spHHSL);
        addHSLRow(2, QObject::tr("饱和度"), 0, 255, initSHSL, &slSHSL, &spSHSL);
        spL = new QSpinBox(formHSL);
        spL->setRange(0, 255);
        spL->setValue(slL->value());
        gridHSL->addWidget(new QLabel(QObject::tr("亮度"), formHSL), 3, 0);
        gridHSL->addWidget(new QWidget(formHSL), 3, 1);
        gridHSL->addWidget(spL, 3, 2);

        auto applyHSL = [&]() {
            QColor c; c.setHsl(spHHSL->value(), spSHSL->value(), spL->value());
            if (useAlpha->isChecked()) c.setAlpha(spA->value());
            slR->setValue(c.red()); spR->setValue(c.red());
            slG->setValue(c.green()); spG->setValue(c.green());
            slB->setValue(c.blue()); spB->setValue(c.blue());
            hexEdit->setText('#' + c.name(QColor::HexRgb).mid(1));
            setPreview(c);
            if (currentOut) *currentOut = c;
        };

        areaHSL->setHS(initHHSL, initSHSL);
        QObject::connect(spHHSL, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ areaHSL->setHS(spHHSL->value(), spSHSL->value()); applyHSL(); });
        QObject::connect(spSHSL, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ areaHSL->setHS(spHHSL->value(), spSHSL->value()); applyHSL(); });
        QObject::connect(spL, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ applyHSL(); });
        QObject::connect(slL, &QSlider::valueChanged, spL, &QSpinBox::setValue);
        QObject::connect(areaHSL, &ColorArea::hsChanged, [&](int h, int s){ spHHSL->setValue(h); spSHSL->setValue(s); applyHSL(); });

        formHSL->setLayout(gridHSL);
        v->addWidget(formHSL);
        formHSL->setVisible(false);

        // 模式切换可见性
        QObject::connect(modeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int idx){
            bool rgb = (idx == 0), hsv = (idx == 1), hsl = (idx == 2);
            form->setVisible(rgb);
            formHSV->setVisible(hsv);
            formHSL->setVisible(hsl);
        });

        // 屏幕取色器：读取鼠标所在像素
        QObject::connect(screenPick, &QPushButton::clicked, [=](){
            QPoint gpos = QCursor::pos();
            QScreen* sc = QGuiApplication::screenAt(gpos);
            if (!sc) sc = QGuiApplication::primaryScreen();
            if (!sc) return;
            QPixmap px = sc->grabWindow(0, gpos.x(), gpos.y(), 1, 1);
            QImage im = px.toImage();
            if (im.isNull()) return;
            QColor c = QColor::fromRgb(im.pixel(0,0));
            useAlpha->setChecked(false);
            slR->setValue(c.red()); spR->setValue(c.red());
            slG->setValue(c.green()); spG->setValue(c.green());
            slB->setValue(c.blue()); spB->setValue(c.blue());
            hexEdit->setText('#' + c.name(QColor::HexRgb).mid(1));
            setPreview(c);
            if (currentOut) *currentOut = c;
            // 同步模式页面的指示器
            area->setHS(c.hue() < 0 ? 0 : c.hue(), c.saturation());
            spH->setValue(c.hue() < 0 ? 0 : c.hue());
            spS->setValue(c.saturation());
            spV->setValue(c.value());
            areaHSL->setHS(c.hue() < 0 ? 0 : c.hue(), c.hslSaturation());
            spHHSL->setValue(c.hue() < 0 ? 0 : c.hue());
            spSHSL->setValue(c.hslSaturation());
            spL->setValue(c.lightness());
        });
    }

    // 自定义颜色收藏
    static QVector<QColor> s_customColors;
    auto* customRow = new QHBoxLayout();
    QPushButton* btnAddCustom = new QPushButton(QObject::tr("加入自定义"), body);
    customRow->addWidget(btnAddCustom);
    for (const QColor& cc : s_customColors) {
        QPushButton* b = new QPushButton(" ", body);
        b->setFixedSize(20, 20);
        QPixmap pix(18, 18);
        pix.fill(cc);
        QPainter p(&pix);
        p.setPen(QColor(0x999999));
        p.drawRect(0, 0, 17, 17);
        b->setIcon(QIcon(pix));
        b->setIconSize(QSize(18, 18));
        b->setFlat(true);
        QObject::connect(b, &QPushButton::clicked, [&, cc]() {
            slR->setValue(cc.red());
            spR->setValue(cc.red());
            slG->setValue(cc.green());
            spG->setValue(cc.green());
            slB->setValue(cc.blue());
            spB->setValue(cc.blue());
            updateHexFromRGB();
        });
        customRow->addWidget(b);
    }
    customRow->addStretch();
    v->addLayout(customRow);
    QObject::connect(btnAddCustom, &QPushButton::clicked, [&, currentOut, customRow]() {
        if (!currentOut)
            return;
        QColor c = *currentOut;
        if (!c.isValid())
            return;
        s_customColors.prepend(c);
        if (s_customColors.size() > 16)
            s_customColors.removeLast();
        QPushButton* b = new QPushButton(" ", body);
        b->setFixedSize(20, 20);
        QPixmap pix(18, 18);
        pix.fill(c);
        QPainter p(&pix);
        p.setPen(QColor(0x999999));
        p.drawRect(0, 0, 17, 17);
        b->setIcon(QIcon(pix));
        b->setIconSize(QSize(18, 18));
        b->setFlat(true);
        QObject::connect(b, &QPushButton::clicked, [&, c]() {
            slR->setValue(c.red());
            spR->setValue(c.red());
            slG->setValue(c.green());
            spG->setValue(c.green());
            slB->setValue(c.blue());
            spB->setValue(c.blue());
            updateHexFromRGB();
        });
        int insertPos = customRow->count() > 0 ? customRow->count() - 1 : 0; // before stretch
        customRow->insertWidget(insertPos, b);
    });
    body->setLayout(v);

    setPreview(initial);
    if (currentOut)
        *currentOut = initial;
    updateHexFromRGB();
    return body;
}

} // namespace QtAdvancedUI
