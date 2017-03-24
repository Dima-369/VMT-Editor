#ifndef DETAILTEXTURE_H
#define DETAILTEXTURE_H

#include "ui_mainwindow.h"

/*!
 * Functions for interacting with the "Detail Texture" groupbox.
 */
namespace detailtexture {

enum Parameter {
	detailscale // $detailscale
};

/*!
 * Resets the widgets.
 *
 * Call from MainWindow::resetWidgets().
 */
void reset(Ui::MainWindow *ui);

/*!
 * Detects if the widgets are not in their default state and returns true if
 * any field differs from the default.
 *
 * Call from MainWindow::isGroupChanged().
 */
bool hasChanged(Ui::MainWindow *ui);

/*!
 * Enables or disables the widgets depending if the detail line edit
 * has text.
 *
 * Triggered when lineEdit_detail fires a changed signal. The main window
 * calls this method in receiving slot.
 */
void processDetailTextureChange(const QString &text, Ui::MainWindow *ui, const bool second = false);

/*!
 * Enables the detail scale Y value if the checkbox is unchecked.
 *
 * Call this when the uniform scale checkbox state was changed.
 */
void toggledUniformScale(bool checked, Ui::MainWindow *ui, const bool second = false);

} // namespace detailtexture

namespace detailtexture {
namespace param {

/*!
 * Parses $detail to set up the state of the VMT and returns the parsed texture.
 *
 * Also checks if $seamless_scale is present, so make sure that this method
 * is called before that parameter is parsed!
 *
 * The returned texture can be empty if the parameter is not present!
 */
QString initialize(Ui::MainWindow *ui, VmtFile *vmt);

/*!
 * Processes the passed parameter from the VmtFile and initializes the
 * user interface with it.
 *
 * The user interface is initialized and errors are returned should the
 * parameter have an invalid format.
 *
 * We only process the parameter if it exists.
 */
void parse(const detailtexture::Parameter &parameter, Ui::MainWindow *ui,
		VmtFile *vmt);

} // namespace detailtexture::param
} // namespace detailtexture

#endif // DETAILTEXTURE_H
