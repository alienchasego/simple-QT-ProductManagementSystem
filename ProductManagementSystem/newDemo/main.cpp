#include "newdemo.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow w;
    w.show();
    if (w.exec() == QDialog::Accepted)  //如果确认登录，跳转主界面
    {
        MainWindow m;
		m.show();
        return a.exec();
    }
}
