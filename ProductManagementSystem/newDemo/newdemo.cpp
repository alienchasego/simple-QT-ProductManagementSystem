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

	setWindowTitle(QStringLiteral("��¼"));
	ui.btnAddAdmin->setEnabled(false);			// ע��¼�밴ť���ڵ�¼ʧ��ǰ���ã���¼ʧ�ܺ�����

	connectAdminDataBase();
	(void)connect(ui.btnLogin, &QPushButton::clicked, this, &LoginWindow::onBtnLoginClicked);
	(void)connect(ui.btnAddAdmin, &QPushButton::clicked, this, &LoginWindow::onBtnAddAdminClicked);
}

LoginWindow::~LoginWindow()
{}

void LoginWindow::connectAdminDataBase()
{
	bool ok;

	admin_db = QSqlDatabase::addDatabase("QSQLITE");// �������ݿ�����,����ӵ����ݿ������б���
	admin_db.setDatabaseName("Admin.db");			// �������ݿ������

	ok = admin_db.open();							// �����ݿ�����
	if (!ok)
		qDebug() << __FUNCTION__ << admin_db.lastError().text();

	m_pSqlTabModel = new QSqlTableModel(this);
	m_pSqlTabModel->setTable("AdminAccount");		//  ���ģ�͹����ı�Ϊ AdminAccount
	m_pSqlTabModel->setEditStrategy(QSqlTableModel::OnManualSubmit);// ����Ϊ�ֶ��ύ
}

void LoginWindow::onBtnLoginClicked()
{
	QString account = ui.lineEdit_account->text();
	QString password = ui.lineEdit_password->text();

	QSqlQuery query;
	query.prepare("SELECT * FROM AdminAccount WHERE account = :account");
	query.bindValue(":account", account);
	if (query.exec() && query.next())						//��ѯ�Ƿ���ڵ�ǰ�˻�
	{
		QString correctPassword = query.value("password").toString();
		if (password == correctPassword)					//�Աȵ�ǰ��������ȷ���룬����ѯ�ɹ�����ת��ϵͳ������
		{
			accept();
		}
		else
		{
			QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("���벻��ȷ��"));
		}
	}
	else
	{
		QSqlQuery queryRecord("SELECT * FROM AdminAccount");	// ��ѯ���м�¼
		qDebug() << "��ǰ���м�¼: ";
		while (queryRecord.next()) {							// ������ѯ����ڿ���̨���
			QString account = queryRecord.value("account").toString();
			QString password = queryRecord.value("password").toString();
			qDebug() << "Account: " << account << ", Password: " << password;
		}
		ui.btnAddAdmin->setEnabled(true);						//����ע�ᰴť�����Խ���ǰ�˺�����¼��
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("��ǰ�˺Ų����ڣ���¼�뵱ǰ�˺�������Ϣ��"));
	}
}

void LoginWindow::onBtnAddAdminClicked()
{
	QString account = ui.lineEdit_account->text();
	QString password = ui.lineEdit_password->text();

	QSqlRecord record = m_pSqlTabModel->record();				// ��ȡ���ݿ���ֶ���Ϣ

	record.setValue("account", account);
	record.setValue("password", password);

	bool ok;

	ok = m_pSqlTabModel->insertRecord(-1, record);				//����Ϊ -1,��ʾ���� model ���һ��
	if (!ok)
	{
		qDebug() << m_pSqlTabModel->lastError().text();
		return;
	}

	ok = m_pSqlTabModel->submitAll();// �ύ�����ݿ⣬��ʱ���ݻ����
	if (!ok)
	{
		qDebug() << __FUNCTION__ << m_pSqlTabModel->lastError().text();
	}
	else
	{
		QMessageBox::warning(this, QStringLiteral("��ʾ"), QStringLiteral("��������Ա�˺ź�����ɹ���"));
	}
}
