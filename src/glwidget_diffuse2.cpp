#include "glwidget_diffuse2.h"

GLWidget_Diffuse2::GLWidget_Diffuse2(QWidget* parent) :
	QOpenGLWidget(parent),
	showDiffuse(false),
	showBumpmap(false),
	diffuseTexture(""),
	bumpmapTexture(""),
	mDiffuseTexture(0),
	mBumpmapTexture(0),
	mAlphaTexture(0),
	diffuseTextTexture(0),
	bumpmapTextTexture(0),
	textureTextTexture(0)
{
	// required for Windows
	setAttribute(Qt::WA_DontCreateNativeAncestors);

	setMouseTracking(true);

	setVisible(false);
}

GLWidget_Diffuse2::~GLWidget_Diffuse2()
{
	makeCurrent();

	delete mDiffuseTexture;
	delete mBumpmapTexture;
	delete mAlphaTexture;

	delete diffuseTextTexture;
	delete bumpmapTextTexture;
	delete textureTextTexture;

	doneCurrent();
}

void GLWidget_Diffuse2::reset()
{
	showDiffuse = false;
	showBumpmap = false;
	diffuseTexture = "";
	bumpmapTexture = "";
	mDiffuseTexture = NULL;
	mBumpmapTexture = NULL;
	setVisible(false);
}

void GLWidget_Diffuse2::initializeGL()
{
	initializeOpenGLFunctions();

	opengl::initialize(this);

	mAlphaTexture = opengl::load(":/textures/alpha");
	diffuseTextTexture = opengl::load(":/overlays/diffuse2");
	bumpmapTextTexture = opengl::load(":/overlays/bump2");
	textureTextTexture = opengl::load(":/overlays/texture2");
}

void GLWidget_Diffuse2::resizeGL(int w, int h)
{
	opengl::resize(w, h);

	offset = opengl::calculateOffset(offset.width, offset.height, w, h);

	update();
}

void GLWidget_Diffuse2::paintGL()
{
	opengl::clear(this);

	if (mDiffuseTexture == 0 && mBumpmapTexture == 0)
		return;

	glDisable(GL_BLEND);

	if(showDiffuse) {

		if (showBumpmap) {
			opengl::drawBottomLeftTriangle(offset, mDiffuseTexture);
			opengl::drawTopRightTriangle(offset, mBumpmapTexture);

		} else {
			opengl::drawQuad(offset, mDiffuseTexture);
		}

	} else {

		if(showBumpmap) {
			opengl::drawQuad(offset, mBumpmapTexture);
		}
	}
	glEnable(GL_BLEND);

	QOpenGLTexture *texture;

	if(showDiffuse && showBumpmap) texture = textureTextTexture;
	else if(showDiffuse) texture = diffuseTextTexture;
	else texture = bumpmapTextTexture;

	opengl::drawQuad(width(), height(), texture);
}

void GLWidget_Diffuse2::loadTexture(const QString &diffuse1,
		const QString &bumpmap1)
{
	if (diffuse1.isEmpty() && bumpmap1.isEmpty()) {
		reset();
		return;
	}

	QImage image_diffuse;
	QImage image_bumpmap;

	showDiffuse = !diffuse1.isEmpty();
	showBumpmap = !bumpmap1.isEmpty();

	if (showDiffuse && !image_diffuse.load(diffuse1)) {
		qDebug() << "Could not load " << diffuse1;
		reset();
		return;
	}

	if (showBumpmap && !image_bumpmap.load(bumpmap1)) {
		qDebug() << "Could not load " << bumpmap1;
		reset();
		return;
	}

	diffuseTexture = diffuse1;
	bumpmapTexture = bumpmap1;

	QImage t;

	if (!diffuse1.isEmpty()) {
		t = image_diffuse;
	} else {
		t = image_bumpmap;
	}

	offset = opengl::calculateOffset(t.width(), t.height(), width(),
		height());

	// The order of showing the widget and loading the textures is very
	// important! This order currently works for Windows but not for Linux!
	setVisible(showDiffuse || showBumpmap);

	if (showDiffuse) {
		delete mDiffuseTexture;
		mDiffuseTexture = opengl::load(diffuse1);
	}

	delete mBumpmapTexture;
	mBumpmapTexture = new QOpenGLTexture(image_bumpmap);

	update();
}

void GLWidget_Diffuse2::mousePressEvent(QMouseEvent* event)
{
	// set exclusively
	bool previewDiffuse = false;
	bool previewBumpmap = false;

	if (showDiffuse && showBumpmap) {
		// (0,0) is in the top left corner from the event
		const bool lowerLeft = (event->x() <= event->y());
		if (lowerLeft) {
			previewDiffuse = true;
		} else {
			previewBumpmap = true;
		}

	} else if (showDiffuse) {
		previewDiffuse = true;
	} else { // showing bumpmap
		previewBumpmap = true;
	}

	if (previewDiffuse) {
		TexturePreviewDialog dialog(diffuseTexture, this);
		dialog.exec();
	}
	if (previewBumpmap) {
		TexturePreviewDialog dialog(bumpmapTexture, this);
		dialog.exec();
	}
}

void GLWidget_Diffuse2::mouseMoveEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	setCursor(Qt::PointingHandCursor);
}
