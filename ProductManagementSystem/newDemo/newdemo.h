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

    void connectAdminDataBase();	// ���ӹ���Ա�˺����ݿ�

public slots:
	void onBtnAddAdminClicked();    //���ע���˺�
	void onBtnLoginClicked();       //�����¼��ť

private:
    Ui::newDemoClass ui;

	QSqlDatabase admin_db;          // ����Ա�˺����ݿ����
	QSqlTableModel* m_pSqlTabModel; // ���ݿ��ģ��
};
