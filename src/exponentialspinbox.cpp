#include "exponentialspinbox.h"

#include "utilities.h"
#include <QDateTime>
#include <QtMath>

ExponentialSpinBox::ExponentialSpinBox(QWidget* parent) :
	QDoubleSpinBox(parent)
{
}

void ExponentialSpinBox::setDoubleSlider(QSlider* slider) {

	this->slider = slider;

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeSpinBox(int)));
	//connect(this, SIGNAL(valueChanged(double)), this, SLOT(changeSlider(double)));
}

int ExponentialSpinBox::convertSpinBoxValueToSlider(double value) {

	double minp = 0.0, maxp = 100.0;
	double minv = qLn(1);
	double maxv = qLn(129.0);

	double scale = (maxv-minv) / (maxp-minp);

	return qRound((qLn(value + 1.0)-minv) / scale + minp);
	//return qRound((value) * (100.0 * spinBoxMax) / spinBoxMax );
}

void ExponentialSpinBox::changeSpinBox(int value) {

	//if (convertSpinBoxValueToSlider(this->value()) != value) {

		double minp = 0.0, maxp = 100;
		double minv = qLn(1), maxv = qLn(129.0);

		double scale = (maxv-minv) / (maxp-minp);
		double dv = value;

		setValue(qExp(minv + (scale*(dv-minp))) - 1.0);


		setValue((qRound(this->value() / 0.125)) * 0.125);

		//setValue(dv * 0.1);
		//Y("value: " + Str(value) + ", calc: " + Str(qExp(minv + (scale*(value-minp))) - 1.0))

		Y("value: " + Str(value) + ", calculated: " + Str(this->value()))

	//}
}

void ExponentialSpinBox::changeSlider(double value) {

	slider->setValue(convertSpinBoxValueToSlider(value));
}
