#ifndef LOGGING_H
#define LOGGING_H

#include "ui_mainwindow.h"

/*!
 * As long as the UI parameter is always called ui, we can safely use this
 * macro to save some keystrokes!
 */
#define ERROR(s_) logging::error(s_, ui);

namespace logging {

/*!
 * Logs the passed string to the console and appends it to the error log
 * console in the user interface.
 */
void error(const QString &s, Ui::MainWindow *ui);

} // namespace logging

#endif // LOGGING_H
