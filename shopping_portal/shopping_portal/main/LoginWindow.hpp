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
#include <QCheckBox>
#include "../templates/userProfile/main.h"
#include "../templates/admin/main.h"
#include "UserStorage.hpp"
#include "AdminStorage.hpp"

// =============================================================================
// LoginWindow
// -----------------------------------------------------------------------------
// Now supports both User and Admin login.
//
// On signup: a checkbox "Sign up as Admin" routes the new account to
//            AdminStorage instead of UserStorage.
// On login : we try AdminStorage first; if no admin matches, fall back to
//            UserStorage. Tells the caller via isAdminUser().
// =============================================================================
class LoginWindow : public QDialog {
public:
    explicit LoginWindow(QWidget *parent = nullptr)
        : QDialog(parent), loggedInAsAdmin(false)
    {
        setWindowTitle("Welcome Back");
        setMinimumSize(420, 550);
        setMaximumSize(420, 800);
        resize(420, 650);
        setStyleSheet("QDialog { background-color: #0a0a0a; }");

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(30, 30, 30, 30);
        mainLayout->setSpacing(15);

        // Logo
        QLabel *logoLabel = new QLabel("()");
        logoLabel->setStyleSheet(
            "QLabel { color: #ffffff; font-size: 28px; font-weight: bold; "
            "background-color: transparent; border: 2px dashed #333333; "
            "border-radius: 50px; padding: 8px; "
            "min-width: 50px; min-height: 50px; max-width: 50px; max-height: 50px; }"
        );
        logoLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
        mainLayout->addSpacing(15);

        stackedWidget = new QStackedWidget();
        stackedWidget->setMinimumHeight(500);
        stackedWidget->addWidget(createLoginPage());
        stackedWidget->addWidget(createSignupPage());
        mainLayout->addWidget(stackedWidget, 1);
    }

    User  getCurrentUser()  const { return currentUser; }
    Admin getCurrentAdmin() const { return currentAdmin; }
    bool  isAdminUser()     const { return loggedInAsAdmin; }

private:
    QWidget* createLoginPage() {
        QWidget *page = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(15);

        QLabel *titleLabel = new QLabel("Welcome Back");
        titleLabel->setStyleSheet(
            "QLabel { color: #ffffff; font-size: 26px; font-weight: bold; "
            "background-color: transparent; }"
        );
        layout->addWidget(titleLabel);

        QLabel *signupLink = new QLabel(
            "Don't have an account yet? "
            "<a href='signup' style='color: #4a9eff; text-decoration: none;'>Sign up</a>");
        signupLink->setStyleSheet(
            "QLabel { color: #888888; font-size: 13px; background-color: transparent; }"
        );
        signupLink->setOpenExternalLinks(false);
        layout->addWidget(signupLink);
        layout->addSpacing(25);

        QFrame *emailFrame = createInputField("Email", "email address");
        loginEmail = qobject_cast<QLineEdit*>(emailFrame->findChild<QLineEdit*>());
        layout->addWidget(emailFrame);

        QFrame *passwordFrame = createInputField("Pass", "Password", true);
        loginPassword = qobject_cast<QLineEdit*>(passwordFrame->findChild<QLineEdit*>());
        layout->addWidget(passwordFrame);

        layout->addSpacing(8);

        QPushButton *loginBtn = new QPushButton("Login");
        loginBtn->setMinimumHeight(48);
        loginBtn->setStyleSheet(
            "QPushButton { background-color: #4a9eff; color: white; "
            "font-size: 15px; font-weight: bold; padding: 12px; "
            "border-radius: 8px; border: none; }"
            "QPushButton:hover { background-color: #3a8eef; }"
        );
        layout->addWidget(loginBtn);
        layout->addStretch();

        QObject::connect(loginBtn, &QPushButton::clicked, this, [this]() {
            QString email = loginEmail->text();
            QString passStr = loginPassword->text();
            if (email.isEmpty() || passStr.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter email and password.");
                return;
            }
            bool okPass = false;
            int enteredPass = passStr.toInt(&okPass);
            if (!okPass) {
                QMessageBox::warning(this, "Error", "Password must be a number.");
                return;
            }

            // Try ADMIN first (admins are rare; users are common)
            if (AdminStorage::adminExists(email.toStdString())) {
                Admin a = AdminStorage::loadAdmin(email.toStdString());
                if (a.checkPassword(enteredPass)) {
                    currentAdmin = a;
                    loggedInAsAdmin = true;
                    accept();
                    return;
                }
            }

            // Fall back to USER
            if (UserStorage::userExists(email.toStdString())) {
                User u = UserStorage::loadUser(email.toStdString());
                if (u.checkPassword(enteredPass)) {
                    currentUser = u;
                    loggedInAsAdmin = false;
                    accept();
                    return;
                }
                QMessageBox::warning(this, "Error", "Incorrect password.");
                return;
            }

            QMessageBox::warning(this, "Error", "Account not found. Please sign up.");
        });

        QObject::connect(signupLink, &QLabel::linkActivated, this,
            [this](const QString& link) {
                if (link == "signup") {
                    stackedWidget->setCurrentIndex(1);
                    resize(420, 760);
                }
            });

        return page;
    }

    QWidget* createSignupPage() {
        QWidget *page = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(15);

        QLabel *titleLabel = new QLabel("Create Account");
        titleLabel->setStyleSheet(
            "QLabel { color: #ffffff; font-size: 26px; font-weight: bold; "
            "background-color: transparent; }"
        );
        layout->addWidget(titleLabel);

        QLabel *loginLink = new QLabel(
            "Already have an account? "
            "<a href='login' style='color: #4a9eff; text-decoration: none;'>Login</a>");
        loginLink->setStyleSheet(
            "QLabel { color: #888888; font-size: 13px; background-color: transparent; }"
        );
        loginLink->setOpenExternalLinks(false);
        layout->addWidget(loginLink);
        layout->addSpacing(20);

        QFrame *usernameFrame = createInputField("User", "Username");
        signupUsername = qobject_cast<QLineEdit*>(usernameFrame->findChild<QLineEdit*>());
        layout->addWidget(usernameFrame);

        QFrame *emailFrame = createInputField("Mail", "Email address");
        signupEmail = qobject_cast<QLineEdit*>(emailFrame->findChild<QLineEdit*>());
        layout->addWidget(emailFrame);

        QFrame *passwordFrame = createInputField("Pass", "Password", true);
        signupPassword = qobject_cast<QLineEdit*>(passwordFrame->findChild<QLineEdit*>());
        layout->addWidget(passwordFrame);

        QFrame *balanceFrame = createInputField("Bal ", "Starting Balance");
        signupBalance = qobject_cast<QLineEdit*>(balanceFrame->findChild<QLineEdit*>());
        layout->addWidget(balanceFrame);

        QFrame *paymentFrame = createInputField("Pay ", "Payment Method");
        signupPayment = qobject_cast<QLineEdit*>(paymentFrame->findChild<QLineEdit*>());
        layout->addWidget(paymentFrame);

        // NEW: Admin checkbox
        adminCheck = new QCheckBox("Sign up as Admin");
        adminCheck->setStyleSheet(
            "QCheckBox { color: #cccccc; font-size: 13px; padding: 5px; }"
            "QCheckBox::indicator { width: 16px; height: 16px; "
            "border: 1px solid #444; border-radius: 3px; background-color: #1a1a1a; }"
            "QCheckBox::indicator:checked { background-color: #ff9900; border-color: #ff9900; }"
        );
        layout->addWidget(adminCheck);

        layout->addSpacing(8);

        QPushButton *signupBtn = new QPushButton("Sign Up");
        signupBtn->setMinimumHeight(48);
        signupBtn->setStyleSheet(
            "QPushButton { background-color: #4a9eff; color: white; "
            "font-size: 15px; font-weight: bold; padding: 12px; "
            "border-radius: 8px; border: none; }"
            "QPushButton:hover { background-color: #3a8eef; }"
        );
        layout->addWidget(signupBtn);
        layout->addStretch();

        QObject::connect(signupBtn, &QPushButton::clicked, this, [this]() {
            QString username = signupUsername->text();
            QString passStr  = signupPassword->text();
            QString balStr   = signupBalance->text();
            QString payStr   = signupPayment->text();

            if (username.isEmpty() || passStr.isEmpty() ||
                balStr.isEmpty() || payStr.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please fill in all fields.");
                return;
            }

            bool okPass = false, okBal = false;
            int    pass    = passStr.toInt(&okPass);
            double balance = balStr.toDouble(&okBal);
            if (!okPass || !okBal) {
                QMessageBox::warning(this, "Error",
                    "Password must be a number; balance must be numeric.");
                return;
            }

            std::string un = username.toStdString();
            // Reject if name is taken in EITHER namespace
            if (UserStorage::userExists(un) || AdminStorage::adminExists(un)) {
                QMessageBox::warning(this, "Error", "Username already exists.");
                return;
            }

            if (adminCheck->isChecked()) {
                currentAdmin = Admin(un, pass, "admin", balance, payStr.toStdString());
                if (!AdminStorage::saveAdmin(currentAdmin)) {
                    QMessageBox::warning(this, "Error", "Failed to save admin account.");
                    return;
                }
                loggedInAsAdmin = true;
                QMessageBox::information(this, "Success",
                    "Admin account created. You have full panel access.");
            } else {
                currentUser = User(un, pass, "customer", balance, payStr.toStdString());
                if (!UserStorage::saveUser(currentUser)) {
                    QMessageBox::warning(this, "Error", "Failed to save account.");
                    return;
                }
                loggedInAsAdmin = false;
                QMessageBox::information(this, "Success", "Account created successfully!");
            }
            accept();
        });

        QObject::connect(loginLink, &QLabel::linkActivated, this,
            [this](const QString& link) {
                if (link == "login") {
                    stackedWidget->setCurrentIndex(0);
                    resize(420, 650);
                }
            });
        return page;
    }

    QFrame* createInputField(const QString &icon, const QString &placeholder, bool isPassword = false) {
        QFrame *frame = new QFrame();
        frame->setMinimumHeight(48);
        frame->setStyleSheet(
            "QFrame { background-color: #1a1a1a; border: 1px solid #333333; "
            "border-radius: 8px; padding: 10px; }"
            "QFrame:hover { border-color: #4a9eff; }"
        );

        QHBoxLayout *layout = new QHBoxLayout(frame);
        layout->setContentsMargins(8, 4, 8, 4);
        layout->setSpacing(10);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet(
            "color: #888888; font-size: 12px; background-color: transparent;"
        );
        iconLabel->setFixedWidth(38);
        layout->addWidget(iconLabel);

        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setPlaceholderText(placeholder);
        lineEdit->setStyleSheet(
            "QLineEdit { background-color: transparent; border: none; "
            "color: #ffffff; font-size: 13px; padding: 4px; }"
        );
        if (isPassword) lineEdit->setEchoMode(QLineEdit::Password);
        layout->addWidget(lineEdit, 1);
        return frame;
    }

    QStackedWidget *stackedWidget;
    QLineEdit *loginEmail{};
    QLineEdit *loginPassword{};
    QLineEdit *signupUsername{};
    QLineEdit *signupEmail{};
    QLineEdit *signupPassword{};
    QLineEdit *signupBalance{};
    QLineEdit *signupPayment{};
    QCheckBox *adminCheck{};

    User  currentUser;
    Admin currentAdmin;
    bool  loggedInAsAdmin;
};
