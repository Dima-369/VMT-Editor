#pragma once

#include <QGLShader>
#include <QGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

#ifdef Q_OS_DARWIN
#   include "OpenGL/glu.h"
#else
#   include "GL/glu.h"
#endif

#include "utilities.h"
#include "opengl/helpers.h"

/*!
 * Complex OpenGL widget to preview diffuse and bumpmap textures and have lots
 * of alpha settings.
 */
class GLWidget_Diffuse1 : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
	GLWidget_Diffuse1(QWidget* parent);

	~GLWidget_Diffuse1();

	void initializeGL();

	void resizeGL(int w, int h);

	void paintGL();

	/*!
	 * Hides the widget if both file paths are empty or they can not be
	 * loaded.
	 */
	void loadTexture(const QString &diffuse1, const QString &bumpmap1);

	// 0.7 is the default for all diffuse textures
	void setAlphaTestReference(float testReference);

	void setAlpha(float alpha);

	void setTransparencyVisible(bool visible);

	void setAlphaVisible(bool visible);

	void setEnableAlphaTest(bool enable);

	/**
	 * Hides the widget and unloads all textures.
	 *
	 * Call from MainWindow::resetWidgets().
	 */
	void reset();

	QString getDiffuse() {
		return diffuseTexture;
	}

	QString getBumpmap() {
		return bumpmapTexture;
	}

	bool isPreviewing() {
		return showDiffuse || showBumpmap;
	}

private:
	bool showDiffuse;
	bool showBumpmap;

	QString diffuseTexture;
	QString bumpmapTexture;

	bool alphaVisible;
	bool transparencyGroupVisible;
	bool enableAlphaTest;

	float mAlpha;
	float mAlphaTestReference;

	opengl::Offset offset;

	QOpenGLTexture *mDiffuseTexture;
	QOpenGLTexture *mBumpmapTexture;
	QOpenGLTexture *mAlphaTexture;

	QOpenGLTexture *diffuseTextTexture;
	QOpenGLTexture *bumpmapTextTexture;
	QOpenGLTexture *textureTextTexture;

	QOpenGLShaderProgram *shaderProgram;
};
