#ifndef PHONG_H
#define PHONG_H

#include "ui_mainwindow.h"

#include "view-helper.h"
#include "vmt/vmt-helper.h"

/*!
 * Functions for interacting with the 2 Specular named groupbox.
 *
 * Both are basically for Phong shaders but we use different groupboxes
 * depending on shader.
 */
namespace phong {

/*!
 * Initializes the group box views.
 *
 * Call from the MainWindow constructor.
 */
void initialize(Ui::MainWindow *ui);

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
 * You should only pass MainWindow::Phong or MainWindow::PhongBrush to the
 * groupBox parameter.
 *
 * Call from MainWindow::isGroupChanged().
 */
bool hasChanged(MainWindow::GroupBoxes groupBox, Ui::MainWindow *ui);

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
 * Parses $phong to initializes the state of the VMT.
 *
 * The user interface is set up and errors are returned should any
 * parameters have an invalid format.
 *
 * We only process the parameter if it exists.
 */
void parseParameters(Ui::MainWindow *ui, VmtFile *vmt);

} // namespace phong

#endif // PHONG_H
