#include "conversionthread.h"

ConversionThread::ConversionThread(MainWindow* mw) :
	QThread(mw),
	mainWindow(mw),
	mLogger(mw->mLogger)
{
	connect(this, SIGNAL(updateTextureOnUi(QString, QString)),
		mw, SLOT(updateTextureOnUi(QString, QString)));
}

void ConversionThread::run()
{
	QProcess process;
	process.start("vtfcmd.exe -file \"" + fileName.replace("/", "\\") + "\" " + outputParameter.replace("/", "\\") + " -resize -version 7.4");

	process.waitForFinished();

	output = process.readAllStandardOutput().simplified();

	if (output.endsWith("1/1 files completed.")) {
		Info("Successfully converted \"" + fileName.replace("\\", "/") + "\"")



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

		if(moveFile) {
			QString toRename = fileName.section("/", -1).section(".", 0, 0);
			toRename += ".vtf";

			if( !QFile::rename(QDir::currentPath() + "\\Cache\\Move\\" + toRename,
							   newFileDir + newFile + ".vtf") ) {

				Error("There was an error moving\"" + toRename + "\"");
			}
		}

		if (!objectName.isEmpty()) {
			emit updateTextureOnUi(objectName, relativeFilePath);
		}


	} else {

		Error("There was an error converting: \"" + fileName.replace("\\", "/") + "\"!")
	}
}
