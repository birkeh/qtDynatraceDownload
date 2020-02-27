#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QSqlQuery>
#include <QSqlError>

#include <QDir>
#include <QList>
#include <QStandardItem>

#include <QSettings>
#include <QDebug>

#include <QMessageBox>


cMainWindow::cMainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::cMainWindow),
	  m_lpDownloadsListModel(nullptr),
	  m_lpPathsListModel(nullptr)
{
	ui->setupUi(this);

	m_lpDownloadsListModel	= new QStandardItemModel(0, 0);
	ui->m_lpDownloadsList->setModel(m_lpDownloadsListModel);

	m_lpPathsListModel	= new QStandardItemModel(0, 0);
	ui->m_lpPathsList->setModel(m_lpPathsListModel);

	QStringList	header;

	header << "server" << "path" << "local folder" << "username" << "password";
	m_lpPathsListModel->setHorizontalHeaderLabels(header);

	m_db	= QSqlDatabase::addDatabase("QSQLITE", "qtDynatraceDownload");
	m_db.setHostName("localhost");
	m_db.setDatabaseName(QDir::homePath() + "/qtDynatraceDownload.db");

	if(!m_db.open())
	{
		qDebug() << "open database: " << m_db.lastError().text();
		return;
	}

	QSqlQuery	query(m_db);

	if(!m_db.tables().contains("paths"))
	{
		query.prepare("CREATE TABLE paths "
					  "( "
					  "    id            INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
					  "    server        TEXT NOT NULL, "
					  "    path          TEXT NOT NULL, "
					  "    localFolder   TEXT NOT NULL, "
					  "    username      TEXT, "
					  "    password      TEXT, "
					  "    active        BOOLEAN, "
					  "    UNIQUE(server,path, localFolder) "
					  ");");

		if(!query.exec())
		{
			qDebug() << "SELECT paths: " << query.lastError().text();
			m_db.close();
			return;
		}
	}

	if(!m_db.tables().contains("downloads"))
	{
		query.prepare("CREATE TABLE downloads "
					  "( "
					  "    id            INTEGER  PRIMARY KEY AUTOINCREMENT UNIQUE, "
					  "    pathsID       INTEGER  REFERENCES paths (id), "
					  "    fileName      TEXT, "
					  "    localFileName TEXT, "
					  "    timestamp     DATETIME "
					  ");");

		if(!query.exec())
		{
			qDebug() << "SELECT paths: " << query.lastError().text();
			m_db.close();
			return;
		}
	}

	query.prepare("SELECT id, server, path, localFolder, username, password, active FROM paths ORDER BY server, path, localFolder;");
	if(!query.exec())
	{
		qDebug() << "SELECT paths: " << query.lastError().text();
		return;
	}

	while(query.next())
	{
		QList<QStandardItem*>	items;

		items.append(new QStandardItem(query.value("server").toString()));
		items.append(new QStandardItem(query.value("path").toString()));
		items.append(new QStandardItem(query.value("localFolder").toString()));
		items.append(new QStandardItem(query.value("userName").toString()));
		items.append(new QStandardItem(query.value("password").toString()));
//		if(query.value("password").toString().isEmpty())
//			items.append(new QStandardItem(""));
//		else
//			items.append(new QStandardItem("*****"));

		items[0]->setCheckable(true);
		items[0]->setCheckState(query.value("active").toBool() ? Qt::Checked : Qt::Unchecked);

		items[0]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
		items[0]->setData(QVariant::fromValue(query.value("server").toString()), Qt::UserRole+2);
		items[0]->setData(QVariant::fromValue(query.value("active").toBool()), Qt::UserRole+3);

		items[1]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
		items[1]->setData(QVariant::fromValue(query.value("path").toString()), Qt::UserRole+2);

		items[2]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
		items[2]->setData(QVariant::fromValue(query.value("localFolder").toString()), Qt::UserRole+2);

		items[3]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
		items[3]->setData(QVariant::fromValue(query.value("userName").toString()), Qt::UserRole+2);

		items[4]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
		items[4]->setData(QVariant::fromValue(query.value("password").toString()), Qt::UserRole+2);

		m_lpPathsListModel->appendRow(items);
	}

	connect(ui->m_lpPathsList,	&QTreeView::customContextMenuRequested,	this,	&cMainWindow::onPathsContextMenu);
	connect(m_lpPathsListModel,	&QStandardItemModel::itemChanged,		this,	&cMainWindow::onPathsChanged);
}

cMainWindow::~cMainWindow()
{
	if(m_db.isOpen())
		m_db.close();
	delete ui;
}

void cMainWindow::closeEvent(QCloseEvent *event)
{
	QSettings	settings;
	settings.setValue("main/width", QVariant::fromValue(size().width()));
	settings.setValue("main/height", QVariant::fromValue(size().height()));
	settings.setValue("main/x", QVariant::fromValue(x()));
	settings.setValue("main/y", QVariant::fromValue(y()));
	if(this->isMaximized())
		settings.setValue("main/maximized", QVariant::fromValue(true));
	else
		settings.setValue("main/maximized", QVariant::fromValue(false));

	event->accept();
}

void cMainWindow::onPathsContextMenu(const QPoint &pos)
{
	QMenu*	lpMenu	= new QMenu(this);

	lpMenu->addAction("add", this, SLOT(onPathsAdd()));

	if(ui->m_lpPathsList->selectionModel()->selectedIndexes().count())
		lpMenu->addAction("delete", this, SLOT(onPathsDelete()));

	lpMenu->popup(ui->m_lpPathsList->viewport()->mapToGlobal(pos));
}

void cMainWindow::onPathsAdd()
{
	QSqlQuery				query(m_db);
	int						number;

	query.prepare("INSERT INTO paths (server, path, localFolder, active) VALUES (:server, :path, :localFolder, :active);");
	query.bindValue(":path", "path");
	query.bindValue(":localFolder", "localFolder");
	query.bindValue(":active", true);

	for(number = 1;;number++)
	{
		query.bindValue(":server", QString("server%1").arg(number));
		if(query.exec())
			break;
	}

	query.prepare("SELECT id FROM paths WHERE server=:server AND path=:path AND localFolder=:localFolder;");
	query.bindValue(":server", QString("server%1").arg(number));
	query.bindValue(":path", "path");
	query.bindValue(":localFolder", "localFolder");
	if(!query.exec())
	{
		qDebug() << "new path: " << query.lastError().text();
		return;
	}
	if(!query.next())
	{
		qDebug() << "new path: " << query.lastError().text();
		return;
	}

	QList<QStandardItem*>	items;

	items.append(new QStandardItem(QString("server%1").arg(number)));
	items.append(new QStandardItem("path"));
	items.append(new QStandardItem("localFolder"));
	items.append(new QStandardItem(""));
	items.append(new QStandardItem(""));

	items[0]->setCheckable(true);
	items[0]->setCheckState(Qt::Checked);

	items[0]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
	items[0]->setData(QVariant::fromValue(QString("server%1").arg(number)), Qt::UserRole+2);
	items[0]->setData(QVariant::fromValue(Qt::Checked), Qt::UserRole+3);

	items[1]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
	items[1]->setData(QVariant::fromValue(QString("path")), Qt::UserRole+2);

	items[2]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
	items[2]->setData(QVariant::fromValue(QString("localFolder")), Qt::UserRole+2);

	items[3]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
	items[3]->setData(QVariant::fromValue(QString("")), Qt::UserRole+2);

	items[4]->setData(QVariant::fromValue(query.value("id").toInt()), Qt::UserRole+1);
	items[4]->setData(QVariant::fromValue(QString("")), Qt::UserRole+2);

	m_lpPathsListModel->appendRow(items);
}

void cMainWindow::onPathsDelete()
{
	if(!ui->m_lpPathsList->selectionModel()->selectedIndexes().count())
		return;

	if(QMessageBox::question(this, "Delete", "Do you want to delete the path?") == QMessageBox::No)
		return;

	QModelIndex	index	= ui->m_lpPathsList->selectionModel()->selectedIndexes()[0];
	QSqlQuery	query(m_db);

	query.prepare("DELETE FROM paths WHERE id=:id;");
	query.bindValue(":id", m_lpPathsListModel->data(index, Qt::UserRole+1).toInt());

	if(!query.exec())
	{
		qDebug() << "onPathsChanged: " << query.lastError().text();
		return;
	}

	m_lpPathsListModel->removeRow(index.row());
}

void cMainWindow::onPathsChanged(QStandardItem* item)
{
	QSqlQuery	query(m_db);
	bool		save	= false;

	if(m_lpPathsListModel->item(item->row(), 0)->data(Qt::UserRole+2).toString() != m_lpPathsListModel->item(item->row(), 0)->text())
		save	= true;
	if(m_lpPathsListModel->item(item->row(), 1)->data(Qt::UserRole+2).toString() != m_lpPathsListModel->item(item->row(), 1)->text())
		save	= true;
	if(m_lpPathsListModel->item(item->row(), 2)->data(Qt::UserRole+2).toString() != m_lpPathsListModel->item(item->row(), 2)->text())
		save	= true;
	if(m_lpPathsListModel->item(item->row(), 3)->data(Qt::UserRole+2).toString() != m_lpPathsListModel->item(item->row(), 3)->text())
		save	= true;
	if(m_lpPathsListModel->item(item->row(), 4)->data(Qt::UserRole+2).toString() != m_lpPathsListModel->item(item->row(), 4)->text())
		save	= true;
	if((m_lpPathsListModel->item(item->row(), 0)->data(Qt::UserRole+3).toBool() != (m_lpPathsListModel->item(item->row(), 0)->checkState() == Qt::Checked)))
		save	= true;

	if(!save)
		return;

	query.prepare("UPDATE paths SET server=:server, path=:path, localFolder=:localFolder, username=:username, password=:password, active=:active WHERE id=:id;");
	query.bindValue(":id",			m_lpPathsListModel->item(item->row(), 0)->data(Qt::UserRole+1).toInt());
	query.bindValue(":server",		m_lpPathsListModel->item(item->row(), 0)->text());
	query.bindValue(":path",		m_lpPathsListModel->item(item->row(), 1)->text());
	query.bindValue(":localFolder",	m_lpPathsListModel->item(item->row(), 2)->text());
	query.bindValue(":username",	m_lpPathsListModel->item(item->row(), 3)->text());
	query.bindValue(":password",	m_lpPathsListModel->item(item->row(), 4)->text());
	query.bindValue(":active",		m_lpPathsListModel->item(item->row(), 0)->checkState() == Qt::Checked);

	disconnect(ui->m_lpPathsList,	&QTreeView::customContextMenuRequested,	this,	&cMainWindow::onPathsContextMenu);

	if(!query.exec())
	{
		item->setText(item->data(Qt::UserRole+2).toString());
		qDebug() << "onPathsChanged: " << query.lastError().text();

		connect(ui->m_lpPathsList,	&QTreeView::customContextMenuRequested,	this,	&cMainWindow::onPathsContextMenu);
		return;
	}

	item->setData(QVariant::fromValue(item->text()), Qt::UserRole+2);
	m_lpPathsListModel->item(item->row(), 0)->setData(QVariant::fromValue(m_lpPathsListModel->item(item->row(), 0)->checkState() == Qt::Checked), Qt::UserRole+3);
	connect(ui->m_lpPathsList,	&QTreeView::customContextMenuRequested,	this,	&cMainWindow::onPathsContextMenu);
}
