#pragma once

#include <QLineEdit>
#include <QMimeData>
#include <QDragEnterEvent>
#include <qdebug.h>

class TextureDragDropLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	TextureDragDropLineEdit(QWidget* parent);

signals:

	void droppedTexture(const QString& filePath);

protected:

	void dropEvent(QDropEvent* event);

	void dragEnterEvent(QDragEnterEvent* event);

	void dragMoveEvent(QDragMoveEvent* event);

	void dragLeaveEvent(QDragLeaveEvent* event);
};
