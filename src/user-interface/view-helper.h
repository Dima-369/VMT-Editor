#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include "ui_mainwindow.h"

#include "faderwidget.h"
#include "glwidget_spec.h"

#include "vmt/vmt-helper.h"
#include "utilities/strings.h"

namespace utils {

/*!
 * Wrapper for the parameters of MainWindow::previewTexture().
 */
struct Preview {
	GLWidget_Spec::Mode mode;
	QString texture;
};

/*!
 * Shows or hides the passed group box and adjust the action checked
 * state so it is in sync.
 *
 * The main window parameter is required as we are internally creating a
 * FadeWidget which might have not been initialized yet, and it requires
 * a parent on creation.
 *
 * If parsingVmt is false, we fade the group box into view. Otherwise we
 * instantly display it as one would expect when opening a new VMT.
 *
 * If the action is disabled, we do nothing.
 */
void toggle(MainWindow *mainWindow, QAction *action, FadeGroupBox *groupBox,
	bool parsingVmt = false);

/*!
 * Shows or hides the passed group box based on the checked parameter.
 *
 * If it is true, we show the group box and hide otherwise!
 *
 * The main window parameter is required as we are internally creating a
 * FadeWidget which might have not been initialized yet, and it requires
 * a parent on creation.
 *
 * If parsingVmt is false, we fade the group box into view. Otherwise we
 * instantly display it as one would expect when opening a new VMT.
 *
 * Call in the MainWindow slots where the menu group box actions are
 * handled.
 */
void toggle(MainWindow *mainWindow, bool checked, FadeGroupBox *groupBox,
	bool parsingVmt);

/*!
 * Initializes the background color of the passed color widget if the value
 * could be parsed correctly.
 *
 * We also update the corresponding tint slider.
 *
 * Make sure that no passed strings are empty!
 */
void applyBackgroundColor(const QString &parameter, const QString &value,
	QPlainTextEdit *colorWidget, TintSlider *sliderWidget,
	Ui::MainWindow *ui);

void applyBackgroundColor(const QString &parameter, const QString &value,
	QPlainTextEdit *colorWidget, Ui::MainWindow *ui, bool toSrgb = false);

/*!
 * Applies a background-color stylesheet to the passed color widget.
 *
 * The color values should be between 0 and 255.
 */
void applyBackgroundColor(int red, int green, int blue,
	QPlainTextEdit *colorWidget);

/*!
 * Parses the stylesheet of the passed widget to cast the background-color
 * key to a QColor.
 *
 * Make sure that only background-color is defined as the only CSS property
 * of the passed widget.
 */
QColor getBG(QPlainTextEdit *widget);

/*!
 * Returns the spin box value if the view is enabled and different to the
 * default value.
 *
 * Make sure to pass "4" for the default value because trailing zeroes
 * are stripped.
 */
QString getNonDef(QDoubleSpinBox *sb, const QString &def);

/*!
 * Strips all trailing zeroes and then compares the resulting string against
 * it.
 *
 * Make sure that you do not include any trailing zeroes in the passed
 * comparion string!
 *
 * If the spin box contains "4.00", pass "4" for the value parameter.
 */
inline bool equal(QDoubleSpinBox *spinBox, const QString &value)
{
    return stripZeroes(spinBox) == value;
}

inline bool isChecked(QCheckBox *cb)
{
	return cb->isEnabled() && cb->isChecked();
}

/*!
 * Returns an empty string if the line edit is not enabled.
 */
inline QString getText(QLineEdit *le)
{
	if (le->isEnabled())
		return le->text().trimmed();
	return "";
}

} // namespace utils

#endif // VIEWHELPER_H
