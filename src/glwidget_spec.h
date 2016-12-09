#ifndef GLWIDGET_SPEC_H
#define GLWIDGET_SPEC_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>

#include "opengl/helpers.h"

class GLWidget_Spec : public QOpenGLWidget, protected QOpenGLFunctions
{
public:

	enum Mode {
		Diffuse,
		Bumpmap,
		Mask,
		None
	};

	GLWidget_Spec(QWidget *parent);

	~GLWidget_Spec();

	void initializeGL();

	void resizeGL(int w, int h);

	void paintGL();

	/*!
	 * Hides the widget if the texture file path is empty or can
	 * not be loaded.
	 */
	void updateValues( Mode mode, const QString& texture_ );

private:
	opengl::Offset offset;

	QOpenGLTexture *texture;
	QOpenGLTexture *textTexture;
};

#endif // GLWIDGET_SPEC_H
