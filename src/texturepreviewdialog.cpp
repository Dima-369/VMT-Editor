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

	const QSize size = image.size();
	const int imagePadding = 56;

	int w = size.width();
	int h = size.height();
	const QRect monitor = QApplication::desktop()->screenGeometry();
	while ((w + imagePadding) > monitor.width() ||
		(h + imagePadding) > monitor.height()) {
		
		w = w / 2;
		h = h / 2;
	}

	// centering dialog at mouse position
	const QPoint mouse = mapFromGlobal(QCursor::pos());
	// Windows taskbar is 40px high
	int x1 = qMin(mouse.x() - 48, monitor.width() - w - 40);
	int y1 = qBound(16, mouse.y() - h/2 + 96, monitor.height() - h - 40);
	move(x1, y1);

	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	setMinimumSize(w, h);
	setMaximumSize(w, h);

	// background-images are causing images with the alpha, so a QLabel is
	// used for the preview
	ui->texture->setPixmap(QPixmap::fromImage(image));
}

void TexturePreviewDialog::mousePressEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	close();
}
