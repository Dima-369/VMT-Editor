#include "vmt-helper.h"

#include "logging/logging.h"
#include "utilities/strings.h"

void utils::addOnUnequal(const QString &name, QDoubleSpinBox *sb, double value,
		VmtFile* vmt)
{
	if (sb->isEnabled()) {
		double sb_value = sb->value();
		if (sb_value != value)
			vmt->parameters.insert(name, QString::number(sb_value));
	}
}

utils::DoubleResult utils::parseDouble(const QString &parameter,
		const QString &value, const QString &def, Ui::MainWindow *ui,
		bool logErrors)
{
	utils::DoubleResult result;
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
				ERROR(parameter + " has default value: " + def)
			} else {
				ERROR(parameter + " has default value: "
					+ def +  ".0")
			}
		}
	} else {
		result.notDefault = true;
		result.value = d;
	}

	return result;
}

utils::IntResult utils::parseInt(const QString &parameter, const QString &value,
		int def, Ui::MainWindow *ui)
{
	utils::IntResult result;

	bool logErrors = false;
	utils::DoubleResult r = utils::parseDouble(parameter, value,
		QString::number(def), ui, logErrors);

	if (r.invalid) {
		result.invalid = true;
		return result;
	}

	if (r.notDefault) {
		result.value = static_cast<int>(r.value);
		result.notDefault = true;

	} else {
		ERROR(parameter + " has default value: " + STR(def))
	}

	return result;
}

utils::ColorResult utils::parseColor(const QString &parameter,
		const QString &value, Ui::MainWindow *ui, bool toSrgb)
{
	utils::ColorResult result;

	// first we check if the value is in the regular [] format
	utils::DoubleTuple dp = utils::toDoubleTuple(value, 3);
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
		result.doubleValues = dp.values;
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

	// now we check the {} format
	QString full = "^\\s*\\{\\s*[0-9]+\\s+[0-9]+\\s+[0-9]+\\s*\\}\\s*$";
	QRegularExpression regex(full);

	if (!regex.match(value).hasMatch()) {
		ERROR(parameter + " has default value: " + value);
		return result;
	}

	result.valid = true;

	QString p = QString(value).remove("{").remove("}");
	QStringList parts = p.trimmed().split(" ", QString::SkipEmptyParts);

	bool allDefault = true;

	foreach (const QString &s, parts) {

		if (s != "255") {
			allDefault = false;
			break;
		}

		bool ok;
		int i = s.toInt(&ok);

		if (i < 0) {
			i = 0;
		} else if (i > 255) {
			i = 255;
		}

		if (ok) {
			result.doubleValues.append(i / 255.0);
			result.intValues.append(static_cast<int>(i));
		} else {
			result.valid = false;
			return result;
		}
	}

	if (allDefault) {
		ERROR(parameter + " has default value: {255 255 255}")
	} else {
		result.notDefault = true;
	}

	return result;
}

utils::BooleanResult utils::parseBoolean(const QString &parameter,
		const QString &value, Ui::MainWindow *ui)
{
	utils::BooleanResult result;
	result.present = true;
	if (value != "0" && value != "1") {
		ERROR(parameter + " has an invalid value: " + value)
		return result;
	}
	if (value == "1") {
		result.value = true;
	} else {
		ERROR(parameter + " has default value: 0")
	}
	return result;
}

utils::BooleanResult utils::takeBoolean(const QString &parameter, VmtFile *vmt,
		Ui::MainWindow *ui)
{
	utils::BooleanResult result;
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
		ERROR(parameter + " has default value: 0")
	}
	return result;
}

QString utils::prepareTexture(const QString &value)
{
	QString s = QString(value).replace("\\", "/");
	if (s.endsWith(".vtf")) {
		s = s.left(s.size() - 4);
	}
	return s;
}

void utils::parseTexture(const QString &parameter, const QString &value,
		Ui::MainWindow *ui, QLineEdit *widget, VmtFile vmt)
{
	QString s = utils::prepareTexture(value);

	if (value.endsWith(".vtf")) {
		ERROR(parameter + " should not end with .vtf!")
		// dropping the .vtf extension
		s = s.left(s.size() - 4);
	}

	if (vmt.state.gameDirectory != 0 && !vmt.state.gameDirectory->exists(
			"materials/" + s + ".vtf")) {

		ERROR(parameter + " file: \"" + s + ".vtf\" cannot be found!")
	}

	widget->setText(s);
}

utils::DoubleTuple utils::toDoubleTuple(const QString &s, int amount)
{
	utils::DoubleTuple result;

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

utils::IntTuple utils::toIntTuple(const QString &input, int amount)
{
	utils::IntTuple result;
	utils::DoubleTuple fromDouble = utils::toDoubleTuple(input, amount);
	if (fromDouble.valid) {
		foreach (double value, fromDouble.values) {
			result.values.append(static_cast<int>(value));
		}
	} else {
		result.valid = false;
	}
	return result;
}

bool utils::isTupleBetween(const utils::DoubleTuple &tuple, double min,
		double max)
{
	foreach(double value, tuple.values) {
		if (!(value >= min && value < max)) {
			return false;
		}
	}
	return true;
}

bool utils::isBetween(const QStringList& list, double min, double max)
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

namespace utils {

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

} // namespace utils
