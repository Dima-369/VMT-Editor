#ifndef NORMALBLEND_H
#define NORMALBLEND_H

#include "ui_mainwindow.h"

#include "view-helper.h"
#include "vmt/vmt-helper.h"

/*!
 * Functions for interacting with the Normal blend named groupbox.
 */
namespace normalblend {

/*!
 * Returns the texture preview assuming that the group box will be toggled
 * after this call.
 *
 * So if the group box is currently visible, we return an empty texture
 * preview and otherwise check the diffuse or bumpmap line edits for the
 * texture.
 */
utils::Preview toggle(Ui::MainWindow *ui);

/*!
 * Detects if the views are not in their default state and returns true if
 * any field differs from the default.
 *
 * Call from MainWindow::isGroupChanged().
 */
bool hasChanged(Ui::MainWindow *ui);

/*!
 * Hides the groupbox and unchecks the action.
 *
 * Call from MainWindow::shaderChanged() for hiding group boxes whose
 * content did not change after a shader change.
 */
void resetAction(Ui::MainWindow *ui);

/*!
 * Resets the widgets to their default state.
 *
 * Call from MainWindow::resetWidgets().
 */
void resetWidgets(Ui::MainWindow *ui);

/*!
 * Processes the passed parameter from the VmtFile and initializes the
 * user interface with it.
 *
 * Checks vmt->state.normalBlendEnabled.
 *
 * The user interface is set up and errors are returned should any
 * parameters have an invalid format.
 *
 * We only process the parameter if it exists.
 */
void parseParameters(Ui::MainWindow *ui, VmtFile *vmt);

} // namespace normalblend

#endif // NORMALBLEND_H
