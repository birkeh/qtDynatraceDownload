#ifndef CDOWNLOADS_H
#define CDOWNLOADS_H


#include "cpaths.h"

#include <QList>
#include <QDateTime>
#include <QMetaType>


class cDownloads
{
public:
	cDownloads(cPaths* lpPaths, const QString& html);
	cDownloads(cPaths* lpPaths, const int id, const QString& fileName, const QDateTime timestamp, const QString& reportName, const QString& localFileName, const QDateTime& downloaded);

	void			setValues(const QString& link);
	bool			save();

	cPaths*			paths();
	QString			fileName();
	QDateTime		timestamp();
	QString			reportName();
	QString			localFileName();
	QDateTime		downloaded();

private:
	cPaths*			m_lpPaths;
	int				m_id;
	QString			m_fileName;
	QDateTime		m_timestamp;
	QString			m_reportName;
	QString			m_localFileName;
	QDateTime		m_downloaded;
};

Q_DECLARE_METATYPE(cDownloads*)

class cDownloadsList : public QList<cDownloads*>
{
public:
	explicit cDownloadsList(cPathsList* lpPathsList);

	void		load();
	void		load(cPaths* lpPaths);

	cDownloads*	add(int pathsID, const QString& html);
	cDownloads*	add(cPaths* lpPaths, const QString& html);
	cDownloads*	add(int pathsID, const int id, const QString& fileName, const QDateTime timestamp, const QString& reportName, const QString& localFileName, const QDateTime& downloaded, bool fast = true);
	cDownloads*	add(cPaths* lpPaths, const int id, const QString& fileName, const QDateTime timestamp, const QString& reportName, const QString& localFileName, const QDateTime& downloaded, bool fast = true);
	cDownloads*	find(cPaths* lpPaths, const QString& fileName);

private:
	cPathsList*	m_lpPathsList;
};

#endif // CDOWNLOADS_H
