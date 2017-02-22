#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <qdebug.h>

#ifdef Q_OS_DARWIN
#   include "OpenGL/glu.h"
#else
#   include "GL/glu.h"
#endif

#include "opengl/helpers.h"

class MainWindow;

class GLWidget_Spec : public QOpenGLWidget, protected QOpenGLFunctions
{
public:

	enum Mode {
		Diffuse,
		Bumpmap,
		Mask,
		None
	};

	GLWidget_Spec(MainWindow* mainWindow);

	~GLWidget_Spec();

	void initializeGL();

	void resizeGL(int w, int h);

	void paintGL();

	/*!
	 * Hides the widget if the texture file path is empty or can
	 * not be loaded.
	 */
	void updateValues(Mode mode, const QString& texture_);

protected:

	void dropEvent(QDropEvent* event);

	void dragEnterEvent(QDragEnterEvent* event);

	void dragMoveEvent(QDragMoveEvent* event);

	void dragLeaveEvent(QDragLeaveEvent* event);

private:
	MainWindow* mainWindow;

	opengl::Offset offset;

	QOpenGLTexture *texture;
	QOpenGLTexture *textTexture;
};
