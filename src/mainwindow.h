#pragma once

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QLabel>
#include <QSettings>
#include <QSpacerItem>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QProcess>


#ifdef Q_OS_WIN
#   include <QWinJumpList>
#   include <QWinJumpListCategory>
#   include <QWinJumpListItem>
#endif

#include "vmtparser.h"
#include "glwidget.h"
#include "glwidget_diffuse1.h"
#include "glwidget_diffuse2.h"
#include "glwidget_spec.h"
#include "texturethread.h"
#include "optionsdialog.h"
#include "tintslider.h"
#include "utilities/strings.h"
#include "utilities/version.h"


namespace Ui {

	class MainWindow;
}

class VmtParser;
struct VmtFile;


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	enum GroupBoxes {
		BaseTexture,
		BaseTexture2,
		BaseTexture3,
		BaseTexture4,
		Transparency,
		DetailTexture,
		Color,
		OtherTexture,
		PhongBrush,
		Phong,
		Reflection,
		SelfIllumination,
		RimLight,
		Water,
		WaterFlowmap,
		WaterReflection,
		WaterRefraction,
		Fog,
		Scroll,
		BaseTextureTransforms,
		BumpMapTransforms,
		Misc,
		Patch,
		Refract,
		Sprite,
		UnlitTwoTexture,
		NormalBlend,
		TreeSway,
		LayerBlend,
		EmissiveBlend
	};

	MainWindow( QString fileToOpen = "", QWidget* parent = NULL );

	void parseVMT( VmtFile vmtFile, bool isTemplate = false );

	VmtFile makeVMT();

	// public for ConversionThread, the deleteFromCache parameter should be
	// set to true for calls from the ConversionThread
	void previewTexture(const QString& object);

	bool previewTexture( const QString& object, const QString& texture, bool basetexture, bool alpha, bool alphatest, bool alphaOnly, bool ignoreCache = false );

	// public for ClickableLabel
	Ui::MainWindow* ui;

	// public for ConversionThread
	QListWidget* mLogger;

protected:

	void closeEvent(QCloseEvent* event);

	/**
	 * Overridden to load the initial command line file.
	 *
	 * If we load it inside the constructor, we crash when the texture
	 * is previewed.
	 */
	void showEvent(QShowEvent *event);

	void dropEvent(QDropEvent* event);

	void dragEnterEvent(QDragEnterEvent* event);

	void dragMoveEvent(QDragMoveEvent* event);

	void dragLeaveEvent(QDragLeaveEvent* event);

private:

	VmtParser* vmtParser;

	QList<GLWidget*> glWidgets;

	GLWidget_Diffuse1* glWidget_diffuse1;
	GLWidget_Diffuse2* glWidget_diffuse2;
	GLWidget_Spec* glWidget_envmap;
	GLWidget_Spec* glWidget_spec;

	QString loadedVmtFile;

	// Game + Directory
	QMap<QString, QString> mAvailableGames;
	QMap<QString, QString> mAvailableGamesSave;

	bool mChildWidgetChanged;

	bool mLoading;

	bool mVMTLoaded;

	bool mExitRequested;

	bool mGameSelected;

	bool mIsResetting;

	bool mLoadedHdrVtf;

	bool mFirstColorDialog;

	bool mDisablePreview;

	bool mIsInConstructor;

	bool mParsingVMT;

	bool mPreviewChanged;

	QTextCursor mCursor;

	int mCursorPos;

	bool mShaderKnownButNotListed;

	QSpacerItem* mHorizontalTextureSpacer;

	QSpacerItem* mVerticalTextureSpacer;

	bool mIgnoreShaderChanged;

	bool mSteamInstalled;

	bool mOpenConvertDialog;

	QString fileToConvert;

	QCompleter* vmtPreviewCompleter;

	//----------------------------------------------------------------------------------------//

	QSettings* mIniSettings;

	QSettings* mIniPaths;

	Settings* mSettings;

	QString initialFile;

	//----------------------------------------------------------------------------------------//

	QString mSteamAppsDir;

	//----------------------------------------------------------------------------------------//

	enum { MaxRecentFiles = 9 };
	enum { MaxTemplates = 16 };

	QAction* recentFileActions[MaxRecentFiles];
	QAction* templateActions[MaxTemplates];

	QAction* separatorAct;
	QAction* separatorTemp;

	//----------------------------------------------------------------------------------------//

	QAction* transparencyAction;
	QAction* detailAction;
	QAction* colorAction;
	QAction* otherAction;

	QAction* reflectionAction;
	QAction* selfillumAction;
	QAction* phongAction;
	QAction* phongBrushAction;

	QAction* miscAction;

	QAction* waterFlowmapAction;
	QAction* waterReflectionAction;
	QAction* waterRefractionAction;
	QAction* waterFogAction;

	//----------------------------------------------------------------------------------------//

	double fresnelYStart;
	bool ignoreFresnelY;
	bool inverseFresnelY = false;

	//----------------------------------------------------------------------------------------//

	QString gameBeforeEditGames;

	//----------------------------------------------------------------------------------------//

	QMap<QLineEdit*, QString> texturesToCopy;

	//----------------------------------------------------------------------------------------//

	void setGameState(bool enabled);

	QString getCurrentGame();

	void setGames( const QStringList& gameList );

	void setCurrentGame( const QString& game );

	void resetWidgets();

	QString currentGameMaterialDir();

	QString steamAppsDirectory();

	QString validateTexture( QString objectName, QString vtf, const QString& command, QDir gameInfoDir );

	void checkCacheSize();

	bool isGroupboxChanged( GroupBoxes groupBox);

	//----------------------------------------------------------------------------------------//

	void changeColor( QToolButton* colorField );

	void changeColor( QToolButton *colorField, TintSlider* slider );

	bool previewTexture( const int type, const QString& texture );

	void saveSettings();

	void readSettings();

	void setCurrentFile( const QString& filename );

	void updateRecentFileActions( bool fullPath );

	// returns an empty string if the dialog was rejected
	QString launchBrowseVtfDialog(QLineEdit* lineEdit);

	// if textureFileName is empty, launchBrowseVtfDialog() will be called
	// this is used for the drag/drop functionality
	// launches the conversion thread and so on
	void processVtf(const QString& objectName,
		const QString& textureFileName, QLineEdit* lineEdit);

	bool transformsModified( uint index );

	QString removeSuffix( const QString fileName, int type = 0);

	QString outputParameters( int type, bool noAlpha );

	//----------------------------------------------------------------------------------------//

	void addGLWidget( const QString& textureOverlay, QVBoxLayout* layout, const QString& widgetObjectName );

	void loadScale( const QString& value, const QString& parameter, QDoubleSpinBox* spinBox1, QDoubleSpinBox* spinBox2 );

	QColor getBackgroundColor( QPlainTextEdit* colorWidget );

	void setBackgroundColor( const QColor& color, QPlainTextEdit* colorWidget );

	//----------------------------------------------------------------------------------------//

	bool loadBoolParameter( const QString& value, const QString& parameter );

	bool loadIntParameter( int* intValue, const QString& value, const QString& parameter );


	bool loadDoubleParameter( double* doubleValue, const QString& value, const QString& parameter );

	bool loadDoubleParameter( double* doubleValue, const QString& value, const QString& parameter, double defaultValue );

	void loadScrollParameter( QString value, const QString& command, uint index );

	void loadVMT( const QString& vmtPath);

	//----------------------------------------------------------------------------------------//

	bool _toInt( int* value, QString input );

	bool _toDouble( double* value, QString input );

	int _displaySaveMessage();

	//----------------------------------------------------------------------------------------//

	void clearCacheFolder();

	void sortShaderComboBox();

	void processTexturesToCopy( const QString& dir );

	//----------------------------------------------------------------------------------------//

	QString addDefaultShader( const QString& shaderName, bool enabled, Settings* settings, QVector< Shader::Groups > groups = QVector< Shader::Groups >() );

	void processShaderGroups( Shader::Groups groups );

	void updateWindowTitle();

public slots:

	void handleTextureDrop(const QString& filePath);

	void finishedLoading();

	void acceptedEditGames();

	void deniedEditGames();

	/*!
	 * mSettings->autoRefresh() needs to be ignored for the Proxies
	 * TextEdit, otherwise we get lots of errors when text is typed into
	 * the view because the text is validated.
	 */
	void widgetChanged();

	void refreshRequested();

	void clearMessageLog();

	void gameChanged( const QString& game );

	void shaderChanged();

	void changeShader();

	void openRecentFile();

	void openTemplate();

	void browseVTF();

	void alphaTestReferenceChanged();

	void detailTextureChanged( QString text );

	void requestedCubemap( bool enabled );

	void changedColor();

	void resetColor();

	void loadExternalVmt();

	void browseVMT();

	void modifiedLineEdit( QString text );

	void modifiedCheckBox( bool enabled );

	void refreshGameList();

	void refreshGameListSoft();

	void displayAboutDialog();

	void changeOption( Settings::Options option, const QString& value );

	void toggledDetailUniformScale(bool checked);

	// required for the conversion thread
	void updateTextureOnUi(
		const QString& objectName, const QString& relativeFilePath);

	void sortDroppedTextures( const QMimeData* mimeData );

	void paste();

	void createReconvertAction(QLineEdit* lineEdit, QString fileName);

private slots:

	void previewTexture();

	void browseToGameDirectory();

	void hideParameterGroupboxes();

	void displayOptionsDialog();

	void displayConversionDialogTexture(QString file);

	void displayConversionDialog();

	void displayBatchDialog();

	void gameTriggered( bool triggered );

	void showEditGamesDialog();

	void opacityChanged( double value );

	void colorChanged();

	void fresnelSliderEdited( int value );

	void fresnelYEdited( double value );

	void addCSGOParameter(QString value, VmtFile& vmt, QString string, QDoubleSpinBox* doubleSpinBox);

	void openReconvertDialogAction();

	void reconvertTexture();

	void reconvertTextureHalf();

	void reconvertTexture(QLineEdit* lineEdit, const QString& objectName, const QString& tooltip, const QString& resize);

	void clearLineEdit();

	void checkForUpdates();

	void checkForUpdatesSilent();

	void saveAsTemplate();

	void notifyOnNewVersion(QString version); 

	void vmtPreviewParse();

	void vmtPreviewChanged();

	void reconvertAll();

	void createBlendToolTexture();

	bool combineMaps(QLineEdit *lineEditBase, QLineEdit *lineEditAlpha);

	void refreshInGame();

	//----------------------------------------------------------------------------------------//

	void action_New();
	void action_Open();
	void action_Save();
	QString action_saveAs();

	void action_RefreshTemplateList();

	void toggleTransparency();
	void toggleDetailTexture();
	void toggleColor();
	void toggleOther();
	//---------------------------//
	void togglePhong();
	void togglePhongBrush();
	void toggleReflection();
	void toggleSelfIllumination();
	void toggleMisc();
	//--------------------------//
	void toggleFlowmap();
	void toggleWaterReflection();
	void toggleWaterRefraction();
	void toggleWaterFog();

	void on_action_baseTexture2_triggered(bool checked);
	void on_action_transparency_triggered(bool checked);
	void on_action_detail_triggered(bool checked);
	void on_action_other_triggered(bool checked);
	void on_action_reflection_triggered(bool checked);
	void on_action_phong_triggered(bool checked);
	void on_action_phongBrush_triggered(bool checked);
	void on_action_selfIllumination_triggered(bool checked);
	void on_action_rimLight_triggered(bool checked);
	void on_action_flowmap_triggered(bool checked);
	void on_action_waterReflection_triggered(bool checked);
	void on_action_refraction_triggered(bool checked);
	void on_action_fog_triggered(bool checked);
	void on_action_baseTextureTransforms_triggered(bool checked);
	void on_action_bumpmapTransforms_triggered(bool checked);
	void on_action_color_triggered(bool checked);
	void on_action_misc_triggered(bool checked);
	void on_action_scroll_triggered(bool checked);
	void on_action_baseTexture_triggered(bool checked);
	void on_action_refract_triggered(bool checked);
	void on_action_sprite_triggered(bool checked);
	void on_action_unlitTwoTexture_triggered(bool checked);
	void on_action_water_triggered(bool checked);
	void on_action_refract_2_triggered(bool checked);
	void on_action_normalBlend_triggered(bool checked);
	void on_action_treeSway_triggered(bool checked);
	void on_action_decal_triggered(bool checked);
	void on_action_layerBlend_triggered(bool checked);
	void on_action_emissiveBlend_triggered(bool checked);
};

//----------------------------------------------------------------------------//

// Needed in this header file as declaring ParameterLineEdit or ValueLineEdit
// in another header will result in an undefined type error of Ui::MainWindow

class ParameterLineEdit : public QLineEdit
{
	Q_OBJECT

public:

	explicit ParameterLineEdit(QWidget* parent = NULL);

public slots:

	void _editingFinished();

private:
	
	QCompleter* completer;
};

//----------------------------------------------------------------------------------------//

// Basically exactly the same thing as ParameterLineEdit

class ValueLineEdit : public QLineEdit
{
	Q_OBJECT

public:

	explicit ValueLineEdit( QWidget* parent = NULL );

public slots:

	void _editingFinished();
};
