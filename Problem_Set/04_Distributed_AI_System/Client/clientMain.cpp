#include "MainWindow.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("Distributed AI System - Client");
    window.show();

    return app.exec();
}
