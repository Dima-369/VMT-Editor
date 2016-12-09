#ifndef GLWIDGET_DIFFUSE2_H
#define GLWIDGET_DIFFUSE2_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>

#include "opengl/helpers.h"

/*!
 * Displays a diffuse and bumpmap texture in a split view if both are loaded.
 */
class GLWidget_Diffuse2 : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
	GLWidget_Diffuse2(QWidget *parent);

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

#endif // GLWIDGET_DIFFUSE1_H
