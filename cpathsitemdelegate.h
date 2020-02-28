#ifndef CPATHSITEMDELEGATE_H
#define CPATHSITEMDELEGATE_H


#include <QStyledItemDelegate>
#include <QStandardItem>


class cPathsItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	virtual QWidget*	createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual QSize		sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
	virtual void		setEditorData(QWidget *editor, const QModelIndex &index) const;
	virtual void		setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	virtual void		updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CPATHSITEMDELEGATE_H
