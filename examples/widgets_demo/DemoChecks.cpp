// 可选的 demo 验证入口（-DUI_DEMO_CHECKS=ON，运行 widgets_demo --verify）。
// 检查渲染/交互/生命周期，不作为独立测试工程或应用安装依赖。
#include <QtTest>
#include <QApplication>
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
#include <basewidgets/Core/FramelessDialog.h>
#include <basewidgets/Core/CustomTitleBar.h>
#include <basewidgets/Containers/ResponsiveGridLayout.h>
#include <basewidgets/Inputs/ElidedLabel.h>
#include <basewidgets/Inputs/DateRangePicker.h>
#include <basewidgets/Views/NavigationList.h>
#include <basewidgets/Views/AdvancedTable.h>
#include <QStandardItemModel>
#include <QClipboard>
#include <basewidgets/Views/TableView.h>
#include <limits>
#include <basewidgets/Core/NativeStyle.h>
#include <basewidgets/Core/StyleManagerAdapter.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <basewidgets/Inputs/CheckBox.h>
#include <basewidgets/Inputs/BusyIndicator.h>
#include <basewidgets/Inputs/PushButton.h>
#include <basewidgets/Inputs/ToolButton.h>
#include <basewidgets/Inputs/RadioButton.h>
#include <basewidgets/Inputs/Slider.h>
#include <basewidgets/Inputs/ProgressBar.h>
#include <basewidgets/Inputs/ToggleSwitch.h>

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
        QVERIFY(partial.pixelColor(8,7).lightness() > partial.pixelColor(8,4).lightness());
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
        QVERIFY(opened); QCOMPARE(QApplication::clipboard()->text(),expected);
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
