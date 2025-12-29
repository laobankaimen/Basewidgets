/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */

#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include <QStringList>

class QTreeView;
class QListView;
class QFileSystemModel;
class QLineEdit;
class QToolButton;
class QComboBox;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QStackedWidget;
class QSplitter;

namespace QtAdvancedUI {

class FilePickerWidget : public QWidget {
    Q_OBJECT
public:
    enum Mode {
        OpenFile,
        OpenFiles,
        SaveFile,
        OpenDirectory
    };

    explicit FilePickerWidget(QWidget* parent,
                              const QString& initialDir,
                              const QString& filter,
                              Mode mode);

    QString selectedPath() const;
    QStringList selectedPaths() const;

signals:
    void selectionChanged(const QString& path);
    void fileActivated(const QString& path); // double click

private:
    void setupUi(const QString& initialDir, const QString& filter);
    void updatePreview(const QString& path);
    void updateNavButtons();
    void pushHistory(const QString& path);
    void setRoot(const QString& path);
    void createNewDir();
    void deleteSelected();

    Mode m_mode;
    QVector<QString> m_history;
    int m_histIndex = -1;
    QString m_selectedPath;
    QStringList m_selectedPaths; // For multi-select

    // UI
    QWidget* m_topBar;
    QToolButton *btnBack, *btnForward, *btnUp, *btnHome, *btnRefresh;
    QToolButton *btnViewMode; // Toggle List/Grid
    QLineEdit* addrEdit;
    QToolButton* btnGo; // Navigate to address
    
    QSplitter* m_splitter;
    QListWidget* m_sidebar; // Places
    
    QStackedWidget* m_viewStack;
    QTreeView* m_treeView; // List/Details View
    QListView* m_iconView; // Grid/Icon View
    
    QFileSystemModel* m_model;
    
    // Bottom area
    QWidget* m_bottomBar;
    QLineEdit* m_nameEdit;
    QComboBox* m_filterCombo;
    QLabel* m_statusLabel;
    
    bool m_isGrid = false;
    
    void onSidebarItemClicked(QListWidgetItem* item);
    void onViewDoubleClicked(const QModelIndex& index);
    void onViewClicked(const QModelIndex& index);
    void onPathChanged(const QString& path);
    void toggleViewMode();
    void updateView(); // Updates UI from current history state
};

class FilePicker {
public:
    // Deprecated wrapper, prefer FilePickerWidget
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
