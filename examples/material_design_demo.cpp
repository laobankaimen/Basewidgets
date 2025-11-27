/*******************************************************************************
 * Material Design 示例应用 - 展示如何使用新的CSS模板
 * 
 * 这个示例展示了如何使用material.css.template中的所有新组件和效果
 *******************************************************************************/

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QFrame>
#include <QToolButton>
#include <QToolBar>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSlider>
#include <QProgressBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>

// 样式刷新辅助函数
void refreshWidgetStyle(QWidget* widget) {
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

// 主窗口类
class MaterialDesignDemo : public QWidget {
    Q_OBJECT

public:
    MaterialDesignDemo(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        applyMaterialDesignStyles();
    }

private:
    void setupUI() {
        setWindowTitle("Material Design 组件演示");
        resize(1200, 800);
        
        // 主布局
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(16);
        mainLayout->setContentsMargins(24, 24, 24, 24);
        
        // 标题区域
        QLabel *titleLabel = new QLabel("Material Design 组件库");
        titleLabel->setStyleSheet("font-size: 32px; font-weight: 700; margin-bottom: 16px;");
        mainLayout->addWidget(titleLabel);
        
        // 创建标签页
        QTabWidget *tabWidget = new QTabWidget();
        tabWidget->addTab(createButtonsTab(), "按钮组件");
        tabWidget->addTab(createInputTab(), "输入控件");
        tabWidget->addTab(createLayoutTab(), "布局组件");
        tabWidget->addTab(createAdvancedTab(), "高级组件");
        
        mainLayout->addWidget(tabWidget);
        
        // 浮动操作按钮 (FAB)
        QPushButton *fabButton = new QPushButton("+");
        fabButton->setProperty("fab", true);
        // 注意：在实际应用中，你需要使用布局或绝对定位来放置FAB
    }
    
    QWidget* createButtonsTab() {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tab);
        layout->setSpacing(16);
        
        // 基础按钮
        QGroupBox *basicGroup = new QGroupBox("基础按钮");
        QVBoxLayout *basicLayout = new QVBoxLayout(basicGroup);
        
        QPushButton *defaultBtn = new QPushButton("默认按钮");
        basicLayout->addWidget(defaultBtn);
        
        QPushButton *primaryBtn = new QPushButton("主要按钮");
        primaryBtn->setProperty("primary", true);
        basicLayout->addWidget(primaryBtn);
        
        QPushButton *secondaryBtn = new QPushButton("次要按钮");
        secondaryBtn->setProperty("secondary", true);
        basicLayout->addWidget(secondaryBtn);
        
        QPushButton *dangerBtn = new QPushButton("危险按钮");
        dangerBtn->setProperty("danger", true);
        basicLayout->addWidget(dangerBtn);
        
        layout->addWidget(basicGroup);
        
        // 浮动操作按钮
        QGroupBox *fabGroup = new QGroupBox("浮动操作按钮 (FAB)");
        QHBoxLayout *fabLayout = new QHBoxLayout(fabGroup);
        
        QPushButton *fabNormal = new QPushButton("+");
        fabNormal->setProperty("fab", true);
        fabLayout->addWidget(fabNormal);
        
        QPushButton *fabExtended = new QPushButton("创建");
        fabExtended->setProperty("fab", true);
        fabExtended->setProperty("extended", true);  // 扩展型FAB
        fabLayout->addWidget(fabExtended);
        
        layout->addWidget(fabGroup);
        
        // 工具栏
        QGroupBox *toolbarGroup = new QGroupBox("工具栏");
        QVBoxLayout *toolbarLayout = new QVBoxLayout(toolbarGroup);
        
        QToolBar *toolBar = new QToolBar();
        
        QToolButton *newBtn = new QToolButton();
        newBtn->setText("新建");
        newBtn->setCheckable(true);
        toolBar->addWidget(newBtn);
        
        QToolButton *openBtn = new QToolButton();
        openBtn->setText("打开");
        openBtn->setCheckable(true);
        toolBar->addWidget(openBtn);
        
        toolBar->addSeparator();
        
        QToolButton *saveBtn = new QToolButton();
        saveBtn->setText("保存");
        toolBar->addWidget(saveBtn);
        
        toolbarLayout->addWidget(toolBar);
        
        layout->addWidget(toolbarGroup);
        
        layout->addStretch();
        return tab;
    }
    
    QWidget* createInputTab() {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tab);
        layout->setSpacing(16);
        
        // 输入框
        QGroupBox *inputGroup = new QGroupBox("输入控件");
        QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
        
        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setPlaceholderText("请输入文本...");
        inputLayout->addWidget(lineEdit);
        
        QTextEdit *textEdit = new QTextEdit();
        textEdit->setPlaceholderText("请输入多行文本...");
        textEdit->setMaximumHeight(100);
        inputLayout->addWidget(textEdit);
        
        layout->addWidget(inputGroup);
        
        // 选择控件
        QGroupBox *choiceGroup = new QGroupBox("选择控件");
        QVBoxLayout *choiceLayout = new QVBoxLayout(choiceGroup);
        
        QCheckBox *checkBox = new QCheckBox("复选框选项");
        checkBox->setChecked(true);
        choiceLayout->addWidget(checkBox);
        
        QRadioButton *radio1 = new QRadioButton("单选按钮 1");
        radio1->setChecked(true);
        choiceLayout->addWidget(radio1);
        
        QRadioButton *radio2 = new QRadioButton("单选按钮 2");
        choiceLayout->addWidget(radio2);
        
        layout->addWidget(choiceGroup);
        
        // 滑块和进度条
        QGroupBox *sliderGroup = new QGroupBox("滑块和进度条");
        QVBoxLayout *sliderLayout = new QVBoxLayout(sliderGroup);
        
        QSlider *slider = new QSlider(Qt::Horizontal);
        slider->setRange(0, 100);
        slider->setValue(50);
        sliderLayout->addWidget(slider);
        
        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(75);
        sliderLayout->addWidget(progressBar);
        
        layout->addWidget(sliderGroup);
        
        layout->addStretch();
        return tab;
    }
    
    QWidget* createLayoutTab() {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tab);
        layout->setSpacing(16);
        
        // 卡片布局
        QGroupBox *cardGroup = new QGroupBox("卡片布局");
        QVBoxLayout *cardLayout = new QVBoxLayout(cardGroup);
        
        // 卡片1
        QFrame *card1 = new QFrame();
        card1->setProperty("card", true);
        QVBoxLayout *card1Layout = new QVBoxLayout(card1);
        card1Layout->setContentsMargins(16, 16, 16, 16);
        
        QLabel *card1Title = new QLabel("卡片标题 1");
        card1Title->setStyleSheet("font-weight: 600; font-size: 16px; margin-bottom: 8px;");
        card1Layout->addWidget(card1Title);
        
        QLabel *card1Content = new QLabel("这是一个使用卡片样式的容器，具有阴影和圆角效果。");
        card1Content->setWordWrap(true);
        card1Layout->addWidget(card1Content);
        
        cardLayout->addWidget(card1);
        
        // 卡片2
        QFrame *card2 = new QFrame();
        card2->setProperty("card", true);
        QVBoxLayout *card2Layout = new QVBoxLayout(card2);
        card2Layout->setContentsMargins(16, 16, 16, 16);
        
        QLabel *card2Title = new QLabel("卡片标题 2");
        card2Title->setStyleSheet("font-weight: 600; font-size: 16px; margin-bottom: 8px;");
        card2Layout->addWidget(card2Title);
        
        QLabel *card2Content = new QLabel("卡片在悬停时会有提升效果，增强交互体验。");
        card2Content->setWordWrap(true);
        card2Layout->addWidget(card2Content);
        
        cardLayout->addWidget(card2);
        
        layout->addWidget(cardGroup);
        
        // 玻璃态效果
        QGroupBox *glassGroup = new QGroupBox("玻璃态效果");
        QVBoxLayout *glassLayout = new QVBoxLayout(glassGroup);
        
        QWidget *glassWidget = new QWidget();
        glassWidget->setProperty("glass", true);
        QVBoxLayout *glassWidgetLayout = new QVBoxLayout(glassWidget);
        glassWidgetLayout->setContentsMargins(16, 16, 16, 16);
        
        QLabel *glassLabel = new QLabel("玻璃态效果");
        glassLabel->setStyleSheet("font-weight: 600; font-size: 16px;");
        glassWidgetLayout->addWidget(glassLabel);
        
        QLabel *glassContent = new QLabel("具有模糊背景效果的玻璃态组件。");
        glassWidgetLayout->addWidget(glassContent);
        
        glassLayout->addWidget(glassWidget);
        
        layout->addWidget(glassGroup);
        
        // 分割线
        QFrame *divider = new QFrame();
        divider->setProperty("divider", "horizontal");
        layout->addWidget(divider);
        
        // 扁平框架
        QGroupBox *flatGroup = new QGroupBox("扁平框架");
        QVBoxLayout *flatLayout = new QVBoxLayout(flatGroup);
        
        QFrame *flatFrame = new QFrame();
        flatFrame->setProperty("flat", true);
        QVBoxLayout *flatFrameLayout = new QVBoxLayout(flatFrame);
        flatFrameLayout->addWidget(new QLabel("这是一个扁平框架，没有边框"));
        flatLayout->addWidget(flatFrame);
        
        layout->addWidget(flatGroup);
        
        layout->addStretch();
        return tab;
    }
    
    QWidget* createAdvancedTab() {
        QWidget *tab = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(tab);
        layout->setSpacing(16);
        
        // 芯片标签
        QGroupBox *chipGroup = new QGroupBox("芯片标签");
        QHBoxLayout *chipLayout = new QHBoxLayout(chipGroup);
        
        QLabel *chip1 = new QLabel("标签1");
        chip1->setProperty("chip", true);
        chipLayout->addWidget(chip1);
        
        QLabel *chip2 = new QLabel("标签2");
        chip2->setProperty("chip", true);
        chip2->setProperty("selected", true);
        chipLayout->addWidget(chip2);
        
        QLabel *chip3 = new QLabel("标签3");
        chip3->setProperty("chip", true);
        chipLayout->addWidget(chip3);
        
        layout->addWidget(chipGroup);
        
        // 徽章
        QGroupBox *badgeGroup = new QGroupBox("徽章");
        QHBoxLayout *badgeLayout = new QHBoxLayout(badgeGroup);
        
        QLabel *badge1 = new QLabel("5");
        badge1->setProperty("badge", true);
        badgeLayout->addWidget(badge1);
        
        QLabel *badge2 = new QLabel("99+");
        badge2->setProperty("badge", true);
        badgeLayout->addWidget(badge2);
        
        QLabel *badge3 = new QLabel("新");
        badge3->setProperty("badge", true);
        badgeLayout->addWidget(badge3);
        
        layout->addWidget(badgeGroup);
        
        // 加载状态
        QGroupBox *loadingGroup = new QGroupBox("加载状态");
        QVBoxLayout *loadingLayout = new QVBoxLayout(loadingGroup);
        
        QLabel *loadingLabel = new QLabel("正在加载中...");
        loadingLabel->setProperty("loading", true);
        loadingLayout->addWidget(loadingLabel);
        
        layout->addWidget(loadingGroup);
        
        // 导航栏示例
        QGroupBox *navGroup = new QGroupBox("导航栏");
        QVBoxLayout *navLayout = new QVBoxLayout(navGroup);
        
        QWidget *navWidget = new QWidget();
        navWidget->setProperty("navigation", true);
        QVBoxLayout *navWidgetLayout = new QVBoxLayout(navWidget);
        navWidgetLayout->setContentsMargins(8, 8, 8, 8);
        
        QPushButton *navBtn1 = new QPushButton("🏠 首页");
        navBtn1->setProperty("active", true);
        navWidgetLayout->addWidget(navBtn1);
        
        QPushButton *navBtn2 = new QPushButton("📊 统计");
        navWidgetLayout->addWidget(navBtn2);
        
        QPushButton *navBtn3 = new QPushButton("⚙️ 设置");
        navWidgetLayout->addWidget(navBtn3);
        
        navLayout->addWidget(navWidget);
        
        layout->addWidget(navGroup);
        
        layout->addStretch();
        return tab;
    }
    
    void applyMaterialDesignStyles() {
        // 为需要动态更新样式的控件连接信号槽
        // 这里可以添加更多动态样式更新的逻辑
    }
};

// 主函数
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序样式为Material Design
    app.setStyle("Fusion");
    
    // 加载Material Design样式表
    QFile styleFile(":/styles/qt_material/material.css");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
        styleFile.close();
    }
    
    MaterialDesignDemo demo;
    demo.show();
    
    return app.exec();
}

#include "material_design_demo.moc"