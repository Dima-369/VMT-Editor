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

private slots:

	void setTemplate();

	void convertRequested();

	void addRequested();

	void removeRequested();

	void clearRequested();

	void convertAskModeChanged();

	void resetWidgets();
};
