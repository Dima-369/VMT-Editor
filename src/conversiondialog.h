#ifndef CONVERSIONDIALOG_H
#define CONVERSIONDIALOG_H

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

	void addFile(QString fileName);
	
private:

	Ui::ConversionDialog* ui;

	QMap< QString, bool> listEntriesWithDirectories;

	int countImagesToConvert();

	bool compareImages( const QImage& image1, const QImage& image2 );

	QSettings* settings;

private slots:

	void convertRequested();

	void addRequested();

	void removeRequested();

	void clearRequested();

	void convertAskModeChanged();
};

#endif // CONVERSIONDIALOG_H
