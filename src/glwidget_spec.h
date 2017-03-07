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
#include "texturepreviewdialog.h"

class GLWidget_Spec : public QOpenGLWidget, protected QOpenGLFunctions
{
public:

	enum Mode {
		Diffuse,
		Bumpmap,
		Mask,
		None
	};

	GLWidget_Spec(QWidget* parent);

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

	void mousePressEvent(QMouseEvent* event) override;

private:
	opengl::Offset offset;

	QOpenGLTexture *texture;
	QOpenGLTexture *textTexture;

	// QOpenGLTexture does not store it and we need this for the preview
	QString file;

};
