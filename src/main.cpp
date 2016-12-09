#include "mainwindow.h"

#include <QApplication>

/*!
 * Initializes the application and starts the main window with command line
 * arguments if provided.
 *
 * We always silently ignore invalid command line parameters like invalid files
 * or directories.
 */
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// required for the OpenGL widgets on Windows
	a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

	QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

	// this ensures that we always set the working directory to the path
	// of the executable, so we do not mess up with Cache
	if( !QDir::setCurrent( QFileInfo(argv[0]).path() ) )
		Q( "Failed to set the current working directory!" )

	QString fileToOpen;
	if(argc == 2) {
		QFileInfo fileInfo(argv[1]);

		// we silently ignore directories and files which do not exist
		if (!fileInfo.exists() || fileInfo.isDir())
			fileToOpen = "";
		else
			fileToOpen = fileInfo.absoluteFilePath();
	}

	MainWindow w(fileToOpen);
	w.show();

	return a.exec();
}
