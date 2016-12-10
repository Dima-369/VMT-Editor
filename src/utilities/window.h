#pragma once

#include "ui_mainwindow.h"

namespace utils {

/*!
 * Logs any errors should Vtfcmd.exe not be present or if present and any
 * DLLs are missing.
 */
void checkVtfCmd(Ui::MainWindow *ui);

} // namespace utils
