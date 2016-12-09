#ifndef HELPERS_H
#define HELPERS_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QImage>

namespace opengl {

/*!
 * Loads the passed file path into an OpenGL texture and returns it.
 *
 * Make sure that the passed file path exists and is a texture as this method
 * does not validate the loaded image.
 */
QOpenGLTexture* load(const QString &filePath);

/*!
 * Required parameters to correctly align the quad or triangle in rendering.
 *
 * This is required as we do not render the full viewport on images with
 * an aspect ratio different from 1:1.
 *
 * Width and height refer to the actual size of the image, not the viewport.
 */
struct Offset
{
	double xPos;
	double yPos;
	double width;
	double height;
};

/*!
 * Calculates the offset of the image relative to the viewport so images with
 * an aspect ratio outside of 1:1 are correctly displayed.
 *
 * You have to recalculate the offset in resizeGL() and on loading a new image.
 */
Offset calculateOffset(int imageW, int imageH, int viewPortW, int viewPortH);

/*!
 * Prepares the clear color and enables/disables flags related to the texture
 * preview.
 *
 * Call from inside initializeGL() in the subclassed OpenGL widgets.
 */
void initialize(QOpenGLFunctions *f);

/*!
 * Clears the color and depth buffer.
 *
 * Call as the first method in paintGL().
 */
void clear(QOpenGLFunctions *f);

/*!
 * Adjusts the viewport and loads some identity matrices for the camera.
 *
 * Call as the first method in resizeGL().
 */
void resize(int w, int h);

/*!
 * Draws a textured triangle having its corner in the bottom left corner of the
 * viewport.
 *
 * This usually corresponds to the diffuse texture.
 */
void drawBottomLeftTriangle(const Offset &offset, QOpenGLTexture *texture,
	bool setColor = true);

/*!
 * Draws a textured triangle having its corner in the top right corner of the
 * viewport.
 *
 * This usually corresponds to the bumpmap texture.
 */
void drawTopRightTriangle(const Offset &offset, QOpenGLTexture *texture,
	bool setColor = true);

/*!
 * Draws a textured quad with the passed with and height.
 *
 * Call from paintGL().
 */
void drawQuad(int w, int h, QOpenGLTexture *texture);

/*!
 * Draws a textured quad with the passed with and height, correctly aligned with
 * the passed offset.
 *
 * The offset is required for images whose aspect ratio is not 1:1.
 *
 * Call from paintGL().
 */
void drawQuad(const Offset &offset, QOpenGLTexture *texture,
	bool setColor = true);

} // namespace opengl

#endif // HELPERS_H
