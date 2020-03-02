#ifndef CDOWNLOADSFILTERPROXYMODEL_H
#define CDOWNLOADSFILTERPROXYMODEL_H


#include <QDate>
#include <QSortFilterProxyModel>


class cDownloadsFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	cDownloadsFilterProxyModel(QObject* parent = nullptr);

	void					setStartDate(const QDate& date);

protected:
	bool					filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
	QDate					m_startDate;	/*!< TODO: describe */
};

#endif // CDOWNLOADSFILTERPROXYMODEL_H
