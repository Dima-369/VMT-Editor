#include "conversionthread.h"

ConversionThread::ConversionThread(MainWindow* mw) :
	QThread(mw),
	mainWindow(mw),
	mLogger(mw->mLogger)
{

}

void ConversionThread::run()
{
	QProcess process;
	process.start("vtfcmd.exe -file \"" + fileName.replace("/", "\\") + "\" " + outputParameter.replace("/", "\\") + " -resize -msharpen SHARPENSOFT -version 7.4");

	process.waitForFinished();

	output = process.readAllStandardOutput().simplified();

	if (output.endsWith("1/1 files completed.")) {
		Info("Successfully converted \"" + fileName.replace("\\", "/") + "\"")

		if (!objectName.isEmpty()) {
			mainWindow->previewTexture( objectName, relativeFilePath, true, false, false, false, true );
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
