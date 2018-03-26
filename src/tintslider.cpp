#include "tintslider.h"
#include "utilities.h"
TintSlider::TintSlider( QWidget* parent ) :
	QSlider(parent),
    colorWidget(NULL),
    color(255, 255, 255)
{
	connect( this, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot(int)) );
}

void TintSlider::wheelEvent(QWheelEvent *event) {

	event->ignore();
}

void TintSlider::valueChangedSlot(int value) {

	if( colorWidget == NULL )
		return;

	QColor tmp(color.toHsl());
	QColor hsl = QColor::fromHsl( tmp.hslHue(), tmp.hslSaturation(), value);

	colorWidget->setStyleSheet( QString("background-color: rgb(%1, %2, %3)")
							   .arg( hsl.red() )
							   .arg( hsl.green() )
							   .arg( hsl.blue() ));
}

void TintSlider::initialize(QToolButton *colorWidget, const QColor& color) {

	this->colorWidget = colorWidget;
	this->color = color;

	setValue( color.toHsl().lightness() );
}
