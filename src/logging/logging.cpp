#include "logging.h"

#include "user-interface/constants.h"

#include <QIcon>
#include <QListWidgetItem>
#include <qdebug.h>

void logging::error(const QString &s, Ui::MainWindow *ui)
{
	qDebug() << QString(s);
	ui->listWidget->addItem(new QListWidgetItem(QIcon(":/icons/error"), s));
	ui->listWidget->scrollToBottom();
}

void logging::info(const QString &s, Ui::MainWindow *ui)
{
	qDebug() << QString(s);
	ui->listWidget->addItem(new QListWidgetItem(QIcon(":/icons/info"), s));
	ui->listWidget->scrollToBottom();
}

void logging::infoReconvert(const QString &s, Ui::MainWindow *ui)
{
	qDebug() << QString(s);
	ui->listWidget->addItem(new QListWidgetItem(QIcon(":/icons/reconvert"), s));
	ui->listWidget->scrollToBottom();
}

