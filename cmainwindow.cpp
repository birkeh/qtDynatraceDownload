#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QSqlQuery>
#include <QSqlError>

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <QList>
#include <QStandardItem>

#include <QSettings>
#include <QDebug>

#include <QMessageBox>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>


QSqlDatabase	g_db;


cMainWindow::cMainWindow(QWidget *parent)
	: QMainWindow(parent),
	  ui(new Ui::cMainWindow),
	  m_lpDownloadsListModel(nullptr),
	  m_lpDownloadsFilterProxyModel(nullptr),
	  m_lpPathsListModel(nullptr),
	  m_downloadsList(&m_pathsList)
{
	ui->setupUi(this);

	m_lpProgressBar					= new QProgressBar(this);
	m_lpProgressBar->setVisible(false);
	ui->m_lpStatusBar->addPermanentWidget(m_lpProgressBar);

	ui->m_lpMainTab->setCurrentIndex(0);

	m_lpDownloadsListModel			= new QStandardItemModel(0, 0);
	m_lpDownloadsFilterProxyModel	= new cDownloadsFilterProxyModel(this);
	ui->m_lpDownloadsList->setModel(m_lpDownloadsFilterProxyModel);
	m_lpDownloadsFilterProxyModel->setSourceModel(m_lpDownloadsListModel);

	m_lpPathsListModel				= new QStandardItemModel(0, 0);
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
					  "    localFolder   TEXT, "
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

	for(int x = 0;x < m_lpPathsListModel->columnCount();x++)
		ui->m_lpPathsList->resizeColumnToContents(x);

	m_downloadsList.load();

	fillDownloadsList();

	QSettings	settings;

	ui->m_lpStartDate->setDate(settings.value("startDate", QVariant::fromValue(QDate(2019, 1, 1))).toDate());
	onStartDateChanged(ui->m_lpStartDate->date());

	connect(ui->m_lpUpdate,		&QPushButton::clicked,					this,	&cMainWindow::onUpdate);
	connect(ui->m_lpDownload,	&QPushButton::clicked,					this,	&cMainWindow::onDownload);

	connect(ui->m_lpPathsList,	&QTreeView::customContextMenuRequested,	this,	&cMainWindow::onPathsContextMenu);
	connect(m_lpPathsListModel,	&QStandardItemModel::itemChanged,		this,	&cMainWindow::onPathsChanged);

	connect(ui->m_lpStartDate,	&QDateEdit::dateChanged,				this,	&cMainWindow::onStartDateChanged);
}

cMainWindow::~cMainWindow()
{
	if(g_db.isOpen())
		g_db.close();

	delete m_lpDownloadsListModel;
	delete m_lpDownloadsFilterProxyModel;
	delete m_lpPathsListModel;
	delete m_lpProgressBar;

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
	m_lpProgressBar->setRange(0, m_lpPathsListModel->rowCount()-1);
	m_lpProgressBar->setVisible(true);

	for(int row = 0;row < m_lpPathsListModel->rowCount();row++)
	{
		cPaths*			lpPaths			= m_lpPathsListModel->item(row, 0)->data(Qt::UserRole+1).value<cPaths*>();

		if(lpPaths->active())
			m_downloadsList.load(lpPaths);

		m_lpProgressBar->setValue(row);
		qApp->processEvents();
	}

	fillDownloadsList();

	m_lpProgressBar->setVisible(false);
}

void cMainWindow::onDownload()
{
	int		totalCount	= 0;

	for(int rowServer = 0;rowServer < m_lpDownloadsListModel->rowCount();rowServer++)
	{
		QStandardItem*	lpServer	= m_lpDownloadsListModel->item(rowServer, 0);

		for(int rowPath = 0;rowPath < lpServer->rowCount();rowPath++)
		{
			QStandardItem*	lpPath	= lpServer->child(rowPath, 0);
			totalCount	+= lpPath->rowCount();
		}
	}

	m_lpProgressBar->setRange(0, totalCount-1);
	m_lpProgressBar->setVisible(true);

	int	curRow	= 0;

	for(int rowServer = 0;rowServer < m_lpDownloadsListModel->rowCount();rowServer++)
	{
		QStandardItem*	lpServer	= m_lpDownloadsListModel->item(rowServer, 0);

		for(int rowPath = 0;rowPath < lpServer->rowCount();rowPath++)
		{
			QStandardItem*	lpPath	= lpServer->child(rowPath, 0);

			for(int row = 0;row < lpPath->rowCount();row++)
			{
				QStandardItem*	lpItem	= lpPath->child(row, 0);
				cDownloads*		lpDownloads	= lpItem->data(Qt::UserRole+1).value<cDownloads*>();

				if(!lpDownloads->downloaded().isValid())
				{
					download(lpDownloads);
					lpPath->child(row, 2)->setText(lpDownloads->reportName());
					lpPath->child(row, 3)->setText(lpDownloads->localFolder());
					lpPath->child(row, 4)->setText(lpDownloads->localFileName());
					lpPath->child(row, 5)->setText(lpDownloads->downloaded().toString("dd.MM.yyyy hh:mm:ss"));
				}

				m_lpProgressBar->setValue(curRow);
				qApp->processEvents();

				curRow++;
			}
		}
	}

	for(int x = 0;x < m_lpDownloadsListModel->columnCount();x++)
		ui->m_lpDownloadsList->resizeColumnToContents(x);

	m_lpProgressBar->setVisible(false);
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

void cMainWindow::onStartDateChanged(const QDate& date)
{
	m_lpDownloadsFilterProxyModel->setStartDate(date);

	QSettings	settings;
	settings.setValue("startDate", QVariant::fromValue(date));
}

void cMainWindow::fillDownloadsList()
{
	m_lpDownloadsListModel->clear();

	QStringList		header;
	QString			oldServer	= "";
	QString			oldPath		= "";
	QStandardItem*	lpServer	= nullptr;
	QStandardItem*	lpPath		= nullptr;

	header << "server/path/filename" << "timestamp" << "report name" << "local folder" << "local filename" << "downloaded";
	m_lpDownloadsListModel->setHorizontalHeaderLabels(header);

	m_downloadsList.sort();

	for(int x = 0;x < m_downloadsList.count();x++)
	{
		cDownloads*	lpDownloads	= m_downloadsList[x];

		QList<QStandardItem*>	items;

		if(lpDownloads->paths()->server() != oldServer)
		{
			oldServer	= lpDownloads->paths()->server();
			oldPath		= "";

			lpServer	= new QStandardItem(oldServer);
			lpServer->setData(QVariant::fromValue(0), Qt::UserRole+2);
			m_lpDownloadsListModel->appendRow(lpServer);
		}

		if(lpDownloads->paths()->path() != oldPath)
		{
			oldPath		= lpDownloads->paths()->path();

			lpPath		= new QStandardItem(oldPath);
			lpPath->setData(QVariant::fromValue(1), Qt::UserRole+2);
			lpServer->appendRow(lpPath);
		}

		items.append(new QStandardItem(lpDownloads->fileName()));
		items.append(new QStandardItem(lpDownloads->timestamp().toString("dd.MM.yyyy hh:mm:ss")));
		items.append(new QStandardItem(lpDownloads->reportName()));
		items.append(new QStandardItem(lpDownloads->paths()->localFolder()));
		items.append(new QStandardItem(lpDownloads->localFileName()));
		items.append(new QStandardItem(lpDownloads->downloaded().toString("dd.MM.yyyy hh:mm:ss")));

		items[0]->setData(QVariant::fromValue(lpDownloads), Qt::UserRole+1);
		items[1]->setData(QVariant::fromValue(lpDownloads), Qt::UserRole+1);
		items[2]->setData(QVariant::fromValue(lpDownloads), Qt::UserRole+1);
		items[3]->setData(QVariant::fromValue(lpDownloads), Qt::UserRole+1);
		items[4]->setData(QVariant::fromValue(lpDownloads), Qt::UserRole+1);
		items[5]->setData(QVariant::fromValue(lpDownloads), Qt::UserRole+1);
		items[0]->setData(QVariant::fromValue(2), Qt::UserRole+2);

		lpPath->appendRow(items);
	}

	ui->m_lpDownloadsList->expandAll();

	for(int x = 0;x < m_lpDownloadsListModel->columnCount();x++)
		ui->m_lpDownloadsList->resizeColumnToContents(x);
}

bool cMainWindow::download(cDownloads* lpDownloads)
{
	QDir					dir;
	QNetworkAccessManager	networkManager;
	QString					szRequest	= QString(lpDownloads->paths()->server() + lpDownloads->paths()->path() + "/" + lpDownloads->fileName());

	QUrl					url(szRequest);
	if(!lpDownloads->paths()->userName().isEmpty())
	{
		url.setUserName(lpDownloads->paths()->userName());
		url.setPassword(lpDownloads->paths()->password());
	}

	QNetworkRequest			request;
	request.setUrl(url);

	QNetworkReply*			reply   = networkManager.get(request);
	QEventLoop				loop;

	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	if(reply->error() == QNetworkReply::NoError)
	{
		QString			strReply		= QString(reply->readAll());
		QStringList		replyList		= strReply.split("\n");

		QString			reportName		= replyList[1];
		if(!reportName.indexOf("dashboardreport"))
		{
			delete reply;
			return(false);
		}

		reportName	= reportName.mid(reportName.indexOf("dashboardreport")+22);
		reportName	= reportName.left(reportName.indexOf("\""));
		lpDownloads->setReportName(reportName);

		lpDownloads->setLocalFolder(lpDownloads->paths()->localFolder() + "/" + lpDownloads->timestamp().toString("yyyy/MM"));

		QString	localFileName	= reportName + " - " + lpDownloads->timestamp().toString("yyyy-MM-dd") + ".xml";
		lpDownloads->setLocalFileName(localFileName);

		dir.mkpath(lpDownloads->localFolder());

		QFile	file(lpDownloads->localFolder() + "/" + lpDownloads->localFileName());

		if(file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream stream(&file);
			stream << strReply;
			file.close();

			lpDownloads->setDownloaded(QDateTime::currentDateTime());
			lpDownloads->save();
		}
	}
	else
	{
		qDebug() << reply->errorString();
		delete reply;
	}

	delete reply;

	return(true);
}
