#ifndef WINDOW_H
#define WINDOW_H

#include "ui_mainwindow.h"

namespace utils {

/*!
 * Logs any errors should Vtfcmd.exe not be present or if it present and any
 * DLLs are missing.
 */
void checkVtfCmd(Ui::MainWindow *ui);

} // namespace utils

#endif // WINDOW_H
