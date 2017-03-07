#include "glwidget_spec.h"

GLWidget_Spec::GLWidget_Spec(QWidget* parent) :
	QOpenGLWidget(parent),
	texture(0),
	textTexture(0)
{
	setObjectName("preview_spec1");

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

void GLWidget_Spec::mousePressEvent(QMouseEvent* event)
{
	Q_UNUSED(event)

	if (!file.isEmpty()) {
		TexturePreviewDialog dialog(file, this);
		dialog.exec();
	}
}
