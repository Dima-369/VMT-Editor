#include "texturethread.h"

#include <QFile>
#include <QProcess>
#include <QDir>

#include "utilities.h"

void TextureThread::run()
{
    QDir().mkdir("Cache");

	QProcess process;
	process.start(input + " -version 7.4");

	process.waitForFinished();

	QFile cacheFile( "Cache/" + vtfFile );
	cacheFile.rename( "Cache/" + output + ".png" );
}

