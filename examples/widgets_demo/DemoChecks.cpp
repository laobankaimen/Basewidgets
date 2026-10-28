// 可选的 demo 验证入口（-DUI_DEMO_CHECKS=ON，运行 widgets_demo --verify）。
// 检查渲染/交互/生命周期，不作为独立测试工程或应用安装依赖。
#include <QtTest>
#include <QApplication>
#include <basewidgets/Gauges/CircularGauge.h>
#include <basewidgets/Gauges/CompassGauge.h>
#include <basewidgets/Gauges/AttitudeMeter.h>
#include "pages/GaugesPage.h"
#include "pages/AcousticPage.h"
#include "pages/DirectionAnalysisPage.h"
#include "pages/DataManagementPage.h"
#include "pages/DashboardPage.h"
#include <basewidgets/Containers/DashboardPanel.h>
#include <basewidgets/Gauges/StorageDonutMeter.h>
#include <basewidgets/Acoustics/DemonAnalysis.h>
#include <basewidgets/Charts/BeamPlots.h>
#include <basewidgets/Charts/LofarDemonPlots.h>
#include <basewidgets/Charts/RateHistoryPlot.h>
#include <basewidgets/Charts/SampledGraph.h>
#include <basewidgets/Gauges/StorageCapacityMeter.h>
#include <basewidgets/Gauges/DataManagementDashboards.h>
#include <QContextMenuEvent>
#include <QShortcut>
#include <basewidgets/Gauges/TransferRateMeter.h>
#include <QScopedPointer>
#include <basewidgets/Acoustics/AcousticAnalysis.h>
#include <basewidgets/Charts/AcousticWaveformPlot.h>
#include <basewidgets/Charts/AcousticSpectrumPlot.h>
#include <basewidgets/Charts/AcousticSpectrogramPlot.h>
#include <basewidgets/Charts/RecordingTimelinePlot.h>
#include <basewidgets/Gauges/SignalQualityMeter.h>
#include <basewidgets/Gauges/AcquisitionStatusPanel.h>
#include <cmath>
#include <QComboBox>
#include <QDirIterator>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QSignalSpy>
#include <QSpinBox>
#include <QStyleOption>
#include <QTabWidget>
#include <QTabBar>
#include <basewidgets/Containers/TabWidget.h>
#include <basewidgets/Dialogs/DialogUtils.h>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QPointer>
#include <QResource>
#include <QScrollArea>
#include <QToolBar>
#include <QStatusBar>
#include <basewidgets/Core/FramelessWidgetWindow.h>
#include <basewidgets/Core/WindowPlacement.h>
#include <basewidgets/Core/ScreenWindow.h>
#include <basewidgets/Core/MultiScreenWorkspace.h>
#include <QScreen>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Inputs/ElidedLabel.h>
#include <basewidgets/Inputs/DateRangePicker.h>
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Views/AdvancedTable.h>
#include <QStandardItemModel>
#include <QAbstractItemModelTester>
#include <QSortFilterProxyModel>
#include <QClipboard>
#include <basewidgets/Views/TableView.h>
#include <limits>
#include <basewidgets/Core/NativeStyle.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/LineEdit.h>
#include <basewidgets/Inputs/BusyIndicator.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Inputs/RadioButton.h>
#include <basewidgets/Inputs/Slider.h>
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Inputs/ToggleSwitch.h>
#include <basewidgets/Inputs/SegmentedControl.h>
#include <basewidgets/Inputs/FilterChip.h>
#include <basewidgets/Containers/MessageBanner.h>
#include <basewidgets/Containers/EmptyState.h>
#include "pages/CommonWidgetsPage.h"
#include <basewidgets/Inputs/StatusBadge.h>
#include <basewidgets/Inputs/NotificationBell.h>
#include <basewidgets/Views/NotificationModel.h>
#include <basewidgets/Containers/NotificationPanel.h>
#include <basewidgets/Containers/AboutWidget.h>
#include <basewidgets/Dialogs/NotificationPopup.h>
#include <basewidgets/Dialogs/AboutDialog.h>
#include "DemoAbout.h"
#include <basewidgets/Inputs/SearchEdit.h>
#include <basewidgets/Inputs/CopyField.h>
#include <basewidgets/Containers/CollapsibleSection.h>
#include <QIntValidator>
#include <QInputMethodEvent>
#include "DemoPageIds.h"
#include <basewidgets/Views/ListView.h>
#include <basewidgets/Views/TreeTable.h>

using namespace basewidgets;

class TestSlider : public Slider {
public:
    using Slider::Slider;
    QStyleOptionSlider option() const { QStyleOptionSlider o; initStyleOption(&o); return o; }
};
class TestToolButton : public ToolButton {
public:
    QStyleOptionToolButton option() const { QStyleOptionToolButton option; initStyleOption(&option); return option; }
};

class TestProgress : public ProgressBar {
public:
    QStyleOptionProgressBar option() const { QStyleOptionProgressBar o; initStyleOption(&o); return o; }
};

static QImage render(QWidget& widget)
{
    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    widget.render(&image);
    return image;
}

// Validate rendered theme combinations, not just individual RGB constants.
static double contrastRatio(const QColor& first, const QColor& second)
{
    auto luminance = [](const QColor& color) {
        auto linear = [](double value) {
            return value <= .04045 ? value / 12.92 : std::pow((value + .055) / 1.055, 2.4);
        };
        return .2126 * linear(color.redF()) + .7152 * linear(color.greenF()) + .0722 * linear(color.blueF());
    };
    const double a = luminance(first), b = luminance(second);
    return (qMax(a, b) + .05) / (qMin(a, b) + .05);
}

static bool writeThemeFixture(const QString& fileName, const QByteArray& data)
{
    if (!QDir().mkpath(QFileInfo(fileName).absolutePath())) return false;
    QFile file(fileName);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

class NativeStyleTests : public QObject {
    Q_OBJECT
    QTemporaryDir output;
private slots:
    void initTestCase()
    {
        // Access tokens before lazy manager initialization: must not recurse.
        QVERIFY(ThemeTokens::instance()->primary().isValid());
        // Validate the actual consumer path: the library supplies its own themes.
        StyleManagerAdapter::initialize(QString(), "themes", "light");
        QCOMPARE(StyleManagerAdapter::instance()->currentTheme(), QString("light"));
        QVERIFY(qobject_cast<NativeStyle*>(qApp->style()));
        QVERIFY(qApp->styleSheet().isEmpty());
        StyleManagerAdapter::instance()->setOutputDirPath(output.path());
    }
    void themesWithoutStylesheets()
    {
        auto* mgr = StyleManagerAdapter::instance();
        for (const QString theme : {QString("dark"), QString("light"), QString("dark")}) {
            QVERIFY(mgr->setCurrentTheme(theme));
            QVERIFY(mgr->updateTheme());
            QCOMPARE(ThemeTokens::instance()->bg0(), mgr->themeColor("pageBackgroundColor"));
            QCOMPARE(qApp->palette().color(QPalette::Window), ThemeTokens::instance()->bg0());
            QCOMPARE(qApp->palette().color(QPalette::Text), ThemeTokens::instance()->textPrimary());
            QVERIFY(qApp->styleSheet().isEmpty());
            QVERIFY(mgr->styleSheet().isEmpty());
        }
        QDirIterator css(output.path(), {"*.css"}, QDir::Files, QDirIterator::Subdirectories);
        QVERIFY(!css.hasNext());
    }
    void builtInThemeTextContrast()
    {
        auto* manager = StyleManagerAdapter::instance();
        for (const QString theme : manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            auto* tokens = ThemeTokens::instance();
            const QPalette palette = tokens->palette();
            for (const QColor& background : {tokens->bg0(), tokens->bg1(), tokens->bg2(),
                                            tokens->bgCtrl(), tokens->bgHover(), tokens->bgPressed(), tokens->titleBarBackground()})
                QVERIFY2(contrastRatio(tokens->textPrimary(), background) >= 4.5, qPrintable(theme));
            for (const QColor& background : {tokens->bg0(), tokens->bg1(), tokens->bg2(), tokens->bgCtrl(), tokens->bgHover()})
                QVERIFY2(contrastRatio(tokens->textSecondary(), background) >= 4.5, qPrintable(theme));
            // Gauge state labels and chart annotations must stay readable in both themes.
            for (const QColor& foreground : {tokens->primary(), tokens->danger(), tokens->warning(), tokens->success()})
                for (const QColor& background : {tokens->bg0(), tokens->bg1()})
                    QVERIFY2(contrastRatio(foreground, background) >= 4.5, qPrintable(theme + " " + foreground.name()));
            QVERIFY(contrastRatio(tokens->textPlaceholder(), tokens->bg1()) >= 4.5);
            QVERIFY(tokens->textPlaceholder() != tokens->textDisabled());
            QCOMPARE(tokens->textPlaceholder(), manager->themeColor("placeholderTextColor"));
            for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive}) {
                QCOMPARE(palette.color(group, QPalette::PlaceholderText), tokens->textPlaceholder());
                QVERIFY(contrastRatio(palette.color(group, QPalette::Highlight),
                                      palette.color(group, QPalette::HighlightedText)) >= 4.5);
            }
            QCOMPARE(palette.color(QPalette::Disabled, QPalette::PlaceholderText), tokens->textDisabled());
            LineEdit input; input.setPlaceholderText("输入录音名称");
            QCOMPARE(input.palette().color(QPalette::PlaceholderText), tokens->textPlaceholder());
            input.setText("录音样本"); input.selectAll();
            QVERIFY(contrastRatio(input.palette().highlight().color(), input.palette().highlightedText().color()) >= 4.5);
        }
    }
    void filledButtonsUseReadableText()
    {
        auto* manager = StyleManagerAdapter::instance();
        for (const QString theme : manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for (ButtonAppearance appearance : {ButtonAppearance::Primary, ButtonAppearance::Danger}) {
                PushButton button("Action"); button.setAppearance(appearance);
                ToolButton tool; tool.setAppearance(appearance);
                for (QStyle::State state : {QStyle::State(QStyle::State_Enabled),
                                           QStyle::State(QStyle::State_Enabled | QStyle::State_MouseOver),
                                           QStyle::State(QStyle::State_Enabled | QStyle::State_Sunken)}) {
                    QStyleOptionButton option;
                    option.initFrom(&button); option.rect = QRect(0, 0, 140, 40); option.state = state;
                    QImage surface(option.rect.size(), QImage::Format_ARGB32_Premultiplied);
                    surface.fill(option.palette.window().color());
                    { QPainter painter(&surface); button.style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &option, &painter, &button); }
                    const QColor ink = option.palette.color(QPalette::BrightText);
                    QVERIFY(contrastRatio(ink, surface.pixelColor(70, 20)) >= 4.5);
                    // Selection text may be dark on the brighter indicator blue; filled
                    // buttons keep their independently readable light caption and arrow.
                    auto containsInk = [&](const QImage& image) {
                        for (int y = 0; y < image.height(); ++y)
                            for (int x = 0; x < image.width(); ++x) {
                                const QColor pixel = image.pixelColor(x, y);
                                if (pixel.alpha() > 220 && qAbs(pixel.red() - ink.red()) < 4
                                    && qAbs(pixel.green() - ink.green()) < 4 && qAbs(pixel.blue() - ink.blue()) < 4) return true;
                            }
                        return false;
                    };
                    QImage label(surface.size(), QImage::Format_ARGB32_Premultiplied); label.fill(Qt::transparent);
                    option.text = "Action";
                    { QPainter painter(&label); button.style()->drawControl(QStyle::CE_PushButtonLabel, &option, &painter, &button); }
                    QVERIFY(containsInk(label));
                    QStyleOptionToolButton toolOption; toolOption.initFrom(&tool);
                    toolOption.rect = option.rect; toolOption.state = state;
                    toolOption.toolButtonStyle = Qt::ToolButtonTextOnly; toolOption.text = "Action"; toolOption.font = tool.font();
                    label.fill(Qt::transparent);
                    { QPainter painter(&label); tool.style()->drawControl(QStyle::CE_ToolButtonLabel, &toolOption, &painter, &tool); }
                    QVERIFY(containsInk(label));
                }
            }
        }
    }
    void themeConfigurationRejectsPartialUpdates()
    {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        const QString style=directory.path()+"/themes";
        QVERIFY(writeThemeFixture(style+"/themes.json", R"({"name":"themes","default_theme":"ocean","resources":{"primary":{"#0000ff":"primaryColor"}}})"));
        QVERIFY(writeThemeFixture(style+"/themes/ocean.xml", R"(<resources dark="1"><color name="primaryColor">#69B5A5</color></resources>)"));
        acss::QtAdvancedStylesheet manager;
        manager.setStylesDirPath(directory.path()); QVERIFY(manager.setCurrentStyle("themes"));
        QVERIFY(manager.themes().contains("ocean")); QVERIFY(manager.setCurrentTheme("ocean"));
        const auto colors=manager.themeColorVariables();
        QSignalSpy changed(&manager,&acss::QtAdvancedStylesheet::currentThemeChanged);
        const QList<QByteArray> invalid{
            R"(<resources dark="0"><color name="primaryColor">#123456</color>)",
            R"(<resources><color name="primaryColor">not-a-color</color></resources>)",
            R"(<resources><color name="primaryColor">#123456</color><color name="primaryColor">#654321</color></resources>)",
            R"(<resources><color name="primaryColor">#123456</color><unknown/></resources>)",
            R"(<resources><color name="primaryColor"><nested/>#123456</color></resources>)",
            R"(<resources/><resources/>)"};
        for(const auto& xml:invalid) {
            QVERIFY(writeThemeFixture(style+"/themes/broken.xml",xml));
            QVERIFY(!manager.setCurrentTheme("broken"));
            QCOMPARE(manager.currentTheme(),QString("ocean")); QVERIFY(manager.isCurrentThemeDark());
            QCOMPARE(manager.themeColorVariables(),colors);
            QCOMPARE(manager.themeVariableValue("primaryColor"),QString("#69B5A5"));
            QVERIFY(!manager.errorString().isEmpty());
        }
        QVERIFY(!manager.setCurrentTheme("missing")); QCOMPARE(changed.count(),0);
    }
    void externalThemeControlsPaletteAndSeries()
    {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        const QString style=directory.path()+"/themes";
        QVERIFY(writeThemeFixture(style+"/themes.json", R"({"name":"themes","default_theme":"ocean","resources":{"primary":{"#0000ff":"primaryColor"}}})"));
        QVERIFY(writeThemeFixture(style+"/themes/ocean.xml", R"(<resources dark="1">
            <color name="primaryColor">#69B5A5</color>
            <color name="selectionColor">#294F65</color><color name="selectionTextColor">#D4E3EC</color>
            <color name="buttonTextColor">#20342C</color><color name="primaryButtonColor">#B8DCCF</color>
            <color name="dangerButtonColor">#EDBCBC</color><color name="plotCursorColor">#A5B4C3</color>
            <color name="seriesColor1">#77C4AC</color><color name="seriesColor2">#D0AF73</color>
            <color name="switchKnobColor">#CFD9D7</color></resources>)"));
        QVERIFY(writeThemeFixture(style+"/themes/paper.xml", R"(<resources><color name="primaryColor">#326CAA</color></resources>)"));
        auto* manager=StyleManagerAdapter::instance();
        struct RestoreTheme {
            QString path,style,theme,output;
            ~RestoreTheme() { StyleManagerAdapter::initialize(path,style,theme); StyleManagerAdapter::instance()->setOutputDirPath(output); }
        } restore{manager->stylesDirPath(),manager->currentStyle(),manager->currentTheme(),manager->outputDirPath()};
        StyleManagerAdapter::initialize(directory.path(),"themes","ocean");
        manager->setOutputDirPath(directory.path()+"/generated");
        QVERIFY(manager->themes().contains("ocean")); QVERIFY(manager->updateTheme());
        auto* tokens=ThemeTokens::instance();
        QCOMPARE(tokens->primary(),QColor("#69B5A5")); QCOMPARE(tokens->bg0(),QColor("#202327"));
        QCOMPARE(qApp->palette().highlight().color(),QColor("#294F65"));
        QCOMPARE(qApp->palette().highlightedText().color(),QColor("#D4E3EC"));
        QCOMPARE(qApp->palette().brightText().color(),QColor("#20342C"));
        QCOMPARE(tokens->primaryButton(),QColor("#B8DCCF")); QCOMPARE(tokens->dangerButton(),QColor("#EDBCBC"));
        QCOMPARE(tokens->plotCursor(),QColor("#A5B4C3")); QCOMPARE(tokens->switchKnob(),QColor("#CFD9D7"));
        QCOMPARE(tokens->seriesColor(0),QColor("#77C4AC")); QCOMPARE(tokens->seriesColor(6),tokens->seriesColor(0));
        QVERIFY(!tokens->seriesColor(-1).isValid());
        AcousticWaveformPlot waveform; waveform.addChannel("Default"); waveform.addChannel("Fixed",QColor("#EC7F91"));
        AcousticSpectrumPlot spectrum;
        RateHistoryPlot rates; rates.addSeries("Default"); rates.addSeries("Fixed",QColor("#EC7F91"));
        BeamPatternPlot beams; QVERIFY(beams.setBeam("a","Default",{0,90},{-5,-10}));
        QVERIFY(beams.setBeam("b","Fixed",{0,90},{-5,-10},QColor("#EC7F91")));
        QCOMPARE(waveform.graph(0)->pen().color(),tokens->seriesColor(0));
        QCOMPARE(spectrum.graph(1)->pen().color(),tokens->seriesColor(1));
        for(ButtonAppearance appearance:{ButtonAppearance::Primary,ButtonAppearance::Danger}) {
            PushButton button("Configured"); button.setAppearance(appearance);
            for(QStyle::State state:{QStyle::State(QStyle::State_Enabled),QStyle::State(QStyle::State_Enabled|QStyle::State_MouseOver),QStyle::State(QStyle::State_Enabled|QStyle::State_Sunken)}) {
                QStyleOptionButton option; option.initFrom(&button); option.rect=QRect(0,0,120,40); option.state=state;
                QImage image(option.rect.size(),QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent);
                { QPainter painter(&image); button.style()->drawPrimitive(QStyle::PE_PanelButtonCommand,&option,&painter,&button); }
                QVERIFY(contrastRatio(image.pixelColor(60,20),option.palette.brightText().color())>=4.5);
            }
        }
        QVERIFY(manager->setCurrentTheme("paper")); QVERIFY(manager->updateTheme());
        QVERIFY(!manager->isCurrentThemeDark()); QCOMPARE(tokens->bg0(),QColor("#E9EDF2"));
        for(StyledPlot* plot:{static_cast<StyledPlot*>(&waveform),static_cast<StyledPlot*>(&rates),static_cast<StyledPlot*>(&beams)}) {
            QCOMPARE(plot->graph(0)->pen().color(),tokens->seriesColor(0));
            QCOMPARE(plot->graph(1)->pen().color(),QColor("#EC7F91"));
        }
        QCOMPARE(spectrum.graph(0)->pen().color(),tokens->seriesColor(0));
        QCOMPARE(spectrum.graph(1)->pen().style(),Qt::DashLine);
        QVERIFY(waveform.setSeriesColor(waveform.graph(1),1)); // restore automatic theme color
        QVERIFY(!waveform.setSeriesColor(rates.graph(0),0)); QVERIFY(!waveform.setSeriesColor(nullptr,0));
        QVERIFY(!waveform.setSeriesColor(waveform.graph(0),-1));
        QPen custom=waveform.graph(0)->pen(); custom.setColor(QColor("#AABBCC")); custom.setWidthF(2.5); waveform.graph(0)->setPen(custom);
        QVERIFY(beams.setBeam("a","Fixed later",{0,90},{-5,-10},QColor("#778899")));
        rates.removeGraph(rates.graph(0)); // deleted graph must not leave a theme callback dangling
        QVERIFY(manager->setCurrentTheme("ocean")); QVERIFY(manager->updateTheme());
        QCOMPARE(waveform.graph(0)->pen().color(),QColor("#AABBCC")); QCOMPARE(waveform.graph(0)->pen().widthF(),2.5);
        QCOMPARE(waveform.graph(1)->pen().color(),tokens->seriesColor(1));
        QCOMPARE(beams.graph(0)->pen().color(),QColor("#778899"));
        QVERIFY(qApp->styleSheet().isEmpty());
    }
    void segmentedControlNavigation()
    {
        SegmentedControl segment; segment.setAccessibleName("Record view");
        segment.addTab("All"); segment.addTab("Disabled"); segment.addTab("Archived");
        segment.setTabData(2,"archived"); segment.setTabEnabled(1,false); segment.resize(380,42); segment.show();
        segment.activateWindow(); segment.setFocus(Qt::TabFocusReason); QTest::qWait(20);
        QSignalSpy changed(&segment,&QTabBar::currentChanged);
        QTest::keyClick(&segment,Qt::Key_Right); QCOMPARE(segment.currentIndex(),2); QCOMPARE(changed.count(),1);
        QCOMPARE(segment.tabData(segment.currentIndex()).toString(),QString("archived"));
        segment.setCurrentIndex(2); QCOMPARE(changed.count(),1);
        QTest::mouseClick(&segment,Qt::LeftButton,Qt::NoModifier,segment.tabRect(1).center()); QCOMPARE(segment.currentIndex(),2);
        segment.setLayoutDirection(Qt::RightToLeft); segment.setCurrentIndex(0);
        QVERIFY(segment.tabRect(0).center().x()>segment.tabRect(2).center().x());
        QTest::keyClick(&segment,Qt::Key_Left); QCOMPARE(segment.currentIndex(),2);
        segment.setTabText(2,QString(100,'W')); segment.resize(180,42);
        QVERIFY(!render(segment).isNull()); // long options use Qt elision/scrolling
        segment.removeTab(2); QVERIFY(segment.currentIndex()!=2);
    }
    void filterChipSelectionAndRemoval()
    {
        FilterChip chip("Station / East Sea"); chip.setRemovable(true); chip.resize(220,38); chip.show();
        auto* close=chip.findChild<QAbstractButton*>("FilterChipRemove"); QVERIFY(close);
        QSignalSpy toggled(&chip,&QAbstractButton::toggled), removed(&chip,&FilterChip::removeRequested);
        chip.activateWindow(); chip.setFocus(Qt::TabFocusReason); QTest::qWait(20);
        QTest::keyClick(&chip,Qt::Key_Space); QVERIFY(chip.isChecked()); QCOMPARE(toggled.count(),1);
        QTest::mouseClick(close,Qt::LeftButton); QCOMPARE(removed.count(),1); QCOMPARE(toggled.count(),1); QVERIFY(chip.isVisible());
        chip.setFocus(); QTest::keyClick(&chip,Qt::Key_Delete); QCOMPARE(removed.count(),2); QVERIFY(chip.isChecked());
        QTest::keyClick(&chip,Qt::Key_Return); QVERIFY(!chip.isChecked());
        chip.setLayoutDirection(Qt::RightToLeft); QVERIFY(close->geometry().center().x()<chip.width()/2);
        QTest::mouseClick(&chip,Qt::LeftButton,Qt::NoModifier,QPoint(chip.width()-24,chip.height()/2)); QVERIFY(chip.isChecked());
        chip.setRemovable(false); QVERIFY(close->isHidden()); QTest::keyClick(&chip,Qt::Key_Delete); QCOMPARE(removed.count(),2);
        chip.setRemovable(true); chip.setEnabled(false); QTest::keyClick(&chip,Qt::Key_Delete); QCOMPARE(removed.count(),2);
        close->click(); QCOMPARE(removed.count(),2);
        chip.setEnabled(true); chip.setCheckable(false); QTest::mouseClick(&chip,Qt::LeftButton,Qt::NoModifier,QPoint(100,18)); QVERIFY(!chip.isChecked());
        chip.setText(QString(100,'W')); chip.resize(180,38); QVERIFY(!render(chip).isNull()); QCOMPARE(chip.text().size(),100);
        // Receivers may destroy the chip synchronously from an action signal.
        auto* transient=new FilterChip("Remove me"); transient->setRemovable(true); QPointer<FilterChip> guard=transient;
        connect(transient,&FilterChip::removeRequested,transient,[transient] { delete transient; });
        transient->findChild<QAbstractButton*>("FilterChipRemove")->click(); QVERIFY(guard.isNull());
    }
    void feedbackActionsAndWrapping()
    {
        MessageBanner banner; banner.setTitle("Import result"); banner.setMessage(QString("Recorded data needs verification. ").repeated(18));
        banner.setActionText("Review"); banner.setTone(FeedbackTone::Warning); banner.resize(350,banner.heightForWidth(350)); banner.show();
        QSignalSpy action(&banner,&MessageBanner::actionRequested), dismissed(&banner,&MessageBanner::dismissed);
        auto* button=banner.findChild<PushButton*>("MessageBannerAction"); QVERIFY(button);
        button->click(); QCOMPARE(action.count(),1); QVERIFY(banner.isVisible());
        banner.setDismissible(false); banner.dismiss(); QCOMPARE(dismissed.count(),0); QVERIFY(banner.isVisible());
        banner.setDismissible(true); banner.findChild<QAbstractButton*>("MessageBannerClose")->click();
        QCOMPARE(dismissed.count(),1); QVERIFY(banner.isHidden()); banner.dismiss(); QCOMPARE(dismissed.count(),1); banner.show();
        QVERIFY(banner.heightForWidth(300)>banner.heightForWidth(600));
        banner.setActionText(""); QVERIFY(button->isHidden());
        banner.setMessage("<b>Literal text</b>"); QCOMPARE(banner.message(),QString("<b>Literal text</b>"));
        for(auto* label:banner.findChildren<QLabel*>())QCOMPARE(label->textFormat(),Qt::PlainText);
        EmptyState empty; empty.setTitle("No matching recordings"); empty.setDescription("Clear filters to see all recordings.");
        empty.setActionText("Clear filters"); empty.setSecondaryActionText("Choose folder"); empty.resize(330,300); empty.show();
        QSignalSpy primary(&empty,&EmptyState::actionRequested), secondary(&empty,&EmptyState::secondaryActionRequested);
        empty.findChild<PushButton*>("EmptyStateAction")->click(); empty.findChild<PushButton*>("EmptyStateSecondaryAction")->click();
        QCOMPARE(primary.count(),1); QCOMPARE(secondary.count(),1); QVERIFY(empty.isVisible());
        empty.setEnabled(false); empty.findChild<PushButton*>("EmptyStateAction")->click(); QCOMPARE(primary.count(),1);
        empty.setEnabled(true); QPixmap icon(40,20); icon.fill(Qt::blue); empty.setIcon(QIcon(icon)); QVERIFY(!empty.icon().isNull());
        empty.setIcon(QIcon()); QVERIFY(empty.icon().isNull()); empty.setSecondaryActionText("");
        QVERIFY(empty.findChild<PushButton*>("EmptyStateSecondaryAction")->isHidden());
    }
    void searchEditDebounceCommitAndCancel()
    {
        SearchEdit search; search.resize(280,40); search.show(); QVERIFY(search.isClearButtonEnabled());
        QVERIFY(search.setDebounceInterval(40)); QVERIFY(!search.setDebounceInterval(-1)); QCOMPARE(search.debounceInterval(),40);
        QSignalSpy requests(&search,&SearchEdit::searchRequested);
        search.setText("a"); search.setText("ab"); search.setText("abc"); QCOMPARE(requests.count(),0);
        QTRY_COMPARE(requests.count(),1); QCOMPARE(requests.takeFirst().first().toString(),QString("abc"));
        search.setText("  station  "); QTest::keyClick(&search,Qt::Key_Return);
        QCOMPARE(requests.count(),1); QCOMPARE(requests.takeFirst().first().toString(),QString("  station  "));
        QTest::qWait(80); QCOMPARE(requests.count(),0); // Return cancels the pending duplicate.
        search.setText("cancel"); search.cancelPendingSearch(); QTest::qWait(80); QCOMPARE(requests.count(),0);
        search.setText("disabled"); search.setEnabled(false); QTest::qWait(80); search.requestSearch(); QCOMPARE(requests.count(),0);
        search.setEnabled(true); QTest::keyClick(&search,Qt::Key_Escape); QVERIFY(search.text().isEmpty());
        QTRY_COMPARE(requests.count(),1); QVERIFY(requests.takeFirst().first().toString().isEmpty());
        QIntValidator validator(1,99,&search); search.setValidator(&validator); search.setText("0"); search.requestSearch();
        QTest::qWait(80); QCOMPARE(requests.count(),0);
        search.setValidator(nullptr); search.clear(); search.cancelPendingSearch();
        QInputMethodEvent preedit("中",{}); QApplication::sendEvent(&search,&preedit); QTest::qWait(80); QCOMPARE(requests.count(),0);
        QInputMethodEvent commit; commit.setCommitString("中"); QApplication::sendEvent(&search,&commit);
        QTRY_COMPARE(requests.count(),1); QCOMPARE(requests.takeFirst().first().toString(),QString("中"));
        QVERIFY(search.setDebounceInterval(0)); search.clear(); QTRY_COMPARE(requests.count(),1);
    }
    void copyFieldUsesFullTextAndSafeLifetime()
    {
        const QString before=QApplication::clipboard()->text();
        CopyField field; field.resize(180,40); field.show();
        auto* button=field.findChild<ToolButton*>("CopyFieldButton"); QVERIFY(button); QVERIFY(!button->isEnabled());
        QVERIFY(field.textEdit()->isReadOnly()); QSignalSpy copied(&field,&CopyField::copied),changed(&field,&CopyField::textChanged);
        field.copy(); QCOMPARE(copied.count(),0);
        const QString longText=QString("录音-α/").repeated(8000); field.setText(longText); QCOMPARE(field.text(),longText); QCOMPARE(changed.count(),1);
        button->click(); QCOMPARE(copied.count(),1); QCOMPARE(copied.takeFirst().first().toString(),longText); QCOMPARE(QApplication::clipboard()->text(),longText);
        QVERIFY(button->accessibleName().contains("已复制")); field.setText("record-002"); QVERIFY(!button->accessibleName().contains("已复制"));
        field.setEnabled(false); field.copy(); QCOMPARE(copied.count(),0);
        field.setEnabled(true); field.setText(""); QVERIFY(!button->isEnabled());
        auto* temporary=new CopyField("temporary"); QPointer<CopyField> guard=temporary;
        connect(temporary,&CopyField::copied,temporary,[temporary] { delete temporary; }); temporary->copy(); QVERIFY(guard.isNull());
        QApplication::clipboard()->setText(before);
    }
    void collapsibleSectionRetainsValuesAndFocus()
    {
        CollapsibleSection section("高级参数"); auto* input=new LineEdit("keep",section.contentWidget()); section.contentLayout()->addWidget(input);
        section.resize(320,150); section.show(); section.activateWindow(); input->setFocus(); QTest::qWait(20);
        auto* header=section.findChild<ToolButton*>("CollapsibleSectionHeader"); QVERIFY(header); QVERIFY(section.isExpanded());
        const int expandedHeight=section.sizeHint().height(); QSignalSpy toggles(&section,&CollapsibleSection::expandedChanged);
        section.setExpanded(false); QCOMPARE(toggles.count(),1); QVERIFY(section.contentWidget()->isHidden()); QVERIFY(header->hasFocus());
        QCOMPARE(input->text(),QString("keep")); QVERIFY(section.sizeHint().height()<expandedHeight);
        section.setExpanded(false); QCOMPARE(toggles.count(),1); QTest::keyClick(header,Qt::Key_Return);
        QVERIFY(section.isExpanded()); QVERIFY(!section.contentWidget()->isHidden()); QCOMPARE(input->text(),QString("keep"));
        QTest::keyClick(header,Qt::Key_Space); QVERIFY(!section.isExpanded());
        section.setLayoutDirection(Qt::RightToLeft); QCOMPARE(header->arrowType(),Qt::LeftArrow);
        section.setEnabled(false); QTest::keyClick(header,Qt::Key_Return); QVERIFY(!section.isExpanded());
        section.setEnabled(true); section.setExpanded(true); QCOMPARE(header->arrowType(),Qt::DownArrow);
        section.setTitle(QString("长标题与连接详情").repeated(30)); section.resize(240,180); QApplication::processEvents();
        QCOMPARE(section.width(),240); QCOMPARE(header->toolTip(),section.title()); QVERIFY(!render(section).isNull());
    }
    void notificationModelBoundsAndSharedUnreadState()
    {
        NotificationModel model; QAbstractItemModelTester check(&model,QAbstractItemModelTester::FailureReportingMode::QtTest);
        NotificationBell bell,other; bell.setModel(&model); other.setModel(&model);
        QSignalSpy unread(&model,&NotificationModel::unreadCountChanged);
        QVERIFY(model.setMaximumCount(2)); QVERIFY(!model.setMaximumCount(0));
        QVERIFY(!model.addMessage({"","Invalid","body"})); QCOMPARE(model.rowCount(),0);
        QVERIFY(model.addMessage({"a","First","body"})); QVERIFY(model.addMessage({"b","Second","body"}));
        QCOMPARE(bell.unreadCount(),2); QCOMPARE(other.unreadCount(),2); QCOMPARE(model.messageAt(0).id,QString("b"));
        QVERIFY(model.markRead("b")); QCOMPARE(bell.unreadCount(),1); const int notifications=unread.count();
        QVERIFY(model.markRead("b")); QCOMPARE(unread.count(),notifications);
        auto replacement=model.message("b"); replacement.title="Updated"; QVERIFY(model.addMessage(replacement)); QCOMPARE(model.rowCount(),2); QCOMPARE(model.unreadCount(),1);
        QVERIFY(model.addMessage({"c","Third","body"})); QVERIFY(model.message("a").id.isEmpty()); QCOMPARE(model.unreadCount(),1);
        QVERIFY(model.markRead("b",false)); QCOMPARE(other.unreadCount(),2); model.markAllRead(); QCOMPARE(bell.unreadCount(),0);
        QVERIFY(model.setMaximumCount(1)); QCOMPARE(model.messageAt(0).id,QString("c")); QVERIFY(!model.removeMessage("missing"));
        model.clear(); QCOMPARE(model.rowCount(),0); QCOMPARE(other.unreadCount(),0);
        auto* transient=new NotificationModel; bell.setModel(transient); transient->addMessage({"x","Temp","body"}); delete transient;
        QCOMPARE(bell.unreadCount(),0); QVERIFY(!bell.model()); bell.setUnreadCount(1000); QVERIFY(bell.toolTip().contains("1000"));
        bell.setDotOnly(true); QVERIFY(bell.dotOnly()); bell.setModel(nullptr); QCOMPARE(bell.unreadCount(),0); bell.setUnreadCount(-1); QCOMPARE(bell.unreadCount(),0);
    }
    void notificationPanelReadFilterAndDetails()
    {
        NotificationModel model; model.addMessage({"a","<b>Literal title</b>","<script>literal body</script>",{},FeedbackTone::Warning,false,"查看任务"});
        model.addMessage({"b","Already read","Second body",{},FeedbackTone::Info,true,{}});
        NotificationPanel panel; panel.setModel(&model); panel.resize(400,520); panel.show(); panel.setUnreadOnly(true);
        auto* view=panel.findChild<QListView*>("NotificationList"); QVERIFY(view); QCOMPARE(view->model()->rowCount(),1);
        QSignalSpy activated(&panel,&NotificationPanel::messageActivated),action(&panel,&NotificationPanel::actionRequested);
        panel.openMessage("a"); QCOMPARE(activated.count(),1); QCOMPARE(model.unreadCount(),0); QCOMPARE(view->model()->rowCount(),0);
        QCOMPARE(panel.findChild<QPlainTextEdit*>("NotificationBody")->toPlainText(),QString("<script>literal body</script>"));
        panel.findChild<PushButton*>("NotificationAction")->click(); QCOMPARE(action.count(),1); QCOMPARE(action.first().first().toString(),QString("a"));
        panel.showList(); panel.setUnreadOnly(false); QCOMPARE(view->model()->rowCount(),2);
        panel.openMessage("b"); QVERIFY(panel.findChild<PushButton*>("NotificationAction")->isHidden());
        model.removeMessage("b"); QVERIFY(view->isVisible());
        auto* borrowed=new NotificationModel; panel.setModel(borrowed); borrowed->addMessage({"x","Temporary","body"}); delete borrowed;
        QVERIFY(panel.model()!=nullptr); QCOMPARE(panel.model()->rowCount(),0);
        panel.setModel(&model); panel.findChild<PushButton*>("NotificationClear")->click(); QCOMPARE(model.rowCount(),0);
    }
    void notificationPopupAndAboutRenderMatrix()
    {
        QWidget window; auto* layout=new QVBoxLayout(&window); auto* bell=new NotificationBell; layout->addWidget(bell); window.resize(220,90); window.show();
        NotificationModel model; model.addMessage({"one","存储容量提醒","实时存储使用率达到预警阈值。请检查归档任务状态。",QDateTime::fromString("2026-10-28T09:45:00+08:00",Qt::ISODate),FeedbackTone::Warning,false,"查看存储"});
        model.addMessage({"two","录音导入完成","12 个录音文件已导入，等待核验。",{},FeedbackTone::Success,false,"查看录音"}); bell->setModel(&model);
        NotificationPopup popup(&window); popup.setModel(&model); AboutDialog about(&window); QVERIFY(about.setInfo(demoAboutInfo()));
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR"); auto* manager=StyleManagerAdapter::instance();
        for(const auto& theme:manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            const QRect available=window.screen()->availableGeometry(); window.move(available.bottomRight()-QPoint(240,100)); QApplication::processEvents();
            popup.popup(bell); QApplication::processEvents(); QVERIFY(popup.isVisible()); QVERIFY2(available.contains(popup.frameGeometry()),qPrintable(QString("available=%1,%2 %3x%4; popup=%5,%6 %7x%8; min=%9x%10").arg(available.x()).arg(available.y()).arg(available.width()).arg(available.height()).arg(popup.x()).arg(popup.y()).arg(popup.width()).arg(popup.height()).arg(popup.minimumWidth()).arg(popup.minimumHeight()))); QCOMPARE(model.unreadCount(),2);
            if(!directory.isEmpty()) { QDir().mkpath(directory); QVERIFY(popup.grab().save(directory+"/notifications-"+theme+".png")); }
            QTest::keyClick(&popup,Qt::Key_Escape); QVERIFY(!popup.isVisible());
            for(const QSize size:{QSize(540,580),QSize(400,480)}) {
                about.resize(size); about.show(); QApplication::processEvents();
                auto* scroll=about.aboutWidget()->findChild<QScrollArea*>("AboutScroll"); QVERIFY(scroll); QCOMPARE(scroll->horizontalScrollBar()->maximum(),0);
                if(!directory.isEmpty())QVERIFY(about.grab().save(directory+QString("/about-%1-%2.png").arg(theme).arg(size.width())));
                about.hide();
            }
        }
    }
    void aboutDialogClosePaths()
    {
        AboutDialog dialog; QVERIFY(dialog.setInfo(demoAboutInfo()));
        QSignalSpy finished(&dialog,&QDialog::finished);
        dialog.open(); QTest::qWait(20); QVERIFY(dialog.isVisible());
        dialog.findChild<PushButton*>("AboutClose")->click();
        QVERIFY(!dialog.isVisible()); QCOMPARE(finished.count(),1); QCOMPARE(dialog.result(),int(QDialog::Accepted));

        dialog.open(); QTest::qWait(20);
        auto* focus=dialog.findChild<PushButton*>("AboutCopyInformation"); QVERIFY(focus); focus->setFocus();
        QTest::keyClick(focus,Qt::Key_Escape); // Escape must propagate from a content widget.
        QVERIFY(!dialog.isVisible()); QCOMPARE(finished.count(),2); QCOMPARE(dialog.result(),int(QDialog::Rejected));

        dialog.open(); QTest::qWait(20);
        auto* close=dialog.titleBar()->findChild<ToolButton*>("TitleBarCloseButton"); QVERIFY(close); close->click();
        QVERIFY(!dialog.isVisible()); QCOMPARE(finished.count(),3); QCOMPARE(dialog.result(),int(QDialog::Rejected));

        dialog.open(); QTest::qWait(20); QVERIFY(dialog.close());
        QVERIFY(!dialog.isVisible()); QCOMPARE(finished.count(),4);
        dialog.open(); QTest::qWait(20); dialog.reject();
        QVERIFY(!dialog.isVisible()); QCOMPARE(finished.count(),5);

        auto* disposable=new AboutDialog; disposable->setAttribute(Qt::WA_DeleteOnClose); QPointer<AboutDialog> guard=disposable;
        disposable->open(); QTest::qWait(20); disposable->titleBar()->findChild<ToolButton*>("TitleBarCloseButton")->click();
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(guard.isNull());
    }
    void aboutConfigurationAndSignals()
    {
        AboutWidget about; QVERIFY(about.informationText().contains(QCoreApplication::applicationName()));
        AboutInfo info=demoAboutInfo(); info.productName="<b>Literal product</b>"; QVERIFY(about.setInfo(info));
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QCOMPARE(about.findChild<QLabel*>("AboutProductName")->text(),info.productName);
        for(auto* text:about.findChildren<QLabel*>())QCOMPARE(text->textFormat(),Qt::PlainText);
        QSignalSpy activated(&about,&AboutWidget::linkActivated); about.findChild<PushButton*>("AboutLink_guide")->click();
        QCOMPARE(activated.count(),1); QCOMPARE(activated.first().at(0).toString(),QString("guide")); QCOMPARE(activated.first().at(1).toUrl(),QUrl("app:guide"));
        auto bad=info; bad.links.append(bad.links.first()); QVERIFY(!about.setInfo(bad)); QCOMPARE(about.info().productName,info.productName);
        QJsonObject config{{"productName","可配置产品"},{"version","2.5"},{"copyButtonVisible",false},
            {"details",QJsonArray{QJsonObject{{"label","协议"},{"value","v2"}}}},
            {"links",QJsonArray{QJsonObject{{"id","help"},{"text","帮助"},{"url","app:help"}}}}};
        QVERIFY(about.setConfiguration(config)); QVERIFY(!about.isCopyButtonVisible()); QVERIFY(about.informationText().contains("协议: v2"));
        auto invalid=config; invalid["version"]=5; QVERIFY(!about.setConfiguration(invalid)); QCOMPARE(about.info().version,QString("2.5"));
        invalid=config; invalid["logo"]="/missing/about-logo.png"; QVERIFY(!about.setConfiguration(invalid));
        QTemporaryDir assets; QImage logo(48,24,QImage::Format_ARGB32); logo.fill(Qt::cyan); QVERIFY(logo.save(assets.filePath("logo.png")));
        auto custom=config; custom["logo"]=assets.filePath("logo.png"); QVERIFY(about.setConfiguration(custom)); QVERIFY(!about.info().logo.isNull());
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); about.setCopyButtonVisible(true);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QSignalSpy copied(&about,&AboutWidget::informationCopied);
        const QString before=QApplication::clipboard()->text(); about.findChild<PushButton*>("AboutCopyInformation")->click(); QCOMPARE(copied.count(),1);
        QCOMPARE(QApplication::clipboard()->text(),about.informationText()); QApplication::clipboard()->setText(before);
        AboutDialog dialog; QVERIFY(dialog.setConfiguration(config)); QVERIFY(dialog.windowTitle().contains("可配置产品"));
    }
    void commonWidgetsDemoIntegration()
    {
        CommonWidgetsPage page; page.resize(900,700); page.show();
        auto* segments=page.findChild<SegmentedControl*>("CommonDemoSegments");
        auto* banner=page.findChild<MessageBanner*>("CommonDemoBanner");
        auto* empty=page.findChild<EmptyState*>("CommonDemoEmptyState");
        auto* summary=page.findChild<QLabel*>("CommonDemoSummary");
        QVERIFY(segments&&banner&&empty&&summary); QTest::qWait(20);
        auto chips=page.findChildren<FilterChip*>(); QCOMPARE(chips.size(),3);
        segments->setCurrentIndex(2); QVERIFY(summary->text().contains("已归档"));
        empty->findChild<PushButton*>("EmptyStateAction")->click();
        for(auto* chip:chips)QVERIFY(!chip->isChecked()); QCOMPARE(banner->tone(),FeedbackTone::Success);
        QVERIFY(summary->text().contains("0 项"));
        empty->findChild<PushButton*>("EmptyStateSecondaryAction")->click(); QCOMPARE(banner->tone(),FeedbackTone::Warning);
        chips.first()->findChild<QAbstractButton*>("FilterChipRemove")->click(); QVERIFY(chips.first()->isHidden());
        auto* nav=page.findChild<NavigationList*>("CommonDemoNavigation");
        auto* list=page.findChild<ListView*>("CommonDemoList");
        auto* table=page.findChild<TableView*>("CommonDemoTable");
        auto* tree=page.findChild<TreeTable*>("CommonDemoTree");
        QVERIFY(nav&&list&&table&&tree); QCOMPARE(nav->count(),6);
        nav->setCurrentButton(10); QVERIFY(page.findChild<QLabel*>("CommonDemoNavigationSelection")->text().contains("ID 10"));
        QCOMPARE(list->model()->rowCount(),3); QCOMPARE(table->model()->rowCount(),3);
        QCOMPARE(list->model()->parent(),static_cast<QObject*>(list)); QCOMPARE(table->model()->parent(),static_cast<QObject*>(table));
        QCOMPARE(tree->columnAggregation(2),TreeTable::Sum); QCOMPARE(tree->model()->rowCount(),2);
        auto* search=page.findChild<SearchEdit*>("CommonDemoSearch"); QVERIFY(search);
        search->setText("任务 B"); search->requestSearch(); QCOMPARE(list->model()->rowCount(),1);
        QCOMPARE(list->model()->index(0,0).data().toString(),QString("任务 B"));
        search->clear(); search->requestSearch(); QCOMPARE(list->model()->rowCount(),3);
        auto* status=page.findChild<QComboBox*>("CommonDemoConnectionChoice"); auto* badge=page.findChild<StatusBadge*>("CommonDemoConnectionStatus"); QVERIFY(status&&badge);
        status->setCurrentIndex(1); QCOMPARE(badge->tone(),FeedbackTone::Warning); QCOMPARE(badge->text(),QString("连接中"));
        badge->setDotVisible(false); QVERIFY(!badge->isDotVisible()); badge->setDotVisible(true);
        const QString badgeText="很长的状态描述，用于检查省略与无障碍原文"; badge->setText(badgeText); QCOMPARE(badge->accessibleName(),badgeText); QCOMPARE(badge->toolTip(),badgeText);
        status->setCurrentIndex(0); QCOMPARE(badge->tone(),FeedbackTone::Success);
        auto* advanced=page.findChild<CollapsibleSection*>("CommonDemoAdvancedSection"); QVERIFY(advanced); advanced->setExpanded(false); advanced->setExpanded(true);
        QCOMPARE(page.findChildren<QScrollArea*>().size(),1); // 页面共享一个外层滚动区
        auto* manager=StyleManagerAdapter::instance(); const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        for(const QString& theme:manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(int width:{1100,640,400}) {
                page.resize(width,700); QTest::qWait(20); QVERIFY(!render(page).isNull());
                QCOMPARE(page.width(),width);
                auto* scroll=page.findChild<QScrollArea*>("CommonDemoScroll"); QVERIFY(scroll);
                QCOMPARE(scroll->horizontalScrollBar()->maximum(),0);
                if(!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(page.grab().save(capture+QString("/common-%1-%2.png").arg(theme).arg(width))); }
            }
        }
        page.setLayoutDirection(Qt::RightToLeft); QFont large=page.font(); large.setPointSize(18); page.setFont(large);
        QTest::qWait(20); QVERIFY(!render(page).isNull()); QVERIFY(qApp->styleSheet().isEmpty());
    }
    void checkboxAndRadioInteraction()
    {
        QWidget host;
        QVBoxLayout layout(&host);
        CheckBox box("&Choice"); box.setTristate(true);
        RadioButton a("First"), b("Second");
        layout.addWidget(&box); layout.addWidget(&a); layout.addWidget(&b);
        host.show(); host.activateWindow(); box.setFocus();
        QTest::qWait(20);
        QTest::keyClick(&box, Qt::Key_Space);
        QCOMPARE(box.checkState(), Qt::PartiallyChecked);
        QTest::keyClick(&box, Qt::Key_Space);
        QCOMPARE(box.checkState(), Qt::Checked);
        QTest::keyClick(&box, Qt::Key_Space);
        QCOMPARE(box.checkState(), Qt::Unchecked);
        box.setLayoutDirection(Qt::RightToLeft);
        QStyleOptionButton opt; opt.initFrom(&box); opt.text = box.text();
        const QRect indicator = box.style()->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, &box);
        QVERIFY(indicator.center().x() > box.width()/2);
        QTest::mouseClick(&box, Qt::LeftButton, Qt::NoModifier, indicator.center());
        QCOMPARE(box.checkState(), Qt::PartiallyChecked);
        box.setEnabled(false); QTest::keyClick(&box, Qt::Key_Space);
        QCOMPARE(box.checkState(), Qt::PartiallyChecked);
        a.click(); QVERIFY(a.isChecked()); b.click(); QVERIFY(b.isChecked()); QVERIFY(!a.isChecked());
        // The public QSize API remains available and includes icon width.
        const QSize before = box.sizeHint();
        QPixmap icon(24, 16); icon.fill(Qt::red); box.setIcon(QIcon(icon));
        QVERIFY(box.sizeHint().width() > before.width());
    }
    void partialIndicatorIsDash()
    {
        QStyleOptionButton opt;
        opt.rect = QRect(0,0,16,16); opt.palette = qApp->palette();
        opt.state = QStyle::State_Enabled | QStyle::State_On | QStyle::State_NoChange;
        QImage partial(16,16,QImage::Format_ARGB32_Premultiplied); partial.fill(Qt::transparent);
        { QPainter p(&partial); qApp->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &p); }
        opt.state &= ~QStyle::State_NoChange;
        QImage checked(16,16,QImage::Format_ARGB32_Premultiplied); checked.fill(Qt::transparent);
        { QPainter p(&checked); qApp->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &p); }
        QVERIFY(partial != checked);
        QVERIFY(contrastRatio(partial.pixelColor(8,7), partial.pixelColor(8,4)) >= 3.0);
    }
    void buttonAndSwitchKeyboard()
    {
        PushButton button("&Apply"); button.setCheckable(true); button.show();
        QSignalSpy clicks(&button, &QPushButton::clicked);
        QTest::keyClick(&button, Qt::Key_Space);
        QCOMPARE(clicks.count(), 1); QVERIFY(button.isChecked());
        button.setEnabled(false); button.click();
        QCOMPARE(clicks.count(), 1);
        ToggleSwitch toggle; toggle.show(); toggle.setFocus();
        QTest::keyClick(&toggle, Qt::Key_Space);
        QVERIFY(toggle.isChecked());
        QTRY_VERIFY(toggle.property("offset").toReal() > .99);
        toggle.setChecked(false);
        QTRY_VERIFY(toggle.property("offset").toReal() < .01);
    }
    void sliderGeometryAndDrag_data()
    {
        QTest::addColumn<bool>("vertical"); QTest::addColumn<bool>("rtl"); QTest::addColumn<bool>("inverted");
        for (bool vertical : {false,true}) for (bool rtl : {false,true}) for (bool inverted : {false,true})
            QTest::newRow(qPrintable(QString("v%1-rtl%2-inv%3").arg(vertical).arg(rtl).arg(inverted)))
                << vertical << rtl << inverted;
    }
    void sliderGeometryAndDrag()
    {
        QFETCH(bool, vertical); QFETCH(bool, rtl); QFETCH(bool, inverted);
        TestSlider slider(vertical ? Qt::Vertical : Qt::Horizontal);
        slider.resize(vertical ? QSize(32,220) : QSize(220,32));
        slider.setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
        slider.setInvertedAppearance(inverted); slider.setRange(0,100); slider.show();
        for (int value : {0,50,100}) {
            slider.setValue(value); const auto opt = slider.option();
            const QRect handle = slider.style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, &slider);
            QVERIFY(slider.rect().contains(handle));
            QCOMPARE(slider.style()->hitTestComplexControl(QStyle::CC_Slider, &opt, handle.center(), &slider), QStyle::SC_SliderHandle);
        }
        slider.setValue(50); auto opt = slider.option();
        const QRect handle = slider.style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, &slider);
        const QPoint end = handle.center() + (vertical ? QPoint(0,30) : QPoint(30,0));
        QTest::mousePress(&slider, Qt::LeftButton, Qt::NoModifier, handle.center());
        QMouseEvent move(QEvent::MouseMove, end, slider.mapToGlobal(end), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&slider, &move);
        QTest::mouseRelease(&slider, Qt::LeftButton, Qt::NoModifier, end);
        QVERIFY(slider.value() != 50);
        QCOMPARE(slider.value() < 50, opt.upsideDown);
        slider.setValue(50); QTest::keyClick(&slider, Qt::Key_Right); QVERIFY(slider.value() != 50);
    }
    void progressDirectionsAndLimits()
    {
        TestProgress bar; bar.resize(200,24); bar.setTextVisible(false); bar.setRange(0,100); bar.setValue(25);
        const QColor fill = bar.palette().color(QPalette::Highlight);
        auto image = render(bar);
        QCOMPARE(image.pixelColor(12,12), fill); QVERIFY(image.pixelColor(180,12) != fill);
        bar.setInvertedAppearance(true); image = render(bar);
        QCOMPARE(image.pixelColor(180,12), fill); QVERIFY(image.pixelColor(12,12) != fill);
        bar.setLayoutDirection(Qt::RightToLeft); image = render(bar);
        QCOMPARE(image.pixelColor(12,12), fill);
        bar.setOrientation(Qt::Vertical); bar.resize(24,200); bar.setInvertedAppearance(false); image = render(bar);
        QCOMPARE(image.pixelColor(12,180), fill); QVERIFY(image.pixelColor(12,12) != fill);
        bar.setInvertedAppearance(true); image = render(bar); QCOMPARE(image.pixelColor(12,12), fill);
        bar.setOrientation(Qt::Horizontal); bar.resize(200,24); bar.setInvertedAppearance(false); bar.setLayoutDirection(Qt::LeftToRight);
        bar.setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()); bar.setValue(0);
        image = render(bar); QCOMPARE(image.pixelColor(12,12), fill); QVERIFY(image.pixelColor(180,12) != fill);
        bar.reset(); image = render(bar); QVERIFY(image.pixelColor(12,12) != fill);
        bar.setRange(10,110); bar.setValue(60); bar.setFormat("%v/%m (%p%)");
        QCOMPARE(bar.text(), QString("60/100 (50%)"));
    }
    void busyIndicatorKeepsExplicitColor()
    {
        BusyIndicator indicator; indicator.stop(); indicator.resize(24,24);
        const QColor custom("#ff0080"); indicator.setColor(custom);
        QCOMPARE(render(indicator).pixelColor(18,12), custom);
        auto* mgr = StyleManagerAdapter::instance();
        QVERIFY(mgr->setCurrentTheme("light")); QVERIFY(mgr->updateTheme());
        QCOMPARE(render(indicator).pixelColor(18,12), custom);
        QVERIFY(mgr->setCurrentTheme("dark")); QVERIFY(mgr->updateTheme());
    }
    void busyProgressAnimates()
    {
        ProgressBar bar; bar.setRange(0,0); bar.resize(220,24); bar.show();
        QTest::qWait(50); const QImage before = render(bar);
        QTest::qWait(180); QVERIFY(render(bar) != before);
        bar.setRange(0,100); bar.setValue(30);
    }
    void complexControlsKeepInput()
    {
        QWidget host; QVBoxLayout layout(&host);
        QLineEdit line; QComboBox combo; QSpinBox spin; QToolButton tool;
        QMenu menu; menu.addAction("Action"); tool.setMenu(&menu); tool.setPopupMode(QToolButton::MenuButtonPopup);
        combo.addItems({"One","Two","Three"}); spin.setRange(0,9);
        layout.addWidget(&line); layout.addWidget(&combo); layout.addWidget(&spin); layout.addWidget(&tool);
        host.show(); QTest::keyClicks(&line, "input"); QCOMPARE(line.text(), QString("input"));
        QTest::keyClick(&combo, Qt::Key_Down); QCOMPARE(combo.currentIndex(),1);
        QTest::keyClick(&spin, Qt::Key_Up); QCOMPARE(spin.value(),1);
        QStyleOptionSpinBox so; so.initFrom(&spin); so.subControls = QStyle::SC_All;
        so.buttonSymbols = spin.buttonSymbols(); so.stepEnabled = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
        QRect up = spin.style()->subControlRect(QStyle::CC_SpinBox,&so,QStyle::SC_SpinBoxUp,&spin);
        QTest::mouseClick(&spin,Qt::LeftButton,Qt::NoModifier,up.center()); QCOMPARE(spin.value(),2);
        combo.showPopup(); QVERIFY(combo.view()->isVisible()); combo.hidePopup();
        QSignalSpy clicked(&tool,&QToolButton::clicked);
        QTest::mouseClick(&tool,Qt::LeftButton,Qt::NoModifier,QPoint(4,tool.height()/2)); QCOMPARE(clicked.count(),1);
        bool menuShown = false;
        QTimer::singleShot(50, &menu, [&] { menuShown = menu.isVisible(); menu.close(); });
        QTest::mouseClick(&tool,Qt::LeftButton,Qt::NoModifier,QPoint(tool.width()-3,tool.height()/2));
        QVERIFY(menuShown); QCOMPARE(clicked.count(),1);
        for (QWidget* child : host.findChildren<QWidget*>()) QVERIFY(child->styleSheet().isEmpty());
    }
    void scrollBarGeometryAndButtons()
    {
        for (Qt::Orientation orientation : {Qt::Horizontal, Qt::Vertical}) {
            QScrollBar bar(orientation); bar.setRange(0,100); bar.setPageStep(20); bar.setValue(50);
            bar.resize(orientation == Qt::Horizontal ? QSize(240,14) : QSize(14,240)); bar.show();
            QStyleOptionSlider opt; opt.initFrom(&bar); opt.orientation = orientation;
            opt.minimum = 0; opt.maximum = 100; opt.sliderPosition = 50; opt.sliderValue = 50;
            opt.pageStep = 20; opt.subControls = QStyle::SC_All;
            const QRect thumb = bar.style()->subControlRect(QStyle::CC_ScrollBar,&opt,QStyle::SC_ScrollBarSlider,&bar);
            QCOMPARE(bar.style()->hitTestComplexControl(QStyle::CC_ScrollBar,&opt,thumb.center(),&bar),QStyle::SC_ScrollBarSlider);
            const QImage image = render(bar);
            QCOMPARE(image.pixelColor(thumb.center()),bar.palette().color(QPalette::Mid));
            const QRect add = bar.style()->subControlRect(QStyle::CC_ScrollBar,&opt,QStyle::SC_ScrollBarAddLine,&bar);
            QTest::mouseClick(&bar,Qt::LeftButton,Qt::NoModifier,add.center()); QVERIFY(bar.value() > 50);
        }
    }
    void cleanThemeBundle()
    {
        const QString path = QStringLiteral(BASEWIDGETS_GENERATED_RCC);
        QVERIFY(QResource::registerResource(path,"/bundle-check"));
        QDirIterator css(":/bundle-check/styles", {"*.css","*.qss","*.css.template"}, QDir::Files, QDirIterator::Subdirectories);
        QVERIFY(!css.hasNext());
        QVERIFY(QFile::exists(":/bundle-check/styles/themes/themes/dark.xml"));
        QVERIFY(QResource::unregisterResource(path,"/bundle-check"));
    }
    void responsiveCards()
    {
        QScrollArea area; area.setWidgetResizable(true);
        auto* content = new QWidget;
        auto* grid = new ResponsiveGridLayout(content,260,2);
        for (int i=0;i<4;++i) {
            auto* card = new QGroupBox(QString("Card %1").arg(i));
            auto* layout = new QVBoxLayout(card);
            auto* text = new QLabel("A wrapping paragraph demonstrates height-for-width when cards change columns.");
            text->setWordWrap(true); layout->addWidget(text);
            grid->addWidget(card);
        }
        area.setWidget(content); area.resize(900,500); area.show(); QTest::qWait(30);
        QVERIFY(grid->itemAt(0)->geometry().x() < grid->itemAt(1)->geometry().x());
        QCOMPARE(grid->itemAt(0)->geometry().y(),grid->itemAt(1)->geometry().y());
        area.resize(420,500); QTest::qWait(30);
        QVERIFY(grid->itemAt(0)->geometry().bottom() < grid->itemAt(1)->geometry().top());
        for (int i=0;i<grid->count();++i) QVERIFY(content->rect().contains(grid->itemAt(i)->geometry()));
        area.resize(900,500); content->setLayoutDirection(Qt::RightToLeft); QTest::qWait(30);
        QVERIFY(grid->itemAt(0)->geometry().x() > grid->itemAt(1)->geometry().x());
    }
    void mainWindowLayoutAndOwnership()
    {
        FramelessWidgetWindow window;
        window.setTitle(QString(200,'x'));
        auto* content = new QLabel("Content"); window.setContentWidget(content);
        auto* menu = new QMenuBar; menu->addMenu("File"); window.setMenuBar(menu);
        auto* toolbar = new QToolBar; toolbar->addAction("Action"); window.setToolBar(toolbar);
        auto* status = new QStatusBar; status->showMessage("Ready"); window.setStatusBar(status);
        window.resize(800,600); window.show(); QTest::qWait(30);
        QCOMPARE(window.contentWidget(),content);
        QCOMPARE(window.windowTitle(),QString(200,'x'));
        QCOMPARE(window.toolBarArea(toolbar),Qt::TopToolBarArea);
        QCOMPARE(static_cast<QMainWindow*>(&window)->statusBar(),status);
        QVERIFY(window.width() < 1000); // the long title must not force the main window wider
        QVERIFY(toolbar->mapTo(&window,QPoint()).y() >= menu->mapTo(&window,QPoint()).y()+menu->height());
        QVERIFY(content->mapTo(&window,QPoint()).y()+content->height() <= status->mapTo(&window,QPoint()).y());
        QPointer<QWidget> old = content;
        auto* replacement = new QLabel("Replacement"); window.setContentWidget(replacement);
        QVERIFY(old->isHidden()); QCOMPARE(window.contentWidget(),replacement);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(old.isNull());
        window.setMenuBar(nullptr); window.setToolBar(nullptr); window.setStatusBar(nullptr);
        QVERIFY(!window.menuBar()); QVERIFY(!window.toolBar()); QVERIFY(!window.statusBar());
        window.setResizableEdgesEnabled(false); window.showMaximized(); window.activateWindow();
        QTest::qWait(30); window.showFullScreen(); QTest::qWait(30);
        QVERIFY(window.isFullScreen());
        QTest::keyClick(&window,Qt::Key_F11); QTest::qWait(30);
        QVERIFY(window.isMaximized()); QVERIFY(!window.isResizeable());
    }
    void dialogContentAndTitleButtons()
    {
        FramelessDialog dialog;
        auto* first = new QLabel("First"); dialog.setContentWidget(first);
        QPointer<QWidget> old = first;
        auto* second = new QLabel("Second"); dialog.setContentWidget(second);
        QCOMPARE(dialog.contentWidget(),second); QVERIFY(old->isHidden());
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete); QVERIFY(old.isNull());
        QCOMPARE(second->parentWidget()->layout()->count(),1);
        dialog.setTitle("Dialog title"); QCOMPARE(dialog.windowTitle(),QString("Dialog title"));
        auto* title = dialog.findChild<CustomTitleBar*>(); QVERIFY(title);
        title->setButtonsVisible(false,false,true); title->updateForWindowState();
        QVERIFY(title->findChild<QToolButton*>("TitleBarMinButton")->isHidden());
        QVERIFY(title->findChild<QToolButton*>("TitleBarMaxButton")->isHidden());
        dialog.setContentWidget(nullptr); QVERIFY(!dialog.contentWidget());
    }
    void titleIconDpiAndAspectRatio()
    {
        CustomTitleBar title; title.resize(600,48); title.setTitle("水声数据管理"); title.setSettingsVisible(true); title.setUserVisible(true);
        title.setUserInfo("演示用户"); title.show(); QCoreApplication::processEvents();
        auto* label=title.findChild<QLabel*>("TitleIcon"); QVERIFY(label);
        QCOMPARE(label->size(),QSize(20,20)); QCOMPARE(title.settingsButton()->iconSize(),QSize(18,18)); QCOMPARE(title.userButton()->iconSize(),QSize(22,22));
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        for(qreal scale:{1.,1.25,1.5,2.}) {
            QImage image(QSize(qRound(title.width()*scale),qRound(title.height()*scale)),QImage::Format_ARGB32_Premultiplied);
            image.setDevicePixelRatio(scale); image.fill(Qt::transparent); title.render(&image);
            if(!directory.isEmpty()) { QDir().mkpath(directory); QVERIFY(image.save(QString("%1/title-vector-%2.png").arg(directory).arg(scale))); }
            QImage mark(QSize(qRound(20*scale),qRound(20*scale)),QImage::Format_ARGB32_Premultiplied);
            mark.setDevicePixelRatio(scale); mark.fill(Qt::transparent); label->render(&mark);
            QRect blue;
            for(int y=0;y<mark.height();++y)for(int x=0;x<mark.width();++x) {
                const auto color=mark.pixelColor(x,y);
                if(color.blue()>color.red()+50 && color.blue()>color.green()+20)blue=blue.united(QRect(x,y,1,1));
            }
            QVERIFY(!blue.isEmpty()); QVERIFY(qAbs(blue.center().x()-mark.rect().center().x())<=1);
            QVERIFY(qAbs(blue.center().y()-mark.rect().center().y())<=1);
            QVERIFY(blue.width()>16*scale); QVERIFY(blue.height()>16*scale);
        }
        // A non-square application icon must fit rather than stretch/crop, even at DPR=2.
        QPixmap wide(80,40); wide.fill(Qt::red); title.setIcon(QIcon(wide));
        QImage image(40,40,QImage::Format_ARGB32_Premultiplied); image.setDevicePixelRatio(2); image.fill(Qt::transparent); label->render(&image);
        QRect red;
        for(int y=0;y<40;++y)for(int x=0;x<40;++x) {
            const auto color=image.pixelColor(x,y); if(color.red()>200&&color.green()<50)red=red.united(QRect(x,y,1,1));
        }
        QVERIFY(qAbs(red.width()-2*red.height())<=2); QVERIFY(qAbs(red.center().y()-19)<=1);
        for(const auto& icon:{"settings","search","more","user"})
            QVERIFY(QFile::exists(QString(":/styles/themes/common/icons/titlebar/%1.svg").arg(icon)));
    }
    void screenWorkspaceRolesAndSharedModel()
    {
        ScreenWindow primary; primary.setTitle("主控"); primary.resize(700,500); primary.show();
        MultiScreenWorkspace workspace(&primary);
        QStandardItemModel model(3,2); model.setData(model.index(0,0),"shared recording");
        int created=0;
        connect(&workspace,&MultiScreenWorkspace::windowCreated,&primary,[&](const QString&,ScreenWindow* window) {
            ++created; auto* table=new AdvancedTable; table->setSourceModel(&model); window->setContentWidget(table);
            window->setContextText("采集源 A / 子屏独立状态");
        });
        QVERIFY(!workspace.openSecondary("",primary.screen())); QVERIFY(!workspace.openSecondary("invalid",nullptr));
        auto* direction=workspace.openSecondary("direction",primary.screen(),"测向子屏");
        auto* data=workspace.openSecondary("data",primary.screen(),"数据管理子屏");
        QVERIFY(direction&&data); QCOMPARE(created,2); QVERIFY(direction!=data);
        QVERIFY(!direction->parentWidget()); QVERIFY(!direction->testAttribute(Qt::WA_QuitOnClose));
        QCOMPARE(primary.role(),ScreenWindow::Role::Primary); QCOMPARE(direction->role(),ScreenWindow::Role::Secondary);
        QCOMPARE(direction->titleBar()->profile(),CustomTitleBar::Profile::SecondaryWindow);
        QCOMPARE(direction->titleBar()->height(),qMax(40,direction->titleBar()->fontMetrics().height()+20)); QCOMPARE(primary.titleBar()->height(),qMax(48,primary.titleBar()->fontMetrics().height()+20));
        QVERIFY(direction->titleBar()->settingsButton()->isHidden()); QVERIFY(direction->titleBar()->userButton()->isHidden());
        QVERIFY(!direction->titleBar()->findChild<QToolButton*>("TitleBarMaxButton")->isHidden());
        QCOMPARE(direction->statusBar()->objectName(),QString("SecondaryScreenStatusBar"));
        QCOMPARE(primary.statusBar()->objectName(),QString("PrimaryScreenStatusBar"));
        QVERIFY(direction->findChild<QToolButton*>("ReturnToPrimaryButton"));
        auto* first=qobject_cast<AdvancedTable*>(direction->contentWidget()); auto* second=qobject_cast<AdvancedTable*>(data->contentWidget());
        QVERIFY(first&&second); QCOMPARE(first->sourceModel(),second->sourceModel());
        model.setData(model.index(0,0),"updated across screens");
        QCOMPARE(first->pager()->index(0,0).data().toString(),QString("updated across screens"));
        QCOMPARE(second->pager()->index(0,0).data().toString(),QString("updated across screens"));
        primary.showMinimized(); QCoreApplication::processEvents(); QVERIFY(direction->isVisible()); QVERIFY(!direction->isMinimized());
        direction->findChild<QToolButton*>("ReturnToPrimaryButton")->click(); QVERIFY(!primary.isMinimized());
        workspace.closeSecondary("direction"); QVERIFY(!direction->isVisible()); QVERIFY(data->isVisible());
        QCOMPARE(workspace.openSecondary("direction",primary.screen()),direction); QCOMPARE(created,2); QVERIFY(direction->isVisible());
        QVERIFY(!workspace.setPrimaryScreen(nullptr)); QVERIFY(workspace.setPrimaryScreen(primary.screen()));
        primary.close(); QVERIFY(!direction->isVisible()); QVERIFY(!data->isVisible());
        QVERIFY(!workspace.openSecondary("after shutdown",primary.screen()));
    }
    void screenWorkspaceTopologyAndOwnership()
    {
        QCOMPARE(MultiScreenWorkspace::secondaryScreenIndexes(1,0),QList<int>());
        QCOMPARE(MultiScreenWorkspace::secondaryScreenIndexes(2,0),QList<int>({1}));
        QCOMPARE(MultiScreenWorkspace::secondaryScreenIndexes(3,1),QList<int>({0,2}));
        QCOMPARE(MultiScreenWorkspace::secondaryScreenIndexes(3,2),QList<int>({0,1}));
        QVERIFY(MultiScreenWorkspace::secondaryScreenIndexes(3,-1).isEmpty());
        ScreenWindow primary;
        auto* workspace=new MultiScreenWorkspace(&primary);
        QPointer<ScreenWindow> a=workspace->openSecondary("view-a",primary.screen());
        QPointer<ScreenWindow> b=workspace->openSecondary("view-b",primary.screen());
        QVERIFY(a&&b); QVERIFY(!MultiScreenWorkspace::screenKey(primary.screen()).isEmpty());
        QCOMPARE(workspace->targetScreen("view-a"),primary.screen());
        workspace->setAutomaticSecondaryWindows(true); QVERIFY(workspace->automaticSecondaryWindows());
        workspace->setAutomaticSecondaryWindows(false); QVERIFY(a->isVisible()); // manually assigned views remain open
        delete a; QVERIFY(!workspace->secondaryIds().contains("view-a"));
        delete workspace; QVERIFY(b.isNull()); // controller owns top-level child-screen lifetimes
        // A window-created hook may reject/delete a window without leaving a dangling entry.
        MultiScreenWorkspace rejecting(&primary);
        connect(&rejecting,&MultiScreenWorkspace::windowCreated,&primary,[](const QString&,ScreenWindow* window) { delete window; });
        QVERIFY(!rejecting.openSecondary("rejected",primary.screen())); QVERIFY(rejecting.secondaryIds().isEmpty());
    }
    void screenWorkspaceSimulatedDisconnect()
    {
        // Exercise only controller signal handling; QGuiApplication's real screen inventory
        // is unchanged. This does not claim a physical hotplug or compositor test.
        ScreenWindow primary; MultiScreenWorkspace workspace(&primary);
        auto* screen=primary.screen(); auto* child=workspace.openSecondary("preview",screen); QVERIFY(child);
        QSignalSpy available(&workspace,&MultiScreenWorkspace::secondaryAvailabilityChanged);
        QVERIFY(QMetaObject::invokeMethod(qGuiApp,"screenRemoved",Qt::DirectConnection,Q_ARG(QScreen*,screen)));
        QVERIFY(!child->isVisible()); QVERIFY(workspace.isSuspended("preview"));
        workspace.synchronizeScreens(); QVERIFY(child->isVisible()); QVERIFY(!workspace.isSuspended("preview"));
        QVERIFY(available.count()>=2);
        workspace.closeSecondary("preview");
        QVERIFY(QMetaObject::invokeMethod(qGuiApp,"screenRemoved",Qt::DirectConnection,Q_ARG(QScreen*,screen)));
        workspace.synchronizeScreens(); QVERIFY(!child->isVisible()); QVERIFY(!workspace.isSuspended("preview"));
    }
    void screenRoleVisuals()
    {
        ScreenWindow primary; ScreenWindow secondary(ScreenWindow::Role::Secondary);
        primary.setTitle("水声数据管理 · 主控"); primary.titleBar()->setSearchVisible(true); primary.titleBar()->setSettingsVisible(true); primary.titleBar()->setUserVisible(true); primary.titleBar()->setUserInfo("演示用户");
        secondary.setTitle("测向分析 · 子屏 2");
        primary.setContextText("主控会话 / 全局任务状态"); secondary.setContextText("测站 A · 测向视图");
        for(auto* window:{&primary,&secondary}) {
            auto* content=new QWidget; auto* layout=new QVBoxLayout(content);
            auto* checked=new CheckBox("选中"); checked->setChecked(true); auto* half=new CheckBox("半选"); half->setTristate(true); half->setCheckState(Qt::PartiallyChecked);
            auto* radio=new RadioButton("选项 A"); radio->setChecked(true); auto* unchecked=new RadioButton("选项 B");
            layout->addWidget(checked); layout->addWidget(half); layout->addWidget(radio); layout->addWidget(unchecked); layout->addStretch();
            window->setContentWidget(content); window->resize(900,360);
        }
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        for(const QString theme:{QString("dark"),QString("light")}) {
            auto* manager=StyleManagerAdapter::instance(); QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(auto* window:{&primary,&secondary}) {
                window->show(); QCoreApplication::processEvents();
                for(qreal dpr:{1.,1.25,1.5,2.}) {
                    QImage image(QSize(qRound(window->width()*dpr),qRound(window->height()*dpr)),QImage::Format_ARGB32_Premultiplied);
                    image.setDevicePixelRatio(dpr); image.fill(Qt::transparent); window->render(&image);
                    if(!directory.isEmpty()) {
                        QDir().mkpath(directory);
                        QVERIFY(image.save(QString("%1/ScreenRole-%2-%3-%4.png").arg(directory,theme).arg(int(window->role())).arg(dpr)));
                    }
                }
            }
        }
    }
    void titleBarIdentityAndProfiles()
    {
        QWidget window,other;
        window.setWindowTitle("Window identity"); other.setWindowTitle("Second identity");
        QPixmap red(24,24); red.fill(Qt::red); QPixmap blue(24,24); blue.fill(Qt::blue);
        const QIcon windowIcon(red),overrideIcon(blue); window.setWindowIcon(windowIcon);
        CustomTitleBar title;
        title.attachToWindow(&window); QCOMPARE(title.title(),QString("Window identity")); QCOMPARE(title.icon().cacheKey(),windowIcon.cacheKey());
        title.setTitle("Independent title"); title.setIcon(overrideIcon);
        window.setWindowTitle("Changed window"); window.setWindowIcon(QIcon()); title.updateIcons();
        QCOMPARE(title.title(),QString("Independent title")); QCOMPARE(title.icon().cacheKey(),overrideIcon.cacheKey());
        QCOMPARE(window.windowTitle(),QString("Changed window"));
        title.attachToWindow(&other); QCOMPARE(title.title(),QString("Independent title"));
        title.resetTitle(); QCOMPARE(title.title(),QString("Second identity"));
        window.setWindowTitle("Old window must not affect new binding"); QCOMPARE(title.title(),QString("Second identity"));
        title.setTitle(""); title.setIcon(QIcon()); QVERIFY(title.title().isEmpty()); QVERIFY(title.icon().isNull());
        title.resetTitle(); title.resetIcon(); QVERIFY(!title.icon().isNull());
        title.setSearchVisible(true); title.setSettingsVisible(true); title.setUserVisible(true);
        title.setProfile(CustomTitleBar::Profile::Dialog);
        QCOMPARE(title.profile(),CustomTitleBar::Profile::Dialog); QVERIFY(title.icon().isNull());
        QVERIFY(title.settingsButton()->isHidden()); QVERIFY(title.userButton()->isHidden()); QVERIFY(title.searchEdit()->isHidden());
        QVERIFY(title.findChild<QToolButton*>("TitleBarMaxButton")->isHidden()); QVERIFY(!title.doubleClickToToggleEnabled());
        title.setIcon(overrideIcon); QCOMPARE(title.icon().cacheKey(),overrideIcon.cacheKey());
        title.setProfile(CustomTitleBar::Profile::MainWindow); QVERIFY(title.doubleClickToToggleEnabled());
        QVERIFY(!title.settingsButton()->isHidden());
        title.attachToWindow(nullptr); title.resetTitle(); QVERIFY(!title.title().isEmpty());
        FramelessWidgetWindow main; FramelessDialog dialog;
        QCOMPARE(main.titleBar()->profile(),CustomTitleBar::Profile::MainWindow);
        QCOMPARE(dialog.titleBar()->profile(),CustomTitleBar::Profile::Dialog);
    }
    void titleBarLayoutAndInteraction()
    {
        QWidget window;
        auto* layout=new QVBoxLayout(&window); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0);
        auto* title=new CustomTitleBar(&window); title->attachToWindow(&window); title->setTitle("水声数据管理中心");
        title->setSearchVisible(true); title->setSettingsVisible(true); title->setUserVisible(true);
        title->setNotificationsVisible(true); title->notificationButton()->setUnreadCount(8);
        title->setUserInfo("演示用户",QPixmap(),"只读分析员"); layout->addWidget(title); layout->addStretch();
        auto* task=new QAction("任务中心",&window); title->addTitleAction(task);
        window.resize(1100,240); window.show(); QTest::qWait(15);
        QVERIFY(!title->searchEdit()->isHidden());
        auto* label=title->findChild<QLabel*>("TitleLabel"); QVERIFY(label);
        auto verifyNoOverlap=[&] {
            const auto children=title->findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly);
            QList<QWidget*> visible;
            for(auto* child:children)if(!child->isWindow()&&!child->isHidden()&&child->width()>0) {
                QVERIFY2(title->rect().contains(child->geometry()),qPrintable(child->objectName())); visible.append(child);
            }
            for(int i=0;i<visible.size();++i)for(int j=i+1;j<visible.size();++j)
                QVERIFY2(!visible[i]->geometry().intersects(visible[j]->geometry()),qPrintable(visible[i]->objectName()+" intersects "+visible[j]->objectName()));
        };
        verifyNoOverlap();
        QSignalSpy settings(title,&CustomTitleBar::settingsRequested),users(title,&CustomTitleBar::userRequested);
        title->settingsButton()->click(); title->userButton()->click(); QCOMPARE(settings.count(),1); QCOMPARE(users.count(),1);
        QVERIFY(!title->isDraggableAt(title->settingsButton()->geometry().center()));
        QVERIFY(!title->isDraggableAt(title->searchEdit()->geometry().center()));
        QVERIFY(title->isDraggableAt(label->geometry().center()));
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        for(auto alignment:{CustomTitleBar::TitleAlignment::Left,CustomTitleBar::TitleAlignment::Center}) {
            title->setTitleAlignment(alignment);
            for(int width:{1100,640,380,240}) {
                window.resize(width,240); QApplication::processEvents(); verifyNoOverlap();
                if(alignment==CustomTitleBar::TitleAlignment::Center && width>=640)
                    QVERIFY(qAbs(label->geometry().center().x()-title->rect().center().x())<=1);
                if(!directory.isEmpty()) {
                    QDir().mkpath(directory);
                    QVERIFY(render(*title).save(QString("%1/TitleBar-%2-%3.png").arg(directory).arg(int(alignment)).arg(width)));
                }
            }
        }
        window.resize(640,240); title->setTitleAlignment(CustomTitleBar::TitleAlignment::Left); QApplication::processEvents();
        auto* more=title->findChild<QToolButton*>("TitleBarMoreButton"); QVERIFY(more); QVERIFY(more->menu()->actions().contains(task));
        QSignalSpy taskTriggered(task,&QAction::triggered); task->trigger(); QCOMPARE(taskTriggered.count(),1);
        task->setVisible(false); QVERIFY(!more->menu()->actions().contains(task));
        task->setVisible(true); QVERIFY(more->menu()->actions().contains(task));
        delete task; QVERIFY(more->isHidden());
        QSignalSpy messages(title,&CustomTitleBar::notificationsRequested); title->notificationButton()->click(); QCOMPARE(messages.count(),1);
        window.resize(240,240); QApplication::processEvents();
        QAction* overflowMessage=nullptr; for(auto* action:more->menu()->actions())if(action->text().contains("消息中心"))overflowMessage=action;
        QVERIFY(overflowMessage); QVERIFY(overflowMessage->text().contains("8")); overflowMessage->trigger(); QCOMPARE(messages.count(),2);
        title->notificationButton()->setEnabled(false); QVERIFY(!overflowMessage->isEnabled()); title->notificationButton()->setEnabled(true); QVERIFY(overflowMessage->isEnabled());
        title->setNotificationsVisible(false); for(auto* action:more->menu()->actions())QVERIFY(!action->text().contains("消息中心"));
        window.resize(640,240); QApplication::processEvents();
        QFont large=title->font(); large.setPixelSize(28); title->setFont(large); QApplication::processEvents();
        QVERIFY(title->height()>=title->fontMetrics().height()+20); verifyNoOverlap();
        title->setLayoutDirection(Qt::RightToLeft); QApplication::processEvents(); verifyNoOverlap();
    }
    void titleBarLeftActionsAndCenteredIdentity()
    {
        QWidget window; auto* layout=new QVBoxLayout(&window); layout->setContentsMargins(0,0,0,0);
        auto* bar=new CustomTitleBar(&window); layout->addWidget(bar); layout->addStretch(); bar->attachToWindow(&window);
        bar->setTitle("水声数据管理中心"); bar->setTitleAlignment(CustomTitleBar::TitleAlignment::Center);
        bar->setIconPlacement(CustomTitleBar::IconPlacement::WithTitle);
        bar->setSearchVisible(true); bar->setSettingsVisible(true); bar->setUserVisible(true);
        bar->setUserInfo("分析员");
        auto* first=new QAction("算法启动",&window); first->setCheckable(true);
        auto* second=new QAction("采集配置",&window); auto* menu=new QMenu(&window); menu->addAction("本机"); second->setMenu(menu);
        auto* right=new QAction("任务",&window); bar->addTitleAction(right);
        bar->addTitleAction(first,CustomTitleBar::ActionPosition::Left);
        bar->addTitleAction(second,CustomTitleBar::ActionPosition::Left);
        bar->addTitleAction(first,CustomTitleBar::ActionPosition::Left);
        auto buttonFor=[&](QAction* action)->QToolButton* {
            for(auto* button:bar->findChildren<QToolButton*>("TitleBarCustomAction"))if(button->defaultAction()==action)return button;
            return nullptr;
        };
        QCOMPARE(bar->findChildren<QToolButton*>("TitleBarCustomAction").size(),3);
        auto* firstButton=buttonFor(first); auto* secondButton=buttonFor(second); QVERIFY(firstButton&&secondButton);
        auto* label=bar->findChild<QLabel*>("TitleLabel"); auto* icon=bar->findChild<QLabel*>("TitleIcon");
        auto* more=bar->findChild<QToolButton*>("TitleBarMoreButton");
        auto verifyLayout=[&] {
            QList<QWidget*> visible;
            for(auto* child:bar->findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly))
                if(!child->isWindow()&&!child->isHidden()&&child->width()>0) {
                    QVERIFY2(bar->rect().contains(child->geometry()),qPrintable(child->objectName())); visible.append(child);
                }
            for(int i=0;i<visible.size();++i)for(int j=i+1;j<visible.size();++j)
                QVERIFY2(!visible[i]->geometry().intersects(visible[j]->geometry()),qPrintable(visible[i]->objectName()+" / "+visible[j]->objectName()));
        };
        window.resize(1400,240); window.show(); QTest::qWait(10); verifyLayout();
        QVERIFY(firstButton->isVisible()&&secondButton->isVisible());
        QVERIFY(firstButton->geometry().right()<secondButton->x());
        QVERIFY(secondButton->geometry().right()<icon->x());
        QVERIFY(qAbs(icon->geometry().united(label->geometry()).center().x()-bar->rect().center().x())<=1);
        QVERIFY(!bar->isDraggableAt(firstButton->geometry().center()));
        QVERIFY(bar->isDraggableAt(icon->geometry().center()));
        QSignalSpy triggered(first,&QAction::triggered); QTest::mouseClick(firstButton,Qt::LeftButton);
        QCOMPARE(triggered.count(),1); QVERIFY(first->isChecked());
        firstButton->setFocus(); QTest::keyClick(firstButton,Qt::Key_Space);
        QCOMPARE(triggered.count(),2); QVERIFY(!first->isChecked()); QVERIFY(firstButton->hasFocus());
        first->setEnabled(false); QVERIFY(!firstButton->isEnabled()); first->setEnabled(true);
        for(int width:{1100,640,500,380,240,220}) {
            window.resize(width,240); QApplication::processEvents(); verifyLayout();
            if(width>=640)QVERIFY(qAbs(icon->geometry().united(label->geometry()).center().x()-bar->rect().center().x())<=1);
        }
        QVERIFY(more->menu()->actions().contains(first)); QVERIFY(more->menu()->actions().contains(second));
        QCOMPARE(second->menu(),menu); QCOMPARE(more->menu()->actions().count(first),1);
        first->setVisible(false); QVERIFY(!more->menu()->actions().contains(first));
        first->setVisible(true); QVERIFY(more->menu()->actions().contains(first));
        // Removing or destroying a left action must leave the caller's other actions intact.
        QPointer<QAction> alive=second; bar->removeTitleAction(second); QVERIFY(alive); QVERIFY(!more->menu()->actions().contains(second));
        delete first; QVERIFY(!more->menu()->actions().contains(first));
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        bar->addTitleAction(second,CustomTitleBar::ActionPosition::Left);
        bar->setProfile(CustomTitleBar::Profile::Dialog); QVERIFY(buttonFor(second)->isHidden()); QVERIFY(more->isHidden());
        bar->setProfile(CustomTitleBar::Profile::MainWindow); window.resize(1400,240); QApplication::processEvents();
        QVERIFY(buttonFor(second)->isVisible());
        bar->addTitleAction(second,CustomTitleBar::ActionPosition::Right); QVERIFY(buttonFor(second)->isHidden()); // centered compact mode
        bar->addTitleAction(second,CustomTitleBar::ActionPosition::Left); QVERIFY(buttonFor(second)->isVisible());
        QFont large=bar->font(); large.setPixelSize(28); bar->setFont(large);
        for(auto direction:{Qt::LeftToRight,Qt::RightToLeft})for(int width:{1100,640,220}) {
            bar->setLayoutDirection(direction); window.resize(width,240); QApplication::processEvents(); verifyLayout();
            bar->setTitleVisible(false); verifyLayout(); bar->setTitleVisible(true);
            bar->setIconVisible(false); verifyLayout(); bar->setIconVisible(true);
        }
    }
    void dashboardStorageSnapshotAndUnits()
    {
        StorageDonutMeter meter;
        QVERIFY(!meter.capacityAvailable()); QVERIFY(std::isnan(meter.percent())); QCOMPARE(meter.usedText(),QString::fromUtf8("—"));
        meter.setUnitSystem(StorageDonutMeter::UnitSystem::SI);
        QVERIFY(meter.setCapacity(200000000000000ULL,2000000000000000ULL)); QCOMPARE(meter.percent(),10.);
        QVERIFY(meter.usedText().endsWith(" TB")); QVERIFY(meter.totalText().endsWith(" PB"));
        QVERIFY(!meter.setCapacity(3,2)); QCOMPARE(meter.usedBytes(),200000000000000ULL);
        meter.setUnitSystem(StorageDonutMeter::UnitSystem::IEC); QVERIFY(meter.usedText().endsWith(" TiB"));
        const auto maximum=std::numeric_limits<quint64>::max(); QVERIFY(meter.setCapacity(maximum,maximum)); QCOMPARE(meter.percent(),100.);
        QVERIFY(meter.setCapacity(0,100)); QCOMPARE(meter.percent(),0.); QVERIFY(meter.capacityAvailable());
        QVERIFY(meter.setCapacity(0,0)); QVERIFY(!meter.capacityAvailable()); QVERIFY(meter.accessibleName().contains("未知"));
    }
    void dashboardPanelAndPageIntegration()
    {
        DashboardPanel panel("<b>纯文本</b>"); auto* label=new QLabel("Content"); panel.contentLayout()->addWidget(label);
        QCOMPARE(panel.title(),QString("<b>纯文本</b>")); QCOMPARE(label->parentWidget(),static_cast<QWidget*>(&panel));
        panel.setTitle(QString()); QVERIFY(panel.title().isEmpty());
        DashboardPage page; QSignalSpy navigation(&page,&DashboardPage::pageRequested);
        page.resize(1400,850); page.show(); QApplication::processEvents();
        auto* records=page.findChild<QPushButton*>("DashboardOpenRecords"); QVERIFY(records); records->click(); QCOMPARE(navigation.takeFirst().first().toInt(),int(DemoPage::Records));
        auto* plot=page.findChild<RateHistoryPlot*>("DashboardPacketLoss"); QVERIFY(plot); QCOMPARE(plot->storedPointCount(),25);
        QCOMPARE(plot->resetXRange().upper,86400.); QCOMPARE(plot->resetYRange().upper,5.);
        plot->setViewRanges(QCPRange(3600,7200),QCPRange(1,2)); plot->resetView(); QCOMPARE(plot->xAxis->range().upper,86400.);
        QScopedPointer<QMenu> menu(plot->createContextMenu(QPoint(50,50)));
        bool found=false; for(auto* action:menu->actions())if(action->text()=="查看数据监控") { action->trigger(); found=true; }
        QVERIFY(found); QCOMPARE(navigation.takeFirst().first().toInt(),int(DemoPage::Monitoring));
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        auto* manager=StyleManagerAdapter::instance();
        for(const auto& theme:manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(int width:{1600,1100,640}) {
                page.resize(width,850); QApplication::processEvents(); QCOMPARE(page.width(),width);
                auto* scroll=page.findChild<QScrollArea*>("DashboardScroll"); QVERIFY(scroll);
                QCOMPARE(scroll->horizontalScrollBar()->maximum(),0);
                for(auto* meter:page.findChildren<StorageDonutMeter*>()) {
                    QCOMPARE(meter->percent(),10.); QVERIFY(meter->width()>=meter->minimumSizeHint().width());
                    QVERIFY(meter->height()>=meter->minimumSizeHint().height());
                }
                if(!directory.isEmpty()) { QDir().mkpath(directory); QVERIFY(render(page).save(QString("%1/Dashboard-%2-%3.png").arg(directory,theme).arg(width))); }
            }
        }
    }
    void titleBarTechnicalDecoration()
    {
        CustomTitleBar bar; bar.resize(1200,48); bar.setTitle("数据管理软件");
        bar.setTitleAlignment(CustomTitleBar::TitleAlignment::Center); bar.setIconPlacement(CustomTitleBar::IconPlacement::WithTitle);
        bar.show(); QApplication::processEvents();
        auto* label=bar.findChild<QLabel*>("TitleLabel"); const QRect geometry=label->geometry();
        const auto plain=render(bar); bar.setDecoration(CustomTitleBar::Decoration::Technical);
        const auto decorated=render(bar); QVERIFY(plain!=decorated); QCOMPARE(label->geometry(),geometry);
        QVERIFY(bar.isDraggableAt(label->geometry().center()));
        bar.setProfile(CustomTitleBar::Profile::Dialog); const auto dialog=render(bar);
        bar.setDecoration(CustomTitleBar::Decoration::Plain); QCOMPARE(render(bar),dialog);
    }
    void titleBarConfiguredBackground()
    {
        CustomTitleBar bar; bar.resize(1100,48); bar.show();
        auto* manager=StyleManagerAdapter::instance();
        for(const auto& theme:manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme()); QApplication::processEvents();
            const auto configured=manager->themeColor("titleBarBackgroundColor");
            const auto expected=configured.isValid()?configured:ThemeTokens::instance()->bg0();
            QCOMPARE(ThemeTokens::instance()->titleBarBackground(),expected);
            QCOMPARE(bar.palette().color(QPalette::Window),expected);
            const auto image=render(bar);
            if(ThemeTokens::instance()->titleBarGradientEnd()==expected)QCOMPARE(image.pixelColor(500,2),expected);
            else QVERIFY(image.pixelColor(500,2)!=image.pixelColor(1000,2));
            QVERIFY(contrastRatio(ThemeTokens::instance()->textPrimary(),ThemeTokens::instance()->titleBarGradientEnd())>=4.5);
        }
    }
    void titleBarSearchAndOverflow()
    {
        QWidget window; auto* layout=new QVBoxLayout(&window); layout->setContentsMargins(0,0,0,0);
        auto* title=new CustomTitleBar(&window); layout->addWidget(title); layout->addStretch(); title->attachToWindow(&window);
        title->setSearchVisible(true); title->setSettingsVisible(true); title->setUserVisible(true);
        window.resize(1100,240); window.show(); window.activateWindow(); QTest::qWait(15);
        QSignalSpy text(title,&CustomTitleBar::searchTextChanged),submit(title,&CustomTitleBar::searchSubmitted);
        title->setSearchText("station_1"); QCOMPARE(text.count(),1); title->setSearchText("station_1"); QCOMPARE(text.count(),1);
        title->focusSearch(); QVERIFY(title->searchEdit()->hasFocus());
        QTest::keyClick(title->searchEdit(),Qt::Key_Return); QCOMPARE(submit.count(),1); QCOMPARE(submit.last().first().toString(),QString("station_1"));
        window.resize(640,240); QApplication::processEvents(); QVERIFY(title->searchEdit()->isHidden());
        title->settingsButton()->setFocus();
        QTest::keyClick(title->settingsButton(),Qt::Key_K,Qt::ControlModifier); QApplication::processEvents();
        auto* popup=title->findChild<QFrame*>("TitleBarSearchPopup"); auto* input=title->findChild<QLineEdit*>("TitleBarPopupSearchEdit");
        QVERIFY(popup && input); QVERIFY(popup->isVisible()); QCOMPARE(input->text(),QString("station_1"));
        input->setText("new query"); QCOMPARE(title->searchText(),QString("new query")); QCOMPARE(text.count(),2);
        QTest::keyClick(input,Qt::Key_Return); QCOMPARE(submit.count(),2); QVERIFY(!popup->isVisible());
        title->focusSearch(); QVERIFY(popup->isVisible()); QTest::keyClick(input,Qt::Key_Escape); QTRY_VERIFY(!popup->isVisible());
        title->setSearchVisible(false); title->focusSearch(); QVERIFY(!popup->isVisible());
        window.resize(240,240); QApplication::processEvents();
        auto* more=title->findChild<QToolButton*>("TitleBarMoreButton"); QVERIFY(!more->isHidden());
        QSignalSpy settings(title,&CustomTitleBar::settingsRequested);
        const auto actions=more->menu()->actions(); QVERIFY(!actions.isEmpty()); actions.first()->trigger(); QCOMPARE(settings.count(),1);
        title->setSearchVisible(true); title->setProfile(CustomTitleBar::Profile::Dialog); title->focusSearch(); QVERIFY(!popup->isVisible());
    }
    void titleBarMenusAndActionOwnership()
    {
        QWidget window; auto* layout=new QVBoxLayout(&window); layout->setContentsMargins(0,0,0,0);
        auto* title=new CustomTitleBar(&window); layout->addWidget(title); layout->addStretch(); title->attachToWindow(&window);
        title->setSettingsVisible(true); title->setUserVisible(true); window.resize(1100,240); window.show(); QTest::qWait(10);
        auto* settings=new QMenu(title); auto* preference=settings->addAction("Preferences"); title->settingsButton()->setMenu(settings);
        QCoreApplication::processEvents(); QSignalSpy selected(preference,&QAction::triggered);
        bool opened=false;
        QTimer::singleShot(20,&window,[&] {
            auto* menu=qobject_cast<QMenu*>(QApplication::activePopupWidget()); if(!menu)return;
            opened=true; menu->setActiveAction(preference); QTest::keyClick(menu,Qt::Key_Return);
        });
        QTest::mouseClick(title->settingsButton(),Qt::LeftButton); QTRY_VERIFY(opened); QCOMPARE(selected.count(),1);
        window.resize(240,240); QCoreApplication::processEvents();
        auto* more=title->findChild<QToolButton*>("TitleBarMoreButton"); QVERIFY(more);
        bool submenu=false;
        for(auto* action:more->menu()->actions())if(action->menu()==settings)submenu=true;
        QVERIFY(submenu); // settings remain reachable when the gear moves into overflow
        auto* external=new QAction("External action",&window); QPointer<QAction> alive=external;
        title->addTitleAction(external); title->addTitleAction(external);
        QCOMPARE(more->menu()->actions().count(external),1);
        title->removeTitleAction(external); QVERIFY(alive); QVERIFY(!more->menu()->actions().contains(external));
        title->setSearchVisible(true); title->setSearchShortcut(QKeySequence());
        QVERIFY(!title->findChild<QShortcut*>()->isEnabled());
        title->setSearchShortcut(QKeySequence("Ctrl+F")); QVERIFY(title->findChild<QShortcut*>()->isEnabled());
    }
    void titleBarThemeAndAvatarRendering()
    {
        FramelessWidgetWindow window; window.setContentWidget(new QLabel("Content"));
        auto* title=window.titleBar(); title->setTitle("项目 / 水声数据处理"); title->setSettingsVisible(true); title->setSearchVisible(true); title->setUserVisible(true);
        QPixmap avatar(100,70); avatar.fill(QColor("#568AB0"));
        { QPainter painter(&avatar); painter.setBrush(Qt::white); painter.drawEllipse(30,10,30,30); }
        title->setUserInfo("演示用户",avatar,"业务角色 / 数据分析");
        auto* menu=new QMenu(title); menu->addAction("Preferences"); title->settingsButton()->setMenu(menu);
        auto* userMenu=new QMenu(title); userMenu->addAction("Profile"); title->userButton()->setMenu(userMenu);
        auto* manager=StyleManagerAdapter::instance();
        title->setIconPlacement(CustomTitleBar::IconPlacement::WithTitle);
        title->addTitleAction(new QAction("算法启动",title),CustomTitleBar::ActionPosition::Left);
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        for(const auto& theme:manager->themes()) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(auto alignment:{CustomTitleBar::TitleAlignment::Left,CustomTitleBar::TitleAlignment::Center}) {
                title->setTitleAlignment(alignment);
                for(int width:{1100,640}) {
                    window.resize(width,300); window.show(); QApplication::processEvents();
                    QVERIFY(!title->userButton()->icon().isNull());
                    if(!directory.isEmpty()) {
                        QDir().mkpath(directory);
                        QVERIFY(render(*title).save(QString("%1/TitleBar-%2-%3-%4.png").arg(directory,theme).arg(int(alignment)).arg(width)));
                    }
                }
            }
            FramelessDialog dialog; dialog.setTitle("录音详情"); dialog.titleBar()->setTitleAlignment(CustomTitleBar::TitleAlignment::Center);
            dialog.resize(440,260); dialog.show(); QApplication::processEvents();
            QVERIFY(dialog.titleBar()->findChild<QLabel*>("TitleIcon")->isHidden());
            if(!directory.isEmpty())QVERIFY(render(dialog).save(directory+"/TitleBar-Dialog-"+theme+".png"));
        }
    }
    void iconSurvivesManager()
    {
        QIcon original;
        {
            acss::QtAdvancedStylesheet manager;
            manager.setStylesDirPath(QStringLiteral(BASEWIDGETS_SOURCE_DIR "/assets/styles"));
            QVERIFY(manager.setCurrentStyle("themes")); QVERIFY(manager.setCurrentTheme("dark"));
            original = manager.loadThemeAwareSvgIcon(QStringLiteral(BASEWIDGETS_SOURCE_DIR "/assets/styles/themes/common/icons/user.svg"));
        }
        QIcon detached = original; detached.addPixmap(QPixmap(8,8)); // forces engine clone
        auto* runtime = StyleManagerAdapter::instance();
        QVERIFY(runtime->setCurrentTheme("light")); QVERIFY(runtime->updateTheme());
        QVERIFY(!original.pixmap(24,24).isNull());
        QVERIFY(runtime->setCurrentTheme("dark")); QVERIFY(runtime->updateTheme());
    }
    void hiddenSpinnerDoesNotTick()
    {
        BusyIndicator spinner;
        auto* timer = spinner.findChild<QTimer*>(); QVERIFY(timer);
        QVERIFY(spinner.isRunning()); QVERIFY(!timer->isActive());
        spinner.show(); QTest::qWait(20); QVERIFY(timer->isActive());
        spinner.hide(); QVERIFY(!timer->isActive()); QVERIFY(spinner.isRunning());
        spinner.show(); QTest::qWait(20); QVERIFY(timer->isActive());
        spinner.stop(); spinner.hide(); spinner.show(); QVERIFY(!timer->isActive());
    }
    void navigationIdsAndSignals()
    {
        NavigationList list; list.addButton("First",4); list.addButton("Second",9);
        list.resize(200,160); list.show(); QTest::qWait(20);
        QSignalSpy clicks(&list,&NavigationList::buttonClicked);
        QTest::mouseClick(list.viewport(),Qt::LeftButton,Qt::NoModifier,list.visualItemRect(list.item(1)).center());
        QCOMPARE(clicks.count(),1); QCOMPARE(clicks.first().at(0).toInt(),9);
        delete list.takeItem(0); list.setCurrentButton(4); QCOMPARE(list.getCurrentButton(),-1);
        list.clear(); list.addButton("Fresh",4); list.setCurrentButton(4); QCOMPARE(list.getCurrentButton(),4);
    }
    void dateRangePresetsAndTimeZone()
    {
        DateRangePicker picker;
        QCOMPARE(picker.range().first.date(),QDate::currentDate());
        picker.setIncludeTime(true); picker.setTimeZone(QTimeZone("UTC"));
        QSignalSpy changes(&picker,&DateRangePicker::rangeChanged);
        const QDateTime start(QDate(2026,10,20),QTime(9,30),QTimeZone("UTC"));
        const QDateTime end(QDate(2026,10,22),QTime(18,15),QTimeZone("UTC"));
        picker.setRange(start,end); QCOMPARE(changes.count(),1);
        QCOMPARE(picker.range().first,start); QCOMPARE(picker.range().second,end);
        QCOMPARE(picker.range().first.timeZone(),QTimeZone("UTC"));
        picker.setRange(end,start); QCOMPARE(picker.range().first,start); QCOMPARE(picker.range().second,end);
        picker.setTimeZone(QTimeZone("Asia/Shanghai"));
        QCOMPARE(picker.range().first.time(),QTime(9,30));
        QCOMPARE(picker.range().first.timeZone(),QTimeZone("Asia/Shanghai"));
        picker.setIncludeTime(false); QCOMPARE(picker.range().first.time(),QTime(0,0));
    }
    void advancedTableRowCheckboxes()
    {
        QStandardItemModel source(8,2);
        for(int row=0;row<8;++row) { source.setData(source.index(row,0),row); source.setData(source.index(row,1),QString("file-%1").arg(row)); }
        AdvancedTable table; table.setSourceModel(&source); table.setPageSize(3);
        table.resize(640,440); table.show(); QTest::qWait(20);
        auto* all=table.findChild<QCheckBox*>("AdvancedTableCheckAll"); QVERIFY(all); QVERIFY(all->isVisible());
        QCOMPARE(table.view()->model()->columnCount(),2); // no synthetic column shifts existing public indexes
        QCOMPARE(table.currentPageCheckState(),Qt::Unchecked);
        auto* rowHeader=table.view()->verticalHeader();
        const QPoint checkbox(12,rowHeader->sectionViewportPosition(0)+rowHeader->sectionSize(0)/2);
        const QRect checkArea(0,rowHeader->sectionViewportPosition(0),28,rowHeader->sectionSize(0));
        const QImage unchecked=render(*rowHeader->viewport()).copy(checkArea);
        QTest::mouseClick(rowHeader->viewport(),Qt::LeftButton,Qt::NoModifier,checkbox);
        QVERIFY(render(*rowHeader->viewport()).copy(checkArea)!=unchecked);
        QCOMPARE(table.checkedSourceRows().size(),1); QCOMPARE(table.checkedSourceRows().first().row(),0);
        QCOMPARE(all->checkState(),Qt::PartiallyChecked);
        QVERIFY(!source.index(0,0).data(Qt::CheckStateRole).isValid());
        QSignalSpy changes(&table,&AdvancedTable::checkedRowsChanged);
        all->click(); QCOMPARE(changes.count(),1); QCOMPARE(table.checkedSourceRows().size(),3); QCOMPARE(all->checkState(),Qt::Checked);
        all->click(); QCOMPARE(table.checkedSourceRows().size(),0); QCOMPARE(all->checkState(),Qt::Unchecked);
        QVERIFY(table.setRowChecked(source.index(1,1),true));
        table.pager()->setCurrentPage(1); QCOMPARE(all->checkState(),Qt::Unchecked);
        all->click(); QCOMPARE(table.checkedSourceRows().size(),4);
        table.pager()->setCurrentPage(0); QCOMPARE(all->checkState(),Qt::PartiallyChecked);
        table.view()->sortByColumn(0,Qt::DescendingOrder);
        QVERIFY(table.isRowChecked(source.index(1,0))); QVERIFY(table.isRowChecked(source.index(4,0)));
        table.setFilterColumn(1); table.setFilterText("file-1"); QCOMPARE(table.pager()->rowCount(),1);
        QCOMPARE(all->checkState(),Qt::Checked); all->click(); QCOMPARE(table.checkedSourceRows().size(),3);
        table.setFilterText("no match"); QVERIFY(!all->isEnabled()); QCOMPARE(all->checkState(),Qt::Unchecked);
        table.setFilterText(""); table.clearCheckedRows();
        QVERIFY(table.setPinnedRows({7})); all->click(); QCOMPARE(table.checkedSourceRows().size(),4);
        table.pager()->setCurrentPage(1); QCOMPARE(all->checkState(),Qt::PartiallyChecked);
        all->click(); QCOMPARE(table.checkedSourceRows().size(),7); // repeated pinned record is counted once
        all->click(); QCOMPARE(table.checkedSourceRows().size(),3);
        table.clearCheckedRows(); table.view()->setCurrentIndex(table.pager()->index(0,0));
        QTest::keyClick(table.view(),Qt::Key_Space,Qt::ControlModifier);
        QCOMPARE(table.checkedSourceRows().size(),1); QVERIFY(table.isRowChecked(source.index(7,0)));
        QTest::keyClick(table.view(),Qt::Key_Space,Qt::ControlModifier); QVERIFY(table.checkedSourceRows().isEmpty());
        table.setRowCheckBoxesVisible(false); QVERIFY(!all->isVisible());
        table.setRowCheckBoxesVisible(true); QVERIFY(all->isVisible());
        const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        if(!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(render(table).save(capture+"/AdvancedTableChecks-ltr.png")); }
        table.setLayoutDirection(Qt::RightToLeft); QCoreApplication::processEvents();
        QVERIFY(table.view()->rect().contains(all->geometry()));
        if(!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(render(table).save(capture+"/AdvancedTableChecks-rtl.png")); }
    }
    void advancedTableCheckedRecordLifecycle()
    {
        AdvancedTable table;
        auto* source=new QStandardItemModel(5,1);
        for(int row=0;row<5;++row)source->setData(source->index(row,0),row);
        table.setSourceModel(source); table.setPageSize(5);
        source->item(2)->setEnabled(false);
        table.setCurrentPageChecked(true); QCOMPARE(table.checkedSourceRows().size(),4);
        QCOMPARE(table.currentPageCheckState(),Qt::Checked); QVERIFY(!table.setRowChecked(source->index(2,0),true));
        QStandardItemModel foreign(1,1); QVERIFY(!table.setRowChecked(foreign.index(0,0),true));
        source->insertRow(0); QCOMPARE(table.checkedSourceRows().first().row(),1);
        QVERIFY(table.isRowChecked(source->index(5,0)));
        source->sort(0,Qt::DescendingOrder); QCOMPARE(table.checkedSourceRows().size(),4);
        QVERIFY(table.checkedSourceRows().first().data().toInt()==4);
        QSignalSpy changes(&table,&AdvancedTable::checkedRowsChanged);
        source->removeRow(table.checkedSourceRows().first().row()); QCOMPARE(table.checkedSourceRows().size(),3); QCOMPARE(changes.count(),1);
        source->clear(); QVERIFY(table.checkedSourceRows().isEmpty());
        source->setColumnCount(1); source->setRowCount(2); table.setCurrentPageChecked(true); QCOMPARE(table.checkedSourceRows().size(),2);
        delete source; QVERIFY(table.checkedSourceRows().isEmpty());
        QStandardItemModel replacement(2,1); table.setSourceModel(&replacement); table.setCurrentPageChecked(true);
        QCOMPARE(table.checkedSourceRows().size(),2); table.setSourceModel(&foreign); QVERIFY(table.checkedSourceRows().isEmpty());
    }
    void compactTableAndJump()
    {
        AdvancedTable table;
        QStandardItemModel model(100,2);
        for (int row=0;row<100;++row) model.setData(model.index(row,0),row);
        table.setSourceModel(&model); table.resize(560,420); table.show(); QTest::qWait(30);
        QVERIFY(table.minimumSizeHint().width()<600);
        table.pager()->setCurrentPage(2);
        const auto spinners = table.findChildren<QSpinBox*>();
        QVERIFY(!spinners.isEmpty());
        for (auto* spinner : spinners) QCOMPARE(spinner->value(),3);
    }
    void copySortedPagedRow()
    {
        AdvancedTable table;
        QStandardItemModel model(60,2);
        for (int row=0;row<60;++row) {
            model.setData(model.index(row,0),row);
            model.setData(model.index(row,1),QString("value,%1").arg(row));
        }
        table.setSourceModel(&model); table.view()->sortByColumn(0,Qt::DescendingOrder);
        table.pager()->setCurrentPage(1); table.resize(640,420); table.show(); QTest::qWait(20);
        const QModelIndex selected = table.view()->model()->index(0,0);
        table.view()->selectionModel()->select(selected,QItemSelectionModel::ClearAndSelect);
        const QString expected = QString("%1,\"value,%1\"\n").arg(selected.data().toInt());
        bool opened = false;
        QTimer::singleShot(30,&table,[&] {
            auto* menu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
            if (!menu) return;
            opened = true; menu->setActiveAction(menu->actions().first()); QTest::keyClick(menu,Qt::Key_Return);
        });
        table.view()->customContextMenuRequested(table.view()->visualRect(selected).center());
        QTRY_VERIFY(opened); QCOMPARE(QApplication::clipboard()->text(),expected);
    }
    void pinnedTableRecords()
    {
        QStandardItemModel model(12,2);
        for(int r=0;r<12;++r) { model.setData(model.index(r,0),r); model.setData(model.index(r,1),QString("record %1").arg(r)); }
        AdvancedTable table; table.setSourceModel(&model); table.setPageSize(3);
        QAbstractItemModelTester tester(table.pager(),QAbstractItemModelTester::FailureReportingMode::QtTest);
        QVERIFY(table.setPinnedRows({7,2},{9}));
        QCOMPARE(table.pager()->pageCount(),3); QCOMPARE(table.pager()->rowCount(),6);
        for(const auto order:{Qt::AscendingOrder,Qt::DescendingOrder}) {
            table.view()->sortByColumn(0,order);
            for(int page=0;page<3;++page) {
                table.pager()->setCurrentPage(page);
                QCOMPARE(table.pager()->index(0,0).data().toInt(),7);
                QCOMPARE(table.pager()->index(1,0).data().toInt(),2);
                QCOMPARE(table.pager()->index(5,0).data().toInt(),9);
                for(int r=0;r<table.pager()->rowCount();++r) {
                    const auto index=table.pager()->index(r,1);
                    QCOMPARE(table.mapFromSource(table.mapToSource(index)),index);
                }
            }
        }
        QVERIFY(!table.setPinnedRows({7,7})); QVERIFY(!table.setPinnedRows({50}));
        table.setFilterColumn(1); table.setFilterText("record 7");
        QCOMPARE(table.pager()->rowCount(),1); QCOMPARE(table.pager()->pinnedRowCount(),1);
        table.setFilterText(""); QCOMPARE(table.pager()->pinnedRowCount(),3);
        model.insertRow(0); // persistent indexes track source-record identity after insert
        QCOMPARE(table.pager()->index(0,0).data().toInt(),7);
        model.removeRow(8); // remove pinned record 7
        QCOMPARE(table.pager()->index(0,0).data().toInt(),2);
        QCOMPARE(table.pager()->pinnedRowCount(),2);
        table.setPresentationMode(AdvancedTable::PresentationMode::Standard);
        QCOMPARE(table.pager()->rowCount(),model.rowCount()); QCOMPARE(table.pager()->pageCount(),1);
        table.setPresentationMode(AdvancedTable::PresentationMode::Basic);
        QVERIFY(!table.pager()->paginationEnabled());
        table.clearPinnedRows(); QCOMPARE(table.pager()->pinnedRowCount(),0);
        table.setSortingEnabled(false);
        QCOMPARE(table.mapToSource(table.pager()->index(0,0)).row(),0);
        table.setPresentationMode(AdvancedTable::PresentationMode::Paged);
        table.pager()->setCurrentPage(3); model.removeRows(2,10);
        QCOMPARE(table.pager()->currentPage(),0); QCOMPARE(table.pager()->rowCount(),2);
    }
    void standalonePagerModelLifecycle()
    {
        PagedProxyModel pager;
        QAbstractItemModelTester tester(&pager,QAbstractItemModelTester::FailureReportingMode::QtTest);
        auto* model=new QStandardItemModel(5,2);
        for(int r=0;r<5;++r)model->setData(model->index(r,0),r);
        pager.setSourceModel(model); pager.setPageSize(2);
        QVERIFY(pager.setPinnedIndexes({model->index(3,0)},{model->index(1,0)}));
        model->sort(0,Qt::DescendingOrder);
        QCOMPARE(pager.index(0,0).data().toInt(),3);
        QCOMPARE(pager.index(pager.rowCount()-1,0).data().toInt(),1);
        QVERIFY(pager.setData(pager.index(0,1),"edited"));
        QCOMPARE(pager.mapToSource(pager.index(0,1)).data().toString(),QString("edited"));
        QStandardItemModel foreign(1,1); QVERIFY(!pager.mapFromSource(foreign.index(0,0)).isValid());
        delete model;
        QCOMPARE(pager.rowCount(),0); QCOMPARE(pager.columnCount(),0); QCOMPARE(pager.pageCount(),0);
        QStandardItemModel replacement(2,1); pager.setSourceModel(&replacement);
        QCOMPARE(pager.pinnedRowCount(),0); QCOMPARE(pager.rowCount(),2);
        pager.setSourceModel(nullptr); QCOMPARE(pager.rowCount(),0);
    }
    void tableUpdatesAndExports()
    {
        AdvancedTable table; auto* model=new QStandardItemModel(10,2);
        for(int r=0;r<10;++r) { model->setData(model->index(r,0),r); model->setData(model->index(r,1),QString("record,%1").arg(r)); }
        table.setSourceModel(model); table.setPageSize(3);
        QAbstractItemModelTester tester(table.pager(),QAbstractItemModelTester::FailureReportingMode::QtTest);
        const auto first=table.pager()->index(0,1);
        table.view()->selectionModel()->select(first,QItemSelectionModel::ClearAndSelect);
        QSignalSpy reset(table.pager(),&QAbstractItemModel::modelReset);
        QSignalSpy data(table.pager(),&QAbstractItemModel::dataChanged);
        model->setData(model->index(0,1),"updated,record");
        QCOMPARE(reset.count(),0); QVERIFY(data.count()>0);
        QCOMPARE(table.selectedSourceRows().size(),1);
        AdvancedTable::CsvExportOptions options; options.filePath=output.filePath("selected.csv");
        options.scope=AdvancedTable::ExportScope::Selection; options.includeHeader=false;
        QVERIFY(table.exportCsv(options)); QFile file(options.filePath); QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(),QByteArray("0,\"updated,record\"\n"));
        QVERIFY(table.setPinnedRows({0})); table.view()->sortByColumn(0,Qt::DescendingOrder);
        QCOMPARE(table.selectedSourceRows().size(),1); QCOMPARE(table.selectedSourceRows().first().row(),0);
        table.setFilterText("updated"); QCOMPARE(table.pager()->rowCount(),1);
        table.setFilterMode(AdvancedTable::FilterMode::RegularExpression);
        QSignalSpy rejected(&table,&AdvancedTable::filterRejected); table.setFilterText("[");
        QCOMPARE(rejected.count(),1); QCOMPARE(table.pager()->rowCount(),1);
        table.setFilterText("");
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        table.setFilterRegExp(QRegExp("updated*",Qt::CaseInsensitive,QRegExp::Wildcard));
        QCOMPARE(table.pager()->rowCount(),1);
        table.setFilterRegExp(QRegExp("record,9",Qt::CaseSensitive,QRegExp::FixedString));
        QCOMPARE(table.pager()->rowCount(),1);
#endif
        model->clear(); QCOMPARE(table.pager()->rowCount(),0); QCOMPARE(table.pager()->pinnedRowCount(),0);
        delete model; QCoreApplication::processEvents();
        QCOMPARE(table.pager()->rowCount(),0); QCOMPARE(table.pager()->columnCount(),0);
        QVERIFY(!table.sourceModel());
    }
    void tableMenuExtensionUsesSourceIndexes()
    {
        QStandardItemModel model(8,1); for(int r=0;r<8;++r)model.setData(model.index(r,0),r);
        AdvancedTable table; table.setSourceModel(&model); table.setPageSize(3);
        table.view()->sortByColumn(0,Qt::DescendingOrder); table.pager()->setCurrentPage(1);
        table.resize(600,400); table.show(); QTest::qWait(10);
        QPointer<QMenu> opened; int row=-1;
        connect(&table,&AdvancedTable::prepareContextMenu,&table,[&](QMenu* menu,const QModelIndexList& selection) {
            opened=menu; QVERIFY(!selection.isEmpty()); QCOMPARE(selection.first().model(),&model); row=selection.first().row();
            menu->addAction("Business action");
        });
        table.view()->customContextMenuRequested(table.view()->visualRect(table.pager()->index(0,0)).center());
        QVERIFY(opened); QCOMPARE(row,4); QCOMPARE(opened->actions().last()->text(),QString("Business action"));
        model.removeRow(4); // menu action must tolerate the selected source record disappearing
        opened->actions().first()->trigger(); opened->close();
    }
    void multiScreenTopology()
    {
        // Logical pixels: left monitor uses a negative origin; a third monitor sits above/right.
        const QVector<QRect> screens{QRect(0,0,1920,1040),QRect(-1280,0,1280,984),QRect(2560,-900,1600,900)};
        const QRect left(-1200,50,900,700);
        QCOMPARE(WindowPlacement::bestScreen(left,screens),1);
        QCOMPARE(WindowPlacement::fitToScreens(left,screens),left);
        const QRect gap(2100,100,500,600);
        const auto fitted=WindowPlacement::fitToScreens(gap,screens);
        QVERIFY(screens[WindowPlacement::bestScreen(fitted,screens)].contains(fitted));
        const QRect third(2800,-800,1200,700);
        QCOMPARE(WindowPlacement::bestScreen(third,screens),2);
        const auto disconnected=WindowPlacement::fitToScreens(third,{screens[0],screens[1]},0);
        QVERIFY(screens[0].contains(disconnected));
        QCOMPARE(WindowPlacement::fitToScreens(QRect(-4000,-3000,6000,4000),{screens[1]}),screens[1]);
        QCOMPARE(WindowPlacement::bestScreen(left,{}),-1);
        QCOMPARE(WindowPlacement::fitToScreens(left,{}),left);
    }
    void windowPlacementPersistence()
    {
        QWidget window; window.resize(360,260); window.move(30,40);
        auto* guard=WindowPlacement::watch(&window); QVERIFY(guard); QCOMPARE(WindowPlacement::watch(&window),guard);
        const auto saved=WindowPlacement::save(&window); QVERIFY(!saved.isEmpty());
        window.move(4000,4000); QVERIFY(WindowPlacement::restore(&window,saved));
        QCOMPARE(window.size(),QSize(360,260)); QVERIFY(window.screen()->availableGeometry().contains(window.geometry()));
        const QRect before=window.geometry(); QVERIFY(!WindowPlacement::restore(&window,"bad json")); QCOMPARE(window.geometry(),before);
        auto object=QJsonDocument::fromJson(saved).object();
        object["screenName"]="disconnected-screen"; object["serial"]="missing";
        object["normal"]=QJsonArray{9999,-9999,900,700};
        QVERIFY(WindowPlacement::restore(&window,QJsonDocument(object).toJson()));
        QVERIFY(window.screen()->availableGeometry().contains(window.geometry()));
        object["normal"]=QJsonArray{0,0,-10,10};
        QVERIFY(!WindowPlacement::restore(&window,QJsonDocument(object).toJson()));
        QCOMPARE(WindowPlacement::moveToScreen(&window,nullptr),WindowPlacement::MoveResult::InvalidTarget);
        QVERIFY(WindowPlacement::moveToScreen(&window,window.screen())!=WindowPlacement::MoveResult::InvalidTarget);
        QVERIFY(!window.isVisible()); // moving an auxiliary window must not show it
        QWidget child(&window); QVERIFY(!WindowPlacement::watch(&child));
        window.setWindowState(Qt::WindowMaximized);
        const auto maximized=WindowPlacement::save(&window);
        window.setWindowState(Qt::WindowNoState); QVERIFY(WindowPlacement::restore(&window,maximized));
        QVERIFY(window.isMaximized());
    }
    void resizeClampsOppositeEdge()
    {
#ifdef Q_OS_LINUX
        FramelessWidgetWindow window; window.setContentWidget(new QLabel("Resize"));
        window.setMinimumSize(300,240); window.resize(600,400); window.move(30,30); window.show(); QTest::qWait(20);
        const QRect before = window.geometry();
        const QPoint edge(1,window.height()/2);
        QTest::mousePress(&window,Qt::LeftButton,Qt::NoModifier,edge);
        const QPoint global = window.mapToGlobal(edge)+QPoint(900,0);
        QMouseEvent move(QEvent::MouseMove,window.mapFromGlobal(global),global,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(&window,&move);
        QTest::mouseRelease(&window,Qt::LeftButton);
        QCOMPARE(window.width(),300); QCOMPARE(window.geometry().right(),before.right());
#endif
    }
    void buttonHierarchyAndCaptionStates()
    {
        auto* style=qApp->style();
        auto paintPanel = [style](QStyleOptionButton& option,PushButton& button) {
            QImage image(option.rect.size(),QImage::Format_ARGB32_Premultiplied);
            image.fill(option.palette.color(QPalette::Window));
            QPainter p(&image); style->drawPrimitive(QStyle::PE_PanelButtonCommand,&option,&p,&button);
            return image;
        };
        PushButton button;
        QStyleOptionButton option;
        option.rect=QRect(0,0,100,36); option.palette=qApp->palette(); option.state=QStyle::State_Enabled;
        const QImage normal=paintPanel(option,button);
        // The center of a straight edge equals the fill: ordinary buttons have no outline.
        QCOMPARE(normal.pixelColor(1,18),normal.pixelColor(50,18));
        button.setAppearance(ButtonAppearance::Outline);
        QCOMPARE(button.appearance(),ButtonAppearance::Outline);
        QVERIFY(paintPanel(option,button).pixelColor(0,18)!=normal.pixelColor(0,18));
        button.setAppearance(ButtonAppearance::Neutral);
        option.state|=QStyle::State_HasFocus;
        QCOMPARE(paintPanel(option,button),normal); // mouse focus alone adds no persistent outline
        option.state|=QStyle::State_KeyboardFocusChange;
        QVERIFY(paintPanel(option,button)!=normal); // keyboard users retain a visible focus cue
        option.state=QStyle::State_Enabled;
        button.setAppearance(ButtonAppearance::Primary);
        const QImage primary=paintPanel(option,button);
        QVERIFY(primary.pixelColor(50,18)!=normal.pixelColor(50,18));
        button.setAppearance(ButtonAppearance::Neutral); option.state|=QStyle::State_On;
        QVERIFY(paintPanel(option,button)!=primary); // selected state is not the same as a primary action
        option.state=QStyle::State_Enabled;
        button.setAppearance(ButtonAppearance::Subtle);
        QCOMPARE(paintPanel(option,button).pixelColor(50,18),option.palette.color(QPalette::Window));
        button.setAppearance(ButtonAppearance::Danger);
        QVERIFY(paintPanel(option,button).pixelColor(50,18).red()>paintPanel(option,button).pixelColor(50,18).blue());

        ToolButton close;
        close.setProperty("bwWindowControl","close"); close.setAppearance(ButtonAppearance::Danger);
        QStyleOptionToolButton caption; caption.rect=QRect(0,0,46,40); caption.palette=qApp->palette();
        auto paintCaption = [&] {
            QImage image(caption.rect.size(),QImage::Format_ARGB32_Premultiplied);
            image.fill(caption.palette.color(QPalette::Window));
            QPainter p(&image); style->drawComplexControl(QStyle::CC_ToolButton,&caption,&p,&close); return image;
        };
        caption.state=QStyle::State_Enabled;
        const QString captureDir=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        if (!captureDir.isEmpty()) { QDir().mkpath(captureDir); QVERIFY(paintCaption().save(captureDir+"/close-normal.png")); }
        QCOMPARE(paintCaption().pixelColor(4,4),caption.palette.color(QPalette::Window));
        caption.state|=QStyle::State_MouseOver;
        if (!captureDir.isEmpty()) QVERIFY(paintCaption().save(captureDir+"/close-hover.png"));
        const QColor hover=paintCaption().pixelColor(4,4);
        QVERIFY(hover.red()>hover.green()*2);
        caption.state|=QStyle::State_Sunken;
        if (!captureDir.isEmpty()) QVERIFY(paintCaption().save(captureDir+"/close-pressed.png"));
        QVERIFY(paintCaption().pixelColor(4,4).lightness()<hover.lightness());
        caption.state=QStyle::State_None;
        QCOMPARE(paintCaption().pixelColor(4,4),caption.palette.color(QPalette::Window));
    }
    void embeddedThemesNeedNoWrites()
    {
        auto* manager=StyleManagerAdapter::instance();
        QCOMPARE(manager->stylesDirPath(),QString(":/styles"));
        QTemporaryDir empty;
        manager->setOutputDirPath(empty.path());
        for (const QString theme : {QString("light"),QString("dark")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            QVERIFY(!QIcon("bwicon:buttons/hx_custom_close.svg").pixmap(18,18).isNull());
        }
        QDirIterator files(empty.path(),QDir::Files,QDirIterator::Subdirectories);
        QVERIFY(!files.hasNext()); // all variants came from the build, not runtime regeneration
        manager->setOutputDirPath(output.path());
    }
    void toolMenuGeometryAndRendering()
    {
        QWidget gallery;
        QGridLayout layout(&gallery);
        int row=0;
        for (bool rtl : {false,true}) {
            for (auto mode : {QToolButton::InstantPopup,QToolButton::MenuButtonPopup,QToolButton::DelayedPopup}) {
                auto* tool=new TestToolButton;
                tool->setText(QStringLiteral("更多选项")); tool->setToolButtonStyle(Qt::ToolButtonTextOnly);
                QFont font=tool->font(); font.setPointSize(18); tool->setFont(font);
                tool->setLayoutDirection(rtl ? Qt::RightToLeft : Qt::LeftToRight);
                auto* menu=new QMenu(tool); menu->addAction("Action"); tool->setMenu(menu); tool->setPopupMode(mode);
                layout.addWidget(new QLabel(QString("RTL %1 / mode %2").arg(rtl).arg(mode)),row,0);
                layout.addWidget(tool,row++,1);
                tool->resize(tool->sizeHint());
                QVERIFY(tool->width()>tool->fontMetrics().horizontalAdvance(tool->text())+20);
                const auto option=tool->option();
                if (mode==QToolButton::MenuButtonPopup) {
                    const QRect main=tool->style()->subControlRect(QStyle::CC_ToolButton,&option,QStyle::SC_ToolButton,tool);
                    const QRect arrow=tool->style()->subControlRect(QStyle::CC_ToolButton,&option,QStyle::SC_ToolButtonMenu,tool);
                    QVERIFY(!main.intersects(arrow)); QVERIFY(arrow.width()>=20);
                    QCOMPARE(arrow.center().y(),main.center().y());
                    QCOMPARE(arrow.center().x()<main.center().x(),rtl);
                    QCOMPARE(tool->style()->hitTestComplexControl(QStyle::CC_ToolButton,&option,arrow.center(),tool),QStyle::SC_ToolButtonMenu);
                }
            }
        }
        gallery.show(); QTest::qWait(20);
        const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        if (!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(gallery.grab().save(capture+"/tool-menu-layout.png")); }
    }
    void tabHoverIsOptIn()
    {
        TabWidget tabs; tabs.addTab(new QWidget,"First"); tabs.addTab(new QWidget,"Second");
        tabs.show();
        auto* bar=tabs.findChild<QTabBar*>(); QVERIFY(bar); QVERIFY(!tabs.hoverEffectEnabled());
        QStyleOptionTab option; option.initFrom(bar); option.text="Second";
        option.rect=QRect(0,0,140,40); option.shape=QTabBar::RoundedNorth; option.state=QStyle::State_Enabled;
        auto paint=[&] {
            QImage image(140,40,QImage::Format_ARGB32_Premultiplied); image.fill(option.palette.color(QPalette::Window));
            QPainter painter(&image); bar->style()->drawControl(QStyle::CE_TabBarTab,&option,&painter,bar); return image;
        };
        const QImage idle=paint(); option.state|=QStyle::State_MouseOver;
        QCOMPARE(paint(),idle);
        tabs.setHoverEffectEnabled(true); QVERIFY(tabs.hoverEffectEnabled()); QVERIFY(paint()!=idle);
        tabs.setHoverEffectEnabled(false); option.state|=QStyle::State_Selected; QVERIFY(paint()!=idle);
    }
    void containerFramesDoNotHoverByDefault()
    {
        auto* manager=StyleManagerAdapter::instance();
        TabWidget tabs; QGroupBox group; QFrame frame; QLineEdit input;
        for(const QString theme:{QString("dark"),QString("light")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(const auto pair:{qMakePair(static_cast<QWidget*>(&tabs),QStyle::PE_FrameTabWidget),
                                 qMakePair(static_cast<QWidget*>(&group),QStyle::PE_FrameGroupBox),
                                 qMakePair(static_cast<QWidget*>(&frame),QStyle::PE_Frame)}) {
                QStyleOptionFrame option; option.initFrom(pair.first); option.rect=QRect(0,0,180,90); option.state=QStyle::State_Enabled;
                auto paint=[&] {
                    QImage image(option.rect.size(),QImage::Format_ARGB32_Premultiplied); image.fill(option.palette.window().color());
                    QPainter painter(&image); qApp->style()->drawPrimitive(pair.second,&option,&painter,pair.first); return image;
                };
                pair.first->setProperty("bwFrameHoverEnabled",false);
                const auto idle=paint(); option.state|=QStyle::State_MouseOver; QCOMPARE(paint(),idle);
                pair.first->setProperty("bwFrameHoverEnabled",true); QVERIFY(paint()!=idle);
                pair.first->setProperty("bwFrameHoverEnabled",false);
                option.state=QStyle::State_Enabled|QStyle::State_HasFocus|QStyle::State_KeyboardFocusChange; QVERIFY(paint()!=idle);
            }
            QStyleOptionFrame option; option.initFrom(&input); option.rect=QRect(0,0,180,36); option.lineWidth=1; option.state=QStyle::State_Enabled;
            auto inputPaint=[&] { QImage image(option.rect.size(),QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent); QPainter painter(&image); qApp->style()->drawPrimitive(QStyle::PE_PanelLineEdit,&option,&painter,&input); return image; };
            const auto idle=inputPaint(); option.state|=QStyle::State_MouseOver; QVERIFY(inputPaint()!=idle);
        }
        QVERIFY(!tabs.contentHoverEffectEnabled()); tabs.setContentHoverEffectEnabled(true); QVERIFY(tabs.contentHoverEffectEnabled());
        QVERIFY(!tabs.hoverEffectEnabled()); tabs.setContentHoverEffectEnabled(false); QVERIFY(!tabs.contentHoverEffectEnabled());
    }
    void dialogSurfaceAndTextButtons()
    {
        auto* manager=StyleManagerAdapter::instance();
        QWidget parent; parent.resize(700,460); parent.show();
        FramelessDialog dialog(true,&parent); dialog.setTitle("请确认");
        auto* body=new QLabel("是否关闭软件？"); dialog.setContentWidget(body);
        dialog.resize(420,260); dialog.show(); QTest::qWait(20);
        QVERIFY(!dialog.testAttribute(Qt::WA_ShowWithoutActivating));
        auto* box=dialog.findChild<QDialogButtonBox*>(); QVERIFY(box);
        QCOMPARE(dialog.style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons),0);
        for (const QString theme : {QString("dark"),QString("light")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            QApplication::processEvents();
            for (auto* button:box->buttons()) QVERIFY(button->icon().isNull());
            const QImage image=render(dialog);
            QCOMPARE(image.pixelColor(0,dialog.height()/2),ThemeTokens::instance()->border());
            QCOMPARE(image.pixelColor(4,dialog.height()/2),ThemeTokens::instance()->bg1());
            QVERIFY(dialog.palette().color(QPalette::Window)!=parent.palette().color(QPalette::Window));
            const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
            if (!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(dialog.grab().save(capture+"/dialog-"+theme+".png")); }
        }
        dialog.setCustomDialogBtn(false,true);
        QVERIFY(!box->button(QDialogButtonBox::Ok)->isDefault());
        dialog.setCustomDialogBtn(true,true);
        QVERIFY(box->button(QDialogButtonBox::Ok)->isDefault());
        QVERIFY(manager->setCurrentTheme("dark")); QVERIFY(manager->updateTheme());
    }
    void dialogDragRecoversAfterMissedRelease()
    {
#ifdef Q_OS_LINUX
        FramelessDialog dialog; dialog.setContentWidget(new QLabel("Drag check"));
        dialog.move(60,60); dialog.resize(420,260); dialog.show(); QTest::qWait(20);
        auto* title=dialog.findChild<CustomTitleBar*>(); QVERIFY(title);
        const QPoint point(title->width()/2,title->height()/2);
        auto sendMouse=[&](QEvent::Type type,const QPoint& global,Qt::MouseButton button,Qt::MouseButtons buttons) {
            QMouseEvent mouse(type,title->mapFromGlobal(global),global,button,buttons,Qt::NoModifier);
            QApplication::sendEvent(title,&mouse);
        };
        QPoint origin=title->mapToGlobal(point);
        const QPoint before=dialog.pos();
        sendMouse(QEvent::MouseButtonPress,origin,Qt::LeftButton,Qt::LeftButton);
        sendMouse(QEvent::MouseMove,origin+QPoint(25,15),Qt::NoButton,Qt::LeftButton);
        QVERIFY(dialog.pos()!=before); // offscreen exercises the manual fallback
        const QPoint dragged=dialog.pos();
        sendMouse(QEvent::MouseMove,origin+QPoint(50,30),Qt::NoButton,Qt::NoButton);
        QCOMPARE(dialog.pos(),dragged); // missing release no longer leaves dragging active
        origin=title->mapToGlobal(point);
        sendMouse(QEvent::MouseButtonPress,origin,Qt::LeftButton,Qt::LeftButton);
        QEvent lostGrab(QEvent::UngrabMouse); QApplication::sendEvent(title,&lostGrab);
        sendMouse(QEvent::MouseMove,origin+QPoint(30,20),Qt::NoButton,Qt::LeftButton);
        QCOMPARE(dialog.pos(),dragged);
        QTest::mouseRelease(title,Qt::LeftButton);
        auto* button=dialog.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok);
        // QWindow input goes through the same widget hit-test/enter/leave dispatcher as
        // platform events. QWidget mouseMove only warps the offscreen cursor, which may
        // not deliver hover transitions at fractional DPR after moving a top-level window.
        QTest::mouseMove(dialog.windowHandle(),title->mapTo(&dialog,point));
        QApplication::processEvents(); const QImage idle=render(*button);
        QTest::mouseMove(dialog.windowHandle(),button->mapTo(&dialog,button->rect().center()));
        QApplication::processEvents();
        QVERIFY(button->underMouse()); QVERIFY(render(*button)!=idle);
        QSignalSpy clicks(button,&QPushButton::clicked);
        QTest::mouseClick(dialog.windowHandle(),Qt::LeftButton,Qt::NoModifier,
                          button->mapTo(&dialog,button->rect().center()));
        QCOMPARE(clicks.count(),1);
#endif
    }
    void gaugeRangesAndMissingData()
    {
        CircularGauge gauge;
        QVERIFY(gauge.setRange(-20,120)); gauge.setValue(65.25); gauge.setLabelPrecision(2);
        QCOMPARE(gauge.value(),65.25);
        QVERIFY(!gauge.setRange(10,10)); QVERIFY(!gauge.setRange(50,10));
        QVERIFY(!gauge.setRange(-std::numeric_limits<double>::max(),std::numeric_limits<double>::max()));
        QCOMPARE(gauge.minimum(),-20.); QCOMPARE(gauge.maximum(),120.);
        QSignalSpy values(&gauge,&CircularGauge::valueChanged);
        QSignalSpy available(&gauge,&CircularGauge::availabilityChanged);
        gauge.setValue(200); QCOMPARE(gauge.value(),120.); QCOMPARE(values.count(),1);
        gauge.setValue(120); QCOMPARE(values.count(),1);
        gauge.setValue(std::numeric_limits<double>::quiet_NaN()); QVERIFY(!gauge.valueAvailable());
        QCOMPARE(gauge.value(),120.); QCOMPARE(gauge.formattedValue(),QString::fromUtf8("—"));
        QVERIFY(!gauge.statusText().isEmpty());
        gauge.setValue(40); QVERIFY(gauge.valueAvailable()); QCOMPARE(available.count(),2);
        gauge.setTargetValue(500); QCOMPARE(gauge.targetValue(),120.);
        QVERIFY(gauge.setRange(0,20)); QCOMPARE(gauge.value(),20.); QCOMPARE(gauge.targetValue(),20.);
        gauge.setTargetValue(std::numeric_limits<double>::infinity()); QVERIFY(!gauge.hasTargetValue());
        QVERIFY(gauge.setRange(0,100));
        QVERIFY(gauge.setRanges({{0,70,Qt::green,5,"normal"},{70,90,Qt::yellow,5,"warning"},{90,100,Qt::red,5,"danger"}}));
        gauge.setValue(69.9); QCOMPARE(gauge.statusText(),QString("normal"));
        gauge.setValue(70); QCOMPARE(gauge.statusText(),QString("warning"));
        gauge.setValue(100); QCOMPARE(gauge.statusText(),QString("danger"));
        QVERIFY(!gauge.setRanges({{0,80,Qt::green,5},{70,100,Qt::red,5}}));
        QCOMPARE(gauge.ranges().size(),3);
        gauge.useClassicThreeColorPercentPreset("legacy","%",0,50,50);
        QCOMPARE(gauge.ranges().size(),2);
        gauge.useCompletionRatePreset("completion","%");
        QVERIFY(gauge.ranges().isEmpty()); QCOMPARE(gauge.displayMode(),CircularGauge::DisplayMode::Arc);
        gauge.setMinorTicksPerMajor(0); gauge.resize(180,210); QVERIFY(!render(gauge).isNull());
        gauge.setMajorTickStep(std::numeric_limits<double>::denorm_min()); QVERIFY(!render(gauge).isNull());
        QVERIFY(gauge.setRange(0,std::numeric_limits<double>::denorm_min()));
        gauge.setValue(gauge.maximum()); QVERIFY(!render(gauge).isNull());
    }
    void gaugeAngleConsistency()
    {
        CircularGauge gauge;
        QCOMPARE(gauge.angleForValue(0),225.); QCOMPARE(gauge.angleForValue(50),90.); QCOMPARE(gauge.angleForValue(100),-45.);
        QVERIFY(gauge.setAngles(180,0));
        QCOMPARE(gauge.angleForValue(0),180.); QCOMPARE(gauge.angleForValue(100),0.);
        QVERIFY(!gauge.setAngles(0,0)); QVERIFY(!gauge.setAngles(10,45)); QVERIFY(!gauge.setAngles(360,0));
        QCOMPARE(gauge.angleForValue(0),180.);
        gauge.useClassicThreeColorPercentPreset();
        QCOMPARE(gauge.angleForValue(0),225.); QCOMPARE(gauge.angleForValue(100),-45.);
        gauge.useCompletionRatePreset();
        QCOMPARE(gauge.angleForValue(0),225.); QCOMPARE(gauge.angleForValue(100),-45.);
    }
    void compassAndAttitudeConventions()
    {
        CompassGauge heading;
        heading.setHeading(-90); QCOMPARE(heading.heading(),270.);
        heading.setHeading(450); QCOMPARE(heading.heading(),90.); const QString east=heading.directionText();
        heading.setHeading(360); QCOMPARE(heading.heading(),0.); QVERIFY(heading.directionText()!=east);
        heading.setHeading(std::numeric_limits<double>::infinity()); QVERIFY(!heading.valueAvailable());
        heading.setHeading(0); QVERIFY(heading.valueAvailable());
        AttitudeMeter attitude;
        attitude.setAttitude(120,450); QCOMPARE(attitude.pitch(),90.); QCOMPARE(attitude.roll(),90.);
        attitude.setAttitude(-120,-540); QCOMPARE(attitude.pitch(),-90.); QCOMPARE(attitude.roll(),-180.);
        attitude.setAttitude(0,0); attitude.resize(300,300);
        const QImage level=render(attitude);
        // Center is at (150,146), aperture radius=96. Sky is blue, ground is brown.
        QVERIFY(level.pixelColor(195,110).blue()>level.pixelColor(195,110).red());
        QVERIFY(level.pixelColor(195,180).red()>level.pixelColor(195,180).blue());
        attitude.setAttitude(30,0); const QImage noseUp=render(attitude);
        QVERIFY(noseUp.pixelColor(195,180).blue()>noseUp.pixelColor(195,180).red());
        attitude.setAttitude(std::numeric_limits<double>::quiet_NaN(),0); QVERIFY(!attitude.valueAvailable());
    }
    void gaugeRenderMatrix()
    {
        QWidget gallery; QGridLayout layout(&gallery);
        CircularGauge completion,temperature,needle,missing;
        CompassGauge heading; AttitudeMeter attitude;
        completion.useCompletionRatePreset("Completion","%"); completion.setValue(68); completion.setTargetValue(80);
        temperature.setTitle("Temperature"); temperature.setUnit("°C"); temperature.setRange(-20,120);
        temperature.setMajorTickStep(20); temperature.setLabelPrecision(1); temperature.setValue(76); temperature.setTargetValue(65);
        temperature.setRanges({{-20,70,QColor("#67C23A"),5,"Normal"},{70,90,QColor("#E6A23C"),5,"Warning"},{90,120,QColor("#F56C6C"),5,"High"}});
        needle.setTitle("Motor speed"); needle.setUnit("rpm"); needle.setRange(0,8000); needle.setValue(3200);
        needle.setDisplayMode(CircularGauge::DisplayMode::Needle); needle.setTickCount(4);
        needle.setRanges({{0,6000,QColor("#67C23A"),5,"Normal"},{6000,8000,QColor("#F56C6C"),5,"High"}});
        needle.setTargetValue(6000); heading.setHeading(72); attitude.setAttitude(8,18);
        missing.setTitle("No data / long sensor name"); missing.setUnit("bar"); missing.setValueAvailable(false);
        QWidget* widgets[]={&completion,&temperature,&needle,&heading,&attitude,&missing};
        for(int i=0;i<6;++i) layout.addWidget(widgets[i],i/3,i%3);
        const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        auto* manager=StyleManagerAdapter::instance();
        for(const QString theme:{QString("dark"),QString("light")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for (const QSize size : {QSize(300,300),QSize(180,210),QSize(420,240)}) {
                for(auto* widget:widgets) widget->setFixedSize(size);
                gallery.adjustSize(); gallery.show(); QApplication::processEvents();
                for(auto* widget:widgets) QVERIFY(!render(*widget).isNull());
                if(!capture.isEmpty()) {
                    QDir().mkpath(capture);
                    QVERIFY(gallery.grab().save(capture+QString("/gauges-%1-%2x%3.png").arg(theme).arg(size.width()).arg(size.height())));
                }
            }
            needle.setValue(0); QVERIFY(!render(needle).isNull()); needle.setValue(8000); QVERIFY(!render(needle).isNull());
            completion.setEnabled(false); QVERIFY(!render(completion).isNull()); completion.setEnabled(true);
        }
        QVERIFY(manager->setCurrentTheme("dark")); QVERIFY(manager->updateTheme());
    }
    void gaugeDemoTimerStopsWhenHidden()
    {
        GaugesPage page;
        auto* timer=page.findChild<QTimer*>("GaugeDemoTimer"); QVERIFY(timer);
        auto* enabled=page.findChild<QCheckBox*>("GaugeDemoAnimate"); QVERIFY(enabled);
        QVERIFY(!timer->isActive()); enabled->setChecked(true); QVERIFY(!timer->isActive());
        page.show(); QTest::qWait(20); QVERIFY(timer->isActive());
        page.hide(); QVERIFY(!timer->isActive());
        page.show(); QTest::qWait(20); QVERIFY(timer->isActive());
        enabled->setChecked(false); QVERIFY(!timer->isActive());
    }
    void acousticFftNormalization()
    {
        constexpr int n=1024; const double pi=std::acos(-1.);
        QVector<double> samples(n);
        for(int i=0;i<n;++i) samples[i]=.5*std::sin(2*pi*64*i/n);
        AcousticSpectrumFrame frame;
        QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame,AcousticUnit::FullScale,SpectrumScaling::Power,AcousticWindow::Rectangular));
        QVERIFY(frame.isValid()); QCOMPARE(frame.levelsDb.size(),513);
        QVERIFY(std::abs(frame.linearPower[64]-.125)<1e-12);
        QVERIFY(std::abs(frame.levelsDb[64]-10*std::log10(.125))<1e-9);
        for(auto window:{AcousticWindow::Rectangular,AcousticWindow::Hann}) {
            QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame,AcousticUnit::FullScale,SpectrumScaling::PowerSpectralDensity,window));
            QVERIFY(std::abs(AcousticAnalysis::bandPowerDb(frame,0,512)-10*std::log10(.125))<1e-9);
        }
        // A different FFT/bin width changes PSD bin heights but not integrated tone power.
        samples.resize(512); for(int i=0;i<512;++i) samples[i]=.5*std::sin(2*pi*64*i/1024);
        QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame));
        QVERIFY(std::abs(AcousticAnalysis::bandPowerDb(frame,0,512)-10*std::log10(.125))<1e-9);
        samples.fill(.5,1024);
        QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame,AcousticUnit::FullScale,SpectrumScaling::Power,AcousticWindow::Rectangular,false));
        QVERIFY(std::abs(frame.linearPower[0]-.25)<1e-12); // DC is not doubled
        for(int i=0;i<1024;++i) samples[i]=(i%2 ? -.5 : .5);
        QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame,AcousticUnit::FullScale,SpectrumScaling::Power,AcousticWindow::Rectangular,false));
        QVERIFY(std::abs(frame.linearPower[512]-.25)<1e-12); // Nyquist is not doubled
        const AcousticSpectrumFrame saved=frame;
        samples[0]=std::numeric_limits<double>::quiet_NaN(); QString error;
        QVERIFY(!AcousticAnalysis::spectrum(samples,1024,&frame,AcousticUnit::FullScale,SpectrumScaling::Power,AcousticWindow::Hann,true,-160,&error));
        QVERIFY(!error.isEmpty()); QCOMPARE(frame.linearPower,saved.linearPower);
        QVERIFY(!AcousticAnalysis::spectrum(QVector<double>(1000,0),1024,&frame));
        QVERIFY(!AcousticAnalysis::spectrum(QVector<double>(1024,0),0,&frame));
    }
    void acousticCalibrationAndStatistics()
    {
        QVector<double> samples(1024); const double pi=std::acos(-1.);
        for(int i=0;i<samples.size();++i) samples[i]=.5*std::sin(2*pi*64*i/1024);
        QVector<double> pressure;
        QVERIFY(AcousticAnalysis::scaleToMicroPascals(samples,100000,&pressure));
        AcousticSpectrumFrame normalized,calibrated;
        QVERIFY(AcousticAnalysis::spectrum(samples,1024,&normalized));
        QVERIFY(AcousticAnalysis::spectrum(pressure,1024,&calibrated,AcousticUnit::MicroPascal));
        QVERIFY(std::abs(AcousticAnalysis::bandPowerDb(calibrated,0,512)-AcousticAnalysis::bandPowerDb(normalized,0,512)-100)<1e-9);
        QVERIFY(calibrated.levelLabel().contains("μPa²/Hz")); QVERIFY(normalized.levelLabel().contains("dBFS/Hz"));
        const auto prior=pressure; QVERIFY(!AcousticAnalysis::scaleToMicroPascals(samples,0,&pressure)); QCOMPARE(pressure,prior);
        const auto stats=AcousticAnalysis::statistics({-1,-.5,0,.5,1,std::numeric_limits<double>::quiet_NaN()});
        QCOMPARE(stats.validCount,5); QCOMPARE(stats.invalidCount,1); QCOMPARE(stats.clippedCount,2);
        QCOMPARE(stats.mean,0.); QVERIFY(std::abs(stats.rms-std::sqrt(.5))<1e-12);
        const auto silence=AcousticAnalysis::statistics({0,0,0}); QVERIFY(silence.valid()); QVERIFY(std::isinf(silence.rmsDb) && silence.rmsDb<0);
        QVERIFY(!AcousticAnalysis::statistics({std::numeric_limits<double>::infinity()}).valid());
        const auto huge=AcousticAnalysis::statistics({1e300,-1e300}); QVERIFY(std::isfinite(huge.rms)); QCOMPARE(huge.mean,0.);
    }
    void waveformCapacityGapsAndCursors()
    {
        AcousticWaveformPlot plot; QVERIFY(plot.configure(8,16)); const int channel=plot.addChannel("CH1"); QCOMPARE(channel,0);
        QVector<double> block(8,.25);
        QVERIFY(plot.appendSamples(channel,0,block)); QVERIFY(plot.appendSamples(channel,1,block)); QCOMPARE(plot.storedPointCount(channel),16);
        QVERIFY(!plot.appendSamples(channel,1,block)); QCOMPARE(plot.storedPointCount(channel),16);
        QVERIFY(plot.appendSamples(channel,3,block)); QVERIFY(plot.storedPointCount(channel)<=16);
        bool hasGap=false; for(auto it=plot.graph(0)->data()->constBegin();it!=plot.graph(0)->data()->constEnd();++it) if(std::isnan(it->value)) hasGap=true;
        QVERIFY(hasGap); QVERIFY(!plot.appendSamples(channel,4,{std::numeric_limits<double>::infinity()}));
        plot.setFollowLatest(false); const auto range=plot.xAxis->range(); QVERIFY(plot.appendSamples(channel,4,block));
        QCOMPARE(plot.xAxis->range().upper,range.upper);
        QSignalSpy cursors(&plot,&AcousticWaveformPlot::cursorsChanged); QVERIFY(plot.setCursors(1.25,1.5));
        QCOMPARE(plot.cursorDeltaSeconds(),.25); QCOMPARE(cursors.count(),1);
        QVERIFY(plot.fitAmplitude()); plot.clearSamples(); QCOMPARE(plot.storedPointCount(0),0);
        QVERIFY(!plot.fitAmplitude()); QVERIFY(!plot.configure(0,16));
    }
    void pixelSamplingPreservesPeaksGapsAndRawData()
    {
        StyledPlot plot; plot.resize(640,300); plot.show();
        auto* graph=qobject_cast<SampledGraph*>(plot.addGraph()); QVERIFY(graph);
        QVector<double> keys(65536),values(65536,0.);
        for(int i=0;i<keys.size();++i)keys[i]=i;
        values[21001]=11.; values[21002]=-13.; values[40000]=std::numeric_limits<double>::quiet_NaN();
        graph->setData(keys,values,true); plot.setViewRanges(QCPRange(0,65535),QCPRange(-15,15)); plot.replot(QCustomPlot::rpImmediateRefresh);
        const auto raw=graph->data(); const auto samples=graph->displaySamples();
        QVERIFY(samples.size()<=9*(qMin(plot.axisRect()->width(),4096)+2)); QVERIFY(samples.size()<keys.size()/4);
        bool peak=false,trough=false,gap=false; double previous=-1;
        for(const auto& sample:samples) { QVERIFY(sample.key>=previous); previous=sample.key; peak|=sample.value==11.; trough|=sample.value==-13.; gap|=std::isnan(sample.value); }
        QVERIFY(peak&&trough&&gap); QCOMPARE(raw->size(),65536); QCOMPARE(raw->at(21001)->value,11.); QVERIFY(std::isnan(raw->at(40000)->value));
        plot.setViewRanges(QCPRange(20999,21004),QCPRange(-15,15)); plot.replot(QCustomPlot::rpImmediateRefresh);
        auto detail=graph->displaySamples(); QVERIFY(detail.size()>=6); QVERIFY(detail.size()<10);
        plot.xAxis->setRangeReversed(true); QCOMPARE(graph->displaySamples().size(),detail.size());
        plot.xAxis->setScaleType(QCPAxis::stLogarithmic); plot.xAxis->setTicker(QSharedPointer<QCPAxisTickerLog>::create());
        plot.xAxis->setRange(1,65535); plot.replot(QCustomPlot::rpImmediateRefresh);
        QVERIFY(graph->displaySamples().size()<=9*(plot.axisRect()->width()+2)); QCOMPARE(raw->size(),65536);
        // Many alternating missing values must stay bounded and never bridge gaps.
        for(int i=0;i<values.size();++i)values[i]=i%2?std::numeric_limits<double>::quiet_NaN():1.;
        graph->setData(keys,values,true); const auto gaps=graph->displaySamples();
        QVERIFY(gaps.size()<=9*(plot.axisRect()->width()+2));
        for(int i=1;i<gaps.size();++i)QVERIFY(!std::isfinite(gaps[i].value)||!std::isfinite(gaps[i-1].value));
    }
    void plotRefreshRequestsAreCoalesced()
    {
        StyledPlot plot; plot.resize(600,300); plot.show(); QTest::qWait(60); plot.replot(QCustomPlot::rpImmediateRefresh);
        QVERIFY(plot.setMaximumFrameRate(10)); QVERIFY(!plot.setMaximumFrameRate(0)); QVERIFY(!plot.setMaximumFrameRate(121));
        QCOMPARE(plot.maximumFrameRate(),10); QSignalSpy frames(&plot,&QCustomPlot::afterReplot);
        for(int i=0;i<200;++i)plot.replot();
        QTest::qWait(20); QCOMPARE(frames.count(),0); QTest::qWait(130); QCOMPARE(frames.count(),1);
        const QPointF point=plot.axisRect()->rect().center();
        for(int i=0;i<50;++i) {
            QWheelEvent wheel(point,plot.mapToGlobal(point.toPoint()),QPoint(),QPoint(0,i%2?120:-120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
            QApplication::sendEvent(&plot,&wheel);
        }
        QCOMPARE(frames.count(),1); QTest::qWait(130); QCOMPARE(frames.count(),2);
        plot.replot(QCustomPlot::rpImmediateRefresh); QCOMPARE(frames.count(),3);
        // The base queues dragging through SLOT(replot()); derived scheduling must
        // consume it once, rather than leaving mReplotQueued stuck or looping.
        QCustomPlot* base=&plot; base->replot(QCustomPlot::rpQueuedReplot); QTest::qWait(140); QCOMPARE(frames.count(),4);
        base->replot(QCustomPlot::rpQueuedReplot); QTest::qWait(140); QCOMPARE(frames.count(),5);
    }
    void simulatedWaveformZoomDragStaysResponsive()
    {
        AcousticPage page;
        auto* wave=page.findChild<AcousticWaveformPlot*>("AcousticDemoWaveform"); QVERIFY(wave);
        auto* producer=page.findChild<QTimer*>("AcousticDemoTimer"); QVERIFY(producer);
        // Exercise the real demo producer while zooming out a dense two-channel
        // waveform. Flush each iteration so coalescing cannot hide a slow draw.
        wave->setParent(nullptr); QScopedPointer<AcousticWaveformPlot> owner(wave);
        wave->resize(900,360); wave->show(); wave->replot(QCustomPlot::rpImmediateRefresh);
        QVERIFY(wave->plottingHints().testFlag(QCP::phFastPolylines));
        QElapsedTimer total; total.start(); qint64 worst=0;
        for(int i=0;i<80;++i) {
            QElapsedTimer frame; frame.start();
            const QPointF center=wave->axisRect()->rect().center();
            QWheelEvent wheel(center,wave->mapToGlobal(center.toPoint()),QPoint(),QPoint(0,i<40?-120:120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);
            QApplication::sendEvent(wave,&wheel); QVERIFY(QMetaObject::invokeMethod(producer,"timeout",Qt::DirectConnection));
            if(i%4==0) {
                QMouseEvent press(QEvent::MouseButtonPress,center,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier); QApplication::sendEvent(wave,&press);
                const QPointF end=center+QPointF(30,0);
                QMouseEvent move(QEvent::MouseMove,end,Qt::NoButton,Qt::LeftButton,Qt::NoModifier); QApplication::sendEvent(wave,&move);
                QMouseEvent release(QEvent::MouseButtonRelease,end,Qt::LeftButton,Qt::NoButton,Qt::NoModifier); QApplication::sendEvent(wave,&release);
            }
            wave->replot(QCustomPlot::rpImmediateRefresh); QApplication::processEvents(); worst=qMax(worst,frame.elapsed());
            for(int c=0;c<wave->channelCount();++c) {
                auto* curve=qobject_cast<SampledGraph*>(wave->graph(c)); QVERIFY(curve);
                QVERIFY(curve->displaySamples().size()<=9*(wave->axisRect()->width()+2)); QVERIFY(wave->storedPointCount(c)<=32768);
            }
            QVERIFY2(total.elapsed()<10000,"dense waveform interaction exceeded 10 s budget");
        }
        QVERIFY(!wave->followsLatest()); wave->resetView(); QVERIFY(wave->followsLatest());
        qInfo()<<"Waveform 80 streamed zoom/drag frames:"<<total.elapsed()<<"ms; worst frame:"<<worst<<"ms";
    }
    void heatmapDisplaySamplingPreservesSource()
    {
        BearingHistoryPlot plot; plot.resize(640,300); plot.show();
        QVERIFY(plot.configure(0,360,8192,128,1)); QVector<double> values(8192,-80.); values[4097]=-3.;
        for(int row=0;row<128;++row)QVERIFY(plot.appendFrame(row,values));
        plot.replot(QCustomPlot::rpImmediateRefresh);
        QVERIFY(plot.displayGridSize().width()<=1024); QVERIFY(plot.displayGridSize().height()<=512);
        QVERIFY(plot.displayGridSize().width()<plot.binCount()); QCOMPARE(plot.levelAt(4097,127),-3.);
        double peak=-100.; auto* data=plot.colorMap()->data();
        for(int x=0;x<data->keySize();++x)peak=qMax(peak,data->cell(x,data->valueSize()-1));
        QCOMPARE(peak,-3.);
        plot.notifyUserNavigation();
        QVERIFY(plot.setViewRanges(QCPRange(plot.coordinateAt(4090),plot.coordinateAt(4105)),QCPRange(120,127)));
        plot.replot(QCustomPlot::rpImmediateRefresh); QVERIFY(plot.displayGridSize().width()<32); QCOMPARE(plot.levelAt(4097,127),-3.);
        plot.setViewRanges(QCPRange(1000,1100),QCPRange(1000,1100)); plot.replot(QCustomPlot::rpImmediateRefresh); QVERIFY(!plot.colorMap()->visible());
        plot.resetView(); plot.replot(QCustomPlot::rpImmediateRefresh); QVERIFY(plot.colorMap()->visible()); QCOMPARE(plot.storedFrameCount(),128);
    }
    void spectrumHoldAndBandPower()
    {
        QVector<double> samples(1024); for(int i=0;i<1024;++i) samples[i]=.5*std::sin(2*std::acos(-1.)*64*i/1024);
        AcousticSpectrumFrame frame; QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame));
        AcousticSpectrumPlot plot; QVERIFY(plot.setFrame(frame)); QCOMPARE(plot.peakFrequencyHz(),64.);
        plot.setPeakHoldEnabled(true); const double high=plot.peakHoldLevels()[64];
        for(auto& value:samples) value*=.5; QVERIFY(AcousticAnalysis::spectrum(samples,1024,&frame)); QVERIFY(plot.setFrame(frame));
        QCOMPARE(plot.peakHoldLevels()[64],high); QVERIFY(frame.levelsDb[64]<high);
        plot.resetPeakHold(); QCOMPARE(plot.peakHoldLevels()[64],frame.levelsDb[64]);
        QVERIFY(plot.setLogFrequency(true)); QVERIFY(plot.xAxis->range().lower>0); QVERIFY(!plot.setFrequencyRange(0,100));
        QVERIFY(plot.setLogFrequency(false)); QVERIFY(plot.setFrequencyRange(0,512));
        QVERIFY(std::abs(plot.bandPowerDb(0,512)-10*std::log10(.03125))<1e-9);
        frame.scaling=SpectrumScaling::Power; QVERIFY(plot.setFrame(frame)); QVERIFY(std::isnan(plot.bandPowerDb(0,512)));
    }
    void spectrogramRingAndGaps()
    {
        AcousticSpectrumFrame frame; QVERIFY(AcousticAnalysis::spectrum(QVector<double>(16,.25),16,&frame));
        AcousticSpectrogramPlot plot; QVERIFY(plot.configure(frame,4,.5));
        QVERIFY(plot.appendFrame(1,frame)); QVERIFY(plot.appendFrame(1.5,frame)); QVERIFY(plot.appendFrame(2.5,frame));
        QCOMPARE(plot.storedFrameCount(),3); QCOMPARE(plot.timeAt(0),1.); QCOMPARE(plot.timeAt(3),2.5);
        QVERIFY(std::isnan(plot.levelAt(0,2))); QCOMPARE(plot.levelAt(0,3),frame.levelsDb[0]);
        plot.replot(QCustomPlot::rpImmediateRefresh);
        QCOMPARE(plot.colorMap()->data()->alpha(0,2),static_cast<unsigned char>(0));
        QCOMPARE(plot.colorMap()->data()->alpha(0,3),static_cast<unsigned char>(255));
        QVERIFY(!plot.appendFrame(2.5,frame)); QVERIFY(!plot.appendFrame(2.7,frame));
        AcousticSpectrumFrame other=frame; other.unit=AcousticUnit::MicroPascal; QVERIFY(!plot.appendFrame(3,other));
        QVERIFY(plot.appendFrame(20,frame)); QCOMPARE(plot.storedFrameCount(),1); QVERIFY(std::isnan(plot.levelAt(0,0)));
        QVERIFY(plot.setLevelRange(-100,-10)); plot.replot(QCustomPlot::rpImmediateRefresh);
        QCOMPARE(plot.colorMap()->dataRange().lower,-100.); QCOMPARE(plot.levelAt(0,3),frame.levelsDb[0]);
        plot.clearHistory(); QCOMPARE(plot.storedFrameCount(),0); QVERIFY(std::isnan(plot.timeAt(0)));
    }
    void recordingTimelineValidationAndSelection()
    {
        RecordingTimelinePlot plot;
        QVector<RecordingInterval> records{{"a","CH1","one.wav",0,10,RecordingState::Ready},{"b","CH2","two.wav",5,15,RecordingState::Flagged}};
        QVERIFY(plot.setRecords(records)); QVERIFY(plot.selectRecord("b")); QCOMPARE(plot.selectedRecordId(),QString("b"));
        auto invalid=records; invalid[1].id="a"; QVERIFY(!plot.setRecords(invalid)); QCOMPARE(plot.records().size(),2);
        plot.resize(700,300); plot.show(); plot.replot(QCustomPlot::rpImmediateRefresh);
        QSignalSpy selected(&plot,&RecordingTimelinePlot::recordSelected);
        QTest::mouseClick(&plot,Qt::LeftButton,Qt::NoModifier,QPoint(qRound(plot.xAxis->coordToPixel(7)),qRound(plot.yAxis->coordToPixel(1))));
        QCOMPARE(selected.count(),1); QCOMPARE(selected.first().first().toString(),QString("a"));
        plot.setUtcTimeAxis(true); QVERIFY(plot.xAxis->label().contains("UTC"));
        QVERIFY(plot.setRecords({})); QCOMPARE(plot.itemCount(),0); QVERIFY(plot.selectedRecordId().isEmpty());
    }
    void acquisitionAndQualityMetrics()
    {
        AcquisitionStatusPanel status; QVERIFY(std::isnan(status.lossPercent())); QVERIFY(std::isnan(status.remainingSeconds()));
        AcquisitionStatus value; value.state=AcquisitionState::Recording; value.sampleRate=48000; value.channels=4;
        value.receivedSamples=900; value.lostSamples=100; value.totalBytes=1000; value.freeBytes=250; value.writeBytesPerSecond=10;
        QVERIFY(status.setStatus(value)); QCOMPARE(status.lossPercent(),10.); QCOMPARE(status.remainingSeconds(),25.);
        value.freeBytes=2000; QVERIFY(!status.setStatus(value)); QCOMPARE(status.status().freeBytes,quint64(250));
        SignalQualityMeter quality; quality.setSamples({1,-1,.5,-.5,0,std::numeric_limits<double>::quiet_NaN()});
        QCOMPARE(quality.statistics().clippedCount,2); QCOMPARE(quality.statistics().invalidCount,1);
        auto invalid=quality.statistics(); invalid.validCount=100; QVERIFY(!quality.setStatistics(invalid));
        quality.setSamples({0,0}); QVERIFY(std::isinf(quality.statistics().rmsDb));
        quality.resize(360,230); QVERIFY(!render(quality).isNull());
    }
    void acousticDemoRenderingAndPause()
    {
        AcousticPage page; page.resize(1000,760); page.show(); QApplication::processEvents();
        auto* timer=page.findChild<QTimer*>("AcousticDemoTimer"); QVERIFY(timer); QVERIFY(!timer->isActive());
        auto* run=page.findChild<QCheckBox*>("AcousticDemoRun"); QVERIFY(run);
        auto* waveform=page.findChild<AcousticWaveformPlot*>(); QVERIFY(waveform); QCOMPARE(waveform->channelCount(),2);
        auto* spectrum=page.findChild<AcousticSpectrumPlot*>(); QVERIFY(spectrum); QVERIFY(spectrum->frame().isValid());
        auto* spectrogram=page.findChild<AcousticSpectrogramPlot*>(); QVERIFY(spectrogram);
        QVERIFY(spectrogram->storedFrameCount()>80); QVERIFY(spectrogram->storedFrameCount()<spectrogram->historyCapacity());
        run->setChecked(true); QVERIFY(timer->isActive()); page.hide(); QVERIFY(!timer->isActive()); page.show(); QVERIFY(timer->isActive());
        run->setChecked(false); QVERIFY(!timer->isActive());
        const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        auto* manager=StyleManagerAdapter::instance();
        for(const QString theme:{QString("dark"),QString("light")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(auto* plot:page.findChildren<StyledPlot*>()) {
                plot->replot(QCustomPlot::rpImmediateRefresh);
                QCOMPARE(plot->xAxis->tickLabelColor(),ThemeTokens::instance()->textSecondary());
                if(!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(plot->toPixmap(900,320,1.0).save(capture+"/"+plot->objectName()+"-"+theme+".png")); }
            }
        }
        QVERIFY(manager->setCurrentTheme("dark")); QVERIFY(manager->updateTheme());
    }
    void sharedPlotNavigationAndMenu()
    {
        StyledPlot plot; plot.resize(680,360); plot.setPlotTitle("Navigation");
        plot.setViewRanges(QCPRange(0,10),QCPRange(0,10)); plot.captureResetView(); plot.show(); plot.setFocus();
        plot.replot(QCustomPlot::rpImmediateRefresh);
        auto point=[&](double x,double y) { return QPoint(qRound(plot.xAxis->coordToPixel(x)),qRound(plot.yAxis->coordToPixel(y))); };
        QSignalSpy selections(&plot,&StyledPlot::selectionMade);
        plot.setInteractionMode(StyledPlot::InteractionMode::RectangleZoom);
        QTest::mousePress(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(2,8));
        QTest::mouseMove(plot.windowHandle(),point(8,2));
        QTest::mouseRelease(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(8,2));
        QCOMPARE(selections.count(),1); QVERIFY(plot.xAxis->range().size()<7); QVERIFY(plot.yAxis->range().size()<7);
        plot.setInteractionMode(StyledPlot::InteractionMode::Pan);
        QTest::mouseDClick(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(5,5));
        QCOMPARE(plot.xAxis->range().lower,0.); QCOMPARE(plot.xAxis->range().upper,10.);
        plot.setInteractionMode(StyledPlot::InteractionMode::RectangleSelect);
        QTest::mousePress(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(1,9));
        QTest::mouseMove(plot.windowHandle(),point(4,6));
        QTest::mouseRelease(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(4,6));
        QCOMPARE(selections.count(),2); QCOMPARE(plot.xAxis->range().size(),10.);
        plot.setInteractionMode(StyledPlot::InteractionMode::Pan);
        QTest::mousePress(plot.windowHandle(),Qt::LeftButton,Qt::ShiftModifier,point(2,8));
        QTest::mouseMove(plot.windowHandle(),point(8,2));
        QTest::mouseRelease(plot.windowHandle(),Qt::LeftButton,Qt::ShiftModifier,point(8,2));
        QVERIFY(plot.xAxis->range().size()<7); QCOMPARE(plot.interactionMode(),StyledPlot::InteractionMode::Pan);
        QTest::keyClick(&plot,Qt::Key_Home); QCOMPARE(plot.xAxis->range().size(),10.);
        plot.setInteractionMode(StyledPlot::InteractionMode::RectangleZoom);
        QTest::mousePress(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(1,9)); QTest::mouseMove(plot.windowHandle(),point(4,6));
        QTest::keyClick(&plot,Qt::Key_Escape); QVERIFY(!plot.selectionRect()->isActive());
        QTest::mouseRelease(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(4,6)); QCOMPARE(plot.xAxis->range().size(),10.);
        bool actionTriggered=false; int clicks=0;
        connect(&plot,&StyledPlot::coordinateClicked,&plot,[&](double,double,Qt::KeyboardModifiers){ ++clicks; });
        connect(&plot,&StyledPlot::prepareContextMenu,&plot,[&](QMenu* menu,const QPointF& coordinate) {
            QVERIFY(coordinate.x()>0); auto* action=menu->addAction("Custom task"); action->setObjectName("custom.task");
            connect(action,&QAction::triggered,&plot,[&] { actionTriggered=true; });
        });
        QScopedPointer<QMenu> menu(plot.createContextMenu(point(5,5)));
        QVERIFY(menu->findChild<QAction*>("plot.reset")); QVERIFY(menu->findChild<QAction*>("plot.exportPng"));
        menu->findChild<QAction*>("custom.task")->trigger(); QVERIFY(actionTriggered);
        menu->findChild<QAction*>("plot.mode.3")->trigger(); QCOMPARE(plot.interactionMode(),StyledPlot::InteractionMode::Inspect);
        QTest::mouseClick(plot.windowHandle(),Qt::LeftButton,Qt::NoModifier,point(5,5)); QCOMPARE(clicks,1);
    }
    void plotPopupLifetime()
    {
        auto* plot=new StyledPlot; plot->resize(600,300); plot->show(); plot->replot(QCustomPlot::rpImmediateRefresh);
        plot->customContextMenuRequested(plot->axisRect()->rect().center()); QApplication::processEvents();
        QPointer<QMenu> menu=qobject_cast<QMenu*>(QApplication::activePopupWidget()); QVERIFY(menu);
        delete plot; QApplication::processEvents(); QVERIFY(menu.isNull());
    }
    void historyThresholdAndFrozenViewport()
    {
        BearingHistoryPlot plot; QVERIFY(plot.configure(-90,90,3,4,1));
        QVERIFY(!plot.setLevelRange(0,-60));
        QVERIFY(plot.appendFrame(10,{-50,-20,-5})); QVERIFY(plot.setLevelRange(-60,0)); QVERIFY(plot.setDisplayThreshold(-10));
        plot.replot(QCustomPlot::rpImmediateRefresh);
        QCOMPARE(plot.levelAt(0,3),-50.); QCOMPARE(plot.colorMap()->data()->alpha(0,3),static_cast<unsigned char>(0));
        QCOMPARE(plot.colorMap()->data()->alpha(2,3),static_cast<unsigned char>(255));
        plot.clearDisplayThreshold(); plot.replot(QCustomPlot::rpImmediateRefresh);
        QCOMPARE(plot.colorMap()->data()->alpha(0,3),static_cast<unsigned char>(255));
        plot.notifyUserNavigation(); QVERIFY(!plot.followsLatest());
        QVERIFY(plot.setViewRanges(QCPRange(-30,30),QCPRange(8,9))); QVERIFY(plot.appendFrame(11,{-45,-15,-3}));
        QCOMPARE(plot.yAxis->range().lower,8.); QCOMPARE(plot.xAxis->range().upper,30.);
        plot.resetView(); QVERIFY(plot.followsLatest()); QCOMPARE(plot.yAxis->range().upper,11.5); QCOMPARE(plot.xAxis->range().lower,-90.);
        QScopedPointer<QMenu> menu(plot.createContextMenu(QPoint(100,100)));
        QVERIFY(menu->findChild<QAction*>("history.colors")); QVERIFY(menu->findChild<QAction*>("history.follow"));
        QTemporaryDir capture; QVERIFY(plot.saveSnapshot(capture.filePath("history.png"),600,300));
        QVERIFY(QFileInfo(capture.filePath("history.png")).size()>0);
    }
    void multiBeamPickingAndVisibility()
    {
        BeamPatternPlot plot; const QVector<double> angles{0,90,180,270,360};
        QVERIFY(plot.setBeam("A","Array A",angles,{-50,-3,-40,-20,-50}));
        QVERIFY(plot.setBeam("B","Array B",angles,{-45,-12,-30,-8,-45})); QCOMPARE(plot.beamCount(),2);
        QVERIFY(!plot.setBeam("bad","bad",{0,90,80},{1,2,3})); QCOMPARE(plot.beamCount(),2);
        plot.resize(700,320); plot.show(); plot.replot(QCustomPlot::rpImmediateRefresh);
        QSignalSpy picked(&plot,&BeamPatternPlot::beamPicked);
        const QPoint at(qRound(plot.xAxis->coordToPixel(90)),qRound(plot.yAxis->coordToPixel(-3)));
        QTest::mouseClick(&plot,Qt::LeftButton,Qt::NoModifier,at); QCOMPARE(picked.count(),1); QCOMPARE(picked.first().at(0).toString(),QString("A"));
        QVERIFY(plot.setBeamVisible("A",false)); QVERIFY(!plot.graph(0)->visible());
        QVERIFY(plot.removeBeam("A")); QCOMPARE(plot.beamCount(),1);
        QVERIFY(!plot.setBearingRange(-90,90)); plot.clearBeams(); QVERIFY(plot.setBearingRange(-90,90));
    }
    void demonKnownAMAndAntiAlias()
    {
        const int n=4096; const double rate=8192,pi=std::acos(-1.); QVector<double> samples(n);
        for(int i=0;i<n;++i) { const double time=i/rate; samples[i]=(1+.5*std::cos(2*pi*32*time)+.2*std::cos(2*pi*320*time))*std::cos(2*pi*2048*time); }
        DemonSettings settings; settings.carrierLowerHz=1500; settings.carrierUpperHz=2600; settings.maximumModulationHz=100; settings.decimation=16;
        DemonResult result; QString error; QVERIFY2(DemonAnalysis::analyze(samples,rate,settings,&result,&error),qPrintable(error)); QVERIFY(result.isValid());
        QCOMPARE(result.spectrum.sampleRate,512.); QCOMPARE(result.normalizedEnvelope.size(),256);
        QVERIFY(std::abs(result.meanEnvelope-1)<1e-10);
        DemonSpectrumPlot plot; QVERIFY(plot.setResult(result)); QCOMPARE(plot.peakFrequencyHz(),32.);
        DemonResult external=result; external.normalizedEnvelope.clear(); external.meanEnvelope=std::numeric_limits<double>::quiet_NaN();
        QVERIFY(external.isValid()); QVERIFY(plot.setResult(external)); QCOMPARE(plot.peakFrequencyHz(),32.);
        QVERIFY(std::abs(AcousticAnalysis::bandPowerDb(result.spectrum,0,256)-10*std::log10(.125))<1e-8);
        QVERIFY(result.spectrum.linearPower[96]<1e-15); // 320 Hz must not alias to 192 Hz after decimation
        QVERIFY(result.spectrum.levelLabel().contains("包络")); QVERIFY(!result.spectrum.levelLabel().contains("dBFS"));
        QVector<double> amplified=samples; for(auto& value:amplified)value*=3;
        DemonResult gainInvariant; QVERIFY(DemonAnalysis::analyze(amplified,rate,settings,&gainInvariant));
        QVERIFY(std::abs(gainInvariant.meanEnvelope/result.meanEnvelope-3)<1e-10);
        for(int i=0;i<result.spectrum.linearPower.size();++i)
            QVERIFY(std::abs(result.spectrum.linearPower[i]-gainInvariant.spectrum.linearPower[i])<1e-10);
        const auto original=result.spectrum.linearPower;
        auto invalid=settings; invalid.decimation=3; QVERIFY(!DemonAnalysis::analyze(samples,rate,invalid,&result)); QCOMPARE(result.spectrum.linearPower,original);
        invalid=settings; invalid.maximumModulationHz=300; QVERIFY(!DemonAnalysis::analyze(samples,rate,invalid,&result));
        QVERIFY(!DemonAnalysis::analyze(QVector<double>(n,0),rate,settings,&result));
        QVector<double> outside(n); for(int i=0;i<n;++i) outside[i]=std::sin(2*pi*100*i/rate);
        QVERIFY(!DemonAnalysis::analyze(outside,rate,settings,&result));
        DemonHistoryPlot history; QVERIFY(history.configure(result,4,.5)); QVERIFY(history.appendResult(0,result));
        DemonResult changed=result; changed.settings.carrierLowerHz=1600; QVERIFY(!history.appendResult(.5,changed));
        LofarPlot lofar; QVERIFY(!lofar.configure(result.spectrum,4,.5,0,100));
        AcousticSpectrumFrame raw; QVERIFY(AcousticAnalysis::spectrum(samples,rate,&raw)); QVERIFY(lofar.configure(raw,4,.5,0,400)); QVERIFY(lofar.appendFrame(0,raw));
    }
    void rateCounterResetAndUnits()
    {
        TransferRateMeter meter; QSignalSpy resets(&meter,&TransferRateMeter::baselineReset);
        QVERIFY(meter.addCounters(0,100,200)); QVERIFY(!meter.hasRates());
        QVERIFY(meter.addCounters(2,2100,1200)); QCOMPARE(meter.receiveBytesPerSecond(),1000.); QCOMPARE(meter.sendBytesPerSecond(),500.);
        QVERIFY(!meter.addCounters(1,2200,1300)); QCOMPARE(meter.receiveBytesPerSecond(),1000.);
        QVERIFY(meter.addCounters(3,5,7)); QVERIFY(!meter.hasRates()); QCOMPARE(resets.count(),2);
        QVERIFY(meter.addCounters(4,1005,507)); QCOMPARE(meter.receiveBytesPerSecond(),1000.);
        meter.setStale(true); QVERIFY(std::isnan(meter.receiveBytesPerSecond())); meter.setStale(false); QVERIFY(meter.hasRates());
        QVERIFY(meter.addCounters(40,9999,9999)); QVERIFY(!meter.hasRates());
        QVERIFY(meter.setRates(41,1048576,2097152)); QCOMPARE(meter.receiveBytesPerSecond(),1048576.);
        meter.setDisplayUnit(TransferRateMeter::DisplayUnit::BinaryMiBPerSecond); QCOMPARE(meter.displayUnit(),TransferRateMeter::DisplayUnit::BinaryMiBPerSecond);
        QVERIFY(!meter.setLinkCapacity(-1)); QVERIFY(meter.setLinkCapacity(1e9));
        QVERIFY(meter.addCounters(42,12000,12000)); QVERIFY(!meter.hasRates());
    }
    void storageCapacityValidationAndClicks()
    {
        StorageCapacityMeter meter; QVERIFY(meter.setCapacity(1024,{{"raw",256,{}},{"processed",256,{}}})); QCOMPARE(meter.usedBytes(),quint64(512)); QCOMPARE(meter.freeBytes(),quint64(512));
        QVERIFY(!meter.setCapacity(100,{{"bad",101,{}}})); QCOMPARE(meter.totalBytes(),quint64(1024));
        const quint64 largest=std::numeric_limits<quint64>::max(); QVERIFY(!meter.setCapacity(largest,{{"a",largest-10,{}},{"b",11,{}}}));
        meter.resize(400,230); meter.show(); QSignalSpy selected(&meter,&StorageCapacityMeter::segmentSelected);
        QTest::mouseClick(&meter,Qt::LeftButton,Qt::NoModifier,QPoint(40,95)); QCOMPARE(selected.count(),1); QCOMPARE(selected.first().at(0).toInt(),0);
        QTest::mouseClick(&meter,Qt::LeftButton,Qt::NoModifier,QPoint(370,95)); QCOMPARE(selected.last().at(0).toInt(),-1);
        QVERIFY(!meter.setWarningPercent(101)); QVERIFY(meter.setWarningPercent(80));
    }
    void rateHistoryBoundsAndReset()
    {
        RateHistoryPlot plot; QVERIFY(plot.configure(4,10,"MiB/s",2)); plot.addSeries("rx"); plot.addSeries("tx");
        QVERIFY(plot.appendValues(1,{1,2})); QVERIFY(plot.appendValues(2,{2,3})); QVERIFY(plot.appendValues(3,{3,4})); QVERIFY(plot.appendValues(10,{10,20}));
        QCOMPARE(plot.storedPointCount(),4); bool gap=false;
        for(auto it=plot.graph(0)->data()->constBegin();it!=plot.graph(0)->data()->constEnd();++it) if(std::isnan(it->value))gap=true;
        QVERIFY(gap); QVERIFY(!plot.appendValues(10,{1,2}));
        plot.notifyUserNavigation(); plot.setViewRanges(QCPRange(2,3),QCPRange(0,10)); QVERIFY(plot.appendValues(11,{11,21}));
        QCOMPARE(plot.xAxis->range().lower,2.); plot.resetView(); QCOMPARE(plot.xAxis->range().upper,11.); QVERIFY(plot.followsLatest());
    }
    void taskQueueDashboardValidationAndInteraction()
    {
        TaskQueueDashboard tasks; tasks.resize(420,380);
        QVERIFY(tasks.setQueues({{"import","导入",10,2,90,3},{"index","索引",20,4,70,1}}));
        QVERIFY(!tasks.setQueues({{"dup","A",0,0,0,0},{"dup","B",0,0,0,0}}));
        QVERIFY(!tasks.setQueues({{"overflow","Overflow",std::numeric_limits<quint64>::max(),1,0,0}}));
        QCOMPARE(tasks.queues().size(),2); QCOMPARE(tasks.queues().first().id,QString("import"));
        QSignalSpy activated(&tasks,&TaskQueueDashboard::stateActivated);
        tasks.show(); tasks.activateWindow(); tasks.setFocus(); QTest::qWait(10);
        QTest::mouseClick(&tasks,Qt::LeftButton,Qt::NoModifier,QPoint(360,105));
        QCOMPARE(activated.count(),1); QCOMPARE(activated.last().at(0).toString(),QString("import"));
        QCOMPARE(qvariant_cast<TaskQueueDashboard::State>(activated.last().at(1)),TaskQueueDashboard::State::Failed);
        // Reordering a snapshot preserves keyboard focus on the stable business ID.
        QVERIFY(tasks.setQueues({{"index","索引",20,4,70,1},{"import","导入",10,2,90,3}}));
        QTest::keyClick(&tasks,Qt::Key_Left); QTest::keyClick(&tasks,Qt::Key_Return);
        QCOMPARE(activated.last().at(0).toString(),QString("import"));
        QCOMPARE(qvariant_cast<TaskQueueDashboard::State>(activated.last().at(1)),TaskQueueDashboard::State::Succeeded);
        QPointer<QMenu> opened;
        connect(&tasks,&TaskQueueDashboard::prepareContextMenu,&tasks,[&](QMenu* menu,const QString& id,TaskQueueDashboard::State state) {
            opened=menu; QCOMPARE(id,QString("import")); QCOMPARE(state,TaskQueueDashboard::State::Succeeded); menu->addAction("Inspect task");
        });
        QContextMenuEvent event(QContextMenuEvent::Keyboard,QPoint(),QPoint()); QApplication::sendEvent(&tasks,&event);
        QVERIFY(opened); opened->close();
        QVERIFY(tasks.setQueues({})); QVERIFY(tasks.sizeHint().height()<200);
    }
    void qualityDashboardValidationAndInteraction()
    {
        DataQualityDashboard quality; quality.resize(420,380);
        QVERIFY(!quality.hasSnapshot()); QVERIFY(std::isnan(quality.coveragePercent())); QVERIFY(std::isnan(quality.passPercent()));
        DataQualitySnapshot snapshot; snapshot.total=1000; snapshot.valid=700; snapshot.missing=20; snapshot.corrupt=10; snapshot.duplicate=30;
        QVERIFY(quality.setSnapshot(snapshot)); QCOMPARE(quality.checkedRecords(),quint64(760)); QCOMPARE(quality.uncheckedRecords(),quint64(240));
        QCOMPARE(quality.coveragePercent(),76.); QVERIFY(qAbs(quality.passPercent()-70000./760)<1e-10);
        auto invalid=snapshot; invalid.valid=1000; QVERIFY(!quality.setSnapshot(invalid)); QCOMPARE(quality.snapshot().valid,quint64(700));
        QSignalSpy activated(&quality,&DataQualityDashboard::categoryActivated);
        quality.show(); quality.activateWindow(); quality.setFocus(); QTest::qWait(10);
        QTest::mouseClick(&quality,Qt::LeftButton,Qt::NoModifier,QPoint(100,252));
        QCOMPARE(activated.count(),1); QCOMPARE(qvariant_cast<DataQualityDashboard::Category>(activated.last().first()),DataQualityDashboard::Category::Corrupt);
        QTest::keyClick(&quality,Qt::Key_Down); QTest::keyClick(&quality,Qt::Key_Space);
        QCOMPARE(qvariant_cast<DataQualityDashboard::Category>(activated.last().first()),DataQualityDashboard::Category::Duplicate);
        QPointer<QMenu> opened;
        connect(&quality,&DataQualityDashboard::prepareContextMenu,&quality,[&](QMenu* menu,DataQualityDashboard::Category category) {
            opened=menu; QCOMPARE(category,DataQualityDashboard::Category::Duplicate); menu->addAction("Inspect records");
        });
        QContextMenuEvent event(QContextMenuEvent::Keyboard,QPoint(),QPoint()); QApplication::sendEvent(&quality,&event);
        QVERIFY(opened); opened->close();
        snapshot={}; snapshot.total=std::numeric_limits<quint64>::max(); snapshot.valid=snapshot.total-2; snapshot.corrupt=2;
        QVERIFY(quality.setSnapshot(snapshot)); QCOMPARE(quality.uncheckedRecords(),quint64(0)); QCOMPARE(quality.coveragePercent(),100.);
        snapshot.missing=1; QVERIFY(!quality.setSnapshot(snapshot));
        QVERIFY(quality.setSnapshot({})); QVERIFY(quality.hasSnapshot()); QVERIFY(std::isnan(quality.passPercent()));
        quality.clear(); QVERIFY(!quality.hasSnapshot());
    }
    void dashboardRendering()
    {
        auto* manager=StyleManagerAdapter::instance();
        TaskQueueDashboard queues; DataQualityDashboard quality;
        queues.setQueues({{"import","录音导入",12,3,280,5},{"index","索引构建",24,4,240,2},{"archive","归档迁移",60,2,120,0}});
        DataQualitySnapshot snapshot; snapshot.total=10000; snapshot.valid=7800; snapshot.missing=12; snapshot.corrupt=5; snapshot.duplicate=80;
        quality.setSnapshot(snapshot);
        const QString directory=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        for(const QString theme:{QString("dark"),QString("light")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(int width:{280,440}) {
                queues.resize(width,queues.sizeHint().height()); quality.resize(width,quality.sizeHint().height());
                const auto a=render(queues),b=render(quality); QVERIFY(!a.isNull()); QVERIFY(!b.isNull());
                if(!directory.isEmpty()) {
                    QDir().mkpath(directory);
                    QVERIFY(a.save(QString("%1/TaskQueues-%2-%3.png").arg(directory,theme).arg(width)));
                    QVERIFY(b.save(QString("%1/DataQuality-%2-%3.png").arg(directory,theme).arg(width)));
                }
            }
        }
        queues.setEnabled(false); quality.setEnabled(false); QVERIFY(!render(queues).isNull()); QVERIFY(!render(quality).isNull());
    }
    void extendedBusinessDemoRender()
    {
        DirectionAnalysisPage direction; DataManagementPage management;
        direction.resize(1000,760); management.resize(1000,760);
        auto* demon=direction.findChild<DemonSpectrumPlot*>(); QVERIFY(demon); QCOMPARE(demon->peakFrequencyHz(),32.);
        auto* bearing=direction.findChild<BearingHistoryPlot*>(); QVERIFY(bearing); QVERIFY(bearing->storedFrameCount()>50);
        auto verifyPause=[](QWidget& page,const char* timerName,const char* runName) {
            auto* timer=page.findChild<QTimer*>(timerName); auto* run=page.findChild<QCheckBox*>(runName);
            QVERIFY(timer); QVERIFY(run); QVERIFY(!timer->isActive()); run->setChecked(true); QVERIFY(!timer->isActive());
            page.show(); QApplication::processEvents(); QVERIFY(timer->isActive()); page.hide(); QVERIFY(!timer->isActive()); run->setChecked(false);
        };
        verifyPause(direction,"DirectionDemoTimer","DirectionDemoRun"); verifyPause(management,"DataManagementTimer","DataManagementRun");
        const QString capture=qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR"); auto* manager=StyleManagerAdapter::instance();
        for(const QString theme:{QString("dark"),QString("light")}) {
            QVERIFY(manager->setCurrentTheme(theme)); QVERIFY(manager->updateTheme());
            for(QWidget* page:{static_cast<QWidget*>(&direction),static_cast<QWidget*>(&management)}) {
                page->show(); QApplication::processEvents();
                for(auto* plot:page->findChildren<StyledPlot*>()) {
                    QScopedPointer<QMenu> menu(plot->createContextMenu(QPoint(100,100))); QVERIFY(menu->findChild<QAction*>("plot.reset"));
                    if(!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(plot->saveSnapshot(capture+"/"+plot->objectName()+"-"+theme+".png",900,320)); }
                }
                page->hide();
            }
            if(!capture.isEmpty()) {
                auto* capacity=management.findChild<StorageCapacityMeter*>(); auto* rate=management.findChild<TransferRateMeter*>();
                capacity->resize(420,240); rate->resize(420,240);
                QVERIFY(render(*capacity).save(capture+"/Capacity-"+theme+".png")); QVERIFY(render(*rate).save(capture+"/TransferRate-"+theme+".png"));
            }
        }
        QVERIFY(manager->setCurrentTheme("dark")); QVERIFY(manager->updateTheme());
    }
    void renderGallery()
    {
        const QString capture = qEnvironmentVariable("BW_NATIVE_CAPTURE_DIR");
        QWidget gallery; gallery.setWindowTitle("Native painter controls");
        QGridLayout layout(&gallery);
        PushButton normal("&Apply"), checked("Selected"), disabled("Disabled");
        checked.setCheckable(true); checked.setChecked(true); disabled.setEnabled(false);
        layout.addWidget(&normal,0,0); layout.addWidget(&checked,0,1); layout.addWidget(&disabled,0,2);
        PushButton primary("Primary"), subtle("Subtle"), danger("Danger");
        primary.setAppearance(ButtonAppearance::Primary); subtle.setAppearance(ButtonAppearance::Subtle);
        danger.setAppearance(ButtonAppearance::Danger);
        layout.addWidget(&primary,6,0); layout.addWidget(&subtle,6,1); layout.addWidget(&danger,6,2);
        CheckBox check("Checked"), partial("Partial"); check.setChecked(true); partial.setCheckState(Qt::PartiallyChecked);
        RadioButton radio("Selected radio"); radio.setChecked(true);
        layout.addWidget(&check,1,0); layout.addWidget(&partial,1,1); layout.addWidget(&radio,1,2);
        Slider slider(Qt::Horizontal); slider.setValue(50); slider.setTickPosition(QSlider::TicksBelow);
        layout.addWidget(&slider,2,0,1,3);
        QLineEdit line("Editable text"); QComboBox combo; combo.addItems({"Choice one","Choice two"}); QSpinBox spin; spin.setValue(42);
        layout.addWidget(&line,3,0); layout.addWidget(&combo,3,1); layout.addWidget(&spin,3,2);
        QToolButton tool; tool.setText("Tool button"); QMenu menu; menu.addAction("Action"); tool.setMenu(&menu); tool.setPopupMode(QToolButton::MenuButtonPopup);
        ProgressBar progress; progress.setValue(42);
        layout.addWidget(&tool,4,0); layout.addWidget(&progress,4,1,1,2);
        QTabWidget tabs; QWidget page; QVBoxLayout pageLayout(&page);
        QGroupBox group("Checkable group"); group.setCheckable(true); QVBoxLayout groupLayout(&group); QLabel label("Palette + QPainter, no QSS"); groupLayout.addWidget(&label);
        pageLayout.addWidget(&group);
        QTextEdit edit("Text editing, selection, cursor and scrolling stay with Qt."); pageLayout.addWidget(&edit);
        QTreeWidget tree; tree.setHeaderLabels({"Name","Value"});
        for(int i=0;i<12;++i) new QTreeWidgetItem(&tree,{QString("Row %1").arg(i),QString::number(i)});
        pageLayout.addWidget(&tree); tabs.addTab(&page,"Controls"); tabs.addTab(new QWidget,"Second tab");
        layout.addWidget(&tabs,5,0,1,3);
        gallery.resize(700,620); gallery.show();
        for (const QString theme : {QString("dark"),QString("light")}) {
            auto* mgr = StyleManagerAdapter::instance(); QVERIFY(mgr->setCurrentTheme(theme)); QVERIFY(mgr->updateTheme());
            QTest::qWait(30); const QImage image = render(gallery); QVERIFY(!image.isNull());
            if (!capture.isEmpty()) { QDir().mkpath(capture); QVERIFY(gallery.grab().save(capture+"/native-"+theme+".png")); }
            QVERIFY(qApp->styleSheet().isEmpty());
        }
    }
};
int runDemoChecks()
{
    NativeStyleTests checks;
    char name[] = "widgets_demo --verify";
    char* args[] = {name, nullptr};
    return QTest::qExec(&checks, 1, args);
}
#include "DemoChecks.moc"
