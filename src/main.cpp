#include <QApplication>

#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Priority Scheduling");
    QApplication::setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}
