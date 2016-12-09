#include "window.h"

#include <QDir>

#include "logging/logging.h"

void utils::checkVtfCmd(Ui::MainWindow *ui)
{
	QDir dir = QDir();
	if(dir.exists("vtfcmd.exe")) {
		if (!dir.exists("devil.dll")) {
			ERROR("DevIL.dll is missing for texture preview")
		}
		if (!dir.exists("vtflib.dll")) {
			ERROR("VTFLib.dll is missing for texture preview")
		}
	} else {
		ERROR("Missing VTFCmd.exe for texture preview")
	}
}
