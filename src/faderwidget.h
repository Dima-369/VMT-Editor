#ifndef FADERWIDGET_H
#define FADERWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QPointer>
#include <QTimer>

#include "utilities.h"
#include "mainwindow.h"

class FaderWidget : public QWidget
{
	Q_OBJECT

public:

	FaderWidget( MainWindow* mainWindow, QWidget* parent );

	void start(bool fadeToWhite);

protected:

	void paintEvent(QPaintEvent* event);

private:

	QTimer* timer;

    QColor startColor;
    int currentAlpha;
    int duration;

	bool fadingToWhite;

	bool fading;

	MainWindow* mainWindow;

signals:

	// Only called when widget is getting hidden
	void doneFading();

public slots:

	void stop();
};

class FadeGroupBox : public QGroupBox
{
	Q_OBJECT

public:

	FadeGroupBox(QWidget* parent = NULL);

	void fadeShow( MainWindow* mainWindow );

	void fadeHide( MainWindow* mainWindow );

private:

	QPointer<FaderWidget> faderWidget;

	QTimer delayTimer;
};

#endif
