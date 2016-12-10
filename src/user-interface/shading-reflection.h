#pragma once

#include "ui_mainwindow.h"
#include "vmtparser.h"

#include "view-helper.h"

namespace shadingreflection {

/*!
 * Call from MainWindow::makeVMT().
 */
void insertParametersFromViews(VmtFile *vmt, Ui::MainWindow *ui);

} // namespace shadingreflection
