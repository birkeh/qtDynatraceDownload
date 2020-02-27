#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H


#include <QMainWindow>
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
	Ui::cMainWindow*                ui;							/**< TODO: describe */
	QSqlDatabase                    m_db;						/**< TODO: describe */

	QStandardItemModel*				m_lpDownloadsListModel;		/**< TODO: describe */
	QStandardItemModel*				m_lpPathsListModel;			/**< TODO: describe */

protected:
	/**
	 * @brief
	 *
	 * @param event
	 */
	void							closeEvent(QCloseEvent* event);

private slots:
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
};
#endif // CMAINWINDOW_H
