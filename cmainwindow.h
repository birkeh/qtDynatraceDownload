#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H


#include <QMainWindow>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QCloseEvent>


QT_BEGIN_NAMESPACE
namespace Ui { class cMainWindow; }
QT_END_NAMESPACE

class cMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    cMainWindow(QWidget *parent = nullptr);
    ~cMainWindow();

private:
    Ui::cMainWindow*                ui;
	QSqlDatabase                    m_db;

	QStandardItemModel*				m_lpDownloadsListModel;
	QStandardItemModel*				m_lpPathsListModel;

protected:
    void							closeEvent(QCloseEvent* event);

private slots:
	void							onPathsContextMenu(const QPoint &pos);
	void							onPathsChanged(QStandardItem* item);
	void							onPathsAdd();
	void							onPathsDelete();
};
#endif // CMAINWINDOW_H
