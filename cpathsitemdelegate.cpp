#include "cpathsitemdelegate.h"
#include "cpathselectwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include <QDebug>


QWidget* cPathsItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(index.column() == 4)
	{
		QLineEdit*	lpLineEdit	= new QLineEdit(parent);
		lpLineEdit->setEchoMode(QLineEdit::Password);
		return(lpLineEdit);
	}
	else if(index.column() != 2)
		return(QStyledItemDelegate::createEditor(parent, option, index));

	cPathSelectWidget*	lpWidget	= new cPathSelectWidget(parent);

	return(lpWidget);
}

void cPathsItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if(index.column() == 4)
	{
		QLineEdit*	lpLineEdit	= static_cast<QLineEdit*>(editor);
		if(lpLineEdit)
			lpLineEdit->setText(index.data(Qt::UserRole+2).toString());
		return;
	}
	else if(index.column() != 2)
	{
		QStyledItemDelegate::setEditorData(editor, index);
		return;
	}

	cPathSelectWidget*	lpWidget	= static_cast<cPathSelectWidget*>(editor);

	lpWidget->setPath(index.data(Qt::DisplayRole).toString());
}

void cPathsItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if(index.column() != 2)
	{
		QStyledItemDelegate::setModelData(editor, model, index);
		return;
	}

	cPathSelectWidget*	lpWidget	= static_cast<cPathSelectWidget*>(editor);
	model->setData(index, lpWidget->path(), Qt::DisplayRole);
}

void cPathsItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(index.column() != 2)
	{
		QStyledItemDelegate::updateEditorGeometry(editor, option, index);
		return;
	}

	cPathSelectWidget*	lpWidget	= static_cast<cPathSelectWidget*>(editor);
	lpWidget->setGeometry(option.rect);
}

QSize cPathsItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(index.column() != 2)
		return(QStyledItemDelegate::sizeHint(option, index));

	return(cPathSelectWidget(nullptr).sizeHint());
}
