#include "mainwindow.h"

#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	setWindowTitle(QStringLiteral("商品管理系统"));
	ui.toolBox->setCurrentIndex(0);										// 默认打开“商品入库”页面
	ui.StorageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);// 表格只展示，不可编辑
	ui.btnPayOrder->setEnabled(false);									// 付款按钮默认不可用，在加入订单成功后启用，付款后再次禁用
	ui.btnAddInOrder->setEnabled(false);								// 加入订单按钮默认不可编辑，在确定商品名后启用
	ui.amount_buy->setEnabled(false);									// 下单数量默认不可编辑，在确定商品名后启用

	connectProductDataBase();
	initTableView();
	initTypeSelect();
	restrictLineEdit();

	m_pSoldProductModel = new QStandardItemModel(this);					// 初始化售出商品表

	(void)connect(ui.btnAddInStorage, &QPushButton::clicked, this, &MainWindow::onAddInStorageClicked);
	(void)connect(ui.btnClearStorage, &QPushButton::clicked, this, &MainWindow::onClearStorageClicked);
	(void)connect(ui.btnAddInOrder, &QPushButton::clicked, this, &MainWindow::onAddInOrderClicked);
	(void)connect(ui.btnPayOrder, &QPushButton::clicked, this, &MainWindow::onPayOrderClicked);

	// 商品类型与商品名称的切换联动
	connect(ui.typeSelect_buy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTypeChanged);
	// 商品名称确定后，确定单价和库存
	connect(ui.nameSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onNameConfirmed);
	// 修改下单数量，总价改变
	connect(ui.amount_buy, QOverload<int>::of(&QSpinBox::valueChanged), this, & MainWindow::onBuyingAmountChanged);
}

MainWindow::~MainWindow()
{}


void MainWindow::connectProductDataBase()
{
	bool ok;

	//避免重复链接同一数据库
// 	if (QSqlDatabase::contains("qt_sql_default_connection"))
// 	{
// 		product_db = QSqlDatabase::database("qt_sql_default_connection");
// 	}
// 	else
// 	{
// 		product_db = QSqlDatabase::addDatabase("QSQLITE");
// 	}

	product_db = QSqlDatabase::addDatabase("QSQLITE");
	product_db.setDatabaseName("product.db");						// 设置数据库的名称

	ok = product_db.open();											// 打开数据库连接
	if (!ok)
		qDebug() << __FUNCTION__ << product_db.lastError().text();
}

void MainWindow::initTableView()
{
	m_pSqlTabModel = new QSqlTableModel(this);
	m_pSqlTabModel->setTable("products");
	m_pSqlTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

	m_pSqlTabModel->select();

	m_pSqlTabModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("商品名称"));
	m_pSqlTabModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("商品类型"));
	m_pSqlTabModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("进价"));
	m_pSqlTabModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("售价"));
	m_pSqlTabModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("库存"));
	ui.StorageTable->setModel(m_pSqlTabModel);
}

void MainWindow::initTypeSelect()
{
	QSqlQuery query;
	QString typeQuery = "SELECT DISTINCT type FROM products";
	query.exec(typeQuery);

	QSet<QString> types;								// 使用QSet来保存唯一的属性值

	while (query.next()) 
	{
		QString type = query.value("type").toString();	// 获取查询结果中的属性值
		types.insert(type);								// 将属性值添加到QSet中
	}
	// 添加所有的商品属性到下拉框
	for (const QString& type : types) 
	{
		ui.typeSelect_buy->addItem(type);
		ui.typeSelect_store->addItem(type);
	}
}

void MainWindow::restrictLineEdit()
{
	/// <summary>
	/// 对进价、售价、入库数量做限定
	/// </summary>

	// 创建一个正则表达式，只允许输入数字和一个小数点
	QRegularExpression regex("[0-9]*\\.?[0-9]*");

	// 创建一个验证器，使用正则表达式进行验证
	QRegularExpressionValidator* validator_purchase = new QRegularExpressionValidator(regex, ui.purchasePrice);
	QRegularExpressionValidator* validator_sale = new QRegularExpressionValidator(regex, ui.salePrice);

	// 设置验证器到QLineEdit中
	ui.purchasePrice->setValidator(validator_purchase);
	ui.salePrice->setValidator(validator_sale);
}

void MainWindow::onAddInStorageClicked()
{
	QString type = ui.typeSelect_store->currentText();	//商品种类
	QString name = ui.name->text();						//商品名称
	QString purchasePrice = ui.purchasePrice->text();	//进价
	QString salePrice = ui.salePrice->text();			//售价
	QString amount_add = ui.amount_add->text();			//入库数量

	// 查询是否输入完整商品信息
	if (ui.typeSelect_store->currentIndex() == 0 || name.isEmpty() || purchasePrice.isEmpty() || salePrice.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("请输入完整商品信息。"));
		return;
	}

	// 查询是否存在当前输入的商品名称
	QSqlQuery query;
	query.prepare("SELECT * FROM products WHERE name = :name");
	query.bindValue(":name", name);
	if (query.exec() && query.next())					
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("该商品已存在，不可重复录入同名商品。"));
		return;
	}
	// 查询商品入库数量大于0
	if (amount_add.toInt() == 0)						
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("商品入库数量大于0"));
		return;
	}

	QSqlRecord record = m_pSqlTabModel->record();
	record.setValue("name", name);
	record.setValue("type", type);
	record.setValue("purchasePrice", purchasePrice);
	record.setValue("salePrice", salePrice);
	record.setValue("inventory", amount_add);

	bool ok;
	ok = m_pSqlTabModel->insertRecord(-1, record);
	if (!ok)
	{
		qDebug() << m_pSqlTabModel->lastError().text();
		return;
	}

	ok = m_pSqlTabModel->submitAll();
	if (!ok)
	{
		qDebug() << __FUNCTION__ << m_pSqlTabModel->lastError().text();
	}

	/// <summary>
	/// 入库后，恢复各种输入框
	/// </summary>
	ui.typeSelect_store->setCurrentIndex(0);
	ui.name->clear();
	ui.purchasePrice->clear();
	ui.salePrice->clear();
	ui.amount_add->setValue(0);
}

void MainWindow::onClearStorageClicked()
{
	//清空数据库
	m_pSqlTabModel->removeRows(0, m_pSqlTabModel->rowCount());
	bool ok;
	ok = m_pSqlTabModel->submitAll();
	if (!ok)
	{
		qDebug() << __FUNCTION__ << m_pSqlTabModel->lastError().text();
	}
}

void MainWindow::onAddInOrderClicked()
{
	//限定下单数量
	if (ui.amount_buy->value() == 0)
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("购买数量大于0。"));
		return;
	}
	if (ui.amount_buy->value() > ui.lcdNumber_inventory->value())
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("购买数量超过库存商品数。"));
		return;
	}

	// 记录当前下单商品信息
	QString soldTime = QString(QTime::currentTime().toString("hh:mm:ss"));
	QString productName = QStringLiteral("\t售出:\t") + ui.nameSelect->currentText();
	QString buyingAmount = QStringLiteral("\t数量:\t") + QString::number(ui.amount_buy->value());
	QString singlePrice = QStringLiteral("\t单价:\t") + QString::number(ui.lcdNumber_single->value());
	QString totalPrice = QStringLiteral("\t总价:\t") + QString::number(ui.lcdNumber_total->value());
	QString infoList = soldTime + productName + buyingAmount + singlePrice + totalPrice;

	QStandardItem* divid = new QStandardItem("--------------------------------");
  	QStandardItem* info = new QStandardItem(infoList);

	m_pSoldProductModel->appendRow(divid);
	m_pSoldProductModel->appendRow(info);

	//因为没有取消订单按钮，所以认定每次加入订单都会付款购买，所以加入订单的时候就立刻更新库存
	int currentInventory = 0;
	QString selectQuery = QString("SELECT inventory FROM products WHERE name = :productName");
	QSqlQuery queryCurrentInventory;
	queryCurrentInventory.prepare(selectQuery);
	queryCurrentInventory.bindValue(":productName", ui.nameSelect->currentText());
	if (queryCurrentInventory.exec() && queryCurrentInventory.next()) 
	{
		currentInventory = queryCurrentInventory.value("inventory").toInt();				// 当前库存量

		QString updateQuery = QString("UPDATE products SET inventory = :newInventory WHERE name = :currentProductName");
		QSqlQuery update;
		update.prepare(updateQuery);
		update.bindValue(":currentProductName", ui.nameSelect->currentText());
		update.bindValue(":newInventory", currentInventory - ui.amount_buy->value());
		if (update.exec()) 
		{
			/// <summary>
			/// 更新数据库后，界面数据更新
			/// </summary>
			initTableView();											//更新“入库”界面的商品展示表
			ui.btnPayOrder->setEnabled(true);							//设置付款按钮启用
			ui.orderList->setModel(m_pSoldProductModel);				//展示已下单商品信息
			currentTotalConsumption += ui.lcdNumber_total->value();		//累计已下单商品金额
			int inventoryBefore = ui.lcdNumber_inventory->value();		//更新库存显示
			ui.lcdNumber_inventory->display(inventoryBefore - ui.amount_buy->value());
			ui.amount_buy->setValue(0);									//购买数量归零
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("已加入订单。"));
		}
		else 
		{
			// 更新失败
		}
	}
	else 
	{
		// 查询失败
	}
}

void MainWindow::onPayOrderClicked()
{
	/// <summary>
	/// 记录付款信息
	/// </summary>
	QString payTime = QStringLiteral("日期:\t") + QString(QDate::currentDate().toString("yyyy-MM-dd")) + "\t" + QString(QTime::currentTime().toString("hh:mm:ss"));
	QString orderNo = QStringLiteral("\t订单号:\t") + QString::number(currentOrderNo);
	QString totalConsumption = QStringLiteral("\t应付款总额:\t") + QString::number(currentTotalConsumption);
	QString Conclusion = QStringLiteral("\t付款成功！\t");

	QString infoList = payTime + orderNo + totalConsumption + Conclusion;

	QStandardItem* divid = new QStandardItem("**********************************");
	QStandardItem* info = new QStandardItem(infoList);

	m_pSoldProductModel->appendRow(divid);
	m_pSoldProductModel->appendRow(info);

	ui.orderList->setModel(m_pSoldProductModel);

	/// <summary>
	/// 付款后的一些操作
	/// </summary>
	ui.btnPayOrder->setEnabled(false);				//付款后，禁用付款按钮，等待下一次加入订单后启用
	currentOrderNo++;								//订单编号自增
	currentTotalConsumption = 0;					//清空商品总金额
	QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("已完成付款。"));
}

void MainWindow::onTypeChanged()
{
	QString currentType = ui.typeSelect_buy->currentText();			// 获取当前选中商品类型
	m_pSqlTabModel->select();
	ui.nameSelect->clear();
	for (int i = 0; i < m_pSqlTabModel->rowCount(); ++i)			//添加当前商品类型下的商品名称
	{
		QSqlRecord record = m_pSqlTabModel->record(i);
		QString recordType = record.value("type").toString();
		if (recordType == currentType)
		{
			ui.nameSelect->addItem(record.value("name").toString());
		}
	}
}

void MainWindow::onNameConfirmed()
{
	ui.amount_buy->setValue(0);								//清空上次选择商品的数量

	QString currentName = ui.nameSelect->currentText();			// 获取当前选中商品名称

	QSqlQuery query;
	query.prepare("SELECT salePrice,inventory FROM products WHERE name = :name");
	query.bindValue(":name", currentName);
	if (query.exec() && query.next()) 
	{
		ui.lcdNumber_single->display(query.value("salePrice").toDouble());
 		ui.lcdNumber_inventory->display(query.value("inventory").toInt());
		ui.amount_buy->setEnabled(true);						//启用数量编辑
		ui.btnAddInOrder->setEnabled(true);						//启用加入订单
	}
	else 
	{
		// 查询失败或未找到记录
	}
}

void MainWindow::onBuyingAmountChanged()
{
	ui.lcdNumber_total->display(ui.lcdNumber_single->value() * ui.amount_buy->value());
}