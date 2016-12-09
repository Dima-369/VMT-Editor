#include "dialogwithouthelpbutton.h"

#include "utilities.h"

#ifdef Q_OS_WIN
#   include <Windows.h>
#endif

DialogWithoutHelpButton::DialogWithoutHelpButton( QWidget* parent ) :
	QDialog(parent)
{
	setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

#ifdef Q_OS_WIN
	// Setting a higher quality window title icon
	//SendMessage( (HWND)winId(), WM_SETICON, ICON_BIG,   (LPARAM)qt_pixmapToWinHICON(QPixmap(":/icons/vmt_256")));
	//SendMessage( (HWND)winId(), WM_SETICON, ICON_SMALL, (LPARAM)qt_pixmapToWinHICON(QPixmap(":/icons/vmt_16")));
#endif
}
