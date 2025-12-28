#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "AppController.h"
#include "WelcomeDialog.h"
#include "MainWindow.h"
#include "UserManager.h"
#include "LibraryManager.h"
#include "User.h" 

AppController::AppController(QApplication& app)
    : app_(app)
{
    lib_ = new LibraryManager();
    users_ = new UserManager();
}

AppController::~AppController() {
    delete lib_;
    delete users_;
}

void AppController::run() {
    // 【关键】恢复传入 users_ 指针
    WelcomeDialog dlg(users_);
    
    if (dlg.exec() == QDialog::Accepted) {
        auto user = dlg.loggedInUser();
        if (user.has_value()) {
            currentUser_ = user.value();
            
            // 启动主窗口
            auto* mw = new MainWindow(users_, lib_, currentUser_);
            mw->setAttribute(Qt::WA_DeleteOnClose);
            mw->show();
        }
    } else {
        std::exit(0);
    }
}
