#include "cpathsitemdelegate.h"
#include "cpathselectwidget.h"
#include "cpaths.h"

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
	cPaths*	lpPaths	= index.data(Qt::UserRole+1).value<cPaths*>();

	switch(index.column())
	{
	case 0:
	case 1:
	case 3:
		QStyledItemDelegate::setEditorData(editor, index);
		return;
	case 2:
	{
		cPathSelectWidget*	lpWidget	= static_cast<cPathSelectWidget*>(editor);
		lpWidget->setPath(index.data(Qt::DisplayRole).toString());
		return;
	}
	case 4:
	{
		QLineEdit*	lpLineEdit	= static_cast<QLineEdit*>(editor);
		lpLineEdit->setText(lpPaths->password());
		return;
	}
	default:
		return;
	}
}

void cPathsItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	switch(index.column())
	{
	case 0:
	case 1:
	case 3:
	case 4:
		QStyledItemDelegate::setModelData(editor, model, index);
		return;
	case 2:
	{
		cPathSelectWidget*	lpWidget	= static_cast<cPathSelectWidget*>(editor);
		cPaths*				lpPaths		= index.data(Qt::UserRole+1).value<cPaths*>();
		lpPaths->setLocalFolder(lpWidget->path());
		return;
	}
	default:
		return;
	}
}

void cPathsItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch(index.column())
	{
	case 0:
	case 1:
	case 3:
	case 4:
		QStyledItemDelegate::updateEditorGeometry(editor, option, index);
		return;
	case 2:
	{
		cPathSelectWidget*	lpWidget	= static_cast<cPathSelectWidget*>(editor);
		lpWidget->setGeometry(option.rect);
		return;
	}
	default:
		return;
	}
}

QSize cPathsItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch(index.column())
	{
	case 0:
	case 1:
	case 3:
	case 4:
		return(QStyledItemDelegate::sizeHint(option, index));
	case 2:
	{
		return(cPathSelectWidget(nullptr).sizeHint());
	}
	default:
		return(QStyledItemDelegate::sizeHint(option, index));
	}
}
