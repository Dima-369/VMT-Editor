#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <qdebug.h>

#ifdef Q_OS_DARWIN
#   include "OpenGL/glu.h"
#else
#   include "GL/glu.h"
#endif

#include "opengl/helpers.h"
#include "texturepreviewdialog.h"

/*!
 * Displays a diffuse and bumpmap texture in a split view if both are loaded.
 */
class GLWidget_Diffuse2 : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
	GLWidget_Diffuse2(QWidget* parent);

	~GLWidget_Diffuse2();

	void initializeGL();

	void resizeGL(int w, int h);

	void paintGL();

	/*!
	 * Hides the widget if the file is empty or can not be loaded.
	 */
	void loadTexture(const QString &diffuse1, const QString &bumpmap1);

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

protected:

	void mousePressEvent(QMouseEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

private:
	bool showDiffuse;
	bool showBumpmap;

	QString diffuseTexture;
	QString bumpmapTexture;

	opengl::Offset offset;

	QOpenGLTexture *mDiffuseTexture;
	QOpenGLTexture *mBumpmapTexture;
	QOpenGLTexture *mAlphaTexture;

	QOpenGLTexture *diffuseTextTexture;
	QOpenGLTexture *bumpmapTextTexture;
	QOpenGLTexture *textureTextTexture;
};
