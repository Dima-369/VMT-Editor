#include "vmt-helper.h"

#include "logging/logging.h"
#include "utilities/strings.h"

namespace utils {

void addOnUnequal(const QString &name, QDoubleSpinBox *sb, double value,
		VmtFile* vmt)
{
	if (sb->isEnabled()) {
		double sb_value = sb->value();
		if (sb_value != value)
			vmt->parameters.insert(name, QString::number(sb_value));
	}
}

DoubleResult parseDouble(const QString &parameter, const QString &value,
		const QString &def, Ui::MainWindow *ui, bool logErrors)
{
	DoubleResult result;
	result.string = value;

	bool ok;
	double d = value.toDouble(&ok);

	if (!ok) {
		result.invalid = true;
		return result;
	}

	const auto v = stripZeroes(value);
	if (def == v) {
		if (logErrors) {
			if (def.contains(".")) {
				INFO(parameter + " has default value: " + def)
			} else {
				INFO(parameter + " has default value: "
					+ def +  ".0")
			}
		}
	} else {
		result.notDefault = true;
		result.value = d;
	}

	return result;
}

DoubleResult parseDoubleNoDef(const QString &value)
{
	DoubleResult result;
	result.string = value;

	bool ok;
	double d = value.toDouble(&ok);

	if (!ok) {
		result.invalid = true;
		return result;
	}

	result.notDefault = true;
	result.value = d;
	return result;
}

IntResult parseInt(const QString &parameter, const QString &value,
		int def, Ui::MainWindow *ui)
{
	IntResult result;

	bool logErrors = false;
	DoubleResult r = parseDouble(parameter, value,
		QString::number(def), ui, logErrors);

	if (r.invalid) {
		result.invalid = true;
		return result;
	}

	if (r.notDefault) {
		result.value = static_cast<int>(r.value);
		result.notDefault = true;

	} else {
		INFO(parameter + " has default value: " + STR(def))
	}

	return result;
}

IntResult parseIntNoDefault(const QString &value)
{
	DoubleResult r = parseDoubleNoDef(value);
	IntResult result;

	if (r.invalid) {
		result.invalid = true;
		return result;
	}

	result.notDefault = true;
	result.value = static_cast<int>(r.value);
	return result;
}

ColorResult parseColor(const QString &parameter,
		const QString &value, Ui::MainWindow *ui, bool toSrgb)
{
	ColorResult result;

	// first we check if the value is in the regular [] format
	DoubleTuple dp = toDoubleTuple(value, 3);
	if (dp.valid) {
		bool allDefault = true;
		result.valid = true;

		foreach(const QString &s, dp.strings) {
			if (s != "1") {
				allDefault = false;
				break;
			}
		}
		if (allDefault) {
			return result;
		}

		result.valid = true;
		result.notDefault = true;

		foreach(double x, dp.values) {
			if (toSrgb)
				x = pow(x, 0.454545);
			result.doubleValues.append(x);
		}

		foreach(double x, dp.values) {
			if (x < 0.0) {
				x = 0.0;
			} else if (x > 1.0) {
				x = 1.0;
			}
			if (toSrgb)
				x = pow(x, 0.454545);
			result.intValues.append(static_cast<int>(x * 255.0));
		}
		return result;
	}

	// now we check the {} format, like " {12 12.5 13 }"
	const QString doubleRegex = "[0-9]*\\.?[0-9]+";
	const QString full = QString(
			"^\\s*\\{\\s*" + doubleRegex +
			"\\s+" + doubleRegex +
			"\\s+" + doubleRegex +
			"\\s*\\}\\s*$");

	const QRegularExpression regex(full);
	if (!regex.match(value).hasMatch()) {
		ERROR(parameter + " has invalid value: \"" + value + "\"");
		return result;
	}

	result.valid = true;

	QString p = QString(value).remove("{").remove("}");
	QStringList parts = p.trimmed().split(" ", QString::SkipEmptyParts);

	foreach (const QString& s, parts) {
		bool ok;
		double x = s.toDouble(&ok);
		if (!ok) {
			result.valid = false;
			return result;
		}
		if (x < 0) {
			x = 0;
		} else if (x > 255) {
			x = 255;
		}
		if (toSrgb) {
			x = x / 255;
			x = pow(x, 0.454545);
			x = x * 255;
		}
		result.doubleValues.append(x / 255.0);
		result.intValues.append(static_cast<int>(x));
	}

	if (result.intValues[0] == 255 &&
			result.intValues[1] == 255 &&
			result.intValues[2] == 255) {
		INFO(parameter + " has default white value")
	} else {
		result.notDefault = true;
	}

	return result;
}

BooleanResult parseBoolean(const QString &parameter,
		const QString &value, Ui::MainWindow *ui)
{
	BooleanResult result;
	result.present = true;
	if (value != "0" && value != "1") {
		ERROR(parameter + " has an invalid value: " + value)
		return result;
	}
	if (value == "1") {
		result.value = true;
	} else {
		INFO(parameter + " has default value: 0")
	}
	return result;
}

BooleanResult takeBoolean(const QString &parameter, VmtFile *vmt,
		Ui::MainWindow *ui)
{
	BooleanResult result;
	const QString raw = vmt->parameters.take(parameter);
	if (raw.isEmpty())
		return result;

	result.present = true;
	if (raw != "0" && raw != "1") {
		ERROR(parameter + " has an invalid value: " + raw)
		return result;
	}
	if (raw == "1") {
		result.value = true;
	} else {
		INFO(parameter + " has default value: 0")
	}
	return result;
}

QString prepareTexture(const QString &value)
{
	QString s = QString(value).replace("\\", "/");
	if (s.endsWith(".vtf")) {
		s = s.left(s.size() - 4);
	}
	return s;
}

void parseTexture(const QString &parameter, const QString &value,
		Ui::MainWindow *ui, QLineEdit *widget, VmtFile vmt)
{
	QString s = prepareTexture(value);

	if (value.endsWith(".vtf")) {
		ERROR(parameter + " should not end with .vtf!")
		// dropping the .vtf extension
		s = s.left(s.size() - 4);
	}

	if (vmt.state.gameDirectory != 0 && !vmt.state.gameDirectory->exists(
			"materials/" + s + ".vtf")) {

		INFO(parameter + " file: \"" + s + ".vtf\" cannot be found!")
	}

	widget->setText(s);
}

DoubleTuple toDoubleTuple(const QString &s, int amount)
{
	DoubleTuple result;

	QString full = "^\\s*\\[\\s*";
	for (int i = 0; i < amount; i++) {
		full.append("[0-9\\.]+\\s");
		if (i + 1 == amount) {
			full.append("*");
		} else {
			full.append("+");
		}
	}
	full.append("\\]\\s*$");

	QRegularExpression regex(full);

	if (regex.match(s).hasMatch()) {
		// removing [ and ] from the input
		QString copy = QString(s);
		QString p = copy.remove("[").remove("]");
		QStringList parts = p.trimmed().split(" ",
			QString::SkipEmptyParts);

		foreach (const QString &s, parts) {
			bool ok;
			const double c = s.toDouble(&ok);

			if (ok) {
				result.values.append(c);
				result.strings.append(stripZeroes(s));
			} else {
				result.valid = false;
				qDebug() << "Parse error: " << s;
				return result;
			}
		}

		result.valid = true;
	}

	return result;
}

IntTuple toIntTuple(const QString &input, int amount)
{
	IntTuple result;
	DoubleTuple fromDouble = toDoubleTuple(input, amount);
	if (fromDouble.valid) {
		foreach (double value, fromDouble.values) {
			result.values.append(static_cast<int>(value));
		}
	} else {
		result.valid = false;
	}
	return result;
}

bool isTupleBetween(const DoubleTuple &tuple, double min,
		double max)
{
	foreach(double value, tuple.values) {
		if (!(value >= min && value < max)) {
			return false;
		}
	}
	return true;
}

bool isBetween(const QStringList& list, double min, double max)
{
	foreach(const QString& value, list) {
		bool ok;
		const double c = value.toDouble(&ok);
		if (ok) {
			if (!(c >= min && c < max)) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}

QString toParameter(const QColor &color, bool toLinear)
{
	double redf = color.redF();
	double greenf = color.greenF();
	double bluef = color.blueF();

	if(toLinear) {
		redf = pow(redf, 2.2);
		greenf = pow(greenf, 2.2);
		bluef = pow(bluef, 2.2);
	}

	const QString red = stripZeroes(QString::number(redf, 'f', 2));
	const QString green = stripZeroes(QString::number(greenf, 'f', 2));
	const QString blue = stripZeroes(QString::number(bluef, 'f', 2));

	return QString("[%1 %2 %3]").arg(red, green, blue);
}

QString toParameterBig(const QColor &color, const double multiplier, bool toLinear)
{
	double redf = color.redF() * multiplier;
	double greenf = color.greenF() * multiplier;
	double bluef = color.blueF() * multiplier;

	if(toLinear) {
		redf = pow(redf, 2.2);
		greenf = pow(greenf, 2.2);
		bluef = pow(bluef, 2.2);
	}

	const QString red = stripZeroes(QString::number(redf, 'f', 2));
	const QString green = stripZeroes(QString::number(greenf, 'f', 2));
	const QString blue = stripZeroes(QString::number(bluef, 'f', 2));

	return QString("[%1 %2 %3]").arg(red, green, blue);
}

} // namespace utils
