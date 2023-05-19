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

	setWindowTitle(QStringLiteral("��Ʒ����ϵͳ"));
	ui.toolBox->setCurrentIndex(0);										// Ĭ�ϴ򿪡���Ʒ��⡱ҳ��
	ui.StorageTable->setEditTriggers(QAbstractItemView::NoEditTriggers);// ���ֻչʾ�����ɱ༭
	ui.btnPayOrder->setEnabled(false);									// ���ťĬ�ϲ����ã��ڼ��붩���ɹ������ã�������ٴν���
	ui.btnAddInOrder->setEnabled(false);								// ���붩����ťĬ�ϲ��ɱ༭����ȷ����Ʒ��������
	ui.amount_buy->setEnabled(false);									// �µ�����Ĭ�ϲ��ɱ༭����ȷ����Ʒ��������

	connectProductDataBase();
	initTableView();
	initTypeSelect();
	restrictLineEdit();

	m_pSoldProductModel = new QStandardItemModel(this);					// ��ʼ���۳���Ʒ��

	(void)connect(ui.btnAddInStorage, &QPushButton::clicked, this, &MainWindow::onAddInStorageClicked);
	(void)connect(ui.btnClearStorage, &QPushButton::clicked, this, &MainWindow::onClearStorageClicked);
	(void)connect(ui.btnAddInOrder, &QPushButton::clicked, this, &MainWindow::onAddInOrderClicked);
	(void)connect(ui.btnPayOrder, &QPushButton::clicked, this, &MainWindow::onPayOrderClicked);

	// ��Ʒ��������Ʒ���Ƶ��л�����
	connect(ui.typeSelect_buy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTypeChanged);
	// ��Ʒ����ȷ����ȷ�����ۺͿ��
	connect(ui.nameSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onNameConfirmed);
	// �޸��µ��������ܼ۸ı�
	connect(ui.amount_buy, QOverload<int>::of(&QSpinBox::valueChanged), this, & MainWindow::onBuyingAmountChanged);
}

MainWindow::~MainWindow()
{}


void MainWindow::connectProductDataBase()
{
	bool ok;

	//�����ظ�����ͬһ���ݿ�
// 	if (QSqlDatabase::contains("qt_sql_default_connection"))
// 	{
// 		product_db = QSqlDatabase::database("qt_sql_default_connection");
// 	}
// 	else
// 	{
// 		product_db = QSqlDatabase::addDatabase("QSQLITE");
// 	}

	product_db = QSqlDatabase::addDatabase("QSQLITE");
	product_db.setDatabaseName("product.db");						// �������ݿ������

	ok = product_db.open();											// �����ݿ�����
	if (!ok)
		qDebug() << __FUNCTION__ << product_db.lastError().text();
}

void MainWindow::initTableView()
{
	m_pSqlTabModel = new QSqlTableModel(this);
	m_pSqlTabModel->setTable("products");
	m_pSqlTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

	m_pSqlTabModel->select();

	m_pSqlTabModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("��Ʒ����"));
	m_pSqlTabModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("��Ʒ����"));
	m_pSqlTabModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("����"));
	m_pSqlTabModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("�ۼ�"));
	m_pSqlTabModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("���"));
	ui.StorageTable->setModel(m_pSqlTabModel);
}

void MainWindow::initTypeSelect()
{
	QSqlQuery query;
	QString typeQuery = "SELECT DISTINCT type FROM products";
	query.exec(typeQuery);

	QSet<QString> types;								// ʹ��QSet������Ψһ������ֵ

	while (query.next()) 
	{
		QString type = query.value("type").toString();	// ��ȡ��ѯ����е�����ֵ
		types.insert(type);								// ������ֵ��ӵ�QSet��
	}
	// ������е���Ʒ���Ե�������
	for (const QString& type : types) 
	{
		ui.typeSelect_buy->addItem(type);
		ui.typeSelect_store->addItem(type);
	}
}

void MainWindow::restrictLineEdit()
{
	/// <summary>
	/// �Խ��ۡ��ۼۡ�����������޶�
	/// </summary>

	// ����һ��������ʽ��ֻ�����������ֺ�һ��С����
	QRegularExpression regex("[0-9]*\\.?[0-9]*");

	// ����һ����֤����ʹ��������ʽ������֤
	QRegularExpressionValidator* validator_purchase = new QRegularExpressionValidator(regex, ui.purchasePrice);
	QRegularExpressionValidator* validator_sale = new QRegularExpressionValidator(regex, ui.salePrice);

	// ������֤����QLineEdit��
	ui.purchasePrice->setValidator(validator_purchase);
	ui.salePrice->setValidator(validator_sale);
}

void MainWindow::onAddInStorageClicked()
{
	QString type = ui.typeSelect_store->currentText();	//��Ʒ����
	QString name = ui.name->text();						//��Ʒ����
	QString purchasePrice = ui.purchasePrice->text();	//����
	QString salePrice = ui.salePrice->text();			//�ۼ�
	QString amount_add = ui.amount_add->text();			//�������

	// ��ѯ�Ƿ�����������Ʒ��Ϣ
	if (ui.typeSelect_store->currentIndex() == 0 || name.isEmpty() || purchasePrice.isEmpty() || salePrice.isEmpty())
	{
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("������������Ʒ��Ϣ��"));
		return;
	}

	// ��ѯ�Ƿ���ڵ�ǰ�������Ʒ����
	QSqlQuery query;
	query.prepare("SELECT * FROM products WHERE name = :name");
	query.bindValue(":name", name);
	if (query.exec() && query.next())					
	{
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("����Ʒ�Ѵ��ڣ������ظ�¼��ͬ����Ʒ��"));
		return;
	}
	// ��ѯ��Ʒ�����������0
	if (amount_add.toInt() == 0)						
	{
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("��Ʒ�����������0"));
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
	/// ���󣬻ָ����������
	/// </summary>
	ui.typeSelect_store->setCurrentIndex(0);
	ui.name->clear();
	ui.purchasePrice->clear();
	ui.salePrice->clear();
	ui.amount_add->setValue(0);
}

void MainWindow::onClearStorageClicked()
{
	//������ݿ�
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
	//�޶��µ�����
	if (ui.amount_buy->value() == 0)
	{
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("������������0��"));
		return;
	}
	if (ui.amount_buy->value() > ui.lcdNumber_inventory->value())
	{
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("�����������������Ʒ����"));
		return;
	}

	// ��¼��ǰ�µ���Ʒ��Ϣ
	QString soldTime = QString(QTime::currentTime().toString("hh:mm:ss"));
	QString productName = QStringLiteral("\t�۳�:\t") + ui.nameSelect->currentText();
	QString buyingAmount = QStringLiteral("\t����:\t") + QString::number(ui.amount_buy->value());
	QString singlePrice = QStringLiteral("\t����:\t") + QString::number(ui.lcdNumber_single->value());
	QString totalPrice = QStringLiteral("\t�ܼ�:\t") + QString::number(ui.lcdNumber_total->value());
	QString infoList = soldTime + productName + buyingAmount + singlePrice + totalPrice;

	QStandardItem* divid = new QStandardItem("--------------------------------");
  	QStandardItem* info = new QStandardItem(infoList);

	m_pSoldProductModel->appendRow(divid);
	m_pSoldProductModel->appendRow(info);

	//��Ϊû��ȡ��������ť�������϶�ÿ�μ��붩�����Ḷ������Լ��붩����ʱ������̸��¿��
	int currentInventory = 0;
	QString selectQuery = QString("SELECT inventory FROM products WHERE name = :productName");
	QSqlQuery queryCurrentInventory;
	queryCurrentInventory.prepare(selectQuery);
	queryCurrentInventory.bindValue(":productName", ui.nameSelect->currentText());
	if (queryCurrentInventory.exec() && queryCurrentInventory.next()) 
	{
		currentInventory = queryCurrentInventory.value("inventory").toInt();				// ��ǰ�����

		QString updateQuery = QString("UPDATE products SET inventory = :newInventory WHERE name = :currentProductName");
		QSqlQuery update;
		update.prepare(updateQuery);
		update.bindValue(":currentProductName", ui.nameSelect->currentText());
		update.bindValue(":newInventory", currentInventory - ui.amount_buy->value());
		if (update.exec()) 
		{
			/// <summary>
			/// �������ݿ�󣬽������ݸ���
			/// </summary>
			initTableView();											//���¡���⡱�������Ʒչʾ��
			ui.btnPayOrder->setEnabled(true);							//���ø��ť����
			ui.orderList->setModel(m_pSoldProductModel);				//չʾ���µ���Ʒ��Ϣ
			currentTotalConsumption += ui.lcdNumber_total->value();		//�ۼ����µ���Ʒ���
			int inventoryBefore = ui.lcdNumber_inventory->value();		//���¿����ʾ
			ui.lcdNumber_inventory->display(inventoryBefore - ui.amount_buy->value());
			ui.amount_buy->setValue(0);									//������������
			QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("�Ѽ��붩����"));
		}
		else 
		{
			// ����ʧ��
		}
	}
	else 
	{
		// ��ѯʧ��
	}
}

void MainWindow::onPayOrderClicked()
{
	/// <summary>
	/// ��¼������Ϣ
	/// </summary>
	QString payTime = QStringLiteral("����:\t") + QString(QDate::currentDate().toString("yyyy-MM-dd")) + "\t" + QString(QTime::currentTime().toString("hh:mm:ss"));
	QString orderNo = QStringLiteral("\t������:\t") + QString::number(currentOrderNo);
	QString totalConsumption = QStringLiteral("\tӦ�����ܶ�:\t") + QString::number(currentTotalConsumption);
	QString Conclusion = QStringLiteral("\t����ɹ���\t");

	QString infoList = payTime + orderNo + totalConsumption + Conclusion;

	QStandardItem* divid = new QStandardItem("**********************************");
	QStandardItem* info = new QStandardItem(infoList);

	m_pSoldProductModel->appendRow(divid);
	m_pSoldProductModel->appendRow(info);

	ui.orderList->setModel(m_pSoldProductModel);

	/// <summary>
	/// ������һЩ����
	/// </summary>
	ui.btnPayOrder->setEnabled(false);				//����󣬽��ø��ť���ȴ���һ�μ��붩��������
	currentOrderNo++;								//�����������
	currentTotalConsumption = 0;					//�����Ʒ�ܽ��
	QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("����ɸ��"));
}

void MainWindow::onTypeChanged()
{
	QString currentType = ui.typeSelect_buy->currentText();			// ��ȡ��ǰѡ����Ʒ����
	m_pSqlTabModel->select();
	ui.nameSelect->clear();
	for (int i = 0; i < m_pSqlTabModel->rowCount(); ++i)			//��ӵ�ǰ��Ʒ�����µ���Ʒ����
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
	ui.amount_buy->setValue(0);								//����ϴ�ѡ����Ʒ������

	QString currentName = ui.nameSelect->currentText();			// ��ȡ��ǰѡ����Ʒ����

	QSqlQuery query;
	query.prepare("SELECT salePrice,inventory FROM products WHERE name = :name");
	query.bindValue(":name", currentName);
	if (query.exec() && query.next()) 
	{
		ui.lcdNumber_single->display(query.value("salePrice").toDouble());
 		ui.lcdNumber_inventory->display(query.value("inventory").toInt());
		ui.amount_buy->setEnabled(true);						//���������༭
		ui.btnAddInOrder->setEnabled(true);						//���ü��붩��
	}
	else 
	{
		// ��ѯʧ�ܻ�δ�ҵ���¼
	}
}

void MainWindow::onBuyingAmountChanged()
{
	ui.lcdNumber_total->display(ui.lcdNumber_single->value() * ui.amount_buy->value());
}