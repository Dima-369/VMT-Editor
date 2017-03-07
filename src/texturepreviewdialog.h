#pragma once

#include <QPixmap>
#include <QDesktopWidget>
#include <QSize>

#include "ui_texturepreviewdialog.h"


namespace Ui {
	class TexturePreviewDialog;
}

/**
 * Displays a passed texture in a frameless dialog which is closed by click.
 *
 * Only actual texture files are supported, do not try to display a VTF but
 * convert it first.
 */
class TexturePreviewDialog : public QDialog
{
	Q_OBJECT

public:
	explicit TexturePreviewDialog(const QString& file, QWidget* parent);

	~TexturePreviewDialog();

protected:

	void mousePressEvent(QMouseEvent* event) override;

private:

	Ui::TexturePreviewDialog* ui;
};
