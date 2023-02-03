#include "glwidget.h"

GLWidget::GLWidget(
		const QString &overlayTexture, const QString &objectName,
		QWidget* parent) :
	QOpenGLWidget(parent),
	isShowing(false),
	texture(NULL),
	textTexture(NULL),
	overlay(overlayTexture)
{
	setObjectName(objectName);

	// required for Windows
	setAttribute(Qt::WA_DontCreateNativeAncestors);

	setMouseTracking(true);

	const auto size = QSize(192, 192);
	setMinimumSize(size);
	setMaximumSize(size);

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

	glDisable(GL_BLEND);

	opengl::drawQuad(offset, texture);

	glEnable(GL_BLEND);

	opengl::drawQuad(width(), height(), textTexture);
}

void GLWidget::loadTexture(const QString &filePath)
{
	if (filePath.isEmpty()) {
		qDebug() << "Wants to load an empty image?";
		isShowing = false;
		file = "";
		setVisible(false);
		return;
	}

	QImage image;

	if (!image.load(filePath)) {
		qDebug() << "Could not load " << filePath;
		isShowing = false;
		file = "";
		setVisible(false);
		return;
	}

    qDebug() << "Image mode " << image.format();

	isShowing = true;
	setVisible(true);

	offset = opengl::calculateOffset(image.width(), image.height(), width(),
		height());

	delete texture;
	file = filePath;
	texture = new QOpenGLTexture(image.mirrored());

	update();
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
	Q_UNUSED(event)

	if (!file.isEmpty()) {
		TexturePreviewDialog dialog(file, this);
		dialog.exec();
	}
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
	Q_UNUSED(event);

	setCursor(Qt::PointingHandCursor);
}
