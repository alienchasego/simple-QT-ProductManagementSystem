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

	void connectProductDataBase();	// 链接商品数据库
	void initTableView();			// 初始化商品展示表格
	void initTypeSelect();			// 根据数据库初始化商品类型可选项
	void restrictLineEdit();		// 限制进价、售价的输入


signals:

public slots:
	void onAddInStorageClicked();	//入库
	void onClearStorageClicked();	//清仓
	void onAddInOrderClicked();		//加入订单
	void onPayOrderClicked();		//订单付款

	void onTypeChanged();			//下单界面选择商品类型后，动态加载该类型下的商品名称
	void onNameConfirmed();			//下单界面确定商品名称后，动态加载该商品的单价和库存
	void onBuyingAmountChanged();	//确定商品名、单价、购买数量后，得出总价



private:
	Ui::MainWindow ui;

	QSqlDatabase product_db;					// 商品数据库对象
	QSqlTableModel* m_pSqlTabModel;				// 商品数据库表模型
	QStandardItemModel* m_pSoldProductModel;	// 售出商品展示表模型

	int currentOrderNo = 0;						//当前订单号，初始值默认为0，递增序列
	double currentTotalConsumption = 0;			//当前订单总消费
};