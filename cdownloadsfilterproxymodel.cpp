#include "cdownloadsfilterproxymodel.h"
#include "cdownloads.h"


cDownloadsFilterProxyModel::cDownloadsFilterProxyModel(QObject* parent) :
	QSortFilterProxyModel(parent)
{
}

void cDownloadsFilterProxyModel::setStartDate(const QDate& date)
{
	m_startDate	= date;
	invalidateFilter();
}

bool cDownloadsFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex	index0		= sourceModel()->index(sourceRow, 0, sourceParent);
	int			level		= sourceModel()->data(index0, Qt::UserRole+2).value<int>();
	if(level < 2)
		return(true);

	cDownloads*	lpDownloads	= sourceModel()->data(index0, Qt::UserRole+1).value<cDownloads*>();

	if(!lpDownloads)
		return(false);

	if(lpDownloads->timestamp().date() >= m_startDate)
		return(true);
	return(false);
}
