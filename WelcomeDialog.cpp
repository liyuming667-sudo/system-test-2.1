#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "WelcomeDialog.h"
#include "UserManager.h"
#include "User.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QInputDialog>
#include <QFormLayout>

WelcomeDialog::WelcomeDialog(UserManager* users, QWidget* parent)
    : QDialog(parent), users_(users)
{
    setWindowTitle("登录");
    resize(300, 200); // 恢复标准大小
    initUI();
}

void WelcomeDialog::initUI() {
    auto* layout = new QVBoxLayout(this);

    auto* formLayout = new QFormLayout();
    userEdit_ = new QLineEdit(this);
    userEdit_->setPlaceholderText("请输入账号");
    passEdit_ = new QLineEdit(this);
    passEdit_->setEchoMode(QLineEdit::Password);
    passEdit_->setPlaceholderText("请输入密码");

    formLayout->addRow("账号:", userEdit_);
    formLayout->addRow("密码:", passEdit_);
    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* loginBtn = new QPushButton("登录", this);
    auto* regBtn = new QPushButton("注册", this);
    
    btnLayout->addWidget(loginBtn);
    btnLayout->addWidget(regBtn);
    layout->addLayout(btnLayout);

    connect(loginBtn, &QPushButton::clicked, this, &WelcomeDialog::onLoginClicked);
    connect(regBtn, &QPushButton::clicked, this, &WelcomeDialog::onRegisterClicked);
}

void WelcomeDialog::onLoginClicked() {
    if (!users_) return;

    QString u = userEdit_->text().trimmed();
    QString p = passEdit_->text().trimmed();

    if (u.isEmpty() || p.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入账号和密码");
        return;
    }

    auto userOpt = users_->findUser(u.toStdString());
    if (userOpt.has_value()) {
        // 【关键】直接在此处验证密码
        if (userOpt->password() == p.toStdString()) {
            loggedUser_ = u;
            accept(); // 验证成功，返回 Accepted
            return;
        }
    }
    
    QMessageBox::warning(this, "错误", "账号或密码错误");
}

void WelcomeDialog::onRegisterClicked() {
    bool ok = false;
    QString u = QInputDialog::getText(this, "注册", "新账号:", QLineEdit::Normal, "", &ok);
    if (!ok || u.isEmpty()) return;

    if (users_->findUser(u.toStdString()).has_value()) {
        QMessageBox::warning(this, "错误", "账号已存在");
        return;
    }

    QString p = QInputDialog::getText(this, "注册", "新密码:", QLineEdit::Normal, "", &ok);
    if (!ok || p.isEmpty()) return;

    // 默认注册为读者
    User newUser(u.toStdString(), p.toStdString(), UserRole::Reader, "新用户", "无", 3);
    if (users_->addUser(newUser)) {
        QMessageBox::information(this, "成功", "注册成功，请直接登录");
    } else {
        QMessageBox::warning(this, "失败", "注册失败");
    }
}

std::optional<QString> WelcomeDialog::loggedInUser() const {
    return loggedUser_;
}
