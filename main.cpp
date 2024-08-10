#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // 设置窗口标题
    w.setWindowTitle("ibook notes tool");
    // 设置窗口图标
    a.setWindowIcon(QIcon("qrc:/icon/images/title.png"));

    w.show();
    return a.exec();
}
