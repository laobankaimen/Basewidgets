#include <QtAdvancedUI/Dialogs/DialogUtils.h>
#include <QtAdvancedUI/Core/FramelessDialog.h>
#include <QtAdvancedUI/Core/StyleManagerAdapter.h>

#include "ColorPicker.h"
#include "FontPicker.h"
#include "FilePicker.h"

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QColorDialog>
#include <QFontDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QIcon>
#include <QApplication>
#include <QAbstractButton>
#include <QGroupBox>
#include <QEvent>
#include <QCheckBox>
#include <QFileSystemModel>
#include <QTreeView>
#include <QComboBox>
#include <QRegularExpression>
#include <QFontDatabase>
#include <QDir>
#include <QFrame>
#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <QListWidget>
#include <QToolButton>
#include <QSplitter>

using namespace acss;

namespace QtAdvancedUI {

// 颜色选择区域已拆分至 ColorPicker.cpp

static QWidget* makeContent(QWidget* parent, QWidget* body, QDialogButtonBox* box)
{
    QWidget* content = new QWidget(parent);
    auto* v = new QVBoxLayout(content);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);
    v->addWidget(body);
    if (box) v->addWidget(box);
    content->setLayout(v);
    return content;
}

// 统一风格的圆形彩色徽标图标
static QPixmap makeBadgeIcon(const QColor& circleColor, const QString& shape, const QColor& shapeColor)
{
    QPixmap pm(40, 40);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(circleColor);
    p.drawEllipse(QRectF(2, 2, 36, 36));
    p.setBrush(shapeColor);
    p.setPen(QPen(shapeColor, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (shape == "check") {
        p.drawLine(QPointF(13, 22), QPointF(18, 28));
        p.drawLine(QPointF(18, 28), QPointF(28, 14));
    } else if (shape == "cross") {
        p.drawLine(QPointF(13, 13), QPointF(27, 27));
        p.drawLine(QPointF(27, 13), QPointF(13, 27));
    } else if (shape == "info") {
        p.setPen(QPen(shapeColor, 6));
        p.drawPoint(QPointF(20, 12));
        p.setPen(QPen(shapeColor, 4));
        p.drawLine(QPointF(20, 18), QPointF(20, 28));
    } else if (shape == "question") {
        // 使用适中粗细、较小字号的“?”并留出边距，避免溢出圆圈
        p.setBrush(Qt::NoBrush);
        p.setPen(shapeColor);
        QFont f;
        f.setBold(false);
        f.setPointSize(18);
        p.setFont(f);
        p.drawText(QRectF(4, 4, 32, 32), Qt::AlignCenter, "?");
    }
    return pm;
}

// 去除 Automoc 依赖（ColorArea 已迁移到独立文件）

// 自定义颜色选择器主体（RGB 滑条 + Hex 输入 + 预览）
#if 0
static QWidget* makeColorPickerBody(QWidget* parent, const QColor& initial, QColor* currentOut)
{
    QWidget* body = new QWidget(parent);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);

    QLabel* preview = new QLabel(body);
    preview->setFixedSize(64, 32);
    preview->setFrameShape(QFrame::Box);

    auto setPreview = [&](const QColor& c){
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

    auto addRow = [&](int row, const QString& name, int value, QSlider** outSl, QSpinBox** outSp){
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
        *outSl = sl; *outSp = sp;
    };

    QSlider *slR, *slG, *slB; QSpinBox *spR, *spG, *spB;
    addRow(0, QObject::tr("红"), initial.red(), &slR, &spR);
    addRow(1, QObject::tr("绿"), initial.green(), &slG, &spG);
    addRow(2, QObject::tr("蓝"), initial.blue(), &slB, &spB);

    QLineEdit* hexEdit = new QLineEdit(form);
    hexEdit->setPlaceholderText("#RRGGBB");
    auto updateHexFromRGB = [&](){
        QColor c(spR->value(), spG->value(), spB->value());
        hexEdit->setText('#' + c.name(QColor::HexRgb).mid(1));
        setPreview(c);
        if (currentOut) *currentOut = c;
    };
    auto updateRGBFromHex = [&](){
        QString s = hexEdit->text().trimmed();
        if (s.startsWith('#')) s = s.mid(1);
        if (s.size() == 6) {
            QColor c('#' + s);
            if (c.isValid()) {
                slR->setValue(c.red()); spR->setValue(c.red());
                slG->setValue(c.green()); spG->setValue(c.green());
                slB->setValue(c.blue()); spB->setValue(c.blue());
                setPreview(c);
                if (currentOut) *currentOut = c;
            }
        }
    };
    QObject::connect(spR, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ updateHexFromRGB(); });
    QObject::connect(spG, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ updateHexFromRGB(); });
    QObject::connect(spB, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ updateHexFromRGB(); });
    QObject::connect(hexEdit, &QLineEdit::textChanged, [&](const QString&){ updateRGBFromHex(); });

    grid->addWidget(new QLabel(QObject::tr("十六进制"), form), 3, 0);
    grid->addWidget(hexEdit, 3, 1, 1, 2);
    form->setLayout(grid);

    v->addWidget(preview, 0, Qt::AlignLeft);
    v->addWidget(form);
    // 自定义色板
    {
        auto* paletteRow = new QHBoxLayout();
        QList<QColor> paletteColors = {
            QColor("#000000"), QColor("#FFFFFF"), QColor("#FF0000"), QColor("#00FF00"), QColor("#0000FF"),
            QColor("#FFFF00"), QColor("#FF00FF"), QColor("#00FFFF"), QColor("#FFA500"), QColor("#808080")
        };
        for (const QColor& pc : paletteColors) {
            QPushButton* b = new QPushButton(" ", body);
            b->setFixedSize(20, 20);
            QString css = QString("background:%1;border:1px solid #999;").arg(pc.name());
            b->setStyleSheet(css);
            QObject::connect(b, &QPushButton::clicked, [&, pc]{
                slR->setValue(pc.red()); spR->setValue(pc.red());
                slG->setValue(pc.green()); spG->setValue(pc.green());
                slB->setValue(pc.blue()); spB->setValue(pc.blue());
                updateHexFromRGB();
            });
            paletteRow->addWidget(b);
        }
        paletteRow->addStretch();
        v->addLayout(paletteRow);
    }
    // 扩展：交互式 HSV 区域 + 亮度/Alpha 滑块 + 模式切换
    {
        auto* modeRow = new QHBoxLayout();
        QLabel* modeLbl = new QLabel(QObject::tr("使用 HSV"), body);
        QCheckBox* useHSV = new QCheckBox(body);
        modeRow->addWidget(modeLbl);
        modeRow->addWidget(useHSV);
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
        auto addHSVRow = [&](int row, const QString& name, int min, int max, int value, QSlider** outSl, QSpinBox** outSp){
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
            *outSl = sl; *outSp = sp;
        };
        int initH = initial.hue(); if (initH < 0) initH = 0;
        int initS = initial.saturation();
        QSlider *slH, *slS; QSpinBox *spH, *spS; QSpinBox *spV;
        addHSVRow(1, QObject::tr("色相"), 0, 359, initH, &slH, &spH);
        addHSVRow(2, QObject::tr("饱和度"), 0, 255, initS, &slS, &spS);
        // 明度滑条已在右侧，值同步到 SpinBox
        spV = new QSpinBox(formHSV); spV->setRange(0, 255); spV->setValue(slV->value());
        gridHSV->addWidget(new QLabel(QObject::tr("明度"), formHSV), 3, 0);
        gridHSV->addWidget(new QWidget(formHSV), 3, 1);
        gridHSV->addWidget(spV, 3, 2);

        // Alpha 通道（可选）
        QCheckBox* useAlpha = new QCheckBox(QObject::tr("使用 Alpha"), formHSV);
        QSlider* slA = new QSlider(Qt::Horizontal, formHSV);
        slA->setRange(0, 255); slA->setValue(initial.alpha());
        QSpinBox* spA = new QSpinBox(formHSV); spA->setRange(0, 255); spA->setValue(initial.alpha());
        QObject::connect(slA, &QSlider::valueChanged, spA, &QSpinBox::setValue);
        auto* alphaRow = new QHBoxLayout(); alphaRow->addWidget(useAlpha); alphaRow->addWidget(slA, 1); alphaRow->addWidget(spA);
        gridHSV->addLayout(alphaRow, 4, 0, 1, 3);

        auto applyHSV = [&](){
            QColor c; c.setHsv(spH->value(), spS->value(), spV->value());
            if (useAlpha->isChecked()) c.setAlpha(spA->value());
            slR->setValue(c.red()); spR->setValue(c.red());
            slG->setValue(c.green()); spG->setValue(c.green());
            slB->setValue(c.blue()); spB->setValue(c.blue());
            hexEdit->setText('#' + c.name(QColor::HexRgb).mid(1));
            setPreview(c);
            if (currentOut) *currentOut = c;
        };
        area->setHS(initH, initS);
        QObject::connect(spH, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ area->setHS(spH->value(), spS->value()); applyHSV(); });
        QObject::connect(spS, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ area->setHS(spH->value(), spS->value()); applyHSV(); });
        QObject::connect(spV, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ applyHSV(); });
        QObject::connect(slV, &QSlider::valueChanged, spV, &QSpinBox::setValue);
        QObject::connect(area, &ColorArea::hsChanged, [&](int h, int s){ spH->setValue(h); spS->setValue(s); applyHSV(); });

        formHSV->setLayout(gridHSV);
        v->addWidget(formHSV);
        formHSV->setVisible(false);
        QObject::connect(useHSV, &QCheckBox::toggled, [form, formHSV](bool on){ form->setVisible(!on); formHSV->setVisible(on); });
    }

    // 自定义颜色收藏
    static QVector<QColor> s_customColors;
    auto* customRow = new QHBoxLayout();
    QPushButton* btnAddCustom = new QPushButton(QObject::tr("加入自定义"), body);
    customRow->addWidget(btnAddCustom);
    for (const QColor& cc : s_customColors) {
        QPushButton* b = new QPushButton(" ", body);
        b->setFixedSize(20, 20); b->setStyleSheet(QString("background:%1;border:1px solid #999;").arg(cc.name()));
        QObject::connect(b, &QPushButton::clicked, [&, cc]{
            slR->setValue(cc.red()); spR->setValue(cc.red());
            slG->setValue(cc.green()); spG->setValue(cc.green());
            slB->setValue(cc.blue()); spB->setValue(cc.blue());
            updateHexFromRGB();
        });
        customRow->addWidget(b);
    }
    customRow->addStretch();
    v->addLayout(customRow);
    QObject::connect(btnAddCustom, &QPushButton::clicked, [&, currentOut, customRow]{
        if (!currentOut) return;
        QColor c = *currentOut; if (!c.isValid()) return;
        s_customColors.prepend(c);
        if (s_customColors.size() > 16) s_customColors.removeLast();
        QPushButton* b = new QPushButton(" ", body);
        b->setFixedSize(20, 20);
        b->setStyleSheet(QString("background:%1;border:1px solid #999;").arg(c.name()));
        QObject::connect(b, &QPushButton::clicked, [&, c]{
            slR->setValue(c.red()); spR->setValue(c.red());
            slG->setValue(c.green()); spG->setValue(c.green());
            slB->setValue(c.blue()); spB->setValue(c.blue());
            updateHexFromRGB();
        });
        int insertPos = customRow->count() > 0 ? customRow->count() - 1 : 0; // before stretch
        customRow->insertWidget(insertPos, b);
    });
    body->setLayout(v);

    setPreview(initial);
    if (currentOut) *currentOut = initial;
    updateHexFromRGB();
    return body;
}

// 自定义字体选择器主体（Family/Style/Size + 预览）
#endif
#if 0
static QWidget* makeFontPickerBody(QWidget* parent, QFont current, QFont* currentOut)
{
    QWidget* body = new QWidget(parent);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);

    QLabel* preview = new QLabel(QObject::tr("示例文本 AaBbCc 123"), body);
    preview->setMinimumHeight(40);

    auto* row = new QHBoxLayout();
    QComboBox* familyBox = new QComboBox(body);
    QComboBox* styleBox = new QComboBox(body);
    QSpinBox* sizeSpin = new QSpinBox(body);
    sizeSpin->setRange(6, 72);
    QCheckBox* boldCheck = new QCheckBox(QObject::tr("粗体"), body);
    QCheckBox* italicCheck = new QCheckBox(QObject::tr("斜体"), body);
    QCheckBox* underlineCheck = new QCheckBox(QObject::tr("下划线"), body);
    QCheckBox* strikeCheck = new QCheckBox(QObject::tr("删除线"), body);
    QFontDatabase fdb;
    familyBox->addItems(fdb.families());
    int famIdx = familyBox->findText(current.family());
    if (famIdx >= 0) familyBox->setCurrentIndex(famIdx);
    auto fillStyles = [&](){
        styleBox->clear();
        const QString fam = familyBox->currentText();
        styleBox->addItems(fdb.styles(fam));
        int stIdx = styleBox->findText(fdb.styleString(current));
        if (stIdx >= 0) styleBox->setCurrentIndex(stIdx);
    };
    fillStyles();
    sizeSpin->setValue(current.pointSize() > 0 ? current.pointSize() : 12);

    auto updatePreview = [&](){
        const QString fam = familyBox->currentText();
        const QString sty = styleBox->currentText();
        QFont f = fdb.font(fam, sty, sizeSpin->value());
        f.setBold(boldCheck->isChecked());
        f.setItalic(italicCheck->isChecked());
        f.setUnderline(underlineCheck->isChecked());
        f.setStrikeOut(strikeCheck->isChecked());
        preview->setFont(f);
        if (currentOut) *currentOut = f;
    };
    QObject::connect(familyBox, &QComboBox::currentTextChanged, [&](const QString&){ fillStyles(); updatePreview(); });
    QObject::connect(styleBox, &QComboBox::currentTextChanged, [&](const QString&){ updatePreview(); });
    QObject::connect(sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ updatePreview(); });

    row->addWidget(new QLabel(QObject::tr("字体"), body));
    row->addWidget(familyBox, 1);
    row->addWidget(new QLabel(QObject::tr("样式"), body));
    row->addWidget(styleBox, 1);
    row->addWidget(new QLabel(QObject::tr("大小"), body));
    row->addWidget(sizeSpin);
    row->addWidget(boldCheck);
    row->addWidget(italicCheck);
    row->addWidget(underlineCheck);
    row->addWidget(strikeCheck);

    v->addLayout(row);
    v->addWidget(preview);
    body->setLayout(v);
    QObject::connect(boldCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(italicCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(underlineCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(strikeCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    updatePreview();
    return body;
}
#endif

// 自定义文件选择器主体（文件系统视图 + 名称输入 + 过滤）
// 自定义文件选择器主体已弃用，改用 QFileDialog
#if 0
static QWidget* makeFilePickerBody(QWidget* parent,
                                    const QString& initialDir,
                                    const QString& filter,
                                    QString* selectedPathOut,
                                    bool directoryOnly,
                                    bool multiSelect,
                                    bool saveMode,
                                    QTreeView** outView = nullptr,
                                    QFileSystemModel** outModel = nullptr,
                                    QLineEdit** outEdit = nullptr)
{
    QWidget* body = new QWidget(parent);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);

    QFileSystemModel* model = new QFileSystemModel(body);
    QString rootPath = initialDir.isEmpty() ? QDir::homePath() : initialDir;
    model->setRootPath(rootPath);
    model->setFilter(directoryOnly ? (QDir::AllDirs | QDir::NoDotAndDotDot) : (QDir::AllEntries | QDir::NoDotAndDotDot));

    // 导航工具栏
    auto* navRow = new QHBoxLayout();
    QToolButton* btnBack = new QToolButton(body); btnBack->setText("←");
    QToolButton* btnForward = new QToolButton(body); btnForward->setText("→");
    QToolButton* btnUp = new QToolButton(body); btnUp->setText("↑");
    QToolButton* btnHome = new QToolButton(body); btnHome->setText(QObject::tr("家"));
    QLineEdit* addrEdit = new QLineEdit(rootPath, body);
    QCheckBox* showHidden = new QCheckBox(QObject::tr("显示隐藏"), body);
    navRow->addWidget(btnBack); navRow->addWidget(btnForward); navRow->addWidget(btnUp); navRow->addWidget(btnHome);
    navRow->addWidget(new QLabel(QObject::tr("位置:"), body)); navRow->addWidget(addrEdit, 1);
    navRow->addWidget(showHidden);
    v->addLayout(navRow);

    // 主区：侧边栏 + 视图 + 预览
    auto* contentRow = new QHBoxLayout();
    QListWidget* places = new QListWidget(body);
    places->setFixedWidth(140);
    auto addPlace = [&](const QString& name, const QString& path){ QListWidgetItem* it = new QListWidgetItem(name, places); it->setData(Qt::UserRole, path); };
    addPlace(QObject::tr("桌面"), QDir::homePath() + "/Desktop");
    addPlace(QObject::tr("文档"), QDir::homePath() + "/Documents");
    addPlace(QObject::tr("下载"), QDir::homePath() + "/Downloads");
    addPlace(QObject::tr("家目录"), QDir::homePath());

    QTreeView* view = new QTreeView(body);
    view->setModel(model);
    view->setRootIndex(model->index(model->rootPath()));
    view->setSelectionMode(multiSelect ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
    view->setHeaderHidden(false);
    view->setSortingEnabled(true);
    view->sortByColumn(0, Qt::AscendingOrder);

    QWidget* previewPane = new QWidget(body);
    auto* pv = new QVBoxLayout(previewPane);
    pv->setContentsMargins(6,6,6,6); pv->setSpacing(6);
    QLabel* previewIcon = new QLabel(previewPane);
    previewIcon->setFixedSize(160, 120);
    previewIcon->setFrameShape(QFrame::Box);
    QLabel* previewInfo = new QLabel(previewPane);
    previewInfo->setWordWrap(true);
    pv->addWidget(previewIcon, 0, Qt::AlignTop);
    pv->addWidget(previewInfo);
    previewPane->setLayout(pv);

    contentRow->addWidget(places);
    contentRow->addWidget(view, 1);
    contentRow->addWidget(previewPane);
    v->addLayout(contentRow);

    // 操作工具行：新建文件夹/删除
    auto* actionsRow = new QHBoxLayout();
    QPushButton* btnNewDir = new QPushButton(QObject::tr("新建文件夹"), body);
    QPushButton* btnDelete = new QPushButton(QObject::tr("删除"), body);
    actionsRow->addWidget(btnNewDir);
    actionsRow->addWidget(btnDelete);
    actionsRow->addStretch();
    v->addLayout(actionsRow);

    QLineEdit* nameEdit = new QLineEdit(body);
    nameEdit->setPlaceholderText(saveMode ? QObject::tr("文件名") : QObject::tr("路径"));

    QComboBox* filterBox = new QComboBox(body);
    if (!filter.isEmpty()) {
        const auto parts = filter.split(";;");
        for (const QString& part : parts) filterBox->addItem(part);
    }
    // 根据过滤器自动筛选视图文件
    auto patternsFromFilterText = [](const QString& text){
        int l = text.indexOf('('), r = text.indexOf(')');
        QStringList pats;
        if (l>=0 && r>l) {
            QString inside = text.mid(l+1, r-l-1);
            for (const QString& seg : inside.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)) {
                if (seg.startsWith("*")) pats << seg;
            }
        }
        return pats;
    };
    model->setNameFilterDisables(false);
    QObject::connect(filterBox, &QComboBox::currentTextChanged, [&](const QString& t){ model->setNameFilters(patternsFromFilterText(t)); });
    if (!filterBox->currentText().isEmpty()) model->setNameFilters(patternsFromFilterText(filterBox->currentText()));

    // 选择变化更新
    QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection& sel, const QItemSelection&){
        const QModelIndexList idxs = sel.indexes();
        if (!idxs.isEmpty()) {
            const QString path = model->filePath(idxs.first());
            nameEdit->setText(path);
            QFileInfo fi(path);
            previewInfo->setText(QString("%1\n%2字节\n%3").arg(fi.fileName()).arg(fi.size()).arg(fi.absolutePath()));
            if (fi.isFile()) {
                const QString ext = fi.suffix().toLower();
                if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "gif") {
                    QPixmap pm(path);
                    previewIcon->setPixmap(pm.scaled(previewIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    previewIcon->setPixmap(QPixmap());
                }
            } else {
                previewIcon->setPixmap(QPixmap());
            }
        }
    });

    // 显示隐藏文件
    QObject::connect(showHidden, &QCheckBox::toggled, [=](bool on){
        QDir::Filters f = model->filter();
        if (on) f |= QDir::Hidden; else f &= ~QDir::Hidden;
        model->setFilter(f);
    });

    // 导航：地址栏/按钮
    auto setRoot = [&](const QString& p){ QString ap = QDir(p).absolutePath(); addrEdit->setText(ap); model->setRootPath(ap); view->setRootIndex(model->index(ap)); };
    QObject::connect(addrEdit, &QLineEdit::returnPressed, [=](){ setRoot(addrEdit->text()); });
    QObject::connect(places, &QListWidget::itemClicked, [=](QListWidgetItem* it){ setRoot(it->data(Qt::UserRole).toString()); });
    QObject::connect(btnHome, &QToolButton::clicked, [=](){ setRoot(QDir::homePath()); });
    QObject::connect(btnUp, &QToolButton::clicked, [=](){ QDir d(model->rootPath()); d.cdUp(); setRoot(d.absolutePath()); });

    // 简易历史（后退/前进）
    static QVector<QString> s_hist; static int s_index = -1; auto pushHist = [&](const QString& p){ if (s_index<0 || s_hist.value(s_index) != p) { s_hist = s_hist.mid(0, s_index+1); s_hist.append(p); s_index = s_hist.size()-1; } };
    pushHist(rootPath);
    QObject::connect(addrEdit, &QLineEdit::textChanged, [&](const QString& p){ pushHist(QDir(p).absolutePath()); });
    QObject::connect(btnBack, &QToolButton::clicked, [&](){ if (s_index > 0) { s_index--; setRoot(s_hist[s_index]); } });
    QObject::connect(btnForward, &QToolButton::clicked, [&](){ if (s_index+1 < s_hist.size()) { s_index++; setRoot(s_hist[s_index]); } });

    // 新建/删除
    QObject::connect(btnNewDir, &QPushButton::clicked, [model, view]() {
        QModelIndex idx = view->currentIndex();
        QModelIndex parentIdx;
        if (idx.isValid() && model->isDir(idx)) parentIdx = idx;
        else if (idx.isValid()) parentIdx = idx.parent();
        else parentIdx = model->index(model->rootPath());
        QString baseName = QObject::tr("新建文件夹");
        QString name = baseName;
        int n = 1;
        QDir dir(model->filePath(parentIdx));
        while (dir.exists(name)) name = baseName + QString::number(n++);
        model->mkdir(parentIdx, name);
    });
    QObject::connect(btnDelete, &QPushButton::clicked, [model, view]() {
        const QModelIndexList idxs = view->selectionModel()->selectedIndexes();
        QSet<QString> paths;
        for (const auto& idx : idxs) if (idx.column() == 0) paths.insert(model->filePath(idx));
        for (const QString& p : paths) {
            QFileInfo info(p);
            if (info.isDir()) { QDir d(p); d.removeRecursively(); }
            else { QFile::remove(p); }
        }
    });

    v->addWidget(new QLabel(saveMode ? QObject::tr("文件名：") : QObject::tr("路径："), body));
    v->addWidget(nameEdit);
    if (!filterBox->currentText().isEmpty()) {
        v->addWidget(new QLabel(QObject::tr("文件类型："), body));
        v->addWidget(filterBox);
    }
    body->setLayout(v);

    if (outView) *outView = view;
    if (outModel) *outModel = model;
    if (outEdit) *outEdit = nameEdit;
    QObject::connect(nameEdit, &QLineEdit::textChanged, [=](const QString& t){ if (selectedPathOut) *selectedPathOut = t; });
    if (selectedPathOut) *selectedPathOut = nameEdit->text();
    return body;
}
#endif

// 额外本地化 QColorDialog 内部英文标签与功能按钮
static void localizeColorDialogInternals(QColorDialog* cd)
{
    if (!cd) return;

    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 按钮（非底部 OK/Cancel）：PICK SCREEN COLOR / ADD TO CUSTOM COLORS
    const auto btns = cd->findChildren<QAbstractButton*>();
    for (auto* b : btns) {
        const QString t = normalize(b->text());
        if (t.contains("PICK SCREEN COLOR", Qt::CaseInsensitive) || t.contains("Pick Screen Color", Qt::CaseInsensitive)) {
            b->setText(QObject::tr("取屏幕颜色"));
        } else if (t.contains("ADD TO CUSTOM COLORS", Qt::CaseInsensitive) || t.contains("Add to Custom Colors", Qt::CaseInsensitive)) {
            b->setText(QObject::tr("添加到自定义颜色"));
        }
    }

    // 组标题或标签：Basic colors / Custom colors
    const auto groups = cd->findChildren<QGroupBox*>();
    for (auto* g : groups) {
        const QString t = normalize(g->title());
        if (t.contains("Basic colors", Qt::CaseInsensitive)) g->setTitle(QObject::tr("基本颜色"));
        else if (t.contains("Custom colors", Qt::CaseInsensitive)) g->setTitle(QObject::tr("自定义颜色"));
    }

    const auto labels = cd->findChildren<QLabel*>();
    for (auto* l : labels) {
        const QString t = normalize(l->text());
        if (t.startsWith("Hue", Qt::CaseInsensitive)) l->setText(QObject::tr("色相"));
        else if (t.startsWith("Sat", Qt::CaseInsensitive)) l->setText(QObject::tr("饱和度"));
        else if (t.startsWith("Val", Qt::CaseInsensitive)) l->setText(QObject::tr("明度"));
        else if (t.startsWith("Red", Qt::CaseInsensitive)) l->setText(QObject::tr("红"));
        else if (t.startsWith("Green", Qt::CaseInsensitive)) l->setText(QObject::tr("绿"));
        else if (t.startsWith("Blue", Qt::CaseInsensitive)) l->setText(QObject::tr("蓝"));
        else if (t.startsWith("HTML", Qt::CaseInsensitive)) l->setText(QObject::tr("HTML"));
        else if (t.contains("Basic colors", Qt::CaseInsensitive)) l->setText(QObject::tr("基本颜色"));
        else if (t.contains("Custom colors", Qt::CaseInsensitive)) l->setText(QObject::tr("自定义颜色"));
    }
}

// 事件过滤器：在对话框显示/布局后再次本地化，确保子控件已创建
class ColorDialogLocalizer : public QObject {
public:
    explicit ColorDialogLocalizer(QColorDialog* d) : QObject(d), cd(d) {}
protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
        if (obj == cd) {
            switch (e->type()) {
            case QEvent::Show:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::LanguageChange:
                localizeColorDialogInternals(cd);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, e);
    }
private:
    QColorDialog* cd = nullptr;
};

// 额外本地化 QFontDialog 内部英文标签与控件
static void localizeFontDialogInternals(QFontDialog* fd)
{
    if (!fd) return;

    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 组标题或标签：Font / Font style / Style / Size / Writing System / Sample / Effects
    const auto groups = fd->findChildren<QGroupBox*>();
    for (auto* g : groups) {
        const QString t = normalize(g->title());
        const QString tn = t.toLower();
        if (tn.contains("font style")) g->setTitle(QObject::tr("字体样式"));
        else if (tn.contains("font")) g->setTitle(QObject::tr("字体"));
        else if (tn.contains("style")) g->setTitle(QObject::tr("样式"));
        else if (tn.contains("size") || tn.contains("point size")) g->setTitle(QObject::tr("大小"));
        else if (tn.contains("writing system")) g->setTitle(QObject::tr("书写系统"));
        else if (tn.contains("sample") || tn.contains("preview")) g->setTitle(QObject::tr("示例"));
        else if (tn.contains("effects")) g->setTitle(QObject::tr("效果"));
    }

    const auto labels = fd->findChildren<QLabel*>();
    for (auto* l : labels) {
        const QString t = normalize(l->text());
        const QString tn = t.toLower();
        // 注意顺序：先匹配 "font style"，避免被 "font" 误伤
        if (tn.startsWith("font style")) l->setText(QObject::tr("字体样式"));
        else if (tn.startsWith("font")) l->setText(QObject::tr("字体"));
        else if (tn.startsWith("family")) l->setText(QObject::tr("字体"));
        else if (tn.startsWith("style")) l->setText(QObject::tr("样式"));
        else if (tn.startsWith("point size") || tn.startsWith("size")) l->setText(QObject::tr("大小"));
        else if (tn.startsWith("writing system")) l->setText(QObject::tr("书写系统"));
        else if (tn.startsWith("effects")) l->setText(QObject::tr("效果"));
        else if (tn.startsWith("sample") || tn.startsWith("preview")) l->setText(QObject::tr("示例"));
    }

    const auto checks = fd->findChildren<QCheckBox*>();
    for (auto* c : checks) {
        const QString t = normalize(c->text());
        if (t.contains("Bold", Qt::CaseInsensitive)) c->setText(QObject::tr("粗体"));
        else if (t.contains("Italic", Qt::CaseInsensitive)) c->setText(QObject::tr("斜体"));
        else if (t.contains("Underline", Qt::CaseInsensitive) || t.contains("Underlined", Qt::CaseInsensitive)) c->setText(QObject::tr("下划线"));
        else if (t.contains("Strikeout", Qt::CaseInsensitive) || t.contains("Strikethrough", Qt::CaseInsensitive)) c->setText(QObject::tr("删除线"));
        else if (t.contains("Kerning", Qt::CaseInsensitive)) c->setText(QObject::tr("字距"));
    }
}

// 事件过滤器：在字体对话框显示/布局后再次本地化
class FontDialogLocalizer : public QObject {
public:
    explicit FontDialogLocalizer(QFontDialog* d) : QObject(d), fd(d) {}
protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
        if (obj == fd) {
            switch (e->type()) {
            case QEvent::Show:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::LanguageChange:
                localizeFontDialogInternals(fd);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, e);
    }
private:
    QFontDialog* fd = nullptr;
};

// ===== QFileDialog 中文本地化：定义提前放在使用前 =====
static void localizeFileDialogInternals(QFileDialog* fd)
{
    if (!fd) return;
    auto normalize = [](QString s) {
        s.replace('&', "");
        s.replace('_', "");
        return s.trimmed();
    };

    // 底部按钮（QFileDialog 内部自带）：本库外部统一提供按钮，为避免重复，这里仅本地化文本后隐藏内部按钮栏
    if (auto* box = fd->findChild<QDialogButtonBox*>()) {
        if (auto* b = box->button(QDialogButtonBox::Open)) b->setText(QObject::tr("打开"));
        if (auto* b = box->button(QDialogButtonBox::Save)) b->setText(QObject::tr("保存"));
        if (auto* b = box->button(QDialogButtonBox::Ok)) b->setText(QObject::tr("确定"));
        if (auto* b = box->button(QDialogButtonBox::Apply)) b->setText(QObject::tr("应用"));
        if (auto* b = box->button(QDialogButtonBox::Cancel)) b->setText(QObject::tr("取消"));
        if (auto* b = box->button(QDialogButtonBox::Close)) b->setText(QObject::tr("关闭"));
        // 隐藏内部按钮栏，使用外部统一的 QDialogButtonBox（确定/取消）
        box->setVisible(false);
    }

    // 顶部和侧边工具按钮：Back/Forward/Parent/New Folder/Delete/List/Details
    const auto btns = fd->findChildren<QAbstractButton*>();
    for (auto* b : btns) {
        const QString t = normalize(b->text());
        if (t.contains("BACK", Qt::CaseInsensitive)) b->setText(QObject::tr("后退"));
        else if (t.contains("FORWARD", Qt::CaseInsensitive)) b->setText(QObject::tr("前进"));
        else if (t.contains("PARENT", Qt::CaseInsensitive)) b->setText(QObject::tr("上级文件夹"));
        else if (t.contains("NEW FOLDER", Qt::CaseInsensitive)) b->setText(QObject::tr("新建文件夹"));
        else if (t.contains("DELETE", Qt::CaseInsensitive)) b->setText(QObject::tr("删除"));
        else if (t.contains("LIST", Qt::CaseInsensitive)) b->setText(QObject::tr("列表视图"));
        else if (t.contains("DETAIL", Qt::CaseInsensitive)) b->setText(QObject::tr("详细信息"));
    }

    // 标签：Look in / File name / Files of type
    const auto labels = fd->findChildren<QLabel*>();
    for (auto* l : labels) {
        const QString t = normalize(l->text());
        if (t.startsWith("Look in", Qt::CaseInsensitive)) l->setText(QObject::tr("查看位置："));
        else if (t.startsWith("File name", Qt::CaseInsensitive)) l->setText(QObject::tr("文件名："));
        else if (t.contains("Files of type", Qt::CaseInsensitive) || t.contains("File type", Qt::CaseInsensitive)) l->setText(QObject::tr("文件类型："));
    }
}

class FileDialogLocalizer : public QObject {
public:
    explicit FileDialogLocalizer(QFileDialog* d) : QObject(d), fd(d) {}
protected:
    bool eventFilter(QObject* obj, QEvent* e) override {
        if (obj == fd) {
            switch (e->type()) {
            case QEvent::Show:
            case QEvent::Polish:
            case QEvent::LayoutRequest:
            case QEvent::LanguageChange:
                localizeFileDialogInternals(fd);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(obj, e);
    }
private:
    QFileDialog* fd = nullptr;
};

QString DialogUtils::openFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("打开文件") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setFileMode(QFileDialog::ExistingFile);
    fd->setViewMode(QFileDialog::Detail);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget); // 作为主体嵌入
    if (!filter.isEmpty()) fd->setNameFilters(filter.split(";;", Qt::SkipEmptyParts));
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles().value(0) : QString();
}

QStringList DialogUtils::openFiles(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setViewMode(QFileDialog::Detail);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);
    if (!filter.isEmpty()) fd->setNameFilters(filter.split(";;", Qt::SkipEmptyParts));
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles() : QStringList{};
}

QString DialogUtils::saveFile(QWidget* parent, const QString& title, const QString& initialDir, const QString& filter)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("保存文件") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setAcceptMode(QFileDialog::AcceptSave);
    fd->setFileMode(QFileDialog::AnyFile);
    fd->setViewMode(QFileDialog::Detail);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);
    if (!filter.isEmpty()) fd->setNameFilters(filter.split(";;", Qt::SkipEmptyParts));
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles().value(0) : QString();
}

QString DialogUtils::openDirectory(QWidget* parent, const QString& title, const QString& initialDir)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择文件夹") : title);
    QFileDialog* fd = new QFileDialog(&dlg, QString(), initialDir.isEmpty() ? QDir::homePath() : initialDir);
    fd->setFileMode(QFileDialog::Directory);
    fd->setOption(QFileDialog::ShowDirsOnly, true);
    fd->setOption(QFileDialog::DontUseNativeDialog, true);
    fd->setWindowFlags(Qt::Widget);
    fd->installEventFilter(new FileDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    return dlg.exec() == QDialog::Accepted ? fd->selectedFiles().value(0) : QString();
}

QColor DialogUtils::pickColor(QWidget* parent, const QColor& initial, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择颜色") : title);
    QColorDialog* cd = new QColorDialog(initial, &dlg);
    cd->setOption(QColorDialog::ShowAlphaChannel, true);
    cd->setOption(QColorDialog::DontUseNativeDialog, true);
    cd->setOption(QColorDialog::NoButtons, true); // 仅保留外部统一按钮
    cd->setWindowFlags(Qt::Widget);
    cd->installEventFilter(new ColorDialogLocalizer(cd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, cd, box));
    return dlg.exec() == QDialog::Accepted ? cd->currentColor() : QColor();
}

QFont DialogUtils::pickFont(QWidget* parent, const QFont& initial, bool* okOut, const QString& title)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title.isEmpty() ? QObject::tr("选择字体") : title);
    QFontDialog* fd = new QFontDialog(initial, &dlg);
    fd->setOption(QFontDialog::DontUseNativeDialog, true);
    fd->setOption(QFontDialog::NoButtons, true); // 仅保留外部统一按钮
    fd->setWindowFlags(Qt::Widget);
    fd->installEventFilter(new FontDialogLocalizer(fd));

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* caBtn = box->button(QDialogButtonBox::Cancel)) caBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    dlg.setContentWidget(makeContent(&dlg, fd, box));
    const bool ok = (dlg.exec() == QDialog::Accepted);
    if (okOut) *okOut = ok;
    return ok ? fd->selectedFont() : QFont();
}

void DialogUtils::messageInfo(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(35, 155, 249), "info", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

void DialogUtils::messageWarning(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(255, 166, 0), "info", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

void DialogUtils::messageError(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(242, 94, 67), "cross", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

void DialogUtils::messageSuccess(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(76, 175, 80), "check", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    dlg.exec();
}

bool DialogUtils::askQuestion(QWidget* parent, const QString& title, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    auto* body = new QWidget(&dlg);
    auto* h = new QHBoxLayout(body);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(10);
    QLabel* iconLabel = new QLabel(body);
    iconLabel->setPixmap(makeBadgeIcon(QColor(35, 155, 249), "question", Qt::white));
    QLabel* textLabel = new QLabel(text, body);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("font-size: 14px;");
    h->addWidget(iconLabel);
    h->addWidget(textLabel, 1);
    body->setLayout(h);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) { okBtn->setText(QObject::tr("确定")); okBtn->setDefault(true); okBtn->setAutoDefault(true); }
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) { cancelBtn->setText(QObject::tr("取消")); }
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    return dlg.exec() == QDialog::Accepted;
}

QString DialogUtils::inputText(QWidget* parent, const QString& title, const QString& label, const QString& text)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    QWidget* body = new QWidget(&dlg);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);
    QLabel* lbl = new QLabel(label, body);
    QLineEdit* edit = new QLineEdit(text, body);
    v->addWidget(lbl);
    v->addWidget(edit);
    body->setLayout(v);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return edit->text();
    return QString();
}

int DialogUtils::inputInt(QWidget* parent, const QString& title, const QString& label, int value, int min, int max, int step)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    QWidget* body = new QWidget(&dlg);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);
    QLabel* lbl = new QLabel(label, body);
    QSpinBox* spin = new QSpinBox(body);
    spin->setRange(min, max);
    spin->setSingleStep(step);
    spin->setValue(value);
    v->addWidget(lbl);
    v->addWidget(spin);
    body->setLayout(v);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
 
    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

double DialogUtils::inputDouble(QWidget* parent, const QString& title, const QString& label, double value, double min, double max, int decimals)
{
    FramelessDialog dlg(parent);
    dlg.setTitle(title);

    QWidget* body = new QWidget(&dlg);
    auto* v = new QVBoxLayout(body);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(8);
    QLabel* lbl = new QLabel(label, body);
    QDoubleSpinBox* spin = new QDoubleSpinBox(body);
    spin->setRange(min, max);
    spin->setDecimals(decimals);
    spin->setValue(value);
    v->addWidget(lbl);
    v->addWidget(spin);
    body->setLayout(v);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    if (auto* okBtn = box->button(QDialogButtonBox::Ok)) okBtn->setText(QObject::tr("确定"));
    if (auto* cancelBtn = box->button(QDialogButtonBox::Cancel)) cancelBtn->setText(QObject::tr("取消"));
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    dlg.setContentWidget(makeContent(&dlg, body, box));
    if (dlg.exec() == QDialog::Accepted) return spin->value();
    return value;
}

} // namespace QtAdvancedUI