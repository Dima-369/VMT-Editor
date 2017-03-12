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
	
private:

	Ui::ConversionDialog* ui;

	QStringList listEntriesWithDirectories;

	bool compareImages( const QImage& image1, const QImage& image2 );

	QSettings* settings;

	bool removeSuffix = false;

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
