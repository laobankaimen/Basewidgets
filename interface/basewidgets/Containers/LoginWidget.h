#pragma once
#include <QWidget>
#include <basewidgets/QtAdvancedUIExport.h>
class QLineEdit;
class QLabel;
class QCheckBox;
class QPushButton;
namespace basewidgets {
/** UI only: authenticationRequested delegates to the application.
 * The application calls setBusy(false)/setError on completion. No credentials
 * are persisted, logged or verified here. All API calls are on the GUI thread. */
class ACSS_UI_EXPORT LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent=nullptr);
    void setAccount(const QString& account);
    QString account() const;
    void setBusy(bool busy);
    bool isBusy() const { return m_busy; }
    /** Show a caller-provided error and focus the password; empty text clears
     * the visible error and field descriptions without moving focus. */
    void setError(const QString& message);
    void clearPassword();
signals:
    void authenticationRequested(const QString& account,const QString& password);
private:
    void submit();
    void showError(const QString& message, QLineEdit* field);
    QLineEdit* m_account=nullptr;
    QLineEdit* m_password=nullptr;
    QLabel* m_error=nullptr;
    QCheckBox* m_reveal=nullptr;
    QPushButton* m_submit=nullptr;
    bool m_busy=false;
};
}
