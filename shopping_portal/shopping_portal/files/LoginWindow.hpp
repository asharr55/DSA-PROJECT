// main/LoginWindow.hpp
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFrame>
#include <QStackedWidget>
#include <QScrollArea>
#include "../templates/userProfile/main.h"
#include "UserStorage.hpp"

class LoginWindow : public QDialog {
public:
    explicit LoginWindow(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Welcome Back");
        setMinimumSize(420, 550);
        setMaximumSize(420, 800);
        resize(420, 650);
        setStyleSheet(R"(
            QDialog {
                background-color: #0a0a0a;
            }
        )");

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(30, 30, 30, 30);
        mainLayout->setSpacing(15);

        // Logo/Brand area
        QLabel *logoLabel = new QLabel("()");
        logoLabel->setStyleSheet(R"(
            QLabel {
                color: #ffffff;
                font-size: 28px;
                font-weight: bold;
                background-color: transparent;
                border: 2px dashed #333333;
                border-radius: 50px;
                padding: 8px;
                min-width: 50px;
                min-height: 50px;
                max-width: 50px;
                max-height: 50px;
            }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
        mainLayout->addSpacing(15);

        // Stacked widget for Login/Signup pages
        stackedWidget = new QStackedWidget();
        stackedWidget->setMinimumHeight(500);
        
        // ========== LOGIN PAGE ==========
        QWidget *loginPage = createLoginPage();
        stackedWidget->addWidget(loginPage);
        
        // ========== SIGNUP PAGE ==========
        QWidget *signupPage = createSignupPage();
        stackedWidget->addWidget(signupPage);

        mainLayout->addWidget(stackedWidget, 1);
    }

    User getCurrentUser() const {
        return currentUser;
    }

private:
    QWidget* createLoginPage() {
        QWidget *page = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(15);

        // Title
        QLabel *titleLabel = new QLabel("Welcome Back");
        titleLabel->setStyleSheet(R"(
            QLabel {
                color: #ffffff;
                font-size: 26px;
                font-weight: bold;
                background-color: transparent;
            }
        )");
        layout->addWidget(titleLabel);

        // Sign up link
        QLabel *signupLink = new QLabel("Don't have an account yet? <a href='signup' style='color: #4a9eff; text-decoration: none;'>Sign up</a>");
        signupLink->setStyleSheet(R"(
            QLabel {
                color: #888888;
                font-size: 13px;
                background-color: transparent;
            }
        )");
        signupLink->setOpenExternalLinks(false);
        layout->addWidget(signupLink);
        layout->addSpacing(25);

        // Email field
        QFrame *emailFrame = createInputField("📧", "email address");
        loginEmail = qobject_cast<QLineEdit*>(emailFrame->findChild<QLineEdit*>());
        layout->addWidget(emailFrame);

        // Password field
        QFrame *passwordFrame = createInputField("🔒", "Password", true);
        loginPassword = qobject_cast<QLineEdit*>(passwordFrame->findChild<QLineEdit*>());
        layout->addWidget(passwordFrame);

        layout->addSpacing(8);

        // Login button
        QPushButton *loginBtn = new QPushButton("Login");
        loginBtn->setMinimumHeight(48);
        loginBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #4a9eff;
                color: white;
                font-size: 15px;
                font-weight: bold;
                padding: 12px;
                border-radius: 8px;
                border: none;
            }
            QPushButton:hover {
                background-color: #3a8eef;
            }
            QPushButton:pressed {
                background-color: #2a7edf;
            }
        )");
        layout->addWidget(loginBtn);

        layout->addSpacing(15);

        // OR separator
        QHBoxLayout *orLayout = new QHBoxLayout();
        orLayout->setSpacing(8);
        QFrame *line1 = new QFrame();
        line1->setFrameShape(QFrame::HLine);
        line1->setStyleSheet("QFrame { background-color: #333333; max-height: 1px; }");
        QLabel *orLabel = new QLabel("OR");
        orLabel->setStyleSheet("color: #666666; font-size: 11px; padding: 0 8px;");
        QFrame *line2 = new QFrame();
        line2->setFrameShape(QFrame::HLine);
        line2->setStyleSheet("QFrame { background-color: #333333; max-height: 1px; }");
        orLayout->addWidget(line1, 1);
        orLayout->addWidget(orLabel);
        orLayout->addWidget(line2, 1);
        layout->addLayout(orLayout);

        layout->addSpacing(15);

        // Social login buttons
        QHBoxLayout *socialLayout = new QHBoxLayout();
        socialLayout->setSpacing(10);

        QPushButton *appleBtn = createSocialButton("🍎");
        QPushButton *googleBtn = createSocialButton("G");
        QPushButton *twitterBtn = createSocialButton("𝕏");

        socialLayout->addWidget(appleBtn);
        socialLayout->addWidget(googleBtn);
        socialLayout->addWidget(twitterBtn);
        layout->addLayout(socialLayout);

        layout->addStretch();

        // Connections
        QObject::connect(loginBtn, &QPushButton::clicked, this, [this]() {
            QString email = loginEmail->text();
            QString passStr = loginPassword->text();

            if (email.isEmpty() || passStr.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter email and password.");
                return;
            }

            // Load user from file (using email as username for now)
            User loadedUser = UserStorage::loadUser(email.toStdString());
            
            if (loadedUser.getUsername().empty()) {
                QMessageBox::warning(this, "Error", "User not found. Please sign up first.");
                return;
            }

            bool okPass = false;
            int enteredPass = passStr.toInt(&okPass);
            if (!okPass || !loadedUser.checkPassword(enteredPass)) {
                QMessageBox::warning(this, "Error", "Incorrect password.");
                return;
            }

            currentUser = loadedUser;
            accept();
        });

        QObject::connect(signupLink, &QLabel::linkActivated, this, [this](const QString &link) {
            if (link == "signup") {
                stackedWidget->setCurrentIndex(1);
                resize(420, 750); // Make taller for signup
            }
        });

        return page;
    }

    QWidget* createSignupPage() {
        QWidget *page = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(15);

        // Title
        QLabel *titleLabel = new QLabel("Create Account");
        titleLabel->setStyleSheet(R"(
            QLabel {
                color: #ffffff;
                font-size: 26px;
                font-weight: bold;
                background-color: transparent;
            }
        )");
        layout->addWidget(titleLabel);

        // Login link
        QLabel *loginLink = new QLabel("Already have an account? <a href='login' style='color: #4a9eff; text-decoration: none;'>Login</a>");
        loginLink->setStyleSheet(R"(
            QLabel {
                color: #888888;
                font-size: 13px;
                background-color: transparent;
            }
        )");
        loginLink->setOpenExternalLinks(false);
        layout->addWidget(loginLink);
        layout->addSpacing(20);

        // Username field
        QFrame *usernameFrame = createInputField("👤", "Username");
        signupUsername = qobject_cast<QLineEdit*>(usernameFrame->findChild<QLineEdit*>());
        layout->addWidget(usernameFrame);

        // Email field
        QFrame *emailFrame = createInputField("📧", "Email address");
        signupEmail = qobject_cast<QLineEdit*>(emailFrame->findChild<QLineEdit*>());
        layout->addWidget(emailFrame);

        // Password field
        QFrame *passwordFrame = createInputField("🔒", "Password", true);
        signupPassword = qobject_cast<QLineEdit*>(passwordFrame->findChild<QLineEdit*>());
        layout->addWidget(passwordFrame);

        // Balance field
        QFrame *balanceFrame = createInputField("💰", "Starting Balance");
        signupBalance = qobject_cast<QLineEdit*>(balanceFrame->findChild<QLineEdit*>());
        layout->addWidget(balanceFrame);

        // Payment method field
        QFrame *paymentFrame = createInputField("💳", "Payment Method");
        signupPayment = qobject_cast<QLineEdit*>(paymentFrame->findChild<QLineEdit*>());
        layout->addWidget(paymentFrame);

        layout->addSpacing(8);

        // Sign up button
        QPushButton *signupBtn = new QPushButton("Sign Up");
        signupBtn->setMinimumHeight(48);
        signupBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #4a9eff;
                color: white;
                font-size: 15px;
                font-weight: bold;
                padding: 12px;
                border-radius: 8px;
                border: none;
            }
            QPushButton:hover {
                background-color: #3a8eef;
            }
            QPushButton:pressed {
                background-color: #2a7edf;
            }
        )");
        layout->addWidget(signupBtn);

        layout->addStretch();

        // Connections
        QObject::connect(signupBtn, &QPushButton::clicked, this, [this]() {
            QString username = signupUsername->text();
            QString email = signupEmail->text();
            QString passStr = signupPassword->text();
            QString balStr = signupBalance->text();
            QString payStr = signupPayment->text();

            if (username.isEmpty() || email.isEmpty() || passStr.isEmpty() 
                || balStr.isEmpty() || payStr.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please fill in all fields.");
                return;
            }

            if (UserStorage::userExists(username.toStdString())) {
                QMessageBox::warning(this, "Error", "Username already exists.");
                return;
            }

            bool okPass = false, okBal = false;
            int pass = passStr.toInt(&okPass);
            double balance = balStr.toDouble(&okBal);

            if (!okPass || !okBal) {
                QMessageBox::warning(this, "Error", "Password must be a number, balance must be a number.");
                return;
            }

            currentUser = User(username.toStdString(), pass, "customer", balance, payStr.toStdString());
            
            if (!UserStorage::saveUser(currentUser)) {
                QMessageBox::warning(this, "Error", "Failed to save account.");
                return;
            }

            QMessageBox::information(this, "Success", "Account created successfully!");
            accept();
        });

        QObject::connect(loginLink, &QLabel::linkActivated, this, [this](const QString &link) {
            if (link == "login") {
                stackedWidget->setCurrentIndex(0);
                resize(420, 650); // Reset to login size
            }
        });

        return page;
    }

    QFrame* createInputField(const QString &icon, const QString &placeholder, bool isPassword = false) {
        QFrame *frame = new QFrame();
        frame->setMinimumHeight(48);
        frame->setStyleSheet(R"(
            QFrame {
                background-color: #1a1a1a;
                border: 1px solid #333333;
                border-radius: 8px;
                padding: 10px;
            }
            QFrame:hover {
                border-color: #4a9eff;
            }
        )");

        QHBoxLayout *layout = new QHBoxLayout(frame);
        layout->setContentsMargins(8, 4, 8, 4);
        layout->setSpacing(10);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("color: #888888; font-size: 16px; background-color: transparent;");
        iconLabel->setFixedWidth(22);
        layout->addWidget(iconLabel);

        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setPlaceholderText(placeholder);
        lineEdit->setStyleSheet(R"(
            QLineEdit {
                background-color: transparent;
                border: none;
                color: #ffffff;
                font-size: 13px;
                padding: 4px;
            }
            QLineEdit::placeholder {
                color: #666666;
            }
        )");
        
        if (isPassword) {
            lineEdit->setEchoMode(QLineEdit::Password);
        }

        layout->addWidget(lineEdit, 1);
        return frame;
    }

    QPushButton* createSocialButton(const QString &text) {
        QPushButton *btn = new QPushButton(text);
        btn->setFixedSize(48, 48);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #1a1a1a;
                border: 1px solid #333333;
                border-radius: 8px;
                color: #ffffff;
                font-size: 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2a2a2a;
                border-color: #4a9eff;
            }
            QPushButton:pressed {
                background-color: #0a0a0a;
            }
        )");
        return btn;
    }

    QStackedWidget *stackedWidget;
    QLineEdit *loginEmail{};
    QLineEdit *loginPassword{};
    QLineEdit *signupUsername{};
    QLineEdit *signupEmail{};
    QLineEdit *signupPassword{};
    QLineEdit *signupBalance{};
    QLineEdit *signupPayment{};
    User currentUser;
};