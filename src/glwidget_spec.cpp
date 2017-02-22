#include "glwidget_spec.h"

// whatever
#include "mainwindow.h"

GLWidget_Spec::GLWidget_Spec(MainWindow* mainWindow) :
	QOpenGLWidget(mainWindow),
	mainWindow(mainWindow),
	texture(0),
	textTexture(0)
{
	setObjectName("preview_spec1");
	setAcceptDrops(true);

	// required for Windows
	setAttribute(Qt::WA_DontCreateNativeAncestors);

	setVisible(false);
}

GLWidget_Spec::~GLWidget_Spec()
{
	makeCurrent();

	delete texture;
	delete textTexture;

	doneCurrent();
}

void GLWidget_Spec::initializeGL()
{
	initializeOpenGLFunctions();

	opengl::initialize(this);

	textTexture = opengl::load(":/overlays/spec1");
}

void GLWidget_Spec::resizeGL(int w, int h)
{
	opengl::resize(w, h);

	offset = opengl::calculateOffset(offset.width, offset.height, width(),
		height());

	update();
}

void GLWidget_Spec::paintGL()
{
	opengl::clear(this);

	if (texture == 0)
		return;

	opengl::drawQuad(offset, texture);
	opengl::drawQuad(width(), height(), textTexture);
}

void GLWidget_Spec::updateValues(Mode mode, const QString &filePath)
{
	if (filePath.isEmpty()) {
		qDebug() << "Wants to load an empty image?";
		setVisible(false);
		return;
	}

	QImage image;

	if (!image.load(filePath)) {
		qDebug() << "Could not load " << filePath;
		setVisible(false);
		return;
	}

	image = image.mirrored();

	setVisible(true);

	delete texture;

	if (mode == Bumpmap) {
		QImage alpha = image.alphaChannel();
		QColor pixel, pix;

		for (int i = 0; i < alpha.width(); ++i) {

			for (int j = 0; j < alpha.height(); ++j) {
				pixel = alpha.pixel(i, j);
				pix.setRedF(   pixel.redF() );
				pix.setGreenF( pixel.redF() );
				pix.setBlueF(  pixel.redF() );
				pix.setAlphaF( 1.0f );

				image.setPixel(i, j, pix.rgba());
			}
		}
	}

	if (mode == Diffuse) {
		QImage alpha = image.alphaChannel();
		QColor pixel, pix;

		for (int i = 0; i < alpha.width(); ++i) {

			for (int j = 0; j < alpha.height(); ++j) {
				pixel = alpha.pixel(i, j);
				pix.setRedF(   1.0f - pixel.redF() );
				pix.setGreenF( 1.0f - pixel.redF() );
				pix.setBlueF(  1.0f - pixel.redF() );
				pix.setAlphaF( 1.0f );

				image.setPixel(i, j, pix.rgba());
			}
		}
	}

	texture = new QOpenGLTexture(image);

	offset = opengl::calculateOffset(image.width(), image.height(), width(),
		height());

	update();
}

void GLWidget_Spec::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();
	mainWindow->droppedTextureOnGLWidget(
		mimeData->urls().at(0).toLocalFile(), objectName());
}

void GLWidget_Spec::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		foreach (const QUrl& url, event->mimeData()->urls())
		{
			QString str = url.toLocalFile();
			if (!str.isEmpty())
			{
				//if (QFileInfo(str).suffix() == "vmt")
				event->acceptProposedAction();
			}
		}
	}
}

void GLWidget_Spec::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}

void GLWidget_Spec::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}
