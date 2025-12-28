#include "AppController.h"

#include <QApplication>

#include "UserManager.h"
#include "LibraryManager.h"
#include "WelcomeDialog.h"
#include "MainWindow.h"

AppController::AppController(QApplication& app)
    : QObject(nullptr), app_(app)
{
    users_ = new UserManager();
    library_ = new LibraryManager();
}

AppController::~AppController() {
    delete mainWin_;
    delete welcome_;
    delete users_;
    delete library_;
}

void AppController::run() {
    showWelcome();
}

void AppController::showWelcome() {
    if (mainWin_) {
        mainWin_->close();
        delete mainWin_;
        mainWin_ = nullptr;
    }

    welcome_ = new WelcomeDialog(users_);
    int ret = welcome_->exec();

    if (ret != QDialog::Accepted) {
        app_.quit();
        return;
    }

    auto userOpt = welcome_->loggedInUser();
    delete welcome_;
    welcome_ = nullptr;

    if (!userOpt.has_value()) {
        app_.quit();
        return;
    }

    showMainWindow(*userOpt);
}

void AppController::showMainWindow(const QString& username) {
    mainWin_ = new MainWindow(users_, library_, username);
    QObject::connect(mainWin_, &MainWindow::logoutRequested,
        this, &AppController::onLogout);
    mainWin_->show();
}


void AppController::onLogout() {
    showWelcome();
}