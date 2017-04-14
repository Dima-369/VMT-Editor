#include "glwidget_diffuse1.h"

GLWidget_Diffuse1::GLWidget_Diffuse1(QWidget* parent) :
	QOpenGLWidget(parent),
	showDiffuse(false),
	showBumpmap(false),
	diffuseTexture(""),
	bumpmapTexture(""),
	alphaVisible(false),
	transparencyGroupVisible(false),
	enableAlphaTest(false),
	mAlpha(1.0f),
	mAlphaTestReference(0.7f),
	colorVisible(false),
	mRed(1.0f),
	mGreen(1.0f),
	mBlue(1.0f),
	mDiffuseTexture(0),
	mBumpmapTexture(0),
	mAlphaTexture(0),
	diffuseTextTexture(0),
	bumpmapTextTexture(0),
	textureTextTexture(0),
	shaderProgram(0)

{
	// required for Windows
	setAttribute(Qt::WA_DontCreateNativeAncestors);

	setMouseTracking(true);

	setVisible(false);
}

GLWidget_Diffuse1::~GLWidget_Diffuse1()
{
	makeCurrent();

	delete mDiffuseTexture;
	delete mBumpmapTexture;
	delete mAlphaTexture;

	delete diffuseTextTexture;
	delete bumpmapTextTexture;
	delete textureTextTexture;

	delete shaderProgram;

	doneCurrent();
}

void GLWidget_Diffuse1::reset()
{
	alphaVisible = false;
	transparencyGroupVisible = false;
	enableAlphaTest = false;
	mAlpha = 1.0f;
	mAlphaTestReference = 0.7f;
	mRed = 1.0f;
	mGreen = 1.0f;
	mBlue = 1.0f;
	colorVisible = false;
	showDiffuse = false;
	showBumpmap = false;
	mDiffuseTexture = NULL;
	mBumpmapTexture = NULL;
	diffuseTexture = "";
	bumpmapTexture = "";
	setVisible(false);
}

void GLWidget_Diffuse1::initializeGL()
{
	initializeOpenGLFunctions();

	opengl::initialize(this);

	mAlphaTexture = opengl::load(":/textures/alpha");
	diffuseTextTexture = opengl::load(":/overlays/diffuse1");
	bumpmapTextTexture = opengl::load(":/overlays/bump1");
	textureTextTexture = opengl::load(":/overlays/texture1");

	shaderProgram = new QOpenGLShaderProgram(this);

	if( !shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,
		"varying vec2 tex0;"
		"void main(void)"
		"{"
		"	tex0 = gl_MultiTexCoord0.st;"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"}") ) {

		fatalError(QString("Vertex shader error:\n\n%1")
			.arg(shaderProgram->log()));
	}

	if( !shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
		"#version 120 \n"
		"uniform sampler2D texture;"
		"uniform float alphaTest;"
		"uniform bool useAlphaTest = false;"
		"uniform float alpha;"
		"uniform bool transparent = false;"
		"uniform vec3 color;"
		"varying vec2 tex0;"
		"void main(void)"
		"{"
		"	vec4 tex = texture2D(texture, tex0.st);"
		"	if (useAlphaTest) {"
		"		if(tex.a < alphaTest) {"
		"			discard;"
		"		} else {"
		"			gl_FragColor = vec4(tex.rgb * color, 1.0);"
		"		}"
		"	} else {"
		"		if (transparent) {"
		"			gl_FragColor = vec4(tex.rgb * color, alpha * tex.a);"
		"		} else {"
		"			gl_FragColor = vec4(tex.rgb * color, alpha);"
		"		}"
		"	}"
		"}") ) {

		fatalError(QString("Fragment shader error:\n\n%1")
			.arg(shaderProgram->log()));
	}

	shaderProgram->link();
}

void GLWidget_Diffuse1::resizeGL(int w, int h)
{
	opengl::resize(w, h);

	offset = opengl::calculateOffset(offset.width, offset.height, w, h);

	update();
}

void GLWidget_Diffuse1::paintGL()
{
	opengl::clear(this);

	if (mDiffuseTexture == 0 && mBumpmapTexture == 0)
		return;

	if(alphaVisible || enableAlphaTest || mAlpha != 1.0) {
		opengl::drawQuad(offset, mAlphaTexture);
	}

	if(showDiffuse) {

		glEnable(GL_BLEND);

		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float a = 1.0f;

		if(colorVisible) {
			r = mRed;
			g = mGreen;
			b = mBlue;
		}

		if(transparencyGroupVisible)
			a = mAlpha;
		else
			a = 1.0f;

		shaderProgram->bind();
		shaderProgram->setUniformValue("alphaTest", mAlphaTestReference);
		shaderProgram->setUniformValue("useAlphaTest", enableAlphaTest);
		shaderProgram->setUniformValue("transparent", alphaVisible);
		shaderProgram->setUniformValue("alpha", a);
		shaderProgram->setUniformValue("color",	QVector3D(r, g, b) );
		opengl::drawQuad(offset, mDiffuseTexture, false);
		shaderProgram->release();


		if(showBumpmap) {
			glDisable(GL_BLEND);
			opengl::drawTopRightTriangle(offset, mBumpmapTexture);
		}

	} else {

		if(showBumpmap) {
			glDisable(GL_BLEND);
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

void GLWidget_Diffuse1::loadTexture(const QString &diffuse1,
		const QString &bumpmap1)
{
	if (diffuse1.isEmpty() && bumpmap1.isEmpty()) {
		//reset();
		return;
	}

	QImage image_diffuse;
	QImage image_bumpmap;

	showDiffuse = !diffuse1.isEmpty();
	showBumpmap = !bumpmap1.isEmpty();

	if (showDiffuse && !image_diffuse.load(diffuse1)) {
		//reset();
		return;
	}

	if (showBumpmap && !image_bumpmap.load(bumpmap1)) {
		//reset();
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

	// the order of showing the widget and loading the textures is very
	// important! This order currently works for Windows but not for Linux!
	setVisible(true);

	delete mDiffuseTexture;
	mDiffuseTexture = new QOpenGLTexture(image_diffuse.mirrored());

	delete mBumpmapTexture;
	mBumpmapTexture = new QOpenGLTexture(image_bumpmap.mirrored());

	update();
}

void GLWidget_Diffuse1::setAlphaTestReference(float testReference) {

	mAlphaTestReference = testReference;
	update();
}

void GLWidget_Diffuse1::setAlpha( float alpha ) {

	mAlpha = alpha;
	update();
}

void GLWidget_Diffuse1::setColor( float r, float g, float b ) {

	mRed = r;
	mGreen = g;
	mBlue = b;
	update();
}

void GLWidget_Diffuse1::setColorVisible(bool visible ) {

	colorVisible = visible;
	update();
}

void GLWidget_Diffuse1::setTransparencyVisible(bool visible) {

	transparencyGroupVisible = visible;
	update();
}

void GLWidget_Diffuse1::setAlphaVisible( bool visible ) {

	alphaVisible = visible;
	update();
}

void GLWidget_Diffuse1::setEnableAlphaTest(bool enable) {

	enableAlphaTest = enable;
	update();
}

void GLWidget_Diffuse1::mousePressEvent(QMouseEvent* event)
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

void GLWidget_Diffuse1::mouseMoveEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	setCursor(Qt::PointingHandCursor);
}
