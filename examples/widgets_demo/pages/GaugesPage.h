#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QTimer;
class QCheckBox;
/** 仪表示例只生成演示数据；自动演示默认关闭，隐藏页面时停止计时。 */
class GaugesPage : public basewidgets::StyledWidget {
public:
    explicit GaugesPage(QWidget* parent=nullptr);
protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
private:
    QTimer* m_timer=nullptr;
    QCheckBox* m_animate=nullptr;
};
