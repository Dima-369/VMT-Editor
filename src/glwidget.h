#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QTimer>
#include <QMimeData>
#include <QDragEnterEvent>
#include <qdebug.h>

#ifdef Q_OS_DARWIN
#   include "OpenGL/glu.h"
#else
#   include "GL/glu.h"
#endif

#include "opengl/helpers.h"

class MainWindow;

/*!
 * Displays a single texture with the passed overlay texture from
 * the constructor.
 */
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
	GLWidget(const QString &overlayTexture, const QString &objectName,
		MainWindow *mainWindow, QWidget* parent);

	~GLWidget();

	void initializeGL();

	void resizeGL(int w, int h);

	void paintGL();

	/*!
	 * Hides the widget if the file is empty or can not be loaded.
	 */
	void loadTexture(const QString &image);

	bool isPreviewing() const {
		return isShowing;
	}

protected:

	void dropEvent(QDropEvent* event);

	void dragEnterEvent(QDragEnterEvent* event);

	void dragMoveEvent(QDragMoveEvent* event);

	void dragLeaveEvent(QDragLeaveEvent* event);

private:
	MainWindow* mainWindow;

	bool isShowing;

	opengl::Offset offset;

	QOpenGLTexture *texture;
	QOpenGLTexture *textTexture;

	QString overlay;
};
