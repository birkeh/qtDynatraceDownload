#include "cpaths.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>


extern QSqlDatabase	g_db;


cPaths::cPaths(const int& id, const QString& server, const QString& path, const QString& localFolder, const QString& userName, const QString& password, const bool &active) :
	m_id(id),
	m_server(server),
	m_path(path),
	m_localFolder(localFolder),
	m_userName(userName),
	m_password(password),
	m_active(active),
	m_oldServer(server),
	m_oldPath(path),
	m_oldLocalFolder(localFolder),
	m_oldUserName(userName),
	m_oldPassword(password),
	m_oldActive(active)
{
}

void cPaths::setID(const int& id)
{
	m_id	= id;
}

int cPaths::id()
{
	return(m_id);
}

void cPaths::setServer(const QString& server)
{
	m_oldServer	= m_server;
	m_server	= server;
}

QString cPaths::server()
{
	return(m_server);
}

void cPaths::setPath(const QString& path)
{
	m_oldPath	= m_path;
	m_path		= path;
}

QString cPaths::path()
{
	return(m_path);
}

void cPaths::setLocalFolder(const QString& localFolder)
{
	m_oldLocalFolder	= m_localFolder;
	m_localFolder		= localFolder;
}

QString cPaths::localFolder()
{
	return(m_localFolder);
}

void cPaths::setUserName(const QString& userName)
{
	m_oldUserName	= m_userName;
	m_userName		= userName;
}

QString cPaths::userName()
{
	return(m_userName);
}

void cPaths::setPassword(const QString& password)
{
	m_oldPassword	= m_password;
	m_password		= password;
}

QString cPaths::password()
{
	return(m_password);
}

void cPaths::setActive(const bool& active)
{
	m_oldActive	= m_active;
	m_active	= active;
}

bool cPaths::active()
{
	return(m_active);
}

bool cPaths::save()
{
	QSqlQuery	query(g_db);

	query.prepare("UPDATE paths SET server=:server, path=:path, localFolder=:localFolder, username=:username, password=:password, active=:active WHERE id=:id;");
	query.bindValue(":id",			id());
	query.bindValue(":server",		server());
	query.bindValue(":path",		path());
	query.bindValue(":localFolder",	localFolder());
	query.bindValue(":username",	userName());
	query.bindValue(":password",	password());
	query.bindValue(":active",		active());

	if(!query.exec())
	{
		qDebug() << "onPathsChanged: " << query.lastError().text();

		m_server		= m_oldServer;
		m_path			= m_oldPath;
		m_localFolder	= m_oldLocalFolder;
		m_userName		= m_oldUserName;
		m_password		= m_oldPassword;
		m_active		= m_oldActive;

		return(false);
	}

	m_oldServer			= m_server;
	m_oldPath			= m_path;
	m_oldLocalFolder	= m_localFolder;
	m_oldUserName		= m_userName;
	m_oldPassword		= m_password;
	m_oldActive			= m_active;

	return(true);
}

cPathsList::cPathsList()
{
}

void cPathsList::load()
{
	QSqlQuery	query(g_db);

	query.prepare("SELECT id, server, path, localFolder, username, password, active FROM paths ORDER BY server, path, localFolder;");
	if(!query.exec())
	{
		qDebug() << "load paths: " << query.lastError().text();
		return;
	}

	while(query.next())
		add(query.value("id").toInt(), query.value("server").toString(), query.value("path").toString(), query.value("localFolder").toString(), query.value("userName").toString(), query.value("password").toString(), query.value("active").toBool(), true);
}

cPaths* cPathsList::add()
{
	QSqlQuery				query(g_db);
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
		return(nullptr);
	}
	if(!query.next())
	{
		qDebug() << "new path: " << query.lastError().text();
		return(nullptr);
	}

	cPaths*	lpPaths	= new cPaths(query.value("id").toInt(), QString("server%1").arg(number), "path", "localFolder", "", "", true);
	append(lpPaths);
	return(lpPaths);
}

cPaths* cPathsList::add(const int id, const QString& server, const QString& path, const QString& localFolder, const QString& userName, const QString& password, const bool& active, bool fast)
{
	cPaths*	lpPaths	= nullptr;

	if(!fast)
	{
		lpPaths	= find(server, path, localFolder);
		if(lpPaths)
			return(nullptr);
	}

	lpPaths	= new cPaths(id, server, path, localFolder, userName, password, active);
	append(lpPaths);
	return(lpPaths);
}

cPaths* cPathsList::find(const int& id)
{
	for(int x = 0;x < count();x++)
	{
		cPaths*	paths	= at(x);

		if(paths->id() == id)
			return(paths);
	}
	return(nullptr);
}

cPaths* cPathsList::find(const QString& server, const QString& path, const QString& localFolder)
{
	for(int x = 0;x < count();x++)
	{
		cPaths*	paths	= at(x);

		if(paths->server() == server && paths->path() == path && paths->localFolder() == localFolder)
			return(paths);
	}
	return(nullptr);
}

bool cPathsList::del(const QString& server, const QString& path, const QString& localFolder)
{
	cPaths*	lpPaths	= find(server, path, localFolder);
	if(lpPaths)
		return(del(lpPaths));
	return(false);
}

bool cPathsList::del(cPaths* lpPaths)
{
	QSqlQuery	query(g_db);

	query.prepare("DELETE FROM paths WHERE id=:id;");
	query.bindValue(":id", lpPaths->id());

	if(!query.exec())
	{
		qDebug() << "onPathsChanged: " << query.lastError().text();
		return(false);
	}

	removeOne(lpPaths);

	return(true);
}
