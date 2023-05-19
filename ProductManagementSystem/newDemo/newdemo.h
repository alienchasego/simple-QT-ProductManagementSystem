#pragma once

#include <QtWidgets/QDialog>
#include "ui_newdemo.h"
#include <QSqlTableModel>
#include <QSqlDataBase>

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    void connectAdminDataBase();	// 链接管理员账号数据库

public slots:
	void onBtnAddAdminClicked();    //点击注册账号
	void onBtnLoginClicked();       //点击登录按钮

private:
    Ui::newDemoClass ui;

	QSqlDatabase admin_db;          // 管理员账号数据库对象
	QSqlTableModel* m_pSqlTabModel; // 数据库表模型
};
