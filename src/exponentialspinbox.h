#ifndef EXPONENTIALSPINBOX_H
#define EXPONENTIALSPINBOX_H

#include <QDoubleSpinBox>
#include <QSlider>

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

#endif // EXPONENTIALSPINBOX_H
