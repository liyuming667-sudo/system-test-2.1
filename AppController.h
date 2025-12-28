#pragma once
#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>

class QApplication;
class UserManager;
class LibraryManager;
class WelcomeDialog;
class MainWindow;

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QApplication& app);
    ~AppController();

    void run();

private slots:
    void onLogout();

private:
    QApplication& app_;

    UserManager* users_ = nullptr;
    LibraryManager* library_ = nullptr;

    WelcomeDialog* welcome_ = nullptr;
    MainWindow* mainWin_ = nullptr;

    void showWelcome();
    void showMainWindow(const QString& username);
};

#endif // APPCONTROLLER_H