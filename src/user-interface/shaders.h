#ifndef SHADERS_H
#define SHADERS_H

#include "ui_mainwindow.h"

namespace shaders {

	/*!
	 * Enables/hides the related phong/phongbrush/rimlight action and
	 * groupboxes depending on the shader.
	 *
	 * The passed shader string has to be lowercase!
	 *
	 * Call from MainWindow::shaderChanged().
	 */
	void handlePhongBrushRim(Ui::MainWindow *ui, const QString &shader);
}

#endif // SHADERS_H
