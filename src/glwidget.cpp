#include "glwidget.h"

// whatever
#include "mainwindow.h"

GLWidget::GLWidget(const QString &overlayTexture, const QString &objectName,
		MainWindow* mainWindow, QWidget* parent) :
	QOpenGLWidget(parent),
	mainWindow(mainWindow),
	isShowing(false),
	texture(0),
	textTexture(0),
	overlay(overlayTexture)
{
	setObjectName(objectName);
	setAcceptDrops(true);

	// required for Windows
	setAttribute(Qt::WA_DontCreateNativeAncestors);

	setVisible(false);
}

GLWidget::~GLWidget()
{
	makeCurrent();

	delete texture;
	delete textTexture;

	doneCurrent();
}

void GLWidget::initializeGL()
{
	initializeOpenGLFunctions();

	opengl::initialize(this);

	textTexture = opengl::load(overlay);
}

void GLWidget::resizeGL(int w, int h)
{
	opengl::resize(w, h);

	offset = opengl::calculateOffset(offset.width, offset.height, width(),
		height());

	update();
}

void GLWidget::paintGL()
{
	opengl::clear(this);

	if (texture == 0)
		return;

	opengl::drawQuad(offset, texture);
	opengl::drawQuad(width(), height(), textTexture);
}

void GLWidget::loadTexture(const QString &filePath)
{
	if (filePath.isEmpty()) {
		qDebug() << "Wants to load an empty image?";
		isShowing = false;
		setVisible(false);
		return;
	}

	QImage image;

	if (!image.load(filePath)) {
		qDebug() << "Could not load " << filePath;
		isShowing = false;
		setVisible(false);
		return;
	}

	isShowing = true;
	setVisible(true);

	offset = opengl::calculateOffset(image.width(), image.height(), width(),
		height());

	delete texture;
	texture = new QOpenGLTexture(image.mirrored());

	update();
}

void GLWidget::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();
	mainWindow->droppedTextureOnGLWidget(
		mimeData->urls().at(0).toLocalFile(), objectName());
}

void GLWidget::dragEnterEvent(QDragEnterEvent* event)
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

void GLWidget::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}

void GLWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}
