#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QApplication>
#include "AppController.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // 【关键】不设置任何 setStyleSheet，使用系统原生灰色界面
    AppController controller(a);
    controller.run();

    return a.exec();
}
