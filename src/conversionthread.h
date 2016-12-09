#ifndef CONVERSIONTHREAD_H
#define CONVERSIONTHREAD_H

#include <QThread>
#include <QListWidget>

class ConversionThread : public QThread
{
	Q_OBJECT

public:

	ConversionThread( QObject* parent, QListWidget* logger );

	virtual void run();

	QString fileName;
	QString outputParameter;
	QString newFileName;

private:

	QString output;

	QListWidget* mLogger;
};

#endif // CONVERSIONTHREAD_H
