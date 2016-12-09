#include "texturepreviewdockwidget.h"

#include "utilities.h"

TexturePreviewDockWidget::TexturePreviewDockWidget(QWidget* parent)
	: QDockWidget(parent)
	, texturePreviewSize(256)
{
	connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(floatingChanged()));
}

void TexturePreviewDockWidget::setTexturePreviewSize(int size)
{
	texturePreviewSize = size;

	floatingChanged();
}

void TexturePreviewDockWidget::floatingChanged()
{
	if (isFloating()) {

		setMaximumSize(texturePreviewSize, texturePreviewSize);

	} else {

		setMaximumSize(524287, 524287);
	}
}
