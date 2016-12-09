#include "doublesliderspinbox.h"

#include "utilities.h"
#include <QDateTime>

DoubleSliderSpinBox::DoubleSliderSpinBox(QWidget* parent) :
	QDoubleSpinBox(parent)
{
}

void DoubleSliderSpinBox::setDoubleSlider(QSlider* slider, double spinBoxMax) {

	this->slider = slider;
	this->spinBoxMax = spinBoxMax;

	slider->setRange(0, spinBoxMax * 100.0);

	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeSpinBox(int)));
	connect(this, SIGNAL(valueChanged(double)), this, SLOT(changeSlider(double)));
}

int DoubleSliderSpinBox::convertSpinBoxValueToSlider(double value) {

	return qRound((value) * (100.0 * spinBoxMax) / spinBoxMax );
}

void DoubleSliderSpinBox::changeSpinBox(int value) {

	if (convertSpinBoxValueToSlider(this->value()) != value)
		setValue(spinBoxMax * value / (100.0 * spinBoxMax));
}

void DoubleSliderSpinBox::changeSlider(double value) {

	slider->setValue(convertSpinBoxValueToSlider(value));
}
