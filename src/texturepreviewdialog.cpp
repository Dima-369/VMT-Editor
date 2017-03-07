#include "texturepreviewdialog.h"

TexturePreviewDialog::TexturePreviewDialog(
		const QString& file, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::TexturePreviewDialog)
{
	QPixmap image(file);

	if (image.isNull()) {
		// trying to preview an invalid image?
		// the close() signal has to be queued because close() directly
		// does not work in the construction
		QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
		return;
	}
		
	ui->setupUi(this);

	const QSize size = image.size();
	const int imagePadding = 100;

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
	move(mouse.x() - (w/2), mouse.y() - (h/2));

	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	setMinimumSize(w, h);
	setMaximumSize(w, h);

	// just a background-image will not work because it won't be stretched!
	setStyleSheet(QString("border-image: url(%1) 0 0 0 0 stretch stretch;")
		.arg(file));
}

TexturePreviewDialog::~TexturePreviewDialog()
{
	delete ui;
}

void TexturePreviewDialog::mousePressEvent(QMouseEvent* event)
{
	close();
}
