#pragma once

#include <QtWidgets/QWidget>
#include "ui_mainwindow.h"
#include <QSqlTableModel>
#include <QSqlDataBase>
#include <QStandardItemModel>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	void connectProductDataBase();	// ������Ʒ���ݿ�
	void initTableView();			// ��ʼ����Ʒչʾ���
	void initTypeSelect();			// �������ݿ��ʼ����Ʒ���Ϳ�ѡ��
	void restrictLineEdit();		// ���ƽ��ۡ��ۼ۵�����


signals:

public slots:
	void onAddInStorageClicked();	//���
	void onClearStorageClicked();	//���
	void onAddInOrderClicked();		//���붩��
	void onPayOrderClicked();		//��������

	void onTypeChanged();			//�µ�����ѡ����Ʒ���ͺ󣬶�̬���ظ������µ���Ʒ����
	void onNameConfirmed();			//�µ�����ȷ����Ʒ���ƺ󣬶�̬���ظ���Ʒ�ĵ��ۺͿ��
	void onBuyingAmountChanged();	//ȷ����Ʒ�������ۡ����������󣬵ó��ܼ�



private:
	Ui::MainWindow ui;

	QSqlDatabase product_db;					// ��Ʒ���ݿ����
	QSqlTableModel* m_pSqlTabModel;				// ��Ʒ���ݿ��ģ��
	QStandardItemModel* m_pSoldProductModel;	// �۳���Ʒչʾ��ģ��

	int currentOrderNo = 0;						//��ǰ�����ţ���ʼֵĬ��Ϊ0����������
	double currentTotalConsumption = 0;			//��ǰ����������
};