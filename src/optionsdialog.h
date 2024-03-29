#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "dialogwithouthelpbutton.h"
#include "editshaderdialog.h"

#include <QSettings>
#include <QDir>

/**
 * The struct is initialized inside MainWindow::readSettings().
 */
struct Settings
{
	// Used for notifying the MainWindow of changed options
	enum Options
	{
		_FullPathFilenameInWindowTitle,
		_ShowShaderNameInWindowTitle,
		_RecentFileListStyle,
		_RecentFileEntryStyle,
		_AutoRefresh,
		_RemoveSuffix,
		_RemoveAlpha,
		_TemplateNew,
		_CheckForUpdates,
		_AutoSave,
		//----------------------------------------------------------------------------------------//
		_ParameterSortStyle,
		_ShowShaderNameInTab,
		_UseIndentation,
		_UseQuotesForTexture,
		_CustomShaders,
		//----------------------------------------------------------------------------------------//
		_CacheSize,
		_ChangeName
	};


	enum RecentFileListStyle
	{
		FileMenu,
		RecentFileMenu
	};

	enum RecentFileEntryStyle
	{
		FileName,
		FullPath
	};
	//
	// 0 = disable, 1 = enable, [game name] = only available on 1, stores last game
	bool saveLastGame; 

	QString lastGame;

	QString diffuseSuffix;
	QString bumpSuffix;
	QString specSuffix;
	QString glossSuffix;
	QString tintmaskSuffix;

	bool fullPathFilenameInWindowTitle;

	bool showShaderNameInWindowTitle;

    bool autoRefresh;

	bool removeSuffix;

	bool removeAlpha;

	bool templateNew;

	bool autoSave;

	bool checkForUpdates;

	bool changeName;

	bool noNormalSharpen;

	bool uncompressedNormal;

	bool noGlossMip;

    bool newVtflib;

	QString mipmapFilter;
	QString mipmapSharpenFilter;

	RecentFileListStyle recentFileListStyle;

    RecentFileEntryStyle recentFileEntryStyle = Settings::FileName;

	//----------------------------------------------------------------------------------------//

	enum ParameterSortStyle
	{
		Grouped,
		AlphabeticallySorted
	};

	ParameterSortStyle parameterSortStyle;

	bool showShaderNameInTab;

	bool useIndentation;

	bool useQuotesForTexture;

	QVector<Shader> customShaders;

	QVector<Shader> defaultShaders; // used for resetting shaders

	//----------------------------------------------------------------------------------------//

	bool deleteCacheAfterApplicationExit;

	int cacheSize;
};


namespace Ui
{
	class OptionsDialog;
}

class OptionsDialog : public DialogWithoutHelpButton
{
	Q_OBJECT
	
public:

	explicit OptionsDialog( QWidget* parent = NULL );

	~OptionsDialog();

	void parseSettings( QSettings* iniSettings, Settings* settings );

	void updateCustomShaders();

	void updateCustomShaderStats();
	
private:

	Ui::OptionsDialog* ui;

	QSettings* mIniSettings;

	Settings* mSettings;

	bool mUpdateCustomShaders;

	QVector< Shader > mCustomShaders;

	QString constructShaderString( const QVector< Shader >& groups );

	QString frameColor;
	QString activeColor;
	QString hoverColor;

signals:

	// Used to notify the Mainwindow of changed options
	void optionChanged( Settings::Options, const QString& value );

private slots:

	void saveSettings();

	void displayEditShaderDialog();

	void associate();
};

#endif // OPTIONSDIALOG_H
