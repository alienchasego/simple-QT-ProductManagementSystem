#include "newdemo.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent), m_pSqlTabModel(nullptr)
{
    ui.setupUi(this);

	setWindowTitle(QStringLiteral("登录"));
	ui.btnAddAdmin->setEnabled(false);			// 注册录入按钮，在登录失败前禁用，登录失败后启用

	connectAdminDataBase();
	(void)connect(ui.btnLogin, &QPushButton::clicked, this, &LoginWindow::onBtnLoginClicked);
	(void)connect(ui.btnAddAdmin, &QPushButton::clicked, this, &LoginWindow::onBtnAddAdminClicked);
}

LoginWindow::~LoginWindow()
{}

void LoginWindow::connectAdminDataBase()
{
	bool ok;

	admin_db = QSqlDatabase::addDatabase("QSQLITE");// 创建数据库连接,并添加到数据库连接列表中
	admin_db.setDatabaseName("Admin.db");			// 设置数据库的名称

	ok = admin_db.open();							// 打开数据库连接
	if (!ok)
		qDebug() << __FUNCTION__ << admin_db.lastError().text();

	m_pSqlTabModel = new QSqlTableModel(this);
	m_pSqlTabModel->setTable("AdminAccount");		//  设计模型关联的表为 AdminAccount
	m_pSqlTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);// 设置为手动提交
}

void LoginWindow::onBtnLoginClicked()
{
	QString account = ui.lineEdit_account->text();
	QString password = ui.lineEdit_password->text();

	QSqlQuery query;
	query.prepare("SELECT * FROM AdminAccount WHERE account = :account");
	query.bindValue(":account", account);
	if (query.exec() && query.next())						//查询是否存在当前账户
	{
		QString correctPassword = query.value("password").toString();
		if (password == correctPassword)					//对比当前密码与正确密码，若查询成功，跳转到系统主界面
		{
			accept();
		}
		else
		{
			QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("密码不正确。"));
		}
	}
	else
	{
		QSqlQuery queryRecord("SELECT * FROM AdminAccount");	// 查询已有记录
		qDebug() << "当前所有记录: ";
		while (queryRecord.next()) {							// 遍历查询结果在控制台输出
			QString account = queryRecord.value("account").toString();
			QString password = queryRecord.value("password").toString();
			qDebug() << "Account: " << account << ", Password: " << password;
		}
		ui.btnAddAdmin->setEnabled(true);						//启用注册按钮，可以将当前账号密码录入
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("当前账号不存在，请录入当前账号密码信息。"));
	}
}

void LoginWindow::onBtnAddAdminClicked()
{
	QString account = ui.lineEdit_account->text();
	QString password = ui.lineEdit_password->text();

	QSqlRecord record = m_pSqlTabModel->record();				// 获取数据库表字段信息

	record.setValue("account", account);
	record.setValue("password", password);

	bool ok;

	ok = m_pSqlTabModel->insertRecord(-1, record);				//设置为 -1,表示插入 model 最后一列
	if (!ok)
	{
		qDebug() << m_pSqlTabModel->lastError().text();
		return;
	}

	ok = m_pSqlTabModel->submitAll();// 提交到数据库，此时数据会更新
	if (!ok)
	{
		qDebug() << __FUNCTION__ << m_pSqlTabModel->lastError().text();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("创建管理员账号和密码成功。"));
	}
}
