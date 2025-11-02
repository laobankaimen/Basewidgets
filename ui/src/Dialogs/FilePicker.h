#pragma once

#include <QWidget>
#include <QString>

class QTreeView;
class QFileSystemModel;
class QLineEdit;

namespace QtAdvancedUI {

class FilePicker {
public:
    // 构建文件选择器主体。通过 out* 返回关键控件用于在对话框接受时读取选择。
    static QWidget* makeFilePickerBody(QWidget* parent,
                                       const QString& initialDir,
                                       const QString& filter,
                                       QString* selectedPathOut,
                                       bool directoryOnly,
                                       bool multiSelect,
                                       bool saveMode,
                                       QTreeView** outView = nullptr,
                                       QFileSystemModel** outModel = nullptr,
                                       QLineEdit** outEdit = nullptr);
};

} // namespace QtAdvancedUI