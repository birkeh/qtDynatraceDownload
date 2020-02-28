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


QSqlDatabase	g_db;


cMainWindow::cMainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::cMainWindow),
	  m_lpDownloadsListModel(nullptr),
	  m_lpPathsListModel(nullptr),
	  m_downloadsList(&m_pathsList)
{
	ui->setupUi(this);

	ui->m_lpMainTab->setCurrentIndex(0);

	m_lpDownloadsListModel	= new QStandardItemModel(0, 0);
	ui->m_lpDownloadsList->setModel(m_lpDownloadsListModel);

	m_lpPathsListModel	= new QStandardItemModel(0, 0);
	ui->m_lpPathsList->setModel(m_lpPathsListModel);
	ui->m_lpPathsList->setItemDelegate(new cPathsItemDelegate);

	QStringList	header;

	header << "server" << "path" << "local folder" << "username" << "password";
	m_lpPathsListModel->setHorizontalHeaderLabels(header);

	g_db	= QSqlDatabase::addDatabase("QSQLITE", "qtDynatraceDownload");
	g_db.setHostName("localhost");
	g_db.setDatabaseName(QDir::homePath() + "/qtDynatraceDownload.db");

	if(!g_db.open())
	{
		qDebug() << "open database: " << g_db.lastError().text();
		return;
	}

	QSqlQuery	query(g_db);

	if(!g_db.tables().contains("paths"))
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
			g_db.close();
			return;
		}
	}

	if(!g_db.tables().contains("downloads"))
	{
		query.prepare("CREATE TABLE downloads "
					  "( "
					  "    id            INTEGER  PRIMARY KEY AUTOINCREMENT UNIQUE, "
					  "    pathsID       INTEGER  REFERENCES paths (id), "
					  "    fileName      TEXT, "
					  "    timestamp     DATETIME, "
					  "    reportName    TEXT, "
					  "    localFileName TEXT, "
					  "    downloaded    DATETIME "
					  ");");

		if(!query.exec())
		{
			qDebug() << "SELECT paths: " << query.lastError().text();
			g_db.close();
			return;
		}
	}

	m_pathsList.load();

	for(int x = 0;x < m_pathsList.count();x++)
	{
		cPaths*	lpPaths	= m_pathsList[x];

		QList<QStandardItem*>	items;

		items.append(new QStandardItem(lpPaths->server()));
		items.append(new QStandardItem(lpPaths->path()));
		items.append(new QStandardItem(lpPaths->localFolder()));
		items.append(new QStandardItem(lpPaths->userName()));
		items.append(new QStandardItem("*****"));

		items[0]->setCheckable(true);
		items[0]->setCheckState(lpPaths->active() ? Qt::Checked : Qt::Unchecked);

		items[0]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
		items[1]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
		items[2]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
		items[3]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
		items[4]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);

		m_lpPathsListModel->appendRow(items);
	}

	ui->m_lpPathsList->resizeColumnToContents(0);
	ui->m_lpPathsList->resizeColumnToContents(1);
	ui->m_lpPathsList->resizeColumnToContents(2);
	ui->m_lpPathsList->resizeColumnToContents(3);
	ui->m_lpPathsList->resizeColumnToContents(4);

	m_downloadsList.load();

	connect(ui->m_lpUpdate,		&QPushButton::clicked,					this,	&cMainWindow::onUpdate);
	connect(ui->m_lpDownload,	&QPushButton::clicked,					this,	&cMainWindow::onDownload);

	connect(ui->m_lpPathsList,	&QTreeView::customContextMenuRequested,	this,	&cMainWindow::onPathsContextMenu);
	connect(m_lpPathsListModel,	&QStandardItemModel::itemChanged,		this,	&cMainWindow::onPathsChanged);
}

cMainWindow::~cMainWindow()
{
	if(g_db.isOpen())
		g_db.close();
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

void cMainWindow::onUpdate()
{
	for(int row = 0;row < m_lpPathsListModel->rowCount();row++)
	{
		cPaths*			lpPaths			= m_lpPathsListModel->item(row, 0)->data(Qt::UserRole+1).value<cPaths*>();

		if(lpPaths->active())
			m_downloadsList.load(lpPaths);
	}
}

void cMainWindow::onDownload()
{
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
	cPaths*		lpPaths	= m_pathsList.add();

	QList<QStandardItem*>	items;

	items.append(new QStandardItem(lpPaths->server()));
	items.append(new QStandardItem(lpPaths->path()));
	items.append(new QStandardItem(lpPaths->localFolder()));
	items.append(new QStandardItem(lpPaths->userName()));
	items.append(new QStandardItem(lpPaths->password()));

	items[0]->setCheckable(true);
	items[0]->setCheckState(lpPaths->active() ? Qt::Checked : Qt::Unchecked);
	items[0]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
	items[1]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
	items[2]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
	items[3]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);
	items[4]->setData(QVariant::fromValue(lpPaths), Qt::UserRole+1);

	m_lpPathsListModel->appendRow(items);
}

void cMainWindow::onPathsDelete()
{
	if(!ui->m_lpPathsList->selectionModel()->selectedIndexes().count())
		return;

	if(QMessageBox::question(this, "Delete", "Do you want to delete the path?") == QMessageBox::No)
		return;

	QModelIndex	index	= ui->m_lpPathsList->selectionModel()->selectedIndexes()[0];
	cPaths*		lpPaths	= index.data(Qt::UserRole+1).value<cPaths*>();

	if(!m_pathsList.del(lpPaths))
		return;

	m_lpPathsListModel->removeRow(index.row());
}

void cMainWindow::onPathsChanged(QStandardItem* item)
{
	disconnect(m_lpPathsListModel,	&QStandardItemModel::itemChanged,		this,	&cMainWindow::onPathsChanged);

	cPaths*		lpPaths	= item->data(Qt::UserRole+1).value<cPaths*>();

	switch(item->column())
	{
	case 0:
		lpPaths->setServer(m_lpPathsListModel->item(item->row(), 0)->text());
		break;
	case 1:
		lpPaths->setPath(m_lpPathsListModel->item(item->row(), 1)->text());
		break;
	case 2:
		lpPaths->setLocalFolder(m_lpPathsListModel->item(item->row(), 2)->text());
		break;
	case 3:
		lpPaths->setUserName(m_lpPathsListModel->item(item->row(), 3)->text());
		break;
	case 4:
		lpPaths->setPassword(m_lpPathsListModel->item(item->row(), 4)->text());
		break;
	}

	lpPaths->setActive(m_lpPathsListModel->item(item->row(), 0)->checkState() == Qt::Checked);

	if(!lpPaths->save())
	{
		m_lpPathsListModel->item(item->row(), 0)->setText(lpPaths->server());
		m_lpPathsListModel->item(item->row(), 1)->setText(lpPaths->path());
		m_lpPathsListModel->item(item->row(), 2)->setText(lpPaths->localFolder());
		m_lpPathsListModel->item(item->row(), 3)->setText(lpPaths->userName());
		m_lpPathsListModel->item(item->row(), 4)->setText("*****");
		m_lpPathsListModel->item(item->row(), 0)->setCheckState(lpPaths->active() ? Qt::Checked : Qt::Unchecked);
	}
	else
		m_lpPathsListModel->item(item->row(), 4)->setText("*****");

	connect(m_lpPathsListModel,	&QStandardItemModel::itemChanged,		this,	&cMainWindow::onPathsChanged);
}
