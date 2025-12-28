#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QApplication>
#include "AppController.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // 不加载任何样式表，使用系统原生外观
    AppController controller(a);
    controller.run();

    return a.exec();
}