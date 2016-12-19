#pragma once

#include <QThread>
#include <QListWidget>
#include <QProcess>
#include <QDir>

#include "mainwindow.h"
#include "utilities.h"

class ConversionThread : public QThread
{
	Q_OBJECT

public:

	ConversionThread(MainWindow* mw);

	virtual void run();

	QString fileName;
	QString outputParameter;
	QString relativeFilePath;
	QString newFileName;
	QString objectName;
	bool removeSuffix = false;

private:

	QString output;
	MainWindow* mainWindow;

	// required for the ERROR, INFO macros
	QListWidget* mLogger;

signals:

	void updateTextureOnUi(
		const QString& objectName, const QString& relativeFilePath);
};
