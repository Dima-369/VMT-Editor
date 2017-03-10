#pragma once

#include "ui_mainwindow.h"

#include "vmtparser.h"

#include <QString>
#include <QVector>
#include <QDoubleSpinBox>
#include <QtMath>

namespace utils {

/*!
 * Result of parsing a boolean parameter from parseBoolean().
 *
 * It is recommended to only check if value is true because all values are
 * initiatialized to false by default.
 *
 * We assume that the value is false if the value could not be parsed because
 * it was not 1 or 0.
 */
struct BooleanResult
{
	bool value;
	bool present;

	BooleanResult() :
		value(false),
		present(false) {}
};

/*!
 * Result of parsing an integer parameter from parseInt().
 */
struct IntResult
{
	int value;
	bool notDefault;
	bool invalid;

	IntResult() :
		value(0),
		notDefault(false),
		invalid(false) {}
};

/*!
 * Result of parsing color parameter parameter from parseInt().
 *
 * You can check if notDefault is true to quickly skip the validity check.
 *
 * intValues contains values between 0 and 255 (inclusive) while doubleValues
 * contains values from 0 to 1.0 (inclusive).
 */
struct ColorResult
{
	QVector<int> intValues;
	QVector<double> doubleValues;
	bool valid;
	bool notDefault;

	ColorResult() :
		valid(false),
		notDefault(false) {}
};

/*!
 * Result of parsing a double parameter from parseDouble().
 *
 * When working with this result, just check if notDefault is true and retrieve
 * the value then. All other error cases are all handled and logged
 * within parseDouble().
 *
 * The string is set regardless of the invalid state.
 *
 * invalid is true if the passed string can not be parsed as a double.
 *
 * Only check the value if present is set to true.
 */
struct DoubleResult
{
	double value;
	QString string;
	bool notDefault;
	bool invalid;

	DoubleResult() :
		value(0.0),
		string(""),
		notDefault(false),
		invalid(false) {}
};

/*!
 * Used in tuple parameters like [10 1 3].
 *
 * Make sure that you only parse the contents if valid variable is true!
 */
struct IntTuple
{
	bool valid;
	QVector<int> values;

	IntTuple() :
		valid(false),
		values(QVector<int>()) {}
};

/*!
 * Used in tuple parameters like [10.2 1.3333 3.].
 *
 * Make sure that you only parse the contents if valid variable is true!
 *
 * We also store the strings to compare double values. The strings have their
 * trailing zeroes stripped with utils::stripZeroes() so you would compare
 * against "4".
 */
struct DoubleTuple
{
	bool valid;
	QVector<double> values;
	QVector<QString> strings;

	DoubleTuple() :
		valid(false),
		values(QVector<double>()),
		strings(QVector<QString>()) {}
};

/*!
 * Assigns the current spin box value to the passed parameter of the VMT
 * file if the passed spin box is enabled and its value is not equal to the
 * passed value.
 *
 * Call from MainWindow::makeVmt().
 */
void addOnUnequal(const QString &name, QDoubleSpinBox* sb, double value,
		VmtFile *vmt);

/*!
 * Takes the passed parameter and parses it as a double.
 *
 * 4.0, 4. and 4 are all supported!
 *
 * Make sure that the default string contains no trailing dot character or any
 * zeroes. If you want to check for 4.0 you would pass "4" and for 4.25, you
 * would pass "4.25".
 *
 * The UI parameter is required as we need to log errors, with the
 * parameter name.
 */
DoubleResult parseDouble(const QString &parameter, const QString &value,
	const QString &def, Ui::MainWindow *ui, bool logErrors = true);

/*!
 * Does not log anything!
 */
DoubleResult parseDoubleNoDef(const QString &value);

/*!
 * Parses the passed parameter as an integer by using parseDouble() and casting
 * to an integer.
 *
 * The UI parameter is required as we need to log errors.
 */
IntResult parseInt(const QString &parameter, const QString &value, int def,
	Ui::MainWindow *ui);

/*!
 * Like parseInt() but does not consider any default value.
 */
IntResult parseIntNoDefault(const QString &value);

/*!
 * Parses the passed parameter as a color tuple.
 *
 * The format is the same for the DoubleTuple except that {255 255 255} is also
 * supported where each integer goes from 0 to 255 and directly reprents the
 * RGB value, but doubles are also handled but lost upon saving the VMT.
 *
 * The default values are white or as a string: [1 1 1].
 *
 * The UI parameter is required as we need to log errors.
 */
ColorResult parseColor(const QString &parameter, const QString &value,
	Ui::MainWindow *ui, bool toSrgb = false);

/*!
 * Parses the passed boolean parameter with values 0 or 1.
 *
 * We always assume that false or 0 is the default value.
 * Make sure that this method is called with non-empty value strings.
 *
 * The UI parameter is required as we need to log errors.
 */
BooleanResult parseBoolean(const QString &parameter, const QString &value,
	Ui::MainWindow *ui);

/*!
 * Takes the passed parameter and parses it as a boolean with values 0 or 1.
 *
 * We always assume that false or 0 is the default value.
 *
 * The UI parameter is required to log errors.
 */
BooleanResult takeBoolean(const QString &parameter, VmtFile *vmt,
	Ui::MainWindow *ui);

/*!
 * Replaces \ with / for the file system directories and strips .vtf from the
 * end if the passed string ends with that extension.
 */
QString prepareTexture(const QString &value);

/*!
 * Validates the passed value as a texture file location, logs errors and sets
 * the value to the line edit.
 *
 * Make sure that this method is called with non-empty value strings.
 *
 * The vmt parameter is required for the game directory if it exists.
 * It is used in checking if the texture exists in the selected game directory.
 *
 * The UI parameter is required to log errors.
 */
void parseTexture(const QString &parameter, const QString &value,
	Ui::MainWindow *ui, QLineEdit *widget, VmtFile vmt);

/*!
 * Validates the passed string if it is in the form of:
 *
 *   [{n1} {n2} {n3} ]
 *
 * '4.0', '4.' and '4' are all valid values.
 *
 * We can have whitespace everywhere between the characters as long as the
 * numbers itself are not split.
 *
 * The amount parameter designates how many values this array has to have.
 * It has to be >= 1. If violated the returned tuple is not valid or if the
 * format of the passed string is also not in the correct format.
 */
DoubleTuple toDoubleTuple(const QString &s, int amount);

/*!
 * Same as toDoubleTuple() except that the values are all rounded down to
 * integers.
 */
IntTuple toIntTuple(const QString &s, int amount);

/*!
 * Returns true if all values of the tuple are in between the passed min and
 * max values.
 *
 * Make sure that the passed tuple is valid!
 *
 * Min value is inclusive and the max value is exclusive: [min, max)
 */
bool isTupleBetween(const DoubleTuple &tuple, double min, double max);

/*!
 * Returns true if all values of the string list are in between the passed min
 * and max values.
 *
 * Each string value is converted to a double first and false is returned if
 * this conversion failed.
 *
 * Min value is inclusive and the max value is exclusive: [min, max)
 */
bool isBetween(const QStringList &list, double min, double max);

/*!
 * Returns the color in {255 128 0} which maps to the RGB values.
 */
inline QString toWaterParameter(const QColor &color)
{
	return QString("{%1 %2 %3}")
		.arg(color.red())
		.arg(color.green())
		.arg(color.blue());
}

/*!
 * Returns the color in [0.5 0.1 0.5] with 1.0 corresponding to a value of 255.
 */
QString toParameter(const QColor &color, bool toLinear = false);

QString toParameterBig(const QColor &color, const double multiplier, bool toLinear = false);

} // namespace utils
