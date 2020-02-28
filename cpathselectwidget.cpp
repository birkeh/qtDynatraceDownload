#include "cpathselectwidget.h"

#include <QFileDialog>


cPathSelectWidget::cPathSelectWidget(QWidget *parent) :
	QWidget(parent)
{
	m_lpLineEdit				= new QLineEdit;
	m_lpPushButton				= new QPushButton;
	m_lpLayout					= new QHBoxLayout(this);

	m_lpLayout->setContentsMargins(0, 0, 0, 0);

	m_lpLineEdit->setMinimumWidth(1);
	m_lpPushButton->setText("...");

	m_lpLayout->addWidget(m_lpLineEdit, 2);
	m_lpLayout->addWidget(m_lpPushButton, 0);

	connect(m_lpPushButton,	&QPushButton::clicked,	this,	&cPathSelectWidget::onBrowse);
}

void cPathSelectWidget::setPath(const QString& path)
{
	m_lpLineEdit->setText(path);
}

QString cPathSelectWidget::path()
{
	return(m_lpLineEdit->text());
}

void cPathSelectWidget::onBrowse()
{
	QString	dirName	= QFileDialog::getExistingDirectory(this, tr("Open Directory"), m_lpLineEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(dirName.isEmpty())
		return;
	m_lpLineEdit->setText(dirName);
}
