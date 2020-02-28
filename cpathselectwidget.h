#ifndef CPATHSELECTWIDGET_H
#define CPATHSELECTWIDGET_H


#include <QWidget>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QModelIndex>


class cPathSelectWidget : public QWidget
{
	Q_OBJECT

public:
	explicit cPathSelectWidget(QWidget *parent = nullptr);

	void			setPath(const QString& path);
	QString			path();

signals:

private slots:
	void			onBrowse();

private:
	QLineEdit*		m_lpLineEdit;
	QPushButton*	m_lpPushButton;
	QHBoxLayout*	m_lpLayout;
};

#endif // CPATHSELECTWIDGET_H
