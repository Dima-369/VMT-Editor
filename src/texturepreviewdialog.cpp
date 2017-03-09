#include "texturepreviewdialog.h"

TexturePreviewDialog::TexturePreviewDialog(
		const QString& file, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::TexturePreviewDialog)
{
	// dropping alpha channel for preview
	QImage image(QImage(file).convertToFormat(QImage::Format_RGB32));

	if (image.isNull()) {
		// trying to preview an invalid image?
		// the close() signal has to be queued because close() directly
		// does not work in the construction
		QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
		return;
	}
		
	setup(image);
}

TexturePreviewDialog::TexturePreviewDialog(
		const QImage& image, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::TexturePreviewDialog)
{
	if (image.isNull()) {
		// trying to preview an invalid image?
		// the close() signal has to be queued because close() directly
		// does not work in the construction
		QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
		return;
	}

	setup(image);
}

TexturePreviewDialog::~TexturePreviewDialog()
{
	delete ui;
}

void TexturePreviewDialog::setup(const QImage& image)
{
	ui->setupUi(this);

	scene = new QGraphicsScene(this);
	ui->texture->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	const QSize size = image.size();
	const int imagePadding = 56;

	int w = size.width();
	int h = size.height();
	const QRect monitor = QApplication::desktop()->screenGeometry();
	if ((w + imagePadding) > monitor.width()) {
		w = monitor.width() - imagePadding;
		ui->texture->setDragMode(QGraphicsView::ScrollHandDrag);

	}
	if ((h + imagePadding) > monitor.height()) {
		h = monitor.height() - imagePadding;
		ui->texture->setDragMode(QGraphicsView::ScrollHandDrag);
	}

	// centering dialog at mouse position
	const QPoint mouse = mapFromGlobal(QCursor::pos());
	// Windows taskbar is 40px high
	int x1 = qMin(mouse.x() - 48, monitor.width() - w - 48);
	int y1 = qBound(8, mouse.y() - h/2 + 96, monitor.height() - h - 48);
	move(x1, y1);

	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	setMinimumSize(w, h);
	setMaximumSize(w, h);

	ui->texture->setScene(scene);
	scene->addPixmap(QPixmap::fromImage(image));
}

void TexturePreviewDialog::mousePressEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	close();
}
