#include "texturepreviewdialog.h"

TexturePreviewDialog::TexturePreviewDialog(
		const QString& file, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::TexturePreviewDialog)
{
	// dropping alpha channel for preview
	QImage image(QImage(file).convertToFormat(QImage::Format_RGB888));

	if (image.isNull()) {
		// trying to preview an invalid image?
		// the close() signal has to be queued because close() directly
		// does not work in the construction
		QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
		return;
	}
		
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
	int x1 = mouse.x() - 48;
	int y1 = mouse.y() - h/2 + 96;
	if (y1 < 16) {
		y1 = 16;
	//taskbar is 40px high
	} else if (y1 > (monitor.height() - h - 40)) {
		y1 = monitor.height() - h - 40;
	}
	move(x1, y1);

	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	setMinimumSize(w, h);
	setMaximumSize(w, h);

	ui->texture->setPixmap(QPixmap::fromImage(image));
	// just a background-image will not work because it won't be stretched!
	//setStyleSheet(QString("border-image: url(%1) 0 0 0 0 stretch stretch;")
		//.arg(file));
}

TexturePreviewDialog::~TexturePreviewDialog()
{
	delete ui;
}

void TexturePreviewDialog::mousePressEvent(QMouseEvent* event)
{
	close();
}
