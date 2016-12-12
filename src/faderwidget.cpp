#include "faderwidget.h"

#include "mainwindow.h"

#include <QPainter>

FaderWidget::FaderWidget( MainWindow* mainWindow, QWidget* parent) :
	QWidget(parent),
	startColor(QColor(64, 64, 64)), // parent->palette().window().color();
	currentAlpha(0),
	duration(250),
	fadingToWhite(true),
	fading(false),
	mainWindow(mainWindow)
{
	timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));

    setAttribute(Qt::WA_DeleteOnClose);
    resize(parent->size());
}

void FaderWidget::start(bool fadeToWhite)
{
	fadingToWhite = fadeToWhite;

	if (!fading) {

		currentAlpha = (fadeToWhite) ? 0 : 255;
	}

	timer->start(33);
	show();

	fading = true;
}

void FaderWidget::stop()
{
	currentAlpha = 0;

	repaint();
	update();

	timer->stop();

	fading = false;
}

void FaderWidget::paintEvent(QPaintEvent*)
{
	QColor semiTransparentColor = startColor;
    semiTransparentColor.setAlpha(currentAlpha);

	QPainter painter(this);
    painter.fillRect(rect(), semiTransparentColor);

	if (fadingToWhite) {

		currentAlpha += 255 * timer->interval() / duration;

		if (currentAlpha >= 255) {

			emit doneFading();

			timer->stop();
			close();

			fading = false;
		}

	} else {

		currentAlpha -= 255 * timer->interval() / duration;

		if (currentAlpha <= 0) {

			timer->stop();
			close();

			fading = false;
		}
	}
}

FadeGroupBox::FadeGroupBox(QWidget* parent)
	: QGroupBox(parent)
{
	setStyleSheet("QGroupBox{padding-top:25px;border:0px solid gray;font-family:\"Segoe UI\";}"
				  "QLabel{font-family:\"Segoe UI\";}"
				  "QLineEdit{font-family:\"Segoe UI\";}"
				  "QComboBox{font-family:\"Segoe UI\";}"
				  "QCheckBox{font-family:\"Segoe UI\";}"
				  "QToolButton{font-family:\"Segoe UI\";}"
				  "QSpinBox{font-family:\"Segoe UI\";}"
				  "QDoubleSpinBox{font-family:\"Segoe UI\";}");
}

void FadeGroupBox::fadeHide( MainWindow* mainWindow ) {

	if (!faderWidget)
		faderWidget = new FaderWidget(mainWindow, this);

	faderWidget->start(true);

	connect(faderWidget, SIGNAL(doneFading()), this, SLOT(hide()));
}

void FadeGroupBox::fadeShow( MainWindow* mainWindow ) {

	if (!faderWidget)
		faderWidget = new FaderWidget(mainWindow, this);

	faderWidget->start(false);
}
