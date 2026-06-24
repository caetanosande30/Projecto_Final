#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginWindow : public QDialog {
    Q_OBJECT
public:
    explicit LoginWindow(bool isFirstTime = false, QWidget* parent = nullptr);

private slots:
    void onConfirm();

private:
    void setupUi(bool isFirstTime);
    void applyStyles();

    QLineEdit*   m_passwordEdit   = nullptr;
    QLineEdit*   m_confirmEdit    = nullptr;
    QPushButton* m_confirmBtn     = nullptr;
    QLabel*      m_errorLabel     = nullptr;
    bool         m_isFirstTime;
};
