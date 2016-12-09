#ifndef TEXTUREPREVIEWDOCKWIDGET_H
#define TEXTUREPREVIEWDOCKWIDGET_H

#include <QDockWidget>

class TexturePreviewDockWidget : public QDockWidget
{
	Q_OBJECT

public:

	TexturePreviewDockWidget(QWidget* parent = NULL);

	void setTexturePreviewSize(int size);

private:

	int texturePreviewSize;

public slots:

	void floatingChanged();
};

#endif // TEXTUREPREVIEWDOCKWIDGET_H
