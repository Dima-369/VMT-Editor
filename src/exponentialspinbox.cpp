#include "exponentialspinbox.h"

ExponentialSpinBox::ExponentialSpinBox(QWidget* parent) :
	QDoubleSpinBox(parent)
{
}

void ExponentialSpinBox::setDoubleSlider(QSlider* slider) {

	this->slider = slider;

	connect(slider, SIGNAL(valueChanged(int)), SLOT(changeSpinBox(int)));
}

int ExponentialSpinBox::convertSpinBoxValueToSlider(double value) {

	double minp = 0.0, maxp = 100.0;
	double minv = qLn(1);
	double maxv = qLn(129.0);

	double scale = (maxv-minv) / (maxp-minp);

	return qRound((qLn(value + 1.0) - minv) / scale + minp);
}

void ExponentialSpinBox::changeSpinBox(int value)
{
	double minp = 0.0, maxp = 100;
	double minv = qLn(1), maxv = qLn(129.0);

	double scale = (maxv-minv) / (maxp-minp);
	double dv = value;

	setValue(qExp(minv + (scale*(dv-minp))) - 1.0);


	setValue((qRound(this->value() / 0.125)) * 0.125);
}

void ExponentialSpinBox::changeSlider(double value) {

	slider->setValue(convertSpinBoxValueToSlider(value));
}
