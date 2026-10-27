#include <QApplication>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char *argv[]) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    QApplication app(argc, argv);
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
}
