#include "texturedragdroplineedit.h"

TextureDragDropLineEdit::TextureDragDropLineEdit(QWidget* parent) :
	QLineEdit(parent)
{
}

void TextureDragDropLineEdit::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();
	emit droppedTexture(mimeData->urls().at(0).toLocalFile());
}

void TextureDragDropLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls()) {
		const auto url = event->mimeData()->urls().at(0);
		QString str = url.toLocalFile();
		if (!str.isEmpty()) {
			event->acceptProposedAction();
		}
	}
}

void TextureDragDropLineEdit::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}

void TextureDragDropLineEdit::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}
