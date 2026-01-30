#include <QApplication>
#include <QIcon>
#include <QScreen>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/logo.ico"));

    MainWindow w;
    w.show();

    return app.exec();
}
