#include "helpers.h"

#ifdef Q_OS_DARWIN
#   include "OpenGL/glu.h"
#else
#   include "GL/glu.h"
#endif

QOpenGLTexture* opengl::load(const QString &filePath)
{
	return new QOpenGLTexture(QImage(filePath).mirrored());
}

opengl::Offset opengl::calculateOffset(int iw, int ih, int w, int h)
{
	opengl::Offset pos;

	double aspectRatio = (double)(iw) / (double)(ih);

	if (iw == ih) {

		if(iw < w) {
			pos.width = pos.height = iw;
			pos.xPos = pos.yPos = (w - iw) / 2;
		} else {
			pos.width = pos.height = w;
			pos.xPos = pos.yPos = 0.0;
		}

	} else if (aspectRatio < 1.0) {

		if(iw < w && ih < h) {
			pos.width = iw;
			pos.height = ih;
			pos.xPos = (w - pos.width) / 2.0;
			pos.yPos = (h - pos.height) / 2.0;
		} else {
			pos.width = w * aspectRatio;
			pos.height = h;
			pos.xPos = (w - pos.width) / 2.0;
			pos.yPos = 0.0;
		}

	} else {
		if (iw < w && ih < h) {
			pos.width = iw;
			pos.height = ih;
			pos.xPos = (w - pos.width) / 2.0;
			pos.yPos = (h - pos.height) / 2.0;
		} else {
			pos.width = w;
			pos.height = h / aspectRatio;
			pos.xPos = 0.0;
			pos.yPos = (h - pos.height) / 2.0;
		}
	}

	return pos;
}

void opengl::initialize(QOpenGLFunctions *f)
{
	// Qt window color is set as the backbuffer clear color because
	// an image with an aspect ratio different from 1:1 will create borders
	// this is the same color as RGB(64, 64, 64)
	f->glClearColor(0.251f, 0.251f, 0.251f, 1.0f);

	f->glEnable(GL_TEXTURE_2D);

	// Enable tranparency for texture blending
	f->glEnable(GL_BLEND);
	f->glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	f->glDisable(GL_DEPTH_TEST);

	f->glEnable(GL_LINE_SMOOTH);

	// Best quality of color and texture coordinate
	f->glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}

void opengl::clear(QOpenGLFunctions *f)
{
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void opengl::resize(int w, int h)
{
	// Set the width and height of the viewport
	glViewport(0, 0, w, h);

	// Set the identity matrix for the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set orthographic viewing region to the bottom left corner
	gluOrtho2D(0, w, 0, h);

	// Set the identity matrix for the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void opengl::drawBottomLeftTriangle(const Offset &offset,
		QOpenGLTexture *texture, bool setColor)
{
	if (setColor)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	texture->bind();

	glBegin(GL_TRIANGLES);

	glTexCoord2i(0, 0);
	glVertex2i(offset.xPos, offset.yPos);

	glTexCoord2i(1, 0);
	glVertex2i(offset.xPos + offset.width, offset.yPos);

	glTexCoord2i(0, 1);
	glVertex2i(offset.xPos, offset.yPos + offset.height);

	glEnd();
}

void opengl::drawTopRightTriangle(const opengl::Offset &offset,
		QOpenGLTexture *texture, bool setColor)
{
	if (setColor)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	texture->bind();

	glBegin(GL_TRIANGLES);

	glTexCoord2i(0, 1);
	glVertex2i(offset.xPos, offset.yPos + offset.height);

	glTexCoord2i(1, 0);
	glVertex2i(offset.xPos + offset.width, offset.yPos);

	glTexCoord2i(1, 1);
	glVertex2i(offset.xPos + offset.width, offset.yPos + offset.height);

	glEnd();
}

void opengl::drawQuad(int w, int h, QOpenGLTexture *texture)
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	texture->bind();

	glBegin(GL_QUADS);

	glTexCoord2i(0, 0);
	glVertex2i(0, 0);

	glTexCoord2i(1, 0);
	glVertex2i(w, 0);

	glTexCoord2i(1, 1);
	glVertex2i(w, h);

	glTexCoord2i(0, 1);
	glVertex2i(0, h);

	glEnd();
}

void opengl::drawQuad(const opengl::Offset &pos, QOpenGLTexture *texture,
	bool setColor)
{
	if (setColor)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	texture->bind();

	glBegin(GL_QUADS);

	glTexCoord2i(0, 0);
	glVertex2i(pos.xPos, pos.yPos);

	glTexCoord2i(1, 0);
	glVertex2i(pos.xPos + pos.width, pos.yPos);

	glTexCoord2i(1, 1);
	glVertex2i(pos.xPos + pos.width, pos.yPos + pos.height);

	glTexCoord2i(0, 1);
	glVertex2i(pos.xPos, pos.yPos + pos.height);

	glEnd();
}
