# Material Design CSS 模板使用指南

## 🎯 快速开始

本指南将帮助你使用新的 Material Design CSS 模板，展示如何在代码中设置控件属性以获得完美的视觉效果。

## 📋 属性设置速查表

### 🔘 按钮组件

#### 基础按钮类型
```cpp
// 默认按钮（无需特殊属性）
QPushButton *defaultBtn = new QPushButton("默认按钮");

// 主要按钮 - 使用主题色
QPushButton *primaryBtn = new QPushButton("主要按钮");
primaryBtn->setProperty("primary", true);

// 次要按钮 - 轮廓样式
QPushButton *secondaryBtn = new QPushButton("次要按钮");
secondaryBtn->setProperty("secondary", true);

// 危险按钮 - 错误状态
QPushButton *dangerBtn = new QPushButton("危险按钮");
dangerBtn->setProperty("danger", true);
```

#### 浮动操作按钮 (FAB)
```cpp
// 标准FAB
QPushButton *fab = new QPushButton("+");
fab->setProperty("fab", true);

// 扩展型FAB
QPushButton *extendedFab = new QPushButton("创建");
extendedFab->setProperty("fab", true);
extendedFab->setProperty("extended", true);
```

#### 工具栏按钮
```cpp
// 可切换状态的工具栏按钮
QToolButton *toolBtn = new QToolButton();
toolBtn->setCheckable(true);
toolBtn->setText("工具");

// 连接信号槽以更新样式
connect(toolBtn, &QToolButton::toggled, toolBtn, [=](bool checked) {
    toolBtn->setProperty("checked", checked);
    refreshWidgetStyle(toolBtn);
});
```

### 🏷️ 标签组件

#### 芯片标签 (Chips)
```cpp
// 基础芯片
QLabel *chip = new QLabel("标签");
chip->setProperty("chip", true);

// 选中状态的芯片
QLabel *selectedChip = new QLabel("已选中");
selectedChip->setProperty("chip", true);
selectedChip->setProperty("selected", true);

// 可点击的芯片（使用按钮实现）
QPushButton *clickableChip = new QPushButton("可点击");
clickableChip->setProperty("chip", true);
```

#### 徽章 (Badges)
```cpp
// 数字徽章
QLabel *badge = new QLabel("5");
badge->setProperty("badge", true);

// 文本徽章
QLabel *textBadge = new QLabel("新");
textBadge->setProperty("badge", true);
```

#### 加载状态
```cpp
QLabel *loadingLabel = new QLabel("加载中...");
loadingLabel->setProperty("loading", true);
```

### 📦 容器组件

#### 卡片 (Cards)
```cpp
// 基础卡片
QFrame *card = new QFrame();
card->setProperty("card", true);

// 卡片内容布局
QVBoxLayout *cardLayout = new QVBoxLayout(card);
cardLayout->setContentsMargins(16, 16, 16, 16);
```

#### 玻璃态效果 (Glass)
```cpp
QWidget *glassWidget = new QWidget();
glassWidget->setProperty("glass", true);
```

#### 扁平框架
```cpp
QFrame *flatFrame = new QFrame();
flatFrame->setProperty("flat", true);
```

#### 分割线
```cpp
// 水平分割线
QFrame *hDivider = new QFrame();
hDivider->setProperty("divider", "horizontal");

// 垂直分割线
QFrame *vDivider = new QFrame();
vDivider->setProperty("divider", "vertical");
```

#### 导航栏
```cpp
// 导航容器
QWidget *navWidget = new QWidget();
navWidget->setProperty("navigation", true);

// 导航按钮
QPushButton *navBtn = new QPushButton("首页");
navBtn->setProperty("active", true);  // 激活状态
```

### 📝 输入控件

输入控件（QLineEdit、QTextEdit等）会自动应用Material Design样式，无需特殊属性：
```cpp
QLineEdit *lineEdit = new QLineEdit();
lineEdit->setPlaceholderText("请输入文本...");

QTextEdit *textEdit = new QTextEdit();
textEdit->setPlaceholderText("请输入多行文本...");
```

## 🛠️ 样式刷新辅助函数

```cpp
// 在需要时强制刷新控件样式
void refreshWidgetStyle(QWidget* widget) {
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}
```

## 🎨 最佳实践

### 1. 动态属性更新
当需要动态改变控件样式时，记得刷新样式：
```cpp
// 改变按钮状态
button->setProperty("primary", false);
button->setProperty("danger", true);
refreshWidgetStyle(button);
```

### 2. 组织布局
使用适当的间距和边距：
```cpp
QVBoxLayout *layout = new QVBoxLayout();
layout->setSpacing(16);           // 控件间距
layout->setContentsMargins(24, 24, 24, 24);  // 边距
```

### 3. 卡片内容
为卡片设置合适的内边距：
```cpp
QVBoxLayout *cardLayout = new QVBoxLayout(card);
cardLayout->setContentsMargins(16, 16, 16, 16);  // Material Design标准间距
```

### 4. 响应式设计
考虑不同屏幕尺寸：
```cpp
// 为高密度屏幕优化
if (QApplication::primaryScreen()->devicePixelRatio() > 1.5) {
    // 高DPI设置
}
```

## 🔧 常见问题解决

### 样式不生效
1. 确保属性值正确（布尔值用`true`，字符串用引号）
2. 检查是否需要调用`refreshWidgetStyle()`
3. 确认CSS模板已正确加载

### FAB定位
FAB需要手动定位，可以使用绝对布局或计算位置：
```cpp
// 在窗口右下角定位FAB
fab->move(width() - fab->width() - 24, height() - fab->height() - 24);
```

### 主题切换
支持暗色模式自动适配，无需额外设置：
```cpp
// 系统会自动应用暗色模式优化
// 可以通过 prefers-color-scheme 媒体查询检测
```

## 📱 完整示例

```cpp
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置Material Design样式
    app.setStyle("Fusion");
    QFile styleFile(":/styles/qt_material/material.css");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(styleFile.readAll());
    }
    
    // 创建主窗口
    QWidget window;
    window.setWindowTitle("Material Design 示例");
    
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 24, 24, 24);
    
    // 添加卡片
    QFrame *card = new QFrame();
    card->setProperty("card", true);
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->addWidget(new QLabel("欢迎使用Material Design"));
    
    // 添加主要按钮
    QPushButton *primaryBtn = new QPushButton("开始使用");
    primaryBtn->setProperty("primary", true);
    cardLayout->addWidget(primaryBtn);
    
    layout->addWidget(card);
    
    window.show();
    return app.exec();
}
```

## 🎯 总结

通过设置合适的属性，你可以轻松创建具有现代Material Design风格的Qt应用。记住：

1. **使用正确的属性名** - 参考CSS模板中的选择器
2. **适时刷新样式** - 动态改变时使用`refreshWidgetStyle()`
3. **合理组织布局** - 使用标准的间距和边距
4. **测试不同状态** - 悬停、按下、禁用等状态
5. **考虑可访问性** - 确保良好的对比度和可读性

更多高级用法和自定义选项，请参考完整的CSS模板文件。