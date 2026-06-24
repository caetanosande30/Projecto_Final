#include "loginwindow.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

LoginWindow::LoginWindow(bool isFirstTime, QWidget* parent)
    : QDialog(parent), m_isFirstTime(isFirstTime)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(420, isFirstTime ? 380 : 320);
    setupUi(isFirstTime);
    applyStyles();
}

void LoginWindow::setupUi(bool isFirstTime) {
    auto* root = new QWidget(this);
    root->setObjectName("loginCard");
    root->setFixedSize(this->size());

    auto* shadow = new QGraphicsDropShadowEffect(root);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 160));
    root->setGraphicsEffect(shadow);

    auto* layout = new QVBoxLayout(root);
    layout->setSpacing(16);
    layout->setContentsMargins(40, 36, 40, 36);

    // Logo / título
    auto* logo = new QLabel("⬡", root);
    logo->setObjectName("logoIcon");
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo);

    auto* title = new QLabel("CAETA STARK", root);
    title->setObjectName("loginTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* subtitle = new QLabel(isFirstTime
        ? "Crie a sua senha de acesso"
        : "Introduza a sua senha para continuar", root);
    subtitle->setObjectName("loginSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitle);

    layout->addSpacing(8);

    // Campo senha
    m_passwordEdit = new QLineEdit(root);
    m_passwordEdit->setPlaceholderText(isFirstTime ? "Nova senha" : "Senha");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setObjectName("loginInput");
    layout->addWidget(m_passwordEdit);

    // Confirmar senha (só na primeira vez)
    if (isFirstTime) {
        m_confirmEdit = new QLineEdit(root);
        m_confirmEdit->setPlaceholderText("Confirmar senha");
        m_confirmEdit->setEchoMode(QLineEdit::Password);
        m_confirmEdit->setObjectName("loginInput");
        layout->addWidget(m_confirmEdit);
    }

    // Erro
    m_errorLabel = new QLabel("", root);
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setVisible(false);
    layout->addWidget(m_errorLabel);

    // Botão
    m_confirmBtn = new QPushButton(isFirstTime ? "CRIAR ACESSO" : "ENTRAR", root);
    m_confirmBtn->setObjectName("loginBtn");
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_confirmBtn);

    connect(m_confirmBtn, &QPushButton::clicked, this, &LoginWindow::onConfirm);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onConfirm);
    if (m_confirmEdit)
        connect(m_confirmEdit, &QLineEdit::returnPressed, this, &LoginWindow::onConfirm);
}

void LoginWindow::onConfirm() {
    QString pwd = m_passwordEdit->text();
    if (pwd.isEmpty()) {
        m_errorLabel->setText("⚠  A senha não pode estar vazia.");
        m_errorLabel->setVisible(true);
        return;
    }

    if (m_isFirstTime) {
        if (m_confirmEdit && m_confirmEdit->text() != pwd) {
            m_errorLabel->setText("⚠  As senhas não coincidem.");
            m_errorLabel->setVisible(true);
            return;
        }
        Database::instance().setPassword(pwd);
        accept();
    } else {
        if (!Database::instance().checkPassword(pwd)) {
            m_errorLabel->setText("⚠  Senha incorrecta. Tente novamente.");
            m_errorLabel->setVisible(true);
            m_passwordEdit->clear();
            m_passwordEdit->setFocus();
            return;
        }
        accept();
    }
}

void LoginWindow::applyStyles() {
    setStyleSheet(R"(
        QWidget#loginCard {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #1a1f2e, stop:1 #0f1219);
            border-radius: 20px;
            border: 1px solid #2a3045;
        }
        QLabel#logoIcon {
            font-size: 42px;
            color: #e8a020;
        }
        QLabel#loginTitle {
            font-family: 'Segoe UI';
            font-size: 22px;
            font-weight: 800;
            letter-spacing: 4px;
            color: #f0f4ff;
        }
        QLabel#loginSubtitle {
            font-family: 'Segoe UI';
            font-size: 11px;
            color: #6b7a9e;
            letter-spacing: 1px;
        }
        QLineEdit#loginInput {
            background: #242b3d;
            border: 1px solid #2f3a52;
            border-radius: 10px;
            padding: 12px 16px;
            font-size: 14px;
            color: #e0e6f0;
            font-family: 'Segoe UI';
        }
        QLineEdit#loginInput:focus {
            border: 1.5px solid #e8a020;
            background: #2a3347;
        }
        QPushButton#loginBtn {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #e8a020, stop:1 #c97d10);
            border: none;
            border-radius: 10px;
            padding: 13px;
            font-family: 'Segoe UI';
            font-size: 13px;
            font-weight: 700;
            letter-spacing: 2px;
            color: #0f1219;
        }
        QPushButton#loginBtn:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #f0b030, stop:1 #e09020);
        }
        QPushButton#loginBtn:pressed {
            background: #c97d10;
        }
        QLabel#errorLabel {
            font-family: 'Segoe UI';
            font-size: 11px;
            color: #f06060;
        }
    )");
}
