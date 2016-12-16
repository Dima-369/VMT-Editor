#include "conversionthread.h"
#include "mainwindow.h"
#include "utilities.h"

#include <QProcess>
#include <QDir>

ConversionThread::ConversionThread( QObject* parent, QListWidget* logger ) :
	QThread(parent),
	mLogger(logger)
{

}

void ConversionThread::run() {

	QProcess process;
	process.start("vtfcmd.exe -file \"" + fileName.replace("/", "\\") + "\" " + outputParameter.replace("/", "\\") + " -resize -msharpen SHARPENSOFT -version 7.4");

	process.waitForFinished();

	output = process.readAllStandardOutput().simplified();

	if( output.endsWith("1/1 files completed.") ) {

		Info("Successfully converted \"" + fileName.replace("\\", "/") + "\"")

		if (objectName != "") {
			MainWindow::previewTexture(objectName);
		}

		if (newFileName != "") {
			QDir moveDir( QDir::currentPath() + "/Cache/Move/" );
			moveDir.remove(newFileName);

			QString toRename = fileName.right( fileName.length() - fileName.lastIndexOf('/') - 1);
			toRename.chop( fileName.length() - fileName.lastIndexOf('.') );
			toRename += ".vtf";

			if( !QFile::rename(QDir::currentPath() + "\\Cache\\Move\\" + toRename,
							   QDir::currentPath() + "\\Cache\\Move\\" + newFileName) ) {


				Error("There was an error renaming \"" + toRename + "\" to \"" + newFileName + "\"")
			}
		}


	} else {

		Error("There was an error converting: \"" + fileName.replace("\\", "/") + "\"!")
	}
}
