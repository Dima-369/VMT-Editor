#include "view-helper.h"

#include "utilities/strings.h"

void utils::toggle(MainWindow *mainWindow, QAction *action,
	FadeGroupBox *groupBox, bool parsingVmt)
{
	if(action->isEnabled()) {
		if(groupBox->isVisible()) {
			groupBox->fadeHide(mainWindow);
			action->setChecked(false);
		} else {
			groupBox->show();
			if (!parsingVmt) {
				groupBox->fadeShow(mainWindow);
				groupBox->repaint();
			}
			action->setChecked(true);
		}
	}
}

void utils::toggle(MainWindow *mainWindow, bool checked, FadeGroupBox *groupBox,
		bool parsingVmt)
{
	if(checked) {
		groupBox->show();
		if (!parsingVmt)
			groupBox->fadeShow(mainWindow);
		groupBox->repaint();

	} else {
		groupBox->fadeHide(mainWindow);
	}
}

void utils::applyBackgroundColor(const QString &parameter, const QString &value,
		QPlainTextEdit *colorWidget, TintSlider *sliderWidget,
		Ui::MainWindow *ui)
{
	utils::ColorResult r = utils::parseColor(parameter, value, ui);

	if (r.notDefault) {
		int red = r.intValues.at(0);
		int green = r.intValues.at(1);
		int blue = r.intValues.at(2);
		utils::applyBackgroundColor(red, green, blue, colorWidget);

		QColor color;
		color.setRed(red);
		color.setGreen(green);
		color.setBlue(blue);
		sliderWidget->initialize(colorWidget, color);
	}
}

void utils::applyBackgroundColor(const QString &parameter, const QString &value,
		QPlainTextEdit *colorWidget, Ui::MainWindow *ui, bool toSrgb)
{
	utils::ColorResult r = utils::parseColor(parameter, value, ui, toSrgb);

	if (r.notDefault) {
		int red = r.intValues.at(0);
		int green = r.intValues.at(1);
		int blue = r.intValues.at(2);
		utils::applyBackgroundColor(red, green, blue, colorWidget);

		QColor color;
		color.setRed(red);
		color.setGreen(green);
		color.setBlue(blue);
	}
}

void utils::applyBackgroundColor(int red, int green, int blue,
	QPlainTextEdit *colorWidget)
{
	// according to the documentation the cast to the string is not
	// required but we seem to need it
	QString s = QString("background-color: rgb(%1, %2, %3)")
		.arg(STR(red), STR(green), STR(blue));
	colorWidget->setStyleSheet(s);
}

QColor utils::getBG(QPlainTextEdit *widget)
{
	// TODO: Debug and explain how and why this works!
	QColor result;

	QString ss = widget->styleSheet();
	ss = ss.mid(ss.indexOf('(') + 1, ss.length() - ss.indexOf('(') - 2);

	QStringList list = ss.split(',');
	result.setRed(list.at(0).toInt());
	result.setGreen(list.at(1).toInt());
	result.setBlue(list.at(2).toInt());

	return result;
}

namespace utils {

QString getNonDef(QDoubleSpinBox *sb, const QString &def)
{
	if (!sb->isEnabled())
		return "";

	const QString text = sb->cleanText();
	if (stripZeroes(text) == def)
		return "";

	return text;
}

} // namespace utils
