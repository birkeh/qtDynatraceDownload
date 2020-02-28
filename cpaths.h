#ifndef CPATHS_H
#define CPATHS_H


#include <QList>
#include <QMetaType>


class cPaths
{
public:
	cPaths(const int& id, const QString& server, const QString& path, const QString& localFolder, const QString& userName, const QString& password, const bool& active);

	void		setID(const int& id);
	int id();

	void		setServer(const QString& server);
	QString		server();

	void		setPath(const QString& path);
	QString		path();

	void		setLocalFolder(const QString& localFolder);
	QString		localFolder();

	void		setUserName(const QString& userName);
	QString		userName();

	void		setPassword(const QString& password);
	QString		password();

	void		setActive(const bool& active);
	bool		active();

	bool		save();

private:
	int			m_id;
	QString		m_server;
	QString		m_path;
	QString		m_localFolder;
	QString		m_userName;
	QString		m_password;
	bool		m_active;

	QString		m_oldServer;
	QString		m_oldPath;
	QString		m_oldLocalFolder;
	QString		m_oldUserName;
	QString		m_oldPassword;
	bool		m_oldActive;
};

Q_DECLARE_METATYPE(cPaths*)

class cPathsList : public QList<cPaths*>
{
public:
	cPathsList();

	void		load();

	cPaths*		add();
	cPaths*		add(const int id, const QString& server, const QString& path, const QString& localFolder, const QString& userName, const QString& password, const bool& active, bool fast = true);
	cPaths*		find(const int& id);
	cPaths*		find(const QString& server, const QString& path, const QString& localFolder);
	bool		del(const QString& server, const QString& path, const QString& localFolder);
	bool		del(cPaths* lpPaths);
};

#endif // CPATHS_H
