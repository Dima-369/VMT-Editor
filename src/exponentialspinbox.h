#pragma once

#include <QDoubleSpinBox>
#include <QSlider>
#include <QDateTime>
#include <QtMath>

#include "utilities.h"

class ExponentialSpinBox : public QDoubleSpinBox {

	Q_OBJECT

public:

	explicit ExponentialSpinBox(QWidget* parent = NULL);

	void setDoubleSlider(QSlider* slider);

private:

	QSlider* slider;

	int convertSpinBoxValueToSlider(double value);

private slots:

	void changeSpinBox(int value);
	void changeSlider(double value);
};
