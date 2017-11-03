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
	
#ifndef Q_OS_WIN
	ui->groupBox_fileAssociation->hide();
#endif
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

	ui->checkBox_removeSuffix->setChecked( settings->removeSuffix );

	ui->checkBox_removeAlpha->setChecked( settings->removeAlpha );

	ui->checkBox_templateNew->setChecked( settings->templateNew );

	ui->checkBox_checkForUpdates->setChecked( settings->checkForUpdates );

	ui->checkBox_autoSave->setChecked( settings->autoSave );

	ui->checkBox_changeName->setChecked( settings->changeName );

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

	ui->checkBox_useQuotesForTexture->setChecked( !settings->useQuotesForTexture );

	updateCustomShaderStats();

	ui->lineEdit_bumpSuffix->setText( settings->bumpSuffix );
	ui->lineEdit_diffuseSuffix->setText( settings->diffuseSuffix );
	ui->lineEdit_specSuffix->setText( settings->specSuffix );
	ui->lineEdit_glossSuffix->setText( settings->glossSuffix );
	ui->lineEdit_tintmaskSuffix->setText( settings->tintmaskSuffix );

	ui->comboBox_mipmapFilter->setCurrentIndex(ui->comboBox_mipmapFilter->findText(settings->mipmapFilter, Qt::MatchFixedString));
	ui->comboBox_mipmapSharpenFilter->setCurrentIndex(ui->comboBox_mipmapSharpenFilter->findText(settings->mipmapSharpenFilter, Qt::MatchFixedString));

	ui->checkBox_noNormalSharpen->setChecked( settings->noNormalSharpen );
	ui->checkBox_uncompressedNormal->setChecked( settings->uncompressedNormal );
	ui->checkBox_noGlossMip->setChecked( settings->noGlossMip );

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

	if (ui->checkBox_templateNew->isChecked()) {
		if (!mSettings->templateNew) {
			mSettings->templateNew = true;
			mIniSettings->setValue("templateNew", true);
			emit optionChanged(Settings::_TemplateNew, "1");
		}
	} else {
		if(mSettings->templateNew) {
			mSettings->templateNew = false;
			mIniSettings->setValue("templateNew", false);
			emit optionChanged(Settings::_TemplateNew, "0");
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

	if (ui->checkBox_autoSave->isChecked()) {
		if (!mSettings->autoSave) {
			mSettings->autoSave = true;
			mIniSettings->setValue("autoSave", true);
			emit optionChanged(Settings::_AutoSave, "1");
		}
	} else {
		if(mSettings->autoSave) {
			mSettings->autoSave = false;
			mIniSettings->setValue("autoSave", false);
			emit optionChanged(Settings::_AutoSave, "0");
		}
	}

	//----------------------------------------------------------------------------------------//

	if (ui->checkBox_removeSuffix->isChecked()) {
		if (!mSettings->removeSuffix) {
			mSettings->removeSuffix = true;
			mIniSettings->setValue("removeSuffix", true);
			emit optionChanged(Settings::_RemoveSuffix, "1");
		}
	} else {
		if(mSettings->removeSuffix) {
			mSettings->removeSuffix = false;
			mIniSettings->setValue("removeSuffix", false);
			emit optionChanged(Settings::_RemoveSuffix, "0");
		}
	}

	//----------------------------------------------------------------------------------------//

	if (ui->checkBox_removeAlpha->isChecked()) {
		if (!mSettings->removeAlpha) {
			mSettings->removeAlpha = true;
			mIniSettings->setValue("removeAlpha", true);
			emit optionChanged(Settings::_RemoveAlpha, "1");
		}
	} else {
		if(mSettings->removeAlpha) {
			mSettings->removeAlpha = false;
			mIniSettings->setValue("removeAlpha", false);
			emit optionChanged(Settings::_RemoveAlpha, "0");
		}
	}

	if (ui->checkBox_checkForUpdates->isChecked()) {
		if (!mSettings->checkForUpdates) {
			mSettings->checkForUpdates = true;
			mIniSettings->setValue("checkForUpdates", true);
			emit optionChanged(Settings::_CheckForUpdates, "1");
		}
	} else {
		if(mSettings->checkForUpdates) {
			mSettings->checkForUpdates = false;
			mIniSettings->setValue("checkForUpdates", false);
			emit optionChanged(Settings::_CheckForUpdates, "0");
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
	// texture conversion options
	//----------------------------------------------------------------------------------------//
	if (ui->lineEdit_diffuseSuffix->text() != mSettings->diffuseSuffix) {
		mSettings->diffuseSuffix = ui->lineEdit_diffuseSuffix->text();
		mIniSettings->setValue( "diffuseSuffix", ui->lineEdit_diffuseSuffix->text() );
	}
	if (ui->lineEdit_bumpSuffix->text() != mSettings->bumpSuffix) {
		mSettings->bumpSuffix = ui->lineEdit_bumpSuffix->text();
		mIniSettings->setValue( "bumpSuffix", ui->lineEdit_bumpSuffix->text() );
	}
	if (ui->lineEdit_specSuffix->text() != mSettings->diffuseSuffix) {
		mSettings->specSuffix = ui->lineEdit_specSuffix->text();
		mIniSettings->setValue( "specSuffix", ui->lineEdit_specSuffix->text() );
	}
	if (ui->lineEdit_glossSuffix->text() != mSettings->glossSuffix) {
		mSettings->glossSuffix = ui->lineEdit_glossSuffix->text();
		mIniSettings->setValue( "glossSuffix", ui->lineEdit_glossSuffix->text() );
	}
	if (ui->lineEdit_tintmaskSuffix->text() != mSettings->tintmaskSuffix) {
		mSettings->tintmaskSuffix = ui->lineEdit_tintmaskSuffix->text();
		mIniSettings->setValue( "tintmaskSuffix", ui->lineEdit_tintmaskSuffix->text() );
	}


	if (ui->comboBox_mipmapFilter->currentText() != mSettings->mipmapFilter ) {
		mSettings->mipmapFilter = ui->comboBox_mipmapFilter->currentText();
		mIniSettings->setValue( "mipmapFilter", ui->comboBox_mipmapFilter->currentText() );
	}
	if (ui->comboBox_mipmapSharpenFilter->currentText() != mSettings->mipmapSharpenFilter ) {
		mSettings->mipmapSharpenFilter = ui->comboBox_mipmapSharpenFilter->currentText();
		mIniSettings->setValue( "mipmapSharpenFilter", ui->comboBox_mipmapSharpenFilter->currentText() );
	}

	if( ui->checkBox_changeName->isChecked() ) {
		if( !mSettings->changeName ) {
			mSettings->changeName = true;
			mIniSettings->setValue( "changeName", "1" );
		}
	} else {
		if(mSettings->changeName) {
			mSettings->changeName = false;
			mIniSettings->setValue( "changeName", "0" );
		}
	}

	if( ui->checkBox_noNormalSharpen->isChecked() ) {
		if( !mSettings->noNormalSharpen ) {
			mSettings->noNormalSharpen = true;
			mIniSettings->setValue( "noNormalSharpen", "1" );
		}
	} else {
		if(mSettings->noNormalSharpen) {
			mSettings->noNormalSharpen = false;
			mIniSettings->setValue( "noNormalSharpen", "0" );
		}
	}

	if( ui->checkBox_uncompressedNormal->isChecked() ) {
		if( !mSettings->uncompressedNormal ) {
			mSettings->uncompressedNormal = true;
			mIniSettings->setValue( "uncompressedNormal", "1" );
		}
	} else {
		if(mSettings->uncompressedNormal) {
			mSettings->uncompressedNormal = false;
			mIniSettings->setValue( "uncompressedNormal", "0" );
		}
	}

	if( ui->checkBox_noGlossMip->isChecked() ) {
		if( !mSettings->noGlossMip ) {
			mSettings->noGlossMip = true;
			mIniSettings->setValue( "noGlossMip", "1" );
		}
	} else {
		if(mSettings->noGlossMip) {
			mSettings->noGlossMip = false;
			mIniSettings->setValue( "noGlossMip", "0" );
		}
	}

	//----------------------------------------------------------------------------------------//
	// Parameters
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

	if( !ui->checkBox_useQuotesForTexture->isChecked() )
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

void OptionsDialog::associate()
{
	QWidget* caller = qobject_cast<QWidget *>( sender() );

	//pushButton_assignToContext
	//pushButton_changeIconVmt
	//pushButton_changeIconVtf
	//pushButton_undoFile

	QString exeDir = QDir::currentPath() + "/VMT_Editor.exe";

	MsgBox msgBox(this);
		   msgBox.setWindowTitle("Registry changes");
		   msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
		   msgBox.setDefaultButton( QMessageBox::No );
		   msgBox.setIconPixmap(QPixmap(":/icons/info_warning"));

		   msgBox.setText( "This action will change registry keys.\n"
						   "VMT Editor must be ran in Administrator mode.\n"
						   "Are you sure you want to continue?");

	if (msgBox.exec() != QMessageBox::Yes)
		return;

	//QSettings vmtFile("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.vmt\\OpenWithProgids", QSettings::NativeFormat);

	QSettings vmtFileAction("HKEY_CLASSES_ROOT\\VMTFile\\shell\\Open with VMT Editor\\command", QSettings::NativeFormat);

	QSettings vmtFileIcon("HKEY_CLASSES_ROOT\\VMTFile\\DefaultIcon", QSettings::NativeFormat);
	QSettings vmtFileIcon2("HKEY_CLASSES_ROOT\\Applications\\VMT_Editor.exe\\DefaultIcon", QSettings::NativeFormat);
	QSettings vmtFileIcon3("HKEY_CLASSES_ROOT\\.vmt\\DefaultIcon", QSettings::NativeFormat);
	QSettings vtfFileIcon("HKEY_CLASSES_ROOT\\Applications\\VTFEdit.exe\\DefaultIcon", QSettings::NativeFormat);
	QSettings vtfFileIcon2("HKEY_CLASSES_ROOT\\VTFFile\\DefaultIcon", QSettings::NativeFormat);

	QSettings progId("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.vmt\\UserChoice", QSettings::NativeFormat);

	QString progIdName = progId.value("ProgId", "VMTFile").toString();

	QSettings vmtFileAction2("HKEY_CLASSES_ROOT\\" + QDir::toNativeSeparators(progIdName) + "\\shell\\Open with VMT Editor\\command", QSettings::NativeFormat);

	if (caller->objectName() == "pushButton_assignToContext") {
		vmtFileAction.setValue("Default", "\"" + QDir::toNativeSeparators(exeDir) + "\" \"%1\"");
		vmtFileAction2.setValue("Default", "\"" + QDir::toNativeSeparators(exeDir) + "\" \"%1\"");
	}

	if (caller->objectName() == "pushButton_changeIconVmt") {
		vmtFileIcon.setValue("Default", QDir::toNativeSeparators(exeDir) + ",1");
		vmtFileIcon2.setValue("Default", QDir::toNativeSeparators(exeDir) + ",1");
		vmtFileIcon3.setValue("Default", QDir::toNativeSeparators(exeDir) + ",1");
	}

	if (caller->objectName() == "pushButton_changeIconVtf") {
		vtfFileIcon.setValue("Default", QDir::toNativeSeparators(exeDir) + ",2");
		vtfFileIcon2.setValue("Default", QDir::toNativeSeparators(exeDir) + ",2");
	}

}
