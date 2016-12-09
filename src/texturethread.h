#ifndef TEXTURETHREAD_H
#define TEXTURETHREAD_H

#include <QThread>

#include "glwidget_spec.h"

class TextureThread : public QThread
{
public:

	TextureThread( QObject* parent )
		: QThread(parent),
		  baseTexture(true),
		  alpha(false),
		  alphaTest(false),
		  alphaOnly(false)
	{

	}

	virtual void run();

	//----------------------------------------------------------------------------------------//

	QString input;
	QString output;

	QString vtfFile;

	QString object;

	bool baseTexture;

	bool alpha;

	bool alphaTest;

	bool alphaOnly;

	//----------------------------------------------------------------------------------------//

	GLWidget_Spec::Mode mode;
};

#endif // TEXTURETHREAD_H
