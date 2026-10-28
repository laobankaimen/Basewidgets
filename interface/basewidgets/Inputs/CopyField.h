#pragma once
#include <basewidgets/Core/StyledWidget.h>
class QTimer;
namespace basewidgets {
class LineEdit;
class ToolButton;
/** 可选择文本与一键复制字段，适合路径、任务 ID、连接地址。
 * 单行只读展示；复制写入系统剪贴板，保持 text() 原文（包括长路径/Unicode）。
 * 空值不可复制；反馈为短暂勾号，不弹窗、不改正文。GUI 线程使用。
 */
class ACSS_UI_EXPORT CopyField : public StyledWidget {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit CopyField(const QString& text=QString(),QWidget* parent=nullptr);
    QString text() const;
    void setText(const QString& text);
    void setPlaceholderText(const QString& text);
    /** 借用只读输入框，可设置无障碍名称、选择或字体；不要替换/删除或改为可编辑。 */
    LineEdit* textEdit() const { return m_edit; }
public slots:
    void copy(); ///< 复制全部原文；禁用或空值时无操作。Ctrl+C 仍可复制输入框的选区。
signals:
    void textChanged(const QString& text);
    void copied(const QString& text); ///< 写入剪贴板后发出，不代表其他进程已读取。
private:
    void resetFeedback();
    LineEdit* m_edit=nullptr;
    ToolButton* m_copy=nullptr;
    QTimer* m_feedback=nullptr;
};
}
