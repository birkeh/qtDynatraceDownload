#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H


#include "cpathsitemdelegate.h"
#include "cdownloads.h"
#include "cpaths.h"
#include "cdownloadsfilterproxymodel.h"

#include <QMainWindow>
#include <QProgressBar>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QCloseEvent>


QT_BEGIN_NAMESPACE
namespace Ui { class cMainWindow; }
QT_END_NAMESPACE

/**
 * @brief
 *
 */
class cMainWindow : public QMainWindow
{
    Q_OBJECT

public:
	/**
	 * @brief
	 *
	 * @param parent
	 */
	cMainWindow(QWidget *parent = nullptr);
	/**
	 * @brief
	 *
	 */
	~cMainWindow();

private:
	Ui::cMainWindow*                ui;								/**< TODO: describe */

	QProgressBar*					m_lpProgressBar;				/**< TODO: describe */
	QStandardItemModel*				m_lpDownloadsListModel;			/**< TODO: describe */
	cDownloadsFilterProxyModel*		m_lpDownloadsFilterProxyModel;	/*!< TODO: describe */
	QStandardItemModel*				m_lpPathsListModel;				/**< TODO: describe */

	cDownloadsList					m_downloadsList;				/**< TODO: describe */
	cPathsList						m_pathsList;					/**< TODO: describe */

	void							fillDownloadsList();
	QString							download(cDownloads* lpDownloads);

protected:
	/**
	 * @brief
	 *
	 * @param event
	 */
	void							closeEvent(QCloseEvent* event);

private slots:
	void							onUpdate();
	void							onDownload();

	/**
	 * @brief
	 *
	 * @param pos
	 */
	void							onPathsContextMenu(const QPoint &pos);
	/**
	 * @brief
	 *
	 * @param item
	 */
	void							onPathsChanged(QStandardItem* item);
	/**
	 * @brief
	 *
	 */
	void							onPathsAdd();
	/**
	 * @brief
	 *
	 */
	void							onPathsDelete();

	void							onStartDateChanged(const QDate& date);
};
#endif // CMAINWINDOW_H
