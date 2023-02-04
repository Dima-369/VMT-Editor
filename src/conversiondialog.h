#pragma once

#include "dialogwithouthelpbutton.h"

#include <QSettings>

namespace Ui {

	class ConversionDialog;
}

class ConversionDialog : public DialogWithoutHelpButton {

	Q_OBJECT
	
public:

	explicit ConversionDialog( QSettings* iniSettings, QWidget* parent = NULL);

	~ConversionDialog();

	void addFile(const QString& fileName);

protected:

	void dropEvent(QDropEvent* event);

	void dragEnterEvent(QDragEnterEvent* event);

	void dragMoveEvent(QDragMoveEvent* event);

	void dragLeaveEvent(QDragLeaveEvent* event);
	
private:

	Ui::ConversionDialog* ui;

	QStringList listEntriesWithDirectories;

	bool compareImages( const QImage& image1, const QImage& image2 );

	QSettings* settings;

	bool removeSuffix = false;

    bool newVtflib = false;

	QString diffuseSuffix;
	QString bumpSuffix;
	QString specSuffix;
	QString glossSuffix;

	QString mipmapFilter;
	QString mipmapSharpenFilter;

private slots:

	void setTemplate();

	void convertRequested();

	void addRequested();

	void removeRequested();

	void clearRequested();

	void convertAskModeChanged();

	void resetWidgets();

	void browseOutput();
};
