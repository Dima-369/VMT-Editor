#include "glwidget_diffuse1.h"

#include "utilities.h"

#ifdef Q_OS_DARWIN
#   include "OpenGL/glu.h"
#else
#   include "GL/glu.h"
#endif

GLWidget_Diffuse1::GLWidget_Diffuse1(QWidget *parent) :
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

		Q("Vertex shader is broken:\n\n" + shaderProgram->log())
	}

	if( !shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
		"uniform sampler2D texture;"
		"uniform float alphaTest;"
		"varying vec2 tex0;"
		"void main(void)"
		"{"
		"	vec4 tex = texture2D(texture, tex0.st);"
		"	if(tex.a < alphaTest) {"
		"		discard;"
		"	} else {"
		"		gl_FragColor = vec4("
		"			texture2D(texture, tex0).rgb, 1.0);"
		"	}"
		"}") ) {

		Q("Fragment shader is broken:\n\n" + shaderProgram->log())
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

	if(alphaVisible || enableAlphaTest) {
		opengl::drawQuad(width(), height(), mAlphaTexture);
	}

	if(enableAlphaTest) {

		if(showDiffuse) {
			shaderProgram->bind();
			shaderProgram->setUniformValue("alphaTest",
				mAlphaTestReference);
			opengl::drawQuad(offset, mDiffuseTexture);
			shaderProgram->release();
		}

		if(showBumpmap) {
			glDisable(GL_BLEND);
			opengl::drawTopRightTriangle(offset, mBumpmapTexture);
		}

	} else {

		if(showDiffuse) {

			if(alphaVisible) {
				glEnable(GL_BLEND);
			} else {
				glDisable(GL_BLEND);
			}

			if(transparencyGroupVisible)
				glColor4f( 1.0f, 1.0f, 1.0f, mAlpha );
			else
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			bool setColor = false;

			opengl::drawQuad(offset, mDiffuseTexture,
				setColor);

			glDisable(GL_BLEND);

			if(showBumpmap) {
				opengl::drawTopRightTriangle(offset,
					mBumpmapTexture, setColor);
			}

		} else {

			if(showBumpmap) {
				opengl::drawQuad(offset, mBumpmapTexture);
			}
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
		qDebug() << "Both textures are empty?";
		reset();
		return;
	}

	QImage image_diffuse;
	QImage image_bumpmap;

	showDiffuse = !diffuse1.isEmpty();
	showBumpmap = !bumpmap1.isEmpty();

	if (showDiffuse && !image_diffuse.load(diffuse1)) {
		reset();
		return;
	}

	if (showBumpmap && !image_bumpmap.load(bumpmap1)) {
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
