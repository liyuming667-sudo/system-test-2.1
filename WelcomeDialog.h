#pragma once
#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include <QString>
#include <optional>

class QLabel;
class QPushButton;
class QLineEdit;
class UserManager;

class WelcomeDialog : public QDialog {
    Q_OBJECT
public:
    // 恢复：接收 UserManager 指针，用于内部验证登录
    explicit WelcomeDialog(UserManager* users, QWidget* parent = nullptr);

    // 获取登录成功的用户名
    std::optional<QString> loggedInUser() const;

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void initUI();

private:
    UserManager* users_ = nullptr;

    QLineEdit* userEdit_ = nullptr;
    QLineEdit* passEdit_ = nullptr;

    std::optional<QString> loggedUser_ = std::nullopt;
};

#endif // WELCOMEDIALOG_H