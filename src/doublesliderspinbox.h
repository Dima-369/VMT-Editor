#ifndef DOUBLESLIDERSPINBOX_H
#define DOUBLESLIDERSPINBOX_H

#include <QDoubleSpinBox>
#include <QSlider>

class DoubleSliderSpinBox : public QDoubleSpinBox {

	Q_OBJECT

public:

	explicit DoubleSliderSpinBox(QWidget* parent = NULL);

	void setDoubleSlider(QSlider* slider, double spinBoxMax = 1.0);

private:

	QSlider* slider;
	double spinBoxMax;
	double spinBoxDefault;
	int convertSpinBoxValueToSlider(double value);

private slots:

	void changeSpinBox(int value);
	void changeSlider(double value);
};

#endif // DOUBLESLIDERSPINBOX_H
