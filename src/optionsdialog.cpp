#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "editshaderdialog.h"
#include "utilities.h"

OptionsDialog::OptionsDialog(QWidget* parent) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::OptionsDialog),
	mIniSettings(NULL),
	mSettings(NULL),
	mUpdateCustomShaders(false),
	frameColor("252525"),
	activeColor("333"),
	hoverColor("303030")
{
	ui->setupUi(this);

	connect( this, SIGNAL( accepted() ), this, SLOT( saveSettings() ));
	connect( ui->pushButton_editShaders, SIGNAL( released() ), this, SLOT( displayEditShaderDialog() ));
}

OptionsDialog::~OptionsDialog()
{
	delete ui;
}

void OptionsDialog::parseSettings( QSettings* iniSettings, Settings* settings )
{
	mSettings = settings;
	mIniSettings = iniSettings;


	ui->checkBox_saveLastGame->setChecked(settings->saveLastGame);

	ui->checkBox_fullPathFilenameInTitle->setChecked( settings->fullPathFilenameInWindowTitle );

	ui->checkBox_showShaderNameInTitle->setChecked( settings->showShaderNameInWindowTitle );

	ui->checkBox_autoRefresh->setChecked( settings->autoRefresh );

	if( settings->recentFileListStyle == Settings::FileMenu )
		ui->radioButton_recentFileListInMenu->setChecked(true);
	else
		ui->radioButton_recentFileListInSubmenu->setChecked(true);


	if( settings->recentFileEntryStyle == Settings::FullPath )
		ui->radioButton_recentFileEntriesFullPath->setChecked(true);
	else
		ui->radioButton_recentFileEntriesFileNameOnly->setChecked(true);

	//----------------------------------------------------------------------------------------//

	if( settings->parameterSortStyle == Settings::AlphabeticallySorted )
		ui->radioButton_alphabeticallySorted->setChecked(true);
	else
		ui->radioButton_groupedParameters->setChecked(true);

	ui->checkBox_showShaderNameInTab->setChecked( settings->showShaderNameInTab );

	ui->checkBox_useIndentation->setChecked( settings->useIndentation );

	ui->checkBox_useQuotesForTexture->setChecked( settings->useQuotesForTexture );

	updateCustomShaderStats();

	//----------------------------------------------------------------------------------------//

	ui->checkBox_clearCacheAfterExit->setChecked( settings->deleteCacheAfterApplicationExit );

	ui->spinBox_cacheSize->setValue( settings->cacheSize );
}

void OptionsDialog::updateCustomShaders()
{
	if(mUpdateCustomShaders)
	{
		mSettings->customShaders = mCustomShaders;
		mIniSettings->setValue( "shaders", QVariant( constructShaderString( mCustomShaders )));
	}
}

void OptionsDialog::updateCustomShaderStats()
{
	QVector< Shader > shaders = (mUpdateCustomShaders) ? mCustomShaders : mSettings->customShaders;

	int defaultShaders = 0, defaultActivated = 0, customShaders = 0, customActivated = 0;

	for( int i = 0; i < shaders.count(); ++i )
	{
		if( gShaders.contains( shaders.at(i).name ))
		{
			++defaultShaders;

			if( shaders.at(i).enabled )
				++defaultActivated;
		}
		else
		{
			++customShaders;

			if( shaders.at(i).enabled )
				++customActivated;
		}
	}

	ui->label_customShaderStats->setText( Str(customActivated) + "/" + Str(customShaders) + " enabled" );
	ui->label_defaultShaderStats->setText( Str(defaultActivated) + "/" + Str(defaultShaders) + " enabled" );
}

QString OptionsDialog::constructShaderString(const QVector<Shader> &groups)
{
	QString shaderString;

	for( int i = 0; i < groups.count(); ++i )
	{
		QString groupText;

		for( int j = 0; j < groups.at(i).groups.count(); ++j )
		{
			groupText.append( Str( groups.at(i).groups.at(j) ) + "?" );
		}

		shaderString.append( QString("%1?%2?%3?").arg( groups.at(i).name, groups.at(i).enabled ? "1" : "0", groupText ) );
	}

	return shaderString;
}

void OptionsDialog::saveSettings()
{
	if( ui->checkBox_saveLastGame->isChecked() )
	{
		if( !mSettings->saveLastGame )
		{
			mSettings->saveLastGame = true;
			mIniSettings->setValue( "saveLastGame", "1" );
		}
	}
	else
	{
		if( mSettings->saveLastGame )
		{
			mSettings->saveLastGame = false;
			mIniSettings->setValue( "", "0" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_showShaderNameInTitle->isChecked() )
	{
		if( !mSettings->showShaderNameInWindowTitle )
		{
			mSettings->showShaderNameInWindowTitle = true;
			mIniSettings->setValue( "showShaderNameInWindowTitle", "1" );

			emit optionChanged( Settings::_ShowShaderNameInWindowTitle, "1" );
		}
	}
	else
	{
		if(mSettings->showShaderNameInWindowTitle)
		{
			mSettings->showShaderNameInWindowTitle = false;
			mIniSettings->setValue( "showShaderNameInWindowTitle", "0" );

			emit optionChanged( Settings::_ShowShaderNameInWindowTitle, "0" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if (ui->checkBox_autoRefresh->isChecked()) {
		if (!mSettings->autoRefresh) {
			mSettings->autoRefresh = true;
			mIniSettings->setValue("autoRefresh", true);
			emit optionChanged(Settings::_AutoRefresh, "1");
		}
	} else {
		if(mSettings->autoRefresh) {
			mSettings->autoRefresh = false;
			mIniSettings->setValue("autoRefresh", false);
			emit optionChanged(Settings::_AutoRefresh, "0");
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_fullPathFilenameInTitle->isChecked() )
	{
		if( !mSettings->fullPathFilenameInWindowTitle )
		{
			mSettings->fullPathFilenameInWindowTitle = true;
			mIniSettings->setValue( "fullPathFilenameInWindowTitle", "1" );

			emit optionChanged( Settings::_FullPathFilenameInWindowTitle, "1" );
		}
	}
	else
	{
		if(mSettings->fullPathFilenameInWindowTitle)
		{
			mSettings->fullPathFilenameInWindowTitle = false;
			mIniSettings->setValue( "fullPathFilenameInWindowTitle", "0" );

			emit optionChanged( Settings::_FullPathFilenameInWindowTitle, "0" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->radioButton_recentFileListInMenu->isChecked() ) // File Menu
	{
		if( mSettings->recentFileListStyle == Settings::RecentFileMenu )
		{
			mSettings->recentFileListStyle = Settings::FileMenu;
			mIniSettings->setValue( "recentFileListStyle", "0" );

			emit optionChanged( Settings::_RecentFileListStyle, "0" );
		}
	}
	else // Recent File Menu
	{
		if( mSettings->recentFileListStyle == Settings::FileMenu )
		{
			mSettings->recentFileListStyle = Settings::RecentFileMenu;
			mIniSettings->setValue( "recentFileListStyle", "1" );

			emit optionChanged( Settings::_RecentFileListStyle, "1" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->radioButton_recentFileEntriesFileNameOnly->isChecked() ) // File Name
	{
		if( mSettings->recentFileEntryStyle == Settings::FullPath )
		{
			mSettings->recentFileEntryStyle = Settings::FileName;
			mIniSettings->setValue( "recentFileEntryStyle", "0" );

			emit optionChanged( Settings::_RecentFileEntryStyle, "0" );
		}
	}
	else // Full Path
	{
		if( mSettings->recentFileEntryStyle == Settings::FileName )
		{
			mSettings->recentFileEntryStyle = Settings::FullPath;
			mIniSettings->setValue( "recentFileEntryStyle", "1" );

			emit optionChanged( Settings::_RecentFileEntryStyle, "1" );
		}
	}

	//----------------------------------------------------------------------------------------//
	// Second Tab - Parameters
	//----------------------------------------------------------------------------------------//

	if( ui->radioButton_groupedParameters->isChecked() ) // Grouped
	{
		if( mSettings->parameterSortStyle == Settings::AlphabeticallySorted )
		{
			mSettings->parameterSortStyle = Settings::Grouped;
			mIniSettings->setValue( "parameterSortStyle", "0" );

			emit optionChanged( Settings::_ParameterSortStyle, "0" );
		}
	}
	else // AlphabeticallySorted
	{
		if( mSettings->parameterSortStyle == Settings::Grouped )
		{
			mSettings->parameterSortStyle = Settings::AlphabeticallySorted;
			mIniSettings->setValue( "parameterSortStyle", "1" );

			emit optionChanged( Settings::_ParameterSortStyle, "1" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_showShaderNameInTab->isChecked() )
	{
		if( !mSettings->showShaderNameInTab )
		{
			mSettings->showShaderNameInTab = true;
			mIniSettings->setValue( "showShaderNameInTab", "1" );

			emit optionChanged( Settings::_ShowShaderNameInTab, "1" );
		}
	}
	else
	{
		if( mSettings->showShaderNameInTab )
		{
			mSettings->showShaderNameInTab = false;
			mIniSettings->setValue( "showShaderNameInTab", "0" );

			emit optionChanged( Settings::_ShowShaderNameInTab, "0" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_useIndentation->isChecked() )
	{
		if( !mSettings->useIndentation )
		{
			mSettings->useIndentation = true;
			mIniSettings->setValue( "useIndentation", "1" );

			emit optionChanged( Settings::_UseIndentation, "1" );
		}
	}
	else
	{
		if( mSettings->useIndentation )
		{
			mSettings->useIndentation = false;
			mIniSettings->setValue( "useIndentation", "0" );

			emit optionChanged( Settings::_UseIndentation, "0" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_useQuotesForTexture->isChecked() )
	{
		if( !mSettings->useQuotesForTexture )
		{
			mSettings->useQuotesForTexture = true;
			mIniSettings->setValue( "useQuotesForTexture", "1" );

			emit optionChanged( Settings::_UseQuotesForTexture, "1" );
		}
	}
	else
	{
		if( mSettings->useQuotesForTexture )
		{
			mSettings->useQuotesForTexture = false;
			mIniSettings->setValue( "useQuotesForTexture", "0" );

			emit optionChanged( Settings::_UseQuotesForTexture, "0" );
		}
	}

	//----------------------------------------------------------------------------------------//
	// Third Tab - Cache
	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_clearCacheAfterExit->isChecked() )
	{
		if( !mSettings->deleteCacheAfterApplicationExit )
		{
			mSettings->deleteCacheAfterApplicationExit = true;
			mIniSettings->setValue( "clearCacheAfterExit", "1" );
		}
	}
	else
	{
		if( mSettings->deleteCacheAfterApplicationExit )
		{
			mSettings->deleteCacheAfterApplicationExit = false;
			mIniSettings->setValue( "clearCacheAfterExit", "0" );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( static_cast<int>(mSettings->cacheSize) != ui->spinBox_cacheSize->value() )
	{
		mSettings->cacheSize = ui->spinBox_cacheSize->value();
		mIniSettings->setValue( "cacheSize", Str( ui->spinBox_cacheSize->value() ));

		emit optionChanged( Settings::_CacheSize, Str( ui->spinBox_cacheSize->value() ));
	}
}

void OptionsDialog::displayEditShaderDialog()
{
	EditShaderDialog dialog(this);
	dialog.parseSettings( (mUpdateCustomShaders) ? mCustomShaders : mSettings->customShaders, mSettings->defaultShaders );

	dialog.show();

	switch( dialog.exec() )
	{
	case QDialog::Accepted:

		if (mSettings->customShaders != dialog.getShaders()) {

			mUpdateCustomShaders = true;
			mCustomShaders = dialog.getShaders();

		} else {

			mUpdateCustomShaders = false;
		}

		updateCustomShaderStats();
	}
}
