#include "cdownloads.h"


#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

#include <QDebug>


extern QSqlDatabase	g_db;


cDownloads::cDownloads(cPaths* lpPaths, const QString& html) :
	m_lpPaths(lpPaths),
	m_fileName(""),
	m_timestamp(QDateTime()),
	m_reportName(""),
	m_localFileName(""),
	m_downloaded(QDateTime())
{
	QString	tmp	= html.mid(html.indexOf("<a href")+9);
	tmp			= tmp.left(tmp.indexOf("\">"));
	m_fileName	= tmp;
	setValues(tmp);
}

cDownloads::cDownloads(cPaths* lpPaths, const int id, const QString& fileName, const QDateTime timestamp, const QString& reportName, const QString& localFolder, const QString &localFileName, const QDateTime& downloaded) :
	m_lpPaths(lpPaths),
	m_id(id),
	m_fileName(fileName),
	m_timestamp(timestamp),
	m_reportName(reportName),
	m_localFolder(localFolder),
	m_localFileName(localFileName),
	m_downloaded(downloaded)
{
	setValues(fileName);
}

void cDownloads::setValues(const QString& link)
{
	QString	tmp;

	if(link.contains("report_"))
	{
		tmp	= link.mid(link.indexOf("report_")+7);

		int	index	= tmp.indexOf("_");
		index		= tmp.indexOf("_", index+1);
		tmp			= tmp.left(index);

		QDateTime	dateTime	= QDateTime::fromString(tmp, "yyyy-MM-dd_hh-mm-ss");
		if(dateTime.isValid())
			m_timestamp	= dateTime;
	}
}

bool cDownloads::save()
{
	QSqlQuery	query(g_db);

	if(m_id == -1)
	{
		query.prepare("INSERT INTO downloads (pathsID, fileName, timestamp, reportName, localFolder, localFileName, downloaded) VALUES (:pathsID, :fileName, :timestamp, :reportName, :localFolder, :localFileName, :downloaded);");
		query.bindValue(":pathsID", m_lpPaths->id());
		query.bindValue(":fileName", m_fileName);
		query.bindValue(":timestamp", m_timestamp);
		query.bindValue(":reportName", m_reportName);
		query.bindValue(":localFolder", m_localFolder);
		query.bindValue(":localFileName", m_localFileName);
		query.bindValue(":downloaded", m_downloaded);

		if(!query.exec())
		{
			qDebug() << "save downloads: " << query.lastError().text();
			return(false);
		}

		query.prepare("SELECT id FROM downloads WHERE pathsID=:pathsID AND fileName=:fileName;");
		query.bindValue(":pathsID", m_lpPaths->id());
		query.bindValue(":fileName", m_fileName);

		if(!query.exec())
		{
			qDebug() << "save downloads: " << query.lastError().text();
			return(false);
		}

		if(!query.next())
		{
			qDebug() << "save downloads: " << query.lastError().text();
			return(false);
		}
		m_id	= query.value("id").toInt();
	}
	else
	{
		query.prepare("UPDATE downloads SET pathsID=:pathsID, fileName=:fileName, timestamp=:timestamp, reportName=:reportName, localFolder=:localFolder, localFileName=:localFileName, downloaded=:downloaded WHERE id=:id;");
		query.bindValue(":pathsID", m_lpPaths->id());
		query.bindValue(":fileName", m_fileName);
		query.bindValue(":timestamp", m_timestamp);
		query.bindValue(":reportName", m_reportName);
		query.bindValue(":localFolder", m_localFolder);
		query.bindValue(":localFileName", m_localFileName);
		query.bindValue(":downloaded", m_downloaded);
		query.bindValue(":id", m_id);

		if(!query.exec())
		{
			qDebug() << "save downloads: " << query.lastError().text();
			return(false);
		}
	}

	return(true);
}

void cDownloads::setPaths(cPaths* lpPaths)
{
	m_lpPaths	= lpPaths;
}

cPaths* cDownloads::paths()
{
	return(m_lpPaths);
}

void cDownloads::setFileName(const QString& fileName)
{
	m_fileName	= fileName;
}

QString cDownloads::fileName()
{
	return(m_fileName);
}

void cDownloads::setTimestamp(const QDateTime& timestamp)
{
	m_timestamp	= timestamp;
}

QDateTime cDownloads::timestamp()
{
	return(m_timestamp);
}

void cDownloads::setReportName(const QString& reportName)
{
	m_reportName	= reportName;
}

QString cDownloads::reportName()
{
	return(m_reportName);
}

void cDownloads::setLocalFolder(const QString& localFolder)
{
	m_localFolder	= localFolder;
}

QString cDownloads::localFolder()
{
	return(m_localFolder);
}

void cDownloads::setLocalFileName(const QString& localFileName)
{
	m_localFileName	= localFileName;
}

QString cDownloads::localFileName()
{
	return(m_localFileName);
}

void cDownloads::setDownloaded(const QDateTime& downloaded)
{
	m_downloaded	= downloaded;
}

QDateTime cDownloads::downloaded()
{
	return(m_downloaded);
}

bool downloadsSort(cDownloads* downloads1, cDownloads* downloads2)
{
	if(downloads1->paths()->server() < downloads2->paths()->server())
		return(true);
	else if(downloads1->paths()->server() > downloads2->paths()->server())
		return(false);

	if(downloads1->paths()->path() < downloads2->paths()->path())
		return(true);
	else if(downloads1->paths()->path() > downloads2->paths()->path())
		return(false);

	if(downloads1->fileName() < downloads2->fileName())
		return(true);
	else if(downloads1->fileName() > downloads2->fileName())
		return(false);

	return(false);
}

cDownloadsList::cDownloadsList(cPathsList *lpPathsList) :
	m_lpPathsList(lpPathsList)
{
}

void cDownloadsList::load()
{
	QSqlQuery	query(g_db);

	query.prepare("SELECT downloads.id, downloads.pathsID, downloads.fileName, downloads.timestamp, downloads.reportName, downloads.localFolder, downloads.localFileName, downloads.downloaded FROM downloads, paths WHERE downloads.pathsID=paths.id ORDER BY paths.server, paths.path, downloads.fileName;");

	if(!query.exec())
	{
		qDebug() << "load downloads: " << query.lastError().text();
		return;
	}

	while(query.next())
		add(query.value("downloads.pathsID").toInt(), query.value("downloads.id").toInt(), query.value("downloads.fileName").toString(), query.value("downloads.timestamp").toDateTime(),
			query.value("downloads.reportName").toString(), query.value("downloads.localFolder").toString(), query.value("downloads.localFileName").toString(), query.value("downloads.downloaded").toDateTime());
}

void cDownloadsList::load(cPaths* lpPaths)
{
	QNetworkAccessManager	networkManager;
	QString					szRequest	= QString(lpPaths->server() + lpPaths->path());

	QUrl					url(szRequest);
	if(!lpPaths->userName().isEmpty())
	{
		url.setUserName(lpPaths->userName());
		url.setPassword(lpPaths->password());
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

		for(int x = 0;x < replyList.count();x++)
		{
			if(replyList[x].left(4) == "<li>")
			{
				cDownloads*	lpDownloads	= add(lpPaths, replyList[x]);
				if(lpDownloads)
					lpDownloads->save();
			}
		}
	}
	else
	{
		qDebug() << reply->errorString();
		delete reply;
	}

	delete reply;
}

cDownloads* cDownloadsList::add(int pathsID, const QString& html)
{
	return(add(m_lpPathsList->find(pathsID), html));
}

cDownloads* cDownloadsList::add(cPaths* lpPaths, const QString& html)
{
	QString	tmp	= html.mid(html.indexOf("<a href")+9);
	tmp			= tmp.left(tmp.indexOf("\">"));
	return(add(lpPaths, -1, tmp, QDateTime(), "", "", "", QDateTime(), false));
}

cDownloads* cDownloadsList::add(int pathsID, const int id, const QString& fileName, const QDateTime timestamp, const QString& reportName, const QString& localFolder, const QString& localFileName, const QDateTime& downloaded, bool fast)
{
	return(add(m_lpPathsList->find(pathsID), id, fileName, timestamp, reportName, localFolder, localFileName, downloaded, fast));
}

cDownloads* cDownloadsList::add(cPaths* lpPaths, const int id, const QString& fileName, const QDateTime timestamp, const QString& reportName, const QString& localFolder, const QString& localFileName, const QDateTime& downloaded, bool fast)
{
	cDownloads*	lpDownloads	= nullptr;

	if(!fast)
	{
		lpDownloads	= find(lpPaths, fileName);
		if(lpDownloads)
			return(nullptr);
	}

	lpDownloads	= new cDownloads(lpPaths, id, fileName, timestamp, reportName, localFolder, localFileName, downloaded);
	append(lpDownloads);
	return(lpDownloads);
}

cDownloads* cDownloadsList::find(cPaths* lpPaths, const QString& fileName)
{
	for(int x = 0;x < count();x++)
	{
		cDownloads*	lpDownloads	= at(x);
		if(lpDownloads->paths() == lpPaths && lpDownloads->fileName() == fileName)
			return(at(x));
	}
	return(nullptr);
}

void cDownloadsList::sort()
{
	std::sort(begin(), end(), downloadsSort);
}
