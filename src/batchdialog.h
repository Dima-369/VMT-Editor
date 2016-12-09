#ifndef BATCHDIALOG_H
#define BATCHDIALOG_H

#include "dialogwithouthelpbutton.h"
#include "vmtparser.h"

#include <QSettings>
#include <QFileDialog>

#ifdef Q_OS_WIN
#   include <windows.h>
#endif
namespace Ui {

	class BatchDialog;
}

class BatchDialog : public DialogWithoutHelpButton
{
	Q_OBJECT
	
public:

	explicit BatchDialog( const QMap<QString, QString>& availableGames, const VmtFile& VMTFile, QSettings* settings, QWidget* parent = NULL );

	~BatchDialog();
	
private:

	enum Parameters {

		Bumpmap = 0,
		SelfillumMask,
		PhongExponentTexture,
		ToolTexture,
		EnvmapMask
	};

	VmtFile VMTFile;

	Ui::BatchDialog* ui;

	QSettings* settings;

	QString bumpmap;
	QString selfillummask;
	QString phongexponenttexture;
	QString tooltexture;
	QString envmapmask;

	QMap<Parameters, QString> defaultParameters;

	bool basetextureOnly;

	QMap< QString, bool> listEntriesWithDirectories;

	QMap<Parameters, QString> suffixes;

	QMap<QString, QString> availableGames;
	
	static QListWidget* listWidget;

	//----------------------------------------------------------------------------------------//

	int countFilesToConvert();

	bool compareImages( const QImage& image1, const QImage& image2 );

	void rek( QList<QString>* input, QList<QString>* output );

	void resetVMTFile();

	QString isMaterialDirectory( const QString& fileName ) const;

	void _processParameter( const QString& parameter, QString fileName );

private slots:

	void batchRequested();

	void addRequested();

	void removeRequested();

	void clearRequested();

	void convertAskModeChanged();
};

#endif // BATCHDIALOG_H
