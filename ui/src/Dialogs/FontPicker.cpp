#include "FontPicker.h"
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFontDatabase>
 
 namespace QtAdvancedUI {
 
 QWidget* FontPicker::makeFontPickerBody(QWidget* parent, QFont current, QFont* currentOut)
 {
     QWidget* body = new QWidget(parent);
     auto* v = new QVBoxLayout(body);
     v->setContentsMargins(0, 0, 0, 0);
     v->setSpacing(8);
 
     QLabel* preview = new QLabel(QObject::tr("示例文本 AaBbCc 123"), body);
     preview->setMinimumHeight(40);
 
    auto* row = new QHBoxLayout();
    QComboBox* wsBox = new QComboBox(body);
    QComboBox* familyBox = new QComboBox(body);
    QComboBox* styleBox = new QComboBox(body);
    QSpinBox* sizeSpin = new QSpinBox(body);
    sizeSpin->setRange(6, 72);
    QCheckBox* boldCheck = new QCheckBox(QObject::tr("粗体"), body);
    QCheckBox* italicCheck = new QCheckBox(QObject::tr("斜体"), body);
    QCheckBox* underlineCheck = new QCheckBox(QObject::tr("下划线"), body);
    QCheckBox* strikeCheck = new QCheckBox(QObject::tr("删除线"), body);
    QCheckBox* overlineCheck = new QCheckBox(QObject::tr("上划线"), body);
    QCheckBox* kerningCheck = new QCheckBox(QObject::tr("启用字距调整(Kerning)"), body);
    QComboBox* capitalBox = new QComboBox(body);
    capitalBox->addItems({QObject::tr("正常"), QObject::tr("全大写"), QObject::tr("全小写"), QObject::tr("小型大写"), QObject::tr("单词首字母大写")});
    QDoubleSpinBox* letterSpacing = new QDoubleSpinBox(body);
    letterSpacing->setRange(50.0, 200.0);
    letterSpacing->setDecimals(1);
    letterSpacing->setSuffix(" %");
    letterSpacing->setValue(100.0);
    QDoubleSpinBox* wordSpacing = new QDoubleSpinBox(body);
    wordSpacing->setRange(0.0, 20.0);
    wordSpacing->setDecimals(1);
    wordSpacing->setSuffix(" px");
    wordSpacing->setValue(0.0);
    QFontDatabase fdb;
    auto wsName = [&](QFontDatabase::WritingSystem ws){
        switch(ws){
        case QFontDatabase::Latin: return QObject::tr("拉丁");
        case QFontDatabase::Greek: return QObject::tr("希腊");
        case QFontDatabase::Cyrillic: return QObject::tr("西里尔");
        case QFontDatabase::Arabic: return QObject::tr("阿拉伯");
        case QFontDatabase::Hebrew: return QObject::tr("希伯来");
        case QFontDatabase::Japanese: return QObject::tr("日文");
        case QFontDatabase::SimplifiedChinese: return QObject::tr("中文(简体)");
        case QFontDatabase::TraditionalChinese: return QObject::tr("中文(繁体)");
        case QFontDatabase::Korean: return QObject::tr("韩文");
        case QFontDatabase::Thai: return QObject::tr("泰文");
        case QFontDatabase::Vietnamese: return QObject::tr("越南文");
        case QFontDatabase::Other: return QObject::tr("其他");
        default: return QObject::tr("未知");
        }
    };
    QList<QFontDatabase::WritingSystem> systems = fdb.writingSystems();
    // 将常见系统放前面
    systems.removeAll(QFontDatabase::Other);
    systems.prepend(QFontDatabase::Latin);
    systems.append(QFontDatabase::Other);
    for (auto ws : systems) wsBox->addItem(wsName(ws), int(ws));
    auto fillFamilies = [&](){
        familyBox->clear();
        auto ws = QFontDatabase::WritingSystem(wsBox->currentData().toInt());
        familyBox->addItems(fdb.families(ws));
    };
    fillFamilies();
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
        f.setOverline(overlineCheck->isChecked());
        f.setKerning(kerningCheck->isChecked());
        // Capitalization
        switch (capitalBox->currentIndex()) {
        case 1: f.setCapitalization(QFont::AllUppercase); break;
        case 2: f.setCapitalization(QFont::AllLowercase); break;
        case 3: f.setCapitalization(QFont::SmallCaps); break;
        case 4: f.setCapitalization(QFont::Capitalize); break;
        default: f.setCapitalization(QFont::MixedCase); break;
        }
        f.setLetterSpacing(QFont::PercentageSpacing, letterSpacing->value());
        f.setWordSpacing(wordSpacing->value());
        preview->setFont(f);
        if (currentOut) *currentOut = f;
    };
    QObject::connect(wsBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int){ fillFamilies(); fillStyles(); updatePreview(); });
    QObject::connect(familyBox, &QComboBox::currentTextChanged, [&](const QString&){ fillStyles(); updatePreview(); });
    QObject::connect(styleBox, &QComboBox::currentTextChanged, [&](const QString&){ updatePreview(); });
    QObject::connect(sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), [&](int){ updatePreview(); });

    row->addWidget(new QLabel(QObject::tr("书写系统"), body));
    row->addWidget(wsBox);
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
    row->addWidget(overlineCheck);

    v->addLayout(row);
    v->addWidget(preview);
    // 更多效果项
    auto* effRow = new QHBoxLayout();
    effRow->addWidget(new QLabel(QObject::tr("变换"), body));
    effRow->addWidget(capitalBox);
    effRow->addWidget(new QLabel(QObject::tr("字距%"), body));
    effRow->addWidget(letterSpacing);
    effRow->addWidget(new QLabel(QObject::tr("词距(px)"), body));
    effRow->addWidget(wordSpacing);
    effRow->addWidget(kerningCheck);
    effRow->addStretch();
    v->addLayout(effRow);
    body->setLayout(v);
    QObject::connect(boldCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(italicCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(underlineCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(strikeCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(overlineCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(kerningCheck, &QCheckBox::toggled, [&](bool){ updatePreview(); });
    QObject::connect(capitalBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int){ updatePreview(); });
    QObject::connect(letterSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double){ updatePreview(); });
    QObject::connect(wordSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double){ updatePreview(); });
    updatePreview();
    return body;
}
 
 } // namespace QtAdvancedUI