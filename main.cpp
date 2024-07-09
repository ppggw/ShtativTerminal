#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QString filePath = QDir::currentPath();

    qRegisterMetaType<cv::Mat>("cv::Mat");
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QIcon(filePath + "/plane.png"));
    w.setWindowTitle("Ground Terminal");
    w.setWindowState(Qt::WindowMaximized);
    w.show();

    return a.exec();
}
