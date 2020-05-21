#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "logging/logging.h"

#include "aboutdialog.h"
#include "editshaderdialog.h"
#include "shader.h"
#include "conversionthread.h"
#include "conversiondialog.h"
#include "batchdialog.h"
#include "messagebox.h"
#include "editgamesdialog.h"

#include "user-interface/phong.h"
#include "user-interface/normal-blend.h"
#include "user-interface/emissive-blend.h"
#include "user-interface/detail-texture.h"
#include "user-interface/shaders.h"
#include "user-interface/view-helper.h"
#include "user-interface/shading-reflection.h"
#include "user-interface/treesway.h"
#include "user-interface/layerblend.h"
#include "user-interface/errors.h"
#include "vmt/vmt-helper.h"
#include "utilities/window.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QProcess>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QCompleter>
#include <QSplashScreen>
#include <QTime>
#include <QUrl>
#include <QMimeData>
#include <QWidgetAction>
#include <QToolBar>

#ifdef Q_OS_WIN
#   include <Windows.h>
#	pragma comment(lib,"user32.lib")
#endif

// Used for Parameter- and Valuelineedits as they need to modify the layout often
Ui::MainWindow* gUi;

// needed for ParameterLineEdit, we do not want to load this twice
QStringList vmtParameters_;

using namespace utils;


MainWindow::MainWindow(QString fileToOpen, QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mChildWidgetChanged(false),
	mLoading(false),
	mVMTLoaded(false),
	mExitRequested(false),
	mGameSelected(false),
	mIsResetting(false),
	mFirstColorDialog(true),
	mDisablePreview(false),
	mIsInConstructor(true),
	mParsingVMT(false),
	mShaderKnownButNotListed(false),
	mIgnoreShaderChanged(true),
	mSteamInstalled(false),
	mOpenConvertDialog(false),
	mIniSettings(new QSettings("settings.ini", QSettings::IniFormat, this)),
	mIniPaths(new QSettings("paths.ini", QSettings::IniFormat, this)),
	mSettings(new Settings),
	initialFile(fileToOpen),
	mSteamAppsDir(steamAppsDirectory()),
	fresnelYStart(0.5),
	ignoreFresnelY(false)
{
	// this has to be set before setupUi()
	vmtParameters_ = extractLines(":/files/vmtParameters");

	ui->setupUi(this);
	gUi = ui;

	ui->tabWidget->setCurrentIndex(0);

	setAcceptDrops(true);

	QToolBar* toolBar = new QToolBar("toolbar", this);
		toolBar->setObjectName("toolbar");
		toolBar->setMovable(false);
		toolBar->setOrientation(Qt::Vertical);

	QList<QAction*> list;

	transparencyAction = new QAction( QIcon( ":/icons/trans"), "Transparency", this);
	detailAction = new QAction( QIcon( ":/icons/detail"), "Detail Texture", this);
	colorAction = new QAction( QIcon( ":/icons/color"), "Color", this);
	otherAction = new QAction( QIcon( ":/icons/other"), "Lighting", this);

	reflectionAction = new QAction( QIcon( ":/icons/ref"), "Reflection", this);
	selfillumAction = new QAction( QIcon( ":/icons/illu"), "Self Illumination", this);
	phongAction = new QAction( QIcon( ":/icons/phong"), "Specular", this);
	phongBrushAction = new QAction( QIcon( ":/icons/phong"), "Brush Specular", this);

	miscAction = new QAction( QIcon( ":/icons/misc"), "Flags", this);

	waterFlowmapAction = new QAction( QIcon( ":/icons/flowmap"), "Flowmap", this);
	waterReflectionAction = new QAction( QIcon( ":/icons/waterreflect"), "Reflection", this);
	waterRefractionAction = new QAction( QIcon( ":/icons/waterrefract"), "Refraction", this);
	waterFogAction = new QAction( QIcon( ":/icons/waterfog"), "Fog", this);

	QAction *filler = new QAction( QIcon( ":/icons/transparent"), "", this );
	filler->setDisabled(true);

	list.append(filler);

	list.append(transparencyAction);
		connect(transparencyAction, SIGNAL(triggered()), this, SLOT(toggleTransparency()));
	list.append(detailAction);
		connect(detailAction, SIGNAL(triggered()), this, SLOT(toggleDetailTexture()));
	list.append(colorAction);
		connect(colorAction, SIGNAL(triggered()), this, SLOT(toggleColor()));
	list.append(otherAction);
		connect(otherAction, SIGNAL(triggered()), this, SLOT(toggleOther()));

	list.append(waterFlowmapAction);
		connect(waterFlowmapAction, SIGNAL(triggered()), this, SLOT(toggleFlowmap()));
	list.append(waterReflectionAction);
		connect(waterReflectionAction, SIGNAL(triggered()), this, SLOT(toggleWaterReflection()));
	list.append(waterRefractionAction);
		connect(waterRefractionAction, SIGNAL(triggered()), this, SLOT(toggleWaterRefraction()));
	list.append(waterFogAction);
		connect(waterFogAction, SIGNAL(triggered()), this, SLOT(toggleWaterFog()));


	QAction *separator = new QAction(this);
	separator->setSeparator(true);

	list.append(separator);

	list.append(reflectionAction);
		connect(reflectionAction, SIGNAL(triggered()), this, SLOT(toggleReflection()));
	list.append(selfillumAction);
		connect(selfillumAction, SIGNAL(triggered()), this, SLOT(toggleSelfIllumination()));
	list.append(phongAction);
		connect(phongAction, SIGNAL(triggered()), this, SLOT(togglePhong()));
	list.append(phongBrushAction);
		connect(phongBrushAction, SIGNAL(triggered()), this, SLOT(togglePhongBrush()));
	list.append(miscAction);
		connect(miscAction, SIGNAL(triggered()), this, SLOT(toggleMisc()));

	toolBar->addActions(list);
	toolBar->setWindowTitle("Icon Shortcuts");
	addToolBar(Qt::LeftToolBarArea, toolBar);

	//----------------------------------------------------------------------------------------//

	// Needed here because ui->listWidget is only defined after ui->setupUi()
	mLogger = ui->listWidget;

	vmtParser = new VmtParser(mLogger);

	ui->comboBox_shader->setInsertPolicy(QComboBox::InsertAlphabetically);
	
	ui->vmtPreviewTextEdit->setWordList(vmtParameters_);

	//----------------------------------------------------------------------------------------//

	if( !mIniSettings->contains("availableGames") )
	{
		refreshGameList();
	}
	else
	{
		QString values = mIniSettings->value("availableGames").toString();

		QRegExp extractGameNames("[\\|].*[\\?]");
		extractGameNames.setMinimal(true);

		QStringList gameNames( values.split(extractGameNames) );
		gameNames.removeLast();

		setGames(gameNames);

		QRegExp extractGameDirectories("[\\?].*[\\|]");
		extractGameDirectories.setMinimal(true);

		QStringList gameDirectories( values.split(extractGameDirectories) );
		gameDirectories.last().chop(1);
		gameDirectories.first().remove(0, gameDirectories.first().indexOf('|') + 1);

		for(int i = 0; i < gameNames.size(); ++i)
			mAvailableGames.insert( gameNames[i], gameDirectories[i] );
	}

	readSettings();

	if(mSettings->saveLastGame)
		setCurrentGame(mSettings->lastGame);

	if(!mGameSelected)
		Info("No game selected! Texture browse and preview will not work")

	//----------------------------------------------------------------------------------------//

	connect( ui->actionNew,				    SIGNAL(triggered()),  this, SLOT(action_New()));
	connect( ui->actionOpen,			    SIGNAL(triggered()),  this, SLOT(action_Open()));
	connect( ui->actionSave,			    SIGNAL(triggered()),  this, SLOT(action_Save()));
	connect( ui->actionSave_As,			    SIGNAL(triggered()),  this, SLOT(action_saveAs()));
	connect( ui->actionSave_As_Template,    SIGNAL(triggered()), SLOT(saveAsTemplate()));
	connect( ui->actionExit,			    SIGNAL(triggered()),  this, SLOT(close()));

	connect( ui->actionRefresh_List,	    SIGNAL(triggered()),  this, SLOT(action_RefreshTemplateList()));

	connect( ui->action_refreshGameList,    SIGNAL(triggered()),  this, SLOT(refreshGameList()));

	connect( ui->actionRefresh,			    SIGNAL(triggered()),  this, SLOT(refreshRequested()));

	connect( ui->actionClear_Message_Log,   SIGNAL(triggered()),  this, SLOT(clearMessageLog()));

	connect( ui->action_hideAll,		    SIGNAL(triggered()), this, SLOT(hideParameterGroupboxes()));

	connect( ui->action_convertToVTF,       SIGNAL(triggered()), this, SLOT(displayConversionDialog()));

	connect( ui->action_batchVMT,		    SIGNAL(triggered()), this, SLOT(displayBatchDialog()));

	connect( ui->actionParse_VMT,		    SIGNAL(triggered()), this, SLOT(vmtPreviewParse()));

	connect( ui->action_reconvertAll,	    SIGNAL(triggered()), this, SLOT(reconvertAll()));

	connect( ui->action_CreateBlendTexture, SIGNAL(triggered()), this, SLOT(createBlendToolTexture()));

	connect( ui->action_Paste,              SIGNAL(triggered()), this, SLOT(paste()));

	connect( ui->action_refreshInGame,	    SIGNAL(triggered()), this, SLOT(refreshInGame()));


	//----------------------------------------------------------------------------------------//

	ui->doubleSpinBox_refractAmount->setDoubleSlider(ui->horizontalSlider_refractAmount);
	ui->doubleSpinBox_refractBlur->setDoubleSlider(ui->horizontalSlider_refractBlur);

	ui->doubleSpinBox_lumstart1->setDoubleSlider(ui->horizontalSlider_lumstart1);
	ui->doubleSpinBox_lumend1->setDoubleSlider(ui->horizontalSlider_lumend1);

	ui->doubleSpinBox_lumstart2->setDoubleSlider(ui->horizontalSlider_lumstart2);
	ui->doubleSpinBox_lumend2->setDoubleSlider(ui->horizontalSlider_lumend2);
	ui->doubleSpinBox_blendstart2->setDoubleSlider(ui->horizontalSlider_blendstart2);
	ui->doubleSpinBox_blendend2->setDoubleSlider(ui->horizontalSlider_blendend2);
	ui->doubleSpinBox_blendfactor2->setDoubleSlider(ui->horizontalSlider_blendfactor2);

	ui->doubleSpinBox_lumstart3->setDoubleSlider(ui->horizontalSlider_lumstart3);
	ui->doubleSpinBox_lumend3->setDoubleSlider(ui->horizontalSlider_lumend3);
	ui->doubleSpinBox_blendstart3->setDoubleSlider(ui->horizontalSlider_blendstart3);
	ui->doubleSpinBox_blendend3->setDoubleSlider(ui->horizontalSlider_blendend3);
	ui->doubleSpinBox_blendfactor3->setDoubleSlider(ui->horizontalSlider_blendfactor3);

	ui->doubleSpinBox_lumstart4->setDoubleSlider(ui->horizontalSlider_lumstart4);
	ui->doubleSpinBox_lumend4->setDoubleSlider(ui->horizontalSlider_lumend4);
	ui->doubleSpinBox_blendstart4->setDoubleSlider(ui->horizontalSlider_blendstart4);
	ui->doubleSpinBox_blendend4->setDoubleSlider(ui->horizontalSlider_blendend4);
	ui->doubleSpinBox_blendfactor4->setDoubleSlider(ui->horizontalSlider_blendfactor4);

	ui->doubleSpinBox_opacity->setDoubleSlider(ui->horizontalSlider_opacity);

	ui->doubleSpinBox_detailAmount->setDoubleSlider(ui->horizontalSlider_detailAmount);
	ui->doubleSpinBox_detailAmount2->setDoubleSlider(ui->horizontalSlider_detailAmount2);
	ui->doubleSpinBox_detailAmount3->setDoubleSlider(ui->horizontalSlider_detailAmount3);
	ui->doubleSpinBox_detailAmount4->setDoubleSlider(ui->horizontalSlider_detailAmount4);

	ui->doubleSpinBox_saturation->setDoubleSlider(ui->horizontalSlider_saturation);
	ui->doubleSpinBox_contrast->setDoubleSlider(ui->horizontalSlider_contrast);
	ui->doubleSpinBox_fresnelReflection->setDoubleSlider(ui->horizontalSlider_fresnelReflection);
	ui->doubleSpinBox_envmapLight->setDoubleSlider(ui->horizontalSlider_envmapLight);
	ui->doubleSpinBox_envmapAniso->setDoubleSlider(ui->horizontalSlider_envmapAniso);

	ui->doubleSpinBox_boost->setDoubleSlider(ui->horizontalSlider_boost, 8);
	ui->doubleSpinBox_albedoBoost->setDoubleSlider(ui->horizontalSlider_albedoBoost, 8);
	ui->doubleSpinBox_fresnelRangesX->setDoubleSlider(ui->horizontalSlider_fresnelRanges);

	ui->doubleSpinBox_bumpdetailscale->setDoubleSlider(ui->horizontalSlider_bumpdetailscale);
	ui->doubleSpinBox_bumpdetailscale2->setDoubleSlider(ui->horizontalSlider_bumpdetailscale2);

	ui->doubleSpinBox_phongAmount->setDoubleSlider(ui->horizontalSlider_phongAmount, 2.0);
	//ui->doubleSpinBox_maskBrightness->setDoubleSlider(ui->horizontalSlider_maskBrightness, 10.0);
	ui->doubleSpinBox_maskContrast->setDoubleSlider(ui->horizontalSlider_maskContrast, 10.0);

	ui->doubleSpinBox_spec_amount2->setDoubleSlider(ui->horizontalSlider_spec_amount2, 2.0);
	//ui->doubleSpinBox_spec_maskBrightness2->setDoubleSlider(ui->horizontalSlider_spec_maskBrightness2, 10.0);
	ui->doubleSpinBox_spec_maskContrast2->setDoubleSlider(ui->horizontalSlider_spec_maskContrast2, 10.0);

	ui->doubleSpinBox_rimLightBoost->setDoubleSlider(ui->horizontalSlider_rimBoost, 10.0);

	ui->doubleSpinBox_flashlightTint->setDoubleSlider(ui->horizontalSlider_flashlightTint, 10.0);
	ui->doubleSpinBox_reflectionAmount->setDoubleSlider(ui->horizontalSlider_waterReflectAmount, 2.0);
	ui->doubleSpinBox_refractionAmount->setDoubleSlider(ui->horizontalSlider_waterRefractAmount, 2.0);

	ui->doubleSpinBox_treeswayStartHeight->setDoubleSlider(ui->horizontalSlider_treeswayStartHeight);
	ui->doubleSpinBox_treeswayStartRadius->setDoubleSlider(ui->horizontalSlider_treeswayStartRadius);
	ui->doubleSpinBox_treeswayStrength->setDoubleSlider(ui->horizontalSlider_treeswayStrength, 10.0);
	ui->doubleSpinBox_treeswaySpeed->setDoubleSlider(ui->horizontalSlider_treeswaySpeed, 10.0);
	ui->doubleSpinBox_treeswayspeedHighWind->setDoubleSlider(ui->horizontalSlider_treeswayspeedHighWind, 10.0);
	ui->doubleSpinBox_treeswayScrumbleStrength->setDoubleSlider(ui->horizontalSlider_treeswayScrumbleStrength, 10.0);
	ui->doubleSpinBox_treeswayScrumbleSpeed->setDoubleSlider(ui->horizontalSlider_treeswayScrumbleSpeed, 10.0);

	ui->doubleSpinBox_layer1tint->setDoubleSlider(ui->horizontalSlider_layer1tint);
	ui->doubleSpinBox_layer2tint->setDoubleSlider(ui->horizontalSlider_layer2tint);
	ui->doubleSpinBox_layerBlendSoftness->setDoubleSlider(ui->horizontalSlider_layerBlendSoftness);
	ui->doubleSpinBox_layerBorderOffset->setDoubleSlider(ui->horizontalSlider_layerBorderOffset);
	ui->doubleSpinBox_layerBorderSoftness->setDoubleSlider(ui->horizontalSlider_layerBorderSoftness);
	ui->doubleSpinBox_layerBorderStrength->setDoubleSlider(ui->horizontalSlider_layerBorderStrength);
	ui->doubleSpinBox_layerBorderTint->setDoubleSlider(ui->horizontalSlider_layerBorderTint);
	ui->doubleSpinBox_layerEdgeOffset->setDoubleSlider(ui->horizontalSlider_layerEdgeOffset);
	ui->doubleSpinBox_layerEdgeSoftness->setDoubleSlider(ui->horizontalSlider_layerEdgeSoftness);
	ui->doubleSpinBox_layerEdgeStrength->setDoubleSlider(ui->horizontalSlider_layerEdgeStrength);

	ui->doubleSpinBox_emissiveBlendStrength->setDoubleSlider(ui->horizontalSlider_emissiveBlendStrength);
	ui->doubleSpinBox_emissiveBlendTint->setDoubleSlider(ui->horizontalSlider_emissiveBlendTint);

	//----------------------------------------------------------------------------------------//

	ui->doubleSpinBox_envmapTint->setDoubleSlider(ui->horizontalSlider_envmapTint);
	ui->doubleSpinBox_color1->setDoubleSlider(ui->horizontalSlider_color1);
	ui->doubleSpinBox_color2->setDoubleSlider(ui->horizontalSlider_color2);
	ui->doubleSpinBox_reflectivity->setDoubleSlider(ui->horizontalSlider_reflectivity);
	ui->doubleSpinBox_reflectivity_2->setDoubleSlider(ui->horizontalSlider_reflectivity_2);
	ui->doubleSpinBox_selfIllumTint->setDoubleSlider(ui->horizontalSlider_selfIllumTint);
	ui->horizontalSlider_phongTint->initialize(ui->toolButton_phongTint);
	//ui->horizontalSlider_envmapTint->initialize(ui->toolButton_envmapTint);
	//ui->horizontalSlider_selfIllumTint->initialize(ui->toolButton_selfIllumTint);
	//ui->horizontalSlider_reflectivity->initialize(ui->toolButton_reflectivity);
	//ui->horizontalSlider_reflectivity_2->initialize(ui->toolButton_reflectivity_2);

	ui->horizontalSlider_waterReflectColor->initialize(ui->toolButton_reflectionTint);
	ui->horizontalSlider_waterRefractColor->initialize(ui->toolButton_refractionTint);
	ui->horizontalSlider_waterFogColor->initialize(ui->toolButton_fogTint);

	//----------------------------------------------------------------------------------------//

	QRegExpValidator* windowsFilenameValidator = new QRegExpValidator(QRegExp("[^?%*:|\"<>]*"), this);

	ui->lineEdit_diffuse->setValidator(windowsFilenameValidator);
	ui->lineEdit_diffuse2->setValidator(windowsFilenameValidator);
	ui->lineEdit_bumpmap->setValidator(windowsFilenameValidator);
	ui->lineEdit_bumpmap2->setValidator(windowsFilenameValidator);
	ui->lineEdit_detail->setValidator(windowsFilenameValidator);
	ui->lineEdit_detail2->setValidator(windowsFilenameValidator);
	ui->lineEdit_lightWarp->setValidator(windowsFilenameValidator);
	ui->lineEdit_blendmodulate->setValidator(windowsFilenameValidator);
	ui->lineEdit_envmap->setValidator(windowsFilenameValidator);
	ui->lineEdit_specmap->setValidator(windowsFilenameValidator);
	ui->lineEdit_specmap2->setValidator(windowsFilenameValidator);
	ui->lineEdit_exponentTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_refractNormalMap->setValidator(windowsFilenameValidator);
	ui->lineEdit_refractNormalMap2->setValidator(windowsFilenameValidator);
	ui->lineEdit_waterNormalMap->setValidator(windowsFilenameValidator);
	ui->lineEdit_flowMap->setValidator(windowsFilenameValidator);
	ui->lineEdit_noiseTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_toolTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_unlitTwoTextureDiffuse->setValidator(windowsFilenameValidator);
	ui->lineEdit_unlitTwoTextureDiffuse2->setValidator(windowsFilenameValidator);
	ui->lineEdit_refractTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_maskTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_bump2->setValidator(windowsFilenameValidator);
	ui->lineEdit_tintMask->setValidator(windowsFilenameValidator);
	ui->lineEdit_phongWarp->setValidator(windowsFilenameValidator);
	ui->lineEdit_emissiveBlendTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_emissiveBlendBaseTexture->setValidator(windowsFilenameValidator);
	ui->lineEdit_emissiveBlendFlowTexture->setValidator(windowsFilenameValidator);



	//--------------------------------------------------------------------//

	connect(ui->lineEdit_diffuse, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_diffuse2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_diffuse3, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_diffuse4, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_bumpmap, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_bumpmap2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_detail, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_detail2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_lightWarp, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_blendmodulate, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_envmap, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_specmap, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_specmap2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_exponentTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_waterNormalMap, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_refractNormalMap, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_refractNormalMap2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_flowMap, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_noiseTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_toolTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_unlitTwoTextureDiffuse, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_unlitTwoTextureDiffuse2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_refractTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_maskTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_bump2, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_tintMask, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_phongWarp, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_bumpmapAlpha, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_diffuseAlpha, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_emissiveBlendTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_emissiveBlendBaseTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));
	connect(ui->lineEdit_emissiveBlendFlowTexture, SIGNAL(droppedTexture(QString)),
		SLOT(handleTextureDrop(QString)));


	//----------------------------------------------------------------------------------------//

	connect( ui->lineEdit_diffuse,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_diffuse2,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_diffuse3,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_diffuse4,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_bumpmap,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_bumpmap2,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_detail,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_lightWarp,				SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_blendmodulate,			SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_envmap,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_specmap,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_exponentTexture,			SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_waterNormalMap,			SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_refractNormalMap,			SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_refractNormalMap2,		SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_flowMap,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_noiseTexture,				SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_toolTexture,				SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_unlitTwoTextureDiffuse,	SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_unlitTwoTextureDiffuse2,	SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_refractTexture,			SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_maskTexture,				SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));
	connect( ui->lineEdit_bump2,					SIGNAL( returnPressed() ), this, SLOT( previewTexture() ));

	connect( ui->action_about,						SIGNAL( triggered() ), this, SLOT( displayAboutDialog() ));
	connect( ui->action_userGuide,					SIGNAL( triggered() ), this, SLOT( openUserGuide() ));
	connect(ui->action_checkUpdate,                 SIGNAL(triggered()),         SLOT( checkForUpdates() ));

	connect( ui->action_options,					SIGNAL( triggered() ), this, SLOT( displayOptionsDialog() ));

	//----------------------------------------------------------------------------------------//

	connect( ui->dockWidget_vmtPreview,	 SIGNAL(visibilityChanged(bool)), ui->action_vmtPreview,		 SLOT(setChecked(bool)));
	connect( ui->action_vmtPreview,		 SIGNAL(triggered(bool)),		  ui->dockWidget_vmtPreview,	 SLOT(setVisible(bool)));

	connect( ui->dockWidget_messageLog,	 SIGNAL(visibilityChanged(bool)), ui->action_messageLog,		 SLOT(setChecked(bool)));
	connect( ui->action_messageLog,		 SIGNAL(triggered(bool)),		  ui->dockWidget_messageLog,	 SLOT(setVisible(bool)));

	//----------------------------------------------------------------------------------------//

	QFont font;
		font.setFamily("Segoe UI");
		font.setStyleHint(QFont::Monospace);
		font.setFixedPitch(true);
		font.setPointSize(9);

	QFontMetrics metrics(font);
	ui->vmtPreviewTextEdit->setTabStopWidth(4 * metrics.width(' '));

	//----------------------------------------------------------------------------------------//

	QDir cacheFolder;
		cacheFolder.mkdir("Cache");
		cacheFolder.mkdir("Cache/Move");

	QDir templatesFolder;
		templatesFolder.mkdir("templates");

#ifdef Q_OS_WIN
	utils::checkVtfCmd(ui);
#endif

	if(mSettings->checkForUpdates) {
		QDate date;
		int today = date.currentDate().day();
		int savedDate = mIniSettings->value("savedDate").toInt();

		if (today != savedDate) {
			mIniSettings->setValue("latestVersion", getCurrentVersion());
			QTimer::singleShot(100, this, SLOT(checkForUpdatesSilent()));
			mIniSettings->setValue("savedDate", today);
		} else {
			QString cv = getCurrentVersion();
			QString v = mIniSettings->value("latestVersion").toString();
			QStringList cv1 = cv.split(".");
			QStringList v1 = v.split(".");
			int cv2 = cv1[0].toInt() * 10000 +
					  cv1[1].toInt() * 100 +
					  cv1[2].toInt();
			int v2 = v1[0].toInt() * 10000 +
					 v1[1].toInt() * 100 +
					 v1[2].toInt();
			if (cv2 < v2) {
				ui->menuHelp->setTitle("New version avaiable");
				Info(QString("New version available: %1").arg(v));
			}
		}
	}

	//----------------------------------------------------------------------------------------//

	separatorAct = ui->menuFile->addSeparator();
	separatorTemp = ui->menuTemplates->addSeparator();

	for( int i = 0; i < MaxRecentFiles; ++i ) {

		 recentFileActions[i] = new QAction(this);
		 recentFileActions[i]->setVisible(false);

		 if( mSettings->recentFileListStyle == Settings::FileMenu )
			ui->menuFile->insertAction( ui->actionExit, recentFileActions[i] );
		 else
			 ui->menuRecentFiles->insertAction( ui->actionExit, recentFileActions[i] );

		 connect(recentFileActions[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
	}

	for( int i = 0; i < MaxTemplates; ++i ) {

		templateActions[i] = new QAction(this);
		templateActions[i]->setVisible(false);
		ui->menuTemplates->insertAction( ui->actionRefresh_List, templateActions[i] );

		connect(templateActions[i], SIGNAL(triggered()), SLOT(openTemplate()));
	}

	action_RefreshTemplateList();

	updateRecentFileActions( mSettings->recentFileEntryStyle == Settings::FullPath ? true : false );

	//----------------------------------------------------------------------------------------//

	setCentralWidget(ui->tabWidget);

	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setMinimumWidth(740);

	setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
	setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

	setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
	setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );

	ui->dockWidget_messageLog->setStyleSheet("QDockWidget { font-family: Segoe UI; font-size: 9pt; }");
	ui->dockWidget_vmtPreview->setStyleSheet("QDockWidget { font-family: Segoe UI; font-size: 9pt; }");

	//----------------------------------------------------------------------------------------//

	ui->verticalLayout_parameterLeft->setAlignment(Qt::AlignTop);

	//----------------------------------------------------------------------------------------//

	ui->toolButton_envmapTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_phongTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_reflectionTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_refractionTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_fogTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_refractTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_color1->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_color2->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_selfIllumTint->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->toolButton_reflectivity->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->toolButton_reflectivity_2->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->toolButton_layer1tint->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->toolButton_layer2tint->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->toolButton_layerBorderTint->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->toolButton_emissiveBlendTint->setStyleSheet("background-color: rgb(255, 255, 255)");

	phong::initialize(ui);

	//----------------------------------------------------------------------------------------//

	addGLWidget( ":/overlays/bump1", ui->verticalLayout_preview_normalmap1, "preview_normalmap1" );
	addGLWidget( ":/overlays/bump2", ui->verticalLayout_preview_normalmap2, "preview_normalmap2" );

	addGLWidget( ":/overlays/diffuse3", ui->verticalLayout_preview_basetexture3, "preview_basetexture3" );
	addGLWidget( ":/overlays/diffuse4", ui->verticalLayout_preview_basetexture4, "preview_basetexture4" );

	addGLWidget( ":/overlays/detail", ui->verticalLayout_preview_detail, "preview_detail" );
	addGLWidget( ":/overlays/detail2", ui->verticalLayout_preview_detail2, "preview_detail2" );

	addGLWidget( ":/overlays/blendmod", ui->verticalLayout_preview_blendmod, "preview_blendmod" );

	addGLWidget( ":/overlays/exponent1", ui->verticalLayout_preview_exponent1, "preview_exponent" );


	glWidget_diffuse1 = new GLWidget_Diffuse1(this);
		glWidget_diffuse1->setMinimumSize( QSize(192, 192) );
		glWidget_diffuse1->setMaximumSize( QSize(192, 192) );

	ui->verticalLayout_preview_basetexture1->addWidget(glWidget_diffuse1);

	glWidget_diffuse2 = new GLWidget_Diffuse2(this);
		glWidget_diffuse2->setMinimumSize( QSize(192, 192) );
		glWidget_diffuse2->setMaximumSize( QSize(192, 192) );

	ui->verticalLayout_preview_basetexture2->addWidget(glWidget_diffuse2);

	glWidget_envmap = new GLWidget_Spec(this, 0);
		glWidget_envmap->setMinimumSize( QSize(192, 192) );
		glWidget_envmap->setMaximumSize( QSize(192, 192) );

	ui->verticalLayout_preview_envmap1->addWidget(glWidget_envmap);

	glWidget_spec = new GLWidget_Spec(this, 1);
		glWidget_spec->setMinimumSize( QSize(192, 192) );
		glWidget_spec->setMaximumSize( QSize(192, 192) );

	ui->verticalLayout_preview_spec1->addWidget(glWidget_spec);

	//----------------------------------------------------------------------------------------//

	connect(ui->lineEdit_diffuse, SIGNAL(textChanged(QString)), this, SLOT(modifiedLineEdit(QString)));
	connect(ui->lineEdit_bumpmap, SIGNAL(textChanged(QString)), this, SLOT(modifiedLineEdit(QString)));
	connect(ui->lineEdit_specmap, SIGNAL(textChanged(QString)), this, SLOT(modifiedLineEdit(QString)));

	//----------------------------------------------------------------------------------------//

	QDir dir( QDir::currentPath() + "/Cache/Move/");
		dir.setNameFilters( QStringList() << "*.vtf" );
		dir.setFilter(QDir::Files);

	foreach(QString dirFile, dir.entryList())
		dir.remove(dirFile);

	//----------------------------------------------------------------------------------------//

	hideParameterGroupboxes();

	ui->label_bumpmapAlpha->setVisible(false);
	ui->lineEdit_bumpmapAlpha->setVisible(false);
	ui->toolButton_bumpmapAlpha->setVisible(false);

	ui->label_diffuseAlpha->setVisible(false);
	ui->lineEdit_diffuseAlpha->setVisible(false);
	ui->toolButton_diffuseAlpha->setVisible(false);

	ui->comboBox_shader->setCurrentIndex(ui->comboBox_shader->findText("LightmappedGeneric"));
		mIgnoreShaderChanged = false;
	shaderChanged();

	connect( ui->comboBox_shader, SIGNAL(currentIndexChanged(int)), this, SLOT(shaderChanged()) );

	if(	fileToOpen.endsWith("jpg", Qt::CaseInsensitive) ||
		fileToOpen.endsWith("tga", Qt::CaseInsensitive) ||
		fileToOpen.endsWith("dds", Qt::CaseInsensitive) ||
		fileToOpen.endsWith("gif", Qt::CaseInsensitive) ||
		fileToOpen.endsWith("png", Qt::CaseInsensitive) ) {

		initialFile = "";
		mOpenConvertDialog = true;
		fileToConvert = fileToOpen;
	}

	if( fileToOpen.endsWith("vtf", Qt::CaseInsensitive) ) {

		// clearing to avoid loading it in showEvent()
		initialFile = "";

		const QString objectName = "preview_basetexture1";
		const QString fileType = fileToOpen.right( fileToOpen.size() - fileToOpen.lastIndexOf(".") );

		if( fileToOpen.startsWith( currentGameMaterialDir(), Qt::CaseInsensitive) ) {

			texturesToCopy.remove(ui->lineEdit_diffuse);

			if( fileType.toLower() != ".vtf" ) {

				bool convert = true;

				QString vtfFileName = fileToOpen;

				vtfFileName = vtfFileName.left( vtfFileName.lastIndexOf('.') );
				vtfFileName.append(".vtf");

				if( QDir(fileToOpen.replace("\\", "/")).exists(vtfFileName) ) {

					MsgBox msgBox(this);
						msgBox.setWindowTitle("File already exists!");
						msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
						msgBox.setDefaultButton( QMessageBox::No );
						msgBox.setIconPixmap(QPixmap(":/icons/info_warning"));

					msgBox.setText( fileToOpen.left( fileToOpen.lastIndexOf(".") ).append(".vtf") +
									" already exists. Do you want to overwrite it?"  );

					if( msgBox.exec() != QMessageBox::Yes )
						convert = false;
				}

				if(convert) {

					ConversionThread* conversionThread = new ConversionThread(this);
						conversionThread->fileName = fileToOpen;

					if(mVMTLoaded) {

						conversionThread->outputParameter = vmtParser->lastVMTFile().directory + "/" +
								vmtParser->lastVMTFile().fileName.left( vmtParser->lastVMTFile().fileName.size() - 4 ) + ".vtf";
					}

					conversionThread->start();
				}
			}

			goto updateLineEdit;

		} else {

			texturesToCopy.remove(ui->lineEdit_diffuse);

			QString nameWithExtension( fileToOpen.right( fileToOpen.size() - fileToOpen.lastIndexOf("/") ));
			QString fullNewName( currentGameMaterialDir() + nameWithExtension );

			if( fileType.toLower() == ".vtf" ) {

				if( QFile::exists(fullNewName) ) {

					MsgBox msgBox(this);
						msgBox.setWindowTitle("File already exists!");
						QPushButton* overwriteButton = msgBox.addButton( "Overwrite", QMessageBox::YesRole );
						QPushButton* renameButton = msgBox.addButton( "Rename", QMessageBox::NoRole );
						msgBox.addButton( QMessageBox::No );
						msgBox.setDefaultButton( renameButton );
						msgBox.setIcon( QMessageBox::Warning );

					msgBox.setText( fullNewName + " already exists. Do you want to overwrite or rename it?"  );

					msgBox.exec();

					if( msgBox.clickedButton() == overwriteButton ) {

						if( QFile::remove(fullNewName) ) {

							if( QFile::copy(fileToOpen, fullNewName) ) {

								fileToOpen = fullNewName;

								goto updateLineEdit;

							} else {

								Error("\"" + fileToOpen + "\" could not be copied to: \"" + fullNewName + "\"")
							}

						} else {

							Error("\"" + fileToOpen + "\" could not be deleted!")
						}

					} else if( msgBox.clickedButton() == renameButton ) {

						int fileSuffix = 1;

						fullNewName = fullNewName.left( fullNewName.size() - 4 ).append("_");

						while( QFile::exists( fullNewName + Str(fileSuffix) + ".vtf" )) {

							++fileSuffix;
						}

						if( QFile::copy(fileToOpen, fullNewName + Str(fileSuffix) + ".vtf") ) {

							fileToOpen = fullNewName + Str(fileSuffix) + ".vtf";

							goto updateLineEdit;

						} else {

							Error("\"" + fileToOpen + "\" could not be copied to: \"" + fullNewName + Str(fileSuffix) + ".vtf\"")
						}
					}

				} else {

					if( QFile::copy(fileToOpen, fullNewName) ) {

						fileToOpen = fullNewName;

						goto updateLineEdit;

					} else {

						Error("\"" + fileToOpen + "\" could not be copied to: \"" + fullNewName + "\"")
					}
				}

			} else {

				// Image files, fileType != ".vtf"

				if( texturesToCopy.contains(ui->lineEdit_diffuse) ) {

					QString toDelete = "Cache\\Move\\" + ui->lineEdit_diffuse->objectName() + "_" + texturesToCopy.value(ui->lineEdit_diffuse);

					if( !QFile::remove(toDelete) )
						Error("Error while removing \"" + toDelete + "\"")

					texturesToCopy.remove(ui->lineEdit_diffuse);
				}

				QString outputFile = fileToOpen.right( fileToOpen.length() - fileToOpen.lastIndexOf('/') - 1 );
					outputFile = outputFile.left( outputFile.indexOf('.') );

				texturesToCopy.insert(ui->lineEdit_diffuse, outputFile);

				ui->lineEdit_diffuse->setText(fileToOpen.right( fileToOpen.length() - fileToOpen.lastIndexOf('/', fileToOpen.lastIndexOf('/') - 1) ));
				ui->lineEdit_diffuse->setDisabled(true);

				ConversionThread* conversionThread = new ConversionThread(this);
					conversionThread->fileName = fileToOpen;
					conversionThread->newFileName = ui->lineEdit_diffuse->objectName() + "_" + texturesToCopy.value(ui->lineEdit_diffuse) + ".vtf";
					conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\"";

					conversionThread->start();

				fileToOpen.chop(4);

				QString fromFile = fileToOpen.left( fileToOpen.lastIndexOf("/") ) + nameWithExtension;
				QString toFile = QDir::currentPath() + "/Cache/" + objectName + ".png";

				if( QFile::exists(toFile) )
					QFile::remove(toFile);

				QFile::copy(fromFile, toFile);

				previewTexture(objectName);
			}
		}

		goto returnFromMethod;

		updateLineEdit:

		fileToOpen = fileToOpen.left( fileToOpen.size() - 4 );

		mIniSettings->setValue("lastSaveAsDir",
			QDir::toNativeSeparators(fileToOpen).left(QDir::toNativeSeparators(fileToOpen).lastIndexOf("\\")));

		QString relativeFilePath = QDir( currentGameMaterialDir() ).relativeFilePath(fileToOpen);
		ui->lineEdit_diffuse->setText(relativeFilePath);

		previewTexture( objectName, relativeFilePath, true, false, false, false );
	}

	returnFromMethod:

	mIsInConstructor = false;
}

void MainWindow::addCSGOParameter(QString value, VmtFile& vmt, QString string, QDoubleSpinBox* doubleSpinBox) {

	if( !( value = vmt.parameters.take(string)).isEmpty() ) {
		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) )
			Error(string + " only works with the Lightmapped_4WayBlend CS:GO shader!")
		bool ok;
		double number = value.toDouble(&ok);

		if(ok) {
			if( number == 1.0 )
				Info(string + " has value \"1.0\" which is the default!")
			else {
				doubleSpinBox->setValue(number);
				doubleSpinBox->setValue(number);
			}
		} else
			Error(string + " value \"" + Str(number) + "\" has caused an error while parsing!")
	}
}

void MainWindow::vmtPreviewChanged()
{
	mPreviewChanged = true;
	mChildWidgetChanged = true;
	updateWindowTitle();
}

void MainWindow::vmtPreviewParse()
{
	mCursor = ui->vmtPreviewTextEdit->textCursor();
	mCursorPos = mCursor.position();
	mCursor.setPosition(mCursorPos);

	if (ui->vmtPreviewTextEdit->toPlainText().isEmpty()) {
		refreshRequested();
	}

	vmtParser->saveVmtFile( ui->vmtPreviewTextEdit->toPlainText(), QDir::currentPath() + "/Cache/temp.vmt", true );


	mLoading = true;

	bool vmtLoaded = false;
	if (mVMTLoaded)
		vmtLoaded = true;

	//mLogger->clear();

	VmtFile vmt = vmtParser->loadVmtFile(QDir::currentPath() + "/Cache/temp.vmt", true);

	ui->tabWidget->setCurrentIndex(0);
	resetWidgets();

	qDebug() << mCursor.position();
	if(vmtLoaded)
		mVMTLoaded = true;

	mChildWidgetChanged = true;

	updateWindowTitle();

	ui->textEdit_proxies->setPlainText( vmt.subGroups.replace("    ", "\t") );

	parseVMT(vmt, true);

	//----------------------------------------------------------------------------------------//
	mLoading = false;

	if(mSettings->autoSave) {
		if (mVMTLoaded)
			action_Save();
	}

	mCursor.setPosition(mCursorPos);
	ui->vmtPreviewTextEdit->setTextCursor(mCursor);

	//updateWindowTitle();
}

void MainWindow::parseVMT( VmtFile vmt, bool isTemplate )
{
	mParsingVMT = true;

	// Finding game directory by backtracking and searching for gameinfo.txt
	QDir gameinfoDir = vmt.directory;
	bool gameinfoFound = false;
	QString materialDirectory;

	while( gameinfoDir.cdUp() && !isTemplate )
	{
		if( gameinfoDir.exists("gameinfo.txt") )
		{
			gameinfoFound = true;
			break;
		}
	}

	QDir realGameinfoDir;
	if(gameinfoFound)
	{
		realGameinfoDir = gameinfoDir;

		if( !gameinfoDir.cd("materials") )
		{
			Error("\"materials\" directory does not exist!")
		}
		else
		{
			materialDirectory = gameinfoDir.absolutePath();
		}

		if (mGameSelected)
			vmt.state.gameDirectory = new QDir(realGameinfoDir);
	}

	if(isTemplate) {
		realGameinfoDir = currentGameMaterialDir().section("/", 0, -2);
	}

	//----------------------------------------------------------------------------------------//

	int index = ui->comboBox_shader->findText(vmt.shaderName, Qt::MatchFixedString);
	int currentIndex = ui->comboBox_shader->currentIndex();
	if (index == -1) {

		Info("Unknown shader found, adding shader to enabled custom shaders!")

		mIniSettings->setValue("shaders", mIniSettings->value("shaders").toString() + vmt.shaderName + "?1??");

		mSettings->customShaders.push_back(Shader(vmt.shaderName, true));

		ui->comboBox_shader->addItem(vmt.shaderName);

		sortShaderComboBox();

		ui->comboBox_shader->setCurrentIndex(ui->comboBox_shader->findText(vmt.shaderName));

	} else if (!isTemplate || (isTemplate && index != currentIndex) ) {

		sortShaderComboBox();

		ui->comboBox_shader->setCurrentIndex(index);
		shaderChanged();
	}

	//----------------------------------------------------------------------------------------//

	QString bumpmap;

	bool showBaseTexture = false;
	bool showBaseTexture2 = false;
	bool showColor = false;
	bool showOther = false;
	bool showTransparency = false;
	bool showDecal = false;

	bool showShadingReflection = false;
	bool showSelfIllumination = false;
	bool showRimLight = false;

	bool showFlowmap = false;
	bool showWaterReflection = false;
	bool showWaterRefraction = false;
	bool showWaterFog = false;

	bool showBaseTextureTransform = false;
	bool showBumpmapTransform = false;
	bool showMiscellaneous = false;
	bool showScroll = false;

	bool showSprite = false;

	//----------------------------------------------------------------------------------------//

	QString value;
	if( !( value = vmt.parameters.take("$basetexture") ).isEmpty() )
	{
		QString texture = validateTexture( "preview_basetexture1", value, "$basetexture", realGameinfoDir );

		if( vmt.shaderName == "Water" ) {

			Error("$basetexture does not work with the Water shader!")

			if( !texture.isEmpty() )
				ui->lineEdit_diffuse->setText(texture);

		} else if (vmt.shaderName == "UnlitTwoTexture") {

			if( !texture.isEmpty() )
				ui->lineEdit_unlitTwoTextureDiffuse->setText(texture);
				createReconvertAction(ui->lineEdit_unlitTwoTextureDiffuse, value);

		} else {

			if( !texture.isEmpty() )
				ui->lineEdit_diffuse->setText(texture);

			showBaseTexture = true;
			createReconvertAction(ui->lineEdit_diffuse, value);
		}
	}

	addCSGOParameter(value, vmt, "$texture1_lumstart", ui->doubleSpinBox_lumstart1);
	addCSGOParameter(value, vmt, "$texture1_lumend", ui->doubleSpinBox_lumend1);

	addCSGOParameter(value, vmt, "$texture2_lumstart", ui->doubleSpinBox_lumstart2);
	addCSGOParameter(value, vmt, "$texture2_lumend", ui->doubleSpinBox_lumend2);
	addCSGOParameter(value, vmt, "$texture2_blendstart", ui->doubleSpinBox_blendstart2);
	addCSGOParameter(value, vmt, "$texture2_blendend", ui->doubleSpinBox_blendend2);
	addCSGOParameter(value, vmt, "$lumblendfactor2", ui->doubleSpinBox_blendfactor2);

	addCSGOParameter(value, vmt, "$texture3_lumstart", ui->doubleSpinBox_lumstart3);
	addCSGOParameter(value, vmt, "$texture3_lumend", ui->doubleSpinBox_lumend3);
	addCSGOParameter(value, vmt, "$texture3_blendstart", ui->doubleSpinBox_blendstart3);
	addCSGOParameter(value, vmt, "$texture3_blendend", ui->doubleSpinBox_blendend3);
	addCSGOParameter(value, vmt, "$lumblendfactor3", ui->doubleSpinBox_blendfactor3);

	addCSGOParameter(value, vmt, "$texture4_lumstart", ui->doubleSpinBox_lumstart4);
	addCSGOParameter(value, vmt, "$texture4_lumend", ui->doubleSpinBox_lumend4);
	addCSGOParameter(value, vmt, "$texture4_blendstart", ui->doubleSpinBox_blendstart4);
	addCSGOParameter(value, vmt, "$texture4_blendend", ui->doubleSpinBox_blendend4);
	addCSGOParameter(value, vmt, "$lumblendfactor4", ui->doubleSpinBox_blendfactor4);

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$texture2") ).isEmpty() ) {

		if( vmt.shaderName.compare("UnlitTwoTexture", Qt::CaseInsensitive) )
			Error("$texture2 only works with the UnlitTwoTexture shader!")

		QString texture = validateTexture( "preview_basetexture2", value, "$texture2", realGameinfoDir );

		if( !texture.isEmpty() )
			ui->lineEdit_unlitTwoTextureDiffuse2->setText(texture);
			createReconvertAction(ui->lineEdit_unlitTwoTextureDiffuse2, value);
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$basetexture2") ).isEmpty() )
	{
		QString texture = validateTexture( "preview_basetexture2", value, "$basetexture2", realGameinfoDir );

		if( vmt.shaderName.compare("WorldVertexTransition", Qt::CaseInsensitive) &&
			vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) ) {

			Error("$basetexture2 only works with the WorldVertexTransition or the Lightmapped_4WayBlend shader!")
		}

		if( !texture.isEmpty() )
			ui->lineEdit_diffuse2->setText(texture);

		showBaseTexture2 = true;

		createReconvertAction(ui->lineEdit_diffuse2, value);
	}

	if( !( value = vmt.parameters.take("$basetexture3") ).isEmpty() ) {

		QString texture = validateTexture( "preview_basetexture3", value, "$basetexture3", realGameinfoDir );

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) ) \
			Error("$basetexture3 only works with the Lightmapped_4WayBlend CS:GO shader!") \

		if( !texture.isEmpty() )
			ui->lineEdit_diffuse3->setText(texture);
			createReconvertAction(ui->lineEdit_diffuse3, value);
	}

	if( !( value = vmt.parameters.take("$basetexture4") ).isEmpty() ) {

		QString texture = validateTexture( "preview_basetexture4", value, "$basetexture4", realGameinfoDir );

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) ) \
			Error("$basetexture4 only works with the Lightmapped_4WayBlend CS:GO shader!") \

		if( !texture.isEmpty() )
			ui->lineEdit_diffuse4->setText(texture);
			createReconvertAction(ui->lineEdit_diffuse4, value);
	}

	if( !( value = vmt.parameters.take("$texture2_uvscale") ).isEmpty() ) {

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) )
			Error("$texture2_uvscale only works with the Lightmapped_4WayBlend CS:GO shader!")

		loadScale(value, "$texture2_uvscale", ui->doubleSpinBox_uvscalex2, ui->doubleSpinBox_uvscaley2);
	}

	if( !( value = vmt.parameters.take("$texture3_uvscale") ).isEmpty() ) {

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) )
			Error("$texture3_uvscale only works with the Lightmapped_4WayBlend CS:GO shader!")

		loadScale(value, "$texture3_uvscale", ui->doubleSpinBox_uvscalex3, ui->doubleSpinBox_uvscaley3);
	}

	if( !( value = vmt.parameters.take("$texture4_uvscale") ).isEmpty() ) {

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) )
			Error("$texture4_uvscale only works with the Lightmapped_4WayBlend CS:GO shader!")

		loadScale(value, "$texture4_uvscale", ui->doubleSpinBox_uvscalex4, ui->doubleSpinBox_uvscaley4);
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$bumpmap") ).isEmpty() )
	{
		if (vmt.shaderName == "Water") {

			bumpmap = value;

		} else {

			QString texture = validateTexture( "preview_bumpmap1", value, "$bumpmap", realGameinfoDir );

			if( !texture.isEmpty() )
				ui->lineEdit_bumpmap->setText(texture);

			showBaseTexture = true;
			createReconvertAction(ui->lineEdit_bumpmap, value);
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$bumpmap2") ).isEmpty() )
	{
		const QString texture = validateTexture("preview_bumpmap2", value,
			"$bumpmap2", realGameinfoDir);
		if (!texture.isEmpty()) {
			if (vmt.parameters.contains("$addbumpmaps")) {
				ui->lineEdit_bump2->setText(texture);
				createReconvertAction(ui->lineEdit_bump2, value);
			} else {
				ui->lineEdit_bumpmap2->setText(texture);
				showBaseTexture2 = true;
				createReconvertAction(ui->lineEdit_bumpmap2, value);
			}
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$blendmodulatetexture") ).isEmpty() ) {

		if( vmt.shaderName.compare("WorldVertexTransition", Qt::CaseInsensitive) )
			Error("$blendmodulatetexture is only works with WorldVertexTransition shader!");

		const QString texture = validateTexture("preview_blendmod", value,
			"$blendmodulatetexture", realGameinfoDir);

		//utils::parseTexture("$blendmodulatetexture", value, ui, ui->lineEdit_blendmodulate, vmt);
		ui->lineEdit_blendmodulate->setText(texture);
		showBaseTexture2 = true;
		createReconvertAction(ui->lineEdit_blendmodulate, value);
	}

	if( !( value = vmt.parameters.take("$lightwarptexture") ).isEmpty() )
	{
		utils::parseTexture("$lightwarptexture", value, ui,
			ui->lineEdit_lightWarp, vmt);

		showOther = true;
	}

	if( !( value = vmt.parameters.take("$reflectivity") ).isEmpty() ) {
		applyColor("$reflectivity", value,
			ui->toolButton_reflectivity,
			ui->doubleSpinBox_reflectivity, ui);

		showOther = true;
	}

	if( !( value = vmt.parameters.take("$reflectivity2") ).isEmpty() ) {
		if( vmt.shaderName.compare("WorldVertexTransition", Qt::CaseInsensitive) )
			Error("$reflectivity is only works with WorldVertexTransition shader!")
		applyColor("$reflectivity2", value,
			ui->toolButton_reflectivity_2,
			ui->doubleSpinBox_reflectivity_2, ui);

		showOther = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$ssbump") ).isEmpty() )
	{
		if( !(vmt.shaderName == "LightmappedGeneric" || vmt.shaderName == "WorldVertexTransition") )
		{
			Error("$ssbump only works with the LightmappedGeneric and WorldVertexTransition shaders!")
		}
		else
		{
			if( loadBoolParameter( value, "$ssbump") )
				ui->checkBox_ssbump->setChecked(true);
		}

		showBaseTexture = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$alpha") ).isEmpty() ) {

		bool ok;
		double alpha = value.toDouble(&ok);

		if(ok) {

			if( alpha == 1.0 )
				Info("$alpha has value \"1.0\" which is the default!")
			else
				ui->doubleSpinBox_opacity->setValue( alpha );

				opacityChanged(alpha);

		}
		else
			Error("$alpha value \"" + Str(alpha) + "\" has caused an error while parsing!")

		showTransparency = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$alphatest") ).isEmpty() )
	{
		if( vmt.parameters.contains("$translucent") )
		{
			Error("$alphatest and $translucent are not supported at the same time!")

			vmt.parameters.remove( "$translucent");
		}

		if( loadBoolParameter( value, "$alphatest") )
		{
			ui->checkBox_alphaTest->setEnabled(true);
			ui->checkBox_alphaTest->setChecked(true);

			ui->checkBox_transparent->setDisabled(true);
		}

		showTransparency = true;
	}

	if( !( value = vmt.parameters.take("$allowalphatocoverage") ).isEmpty() )
	{
		if( !( ui->checkBox_alphaTest->isChecked() && ui->checkBox_alphaTest->isEnabled() ))
		{
			Error("$allowalphatocoverage only works in combination with $alphatest!")
		}

		if( loadBoolParameter( value, "$allowalphatocoverage") )
		{
			ui->checkBox_alphaToCoverage->setEnabled(true);
			ui->checkBox_alphaToCoverage->setChecked(true);
		}

		showTransparency = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$alphatestreference") ).isEmpty() )
	{
		if( !( ui->checkBox_alphaTest->isChecked() && ui->checkBox_alphaTest->isEnabled() ))
		{
			Error("$alphatestreference only works in combination with $alphatest!")
		}

		bool ok;
		double doubleScale = value.toDouble(&ok);

		if(ok)
		{
			if( doubleScale == 0.7 )
			{
				Error("$alphatestreference has value \"0.7\" which is the default!")
			}

			ui->doubleSpinBox_alphaTestRef->setValue( doubleScale );

			ui->label_23->setEnabled(true);
			ui->doubleSpinBox_alphaTestRef->setEnabled(true);
		}
		else
		{
			Error("$alphatestreference value \"" + value + "\" has caused an error while parsing!")
		}

		showTransparency = true;
	}

	if( !( value = vmt.parameters.take("$additive") ).isEmpty() ) {

		if( loadBoolParameter( value, "$additive") )
			ui->checkBox_additive->setChecked(true);

		showTransparency = true;
	}

	if( !( value = vmt.parameters.take("$decal") ).isEmpty() ) {

		if( loadBoolParameter( value, "$decal") )
			ui->checkBox_decal->setChecked(true);

		showTransparency = true;
	}

	if( !( value = vmt.parameters.take("$translucent") ).isEmpty() ) {

		if( loadBoolParameter( value, "$translucent") ) {

			ui->checkBox_transparent->setEnabled(true);
			ui->checkBox_transparent->setChecked(true);

			ui->checkBox_alphaTest->setDisabled(true);
			ui->checkBox_alphaToCoverage->setDisabled(true);
		}

		showTransparency = true;
	}

	if( !( value = vmt.parameters.take("$nocull") ).isEmpty() ) {

		if( loadBoolParameter( value, "$nocull") )
			ui->checkBox_noCull->setChecked(true);

		showTransparency = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$surfaceprop") ).isEmpty() )
	{
		if( vmt.shaderName == "Water" )
		{
			int index = ui->comboBox_waterSurface->findText( value, Qt::MatchFixedString );
			if( index != -1)
			{
				ui->comboBox_waterSurface->setCurrentIndex(index);
			}
			else
			{
				int index = ui->comboBox_surface->findText( value, Qt::MatchFixedString );
				if( index != -1)
				{
					Error("Odd $surfaceprop value: \"" + value + "\" for Water shader!")
				}
				else
				{
					Error("Unrecognized surface value: \"" + value + "\" found!");
				}
			}
		}
		else if( vmt.shaderName == "Refract" ) {

			int index = ui->comboBox_refractSurface->findText( value, Qt::MatchFixedString );
			if( index != -1) {

				ui->comboBox_refractSurface->setCurrentIndex(index);

			} else {

				Error("Unrecognized surface value: \"" + value + "\" found!");

				ui->comboBox_refractSurface->setCurrentIndex(0);
			}
		}
		else if (vmt.shaderName == "UnlitTwoTexture" ) {

			int index = ui->comboBox_unlitTwoTextureSurface->findText( value, Qt::MatchFixedString );
			if( index != -1) {

				ui->comboBox_unlitTwoTextureSurface->setCurrentIndex(index);

			} else {

				Error("Unrecognized surface value: \"" + value + "\" found!");

				ui->comboBox_unlitTwoTextureSurface->setCurrentIndex(0);
			}

		} else {

			int index = ui->comboBox_surface->findText( value, Qt::MatchFixedString );
			if( index != -1)
			{
				ui->comboBox_surface->setCurrentIndex(index);
			}
			else
			{
				Error("Unrecognized surface value: \"" + value + "\" found!");

				ui->comboBox_surface->setCurrentIndex(0);
			}

			showBaseTexture = true;
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$surfaceprop2") ).isEmpty() )
	{
		if( vmt.shaderName == "Water" )
		{
			Error("$surfaceprop2 does not work with the Water shader!")
		}

		int index = ui->comboBox_surface2->findText( value, Qt::MatchFixedString );
		if( index != -1)
		{
			ui->comboBox_surface2->setCurrentIndex(index);
		}
		else
		{
			Error("Unrecognized surface value: \"" + vmt.parameters.value("$surfaceprop2") + "\" found!");

			ui->comboBox_surface2->setCurrentIndex(0);
		}

		showBaseTexture2 = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$seamless_scale") ).isEmpty() )
	{
		bool ok;
		double doubleScale = value.toDouble(&ok);

		if( !(vmt.shaderName == "LightmappedGeneric" || vmt.shaderName == "WorldVertexTransition" || vmt.shaderName == "VertexLitGeneric") )
		{
			Error("$seamless_scale only works with the LightmappedGeneric and WorldVertexTransition shaders!")
		}
		else
		{
			if(ok)
			{
				if( doubleScale == 0.0 )
				{
					Error("$seamless_scale should not contain \"0.0\"!")
				}
				else
				{
					ui->doubleSpinBox_seamlessScale->setValue( doubleScale );
				}
			}
			else
			{
				Error("$seamless_scale value: \"" + value + "\" has caused an error while parsing!")
			}
		}

		showOther = true;
	}

	//----------------------------------------------------------------------------------------//

	QString detailTex = detailtexture::param::initialize(ui, &vmt);

	if (!detailTex.isEmpty()) {
		QString texture = validateTexture("preview_detail", detailTex,
			"$detail", realGameinfoDir);

		ui->lineEdit_detail->setText(texture);
		createReconvertAction(ui->lineEdit_detail, texture);
	}

	detailTex = detailtexture::param::initialize(ui, &vmt, true);

	if (!detailTex.isEmpty()) {
		utils::parseTexture("$detail2", detailTex, ui,
			ui->lineEdit_detail2, vmt);

		createReconvertAction(ui->lineEdit_detail2, detailTex);
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$detailblendmode") ).isEmpty() ) {
		if (!vmt.state.detailEnabled) {
			ERROR("$detailblendmode is only supported with "
				"$detail!")
		}

		bool ok;
		int blendMode = value.toInt(&ok);

		if(ok) {
			if (blendMode == 0) {
				Info("$detailblendmode has the default value of 0!")
			} else if (blendMode >= 13) {
				Error("$detailblendmode only supports int values ranging from 0 to 12!")
			} else {
				ui->comboBox_detailBlendMode->setCurrentIndex(blendMode);
			}
		} else{
			Error("$detailblendmode value: \"" + value + "\" has caused an error while parsing!")
		}

		vmt.state.showDetail = true;
	}

	/*if( !( value = vmt.parameters.take("$detailblendmode2") ).isEmpty() ) {
		if (!vmt.state.detailEnabled) {
			ERROR("$detailblendmode2 is only supported with "
				"$detail!")
		}

		bool ok;
		int blendMode = value.toInt(&ok);

		if( vmt.shaderName.compare("WorldVertexTransition", Qt::CaseInsensitive))
			Error("$detailblendmode2 only works with the WorldVertexTransition shader!");

		if(ok) {
			if (blendMode == 0) {
				Info("$detailblendmode2 has the default value of 0!")
			} else if (blendMode >= 13) {
				Error("$detailblendmode2 only supports int values ranging from 0 to 12!")
			} else {
				ui->comboBox_detailBlendMode2->setCurrentIndex(blendMode);
			}
		} else{
			Error("$detailblendmode2 value: \"" + value + "\" has caused an error while parsing!")
		}

		vmt.state.showDetail = true;
	}*/

	//----------------------------------------------------------------------------------------//

	detailtexture::param::parse(detailtexture::detailscale, ui, &vmt);
	detailtexture::param::parse(detailtexture::detailscale2, ui, &vmt);

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$detailblendfactor") ).isEmpty() )
	{
		if (!vmt.state.detailEnabled) {
			Error("$detailblendfactor is only supported with "
				"$detail!")
		}

		bool ok;
		double scale = value.toDouble(&ok);

		if(ok)
		{
			if( scale == 1.0 )
			{
				Error("$detailblendfactor has the default value of 1.0!")
			}
			else
			{
				ui->doubleSpinBox_detailAmount->setValue(scale);
			}
		}
		else
		{
			Error("$detailblendfactor value: \"" + value + "\" has caused an error while parsing!")
		}

		vmt.state.showDetail = true;
	}

	if( !( value = vmt.parameters.take("$detailblendfactor2") ).isEmpty() ) {

		if (!vmt.state.detailEnabled) {
			Error("$detailblendfactor2 is only supported with "
				"$detail!")
		}

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) &&
			vmt.shaderName.compare("WorldVertexTransition", Qt::CaseInsensitive))
			Error("$detailblendfactor2 only works with the Lightmapped_4WayBlend or WorldVertexTransition shaders!")

		bool ok;
		double scale = value.toDouble(&ok);

		if(ok) {

			if( scale == 1.0 )
				Error("$detailblendfactor2 has the default value of 1.0!")
			else
				ui->doubleSpinBox_detailAmount2->setValue(scale);

		} else
			Error("$detailblendfactor2 value: \"" + value + "\" has caused an error while parsing!")

        vmt.state.showDetail = true;
	}

	if( !( value = vmt.parameters.take("$detailblendfactor3") ).isEmpty() ) {

		if (!vmt.state.detailEnabled) {
			ERROR("$detailblendfactor3 is only supported with "
				"$detail!")
		}

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) )
			Error("$detailblendfactor3 only works with the Lightmapped_4WayBlend CS:GO shader!")

		double amount;
		if( loadDoubleParameter( &amount, value, "$detailblendfactor3" ) )
			ui->doubleSpinBox_detailAmount3->setValue(amount);

        vmt.state.showDetail = true;
	}

	if( !( value = vmt.parameters.take("$detailblendfactor4") ).isEmpty() ) {

		if (!vmt.state.detailEnabled) {
			Error("$detailblendfactor4 is only supported with "
				"$detail!")
		}

		if( vmt.shaderName.compare("Lightmapped_4WayBlend", Qt::CaseInsensitive) )
			Error("$detailblendfactor4 only works with the Lightmapped_4WayBlend CS:GO shader!")

		double amount;
		if( loadDoubleParameter( &amount, value, "$detailblendfactor4" ) )
			ui->doubleSpinBox_detailAmount4->setValue(amount);

        vmt.state.showDetail = true;
	}

	//----------------------------------------------------------------------------------------//

	bool usingDecal = false;

	if( !( value = vmt.parameters.take("$decaltexture") ).isEmpty() )
	{
		utils::parseTexture("$decaltexture", value, ui,
				ui->lineEdit_decal, vmt);

		usingDecal = true;
		showDecal = true;
		createReconvertAction(ui->lineEdit_decal, value);
	}

	if( !( value = vmt.parameters.take("$decalblendmode") ).isEmpty() ) {
		if (!usingDecal) {
			ERROR("$decalblendmode is only supported with "
				"$decaltexture!")
		}
		bool ok;
		int blendMode = value.toInt(&ok);

		if(ok) {
			if (blendMode >= 2) {
				Error("$decalblendmode only supports values 0 and 1!")
			} else {
				ui->comboBox_decalBlendMode->setCurrentIndex(blendMode);
			}
		} else{
			Error("$decalblendmode value: \"" + value + "\" has caused an error while parsing!")
		}

		showDecal = true;
	}

	bool usingEnvmap = false;

	phong::parseParameters(ui, &vmt);

	//Need to show phong earlier since basealpha checkbox checks if it's
	//visible

	if(vmt.state.showPhong) {
		// showPhong is only true on specific shaders so we can safely
		// branch with the else
		if ((vmt.shader == Shader::S_VertexLitGeneric && !ui->action_phong->isChecked()) ||
			(vmt.shader == Shader::S_Custom && !ui->action_phong->isChecked())	) {
			ui->action_phong->trigger();
		} else if (!ui->action_phongBrush->isChecked()) {
			ui->action_phongBrush->trigger();
		}
	}

	if( !( value = vmt.parameters.take("$envmap") ).isEmpty() )
	{
		if( value == "env_cubemap" )
		{
			ui->checkBox_cubemap->setChecked(true);

			ui->lineEdit_envmap->setDisabled(true);
			ui->toolButton_envmap->setDisabled(true);

			usingEnvmap = true;
		}
		else
		{
			utils::parseTexture("$envmap", value, ui,
				ui->lineEdit_envmap, vmt);

			ui->checkBox_cubemap->setChecked(false);
			usingEnvmap = true;
		}

		showShadingReflection = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$envmapcontrast") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$envmapcontrast is only supported with $envmap!")
		}


		bool ok;
		double scale = value.toDouble(&ok);

		if(ok)
		{
			if( scale == 0.0 )
			{
				Info("$envmapcontrast has the default value of 0.0!")
			}
			else
			{
				ui->doubleSpinBox_contrast->setValue(scale);
			}
		}
		else
		{
			Error("$envmapcontrast value: \"" + value + "\" has caused an error while parsing!")
		}

		showShadingReflection = true;
	}

	if( !( value = vmt.parameters.take("$fresnelreflection") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$fresnelReflection is only supported with $envmap!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$fresnelReflection", 1.0 ))
		{
			ui->doubleSpinBox_fresnelReflection->setValue(1.0 - amount);
		}
	}

	if( !( value = vmt.parameters.take("$envmapfresnel") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$envmapfresnel is only supported with $envmap!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$envmapfresnel", 0.0 ))
		{
			ui->doubleSpinBox_fresnelReflection->setValue(amount);
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$envmapsaturation") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$envmapsaturation is only supported with $envmap!")
		}


		bool ok;
		double scale = value.toDouble(&ok);

		if(ok)
		{
			if( scale == 1.0 )
			{
				Info("$envmapsaturation has the default value of 1.0!")
			}
			else
			{
				ui->doubleSpinBox_saturation->setValue(scale);
			}
		}
		else
		{
			Error("$envmapsaturation value: \"" + value + "\" has caused an error while parsing!")
		}

		showShadingReflection = true;
	}

	if( !( value = vmt.parameters.take("$envmaplightscale") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$envmaplightscale is only supported with $envmap!")
		}


		bool ok;
		double scale = value.toDouble(&ok);

		if(ok)
		{
			if( scale == 0.0 )
			{
				Info("$envmaplightscale has the default value of 0!")
			}
			else
			{
				ui->doubleSpinBox_envmapLight->setValue(scale);
			}
		}
		else
		{
			Error("$envmaplightscale value: \"" + value + "\" has caused an error while parsing!")
		}

		showShadingReflection = true;
	}

	if( !( value = vmt.parameters.take("$envmapanisotropyscale") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$envmapanisotropy is only supported with $envmap!")
		}

		bool ok;
		double scale = value.toDouble(&ok);

		if(ok)
		{
			if( scale == 0.0 )
			{
				Info("$envmapanisotropyscale has the default value of 0!")
			}
			else
			{
				ui->doubleSpinBox_envmapAniso->setValue(scale);
			}
		}
		else
		{
			Error("$envmapanisotropyscale value: \"" + value + "\" has caused an error while parsing!")
		}

		showShadingReflection = true;
	}

	if( !( value = vmt.parameters.take("$envmapanisotropy") ).isEmpty() )
	{
		if( !usingEnvmap )
		{
			Error("$envmapanisotropy is only supported with $envmap!")
		}

		showShadingReflection = true;
	}


	if( !( value = vmt.parameters.take("$envmaplightscaleminmax") ).isEmpty() )
	{
		utils::DoubleTuple r = utils::toDoubleTuple(value, 2);

		if (!r.valid) {
			ERROR("$envmaplightscaleminmax value: \"" + value + "\" has caused an error while parsing!")
			return;
		}

		bool valid = utils::isTupleBetween(r, 0.0, 100.0);
		if (valid) {
			double min = r.values.at(0);
			double max = r.values.at(1);
			ui->doubleSpinBox_envmapLightMin->setValue(min);
			ui->doubleSpinBox_envmapLightMax->setValue(max);
		} else {
			const QString error = ("$envmaplightscaleminmax value: \"" + value + "\" has caused an error while parsing!");
			ERROR(error)
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$envmaptint") ).isEmpty() ) {

		if( !usingEnvmap )
			Error("$envmaptint is only supported with $envmap!")

		applyColor("$envmaptint", value,
			ui->toolButton_envmapTint,
			ui->doubleSpinBox_envmapTint, ui);

		showShadingReflection = true;
	}

	//----------------------------------------------------------------------------------------//

	QString usingSpecmap;
	bool usingBaseAlphaSpecmap = false;
	bool usingBumpmapAlphaSpecmap = false;

	if(!(value = vmt.parameters.take("$envmapmask")).isEmpty()) {
		usingSpecmap = utils::prepareTexture(value);
		showShadingReflection = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$basealphaenvmapmask") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$basealphaenvmapmask") )
		{
			usingBaseAlphaSpecmap = true;
		}

		//showShadingReflection = true;
	}

	if( !( value = vmt.parameters.take("$normalmapalphaenvmapmask") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$normalmapalphaenvmapmask") )
		{
			usingBumpmapAlphaSpecmap = true;
		}

		//showShadingReflection = true;
	}

	if( !usingSpecmap.isEmpty() || usingBaseAlphaSpecmap || usingBumpmapAlphaSpecmap )
	{
		if(!usingEnvmap)
		{
			// Error("$envmapmask, $basealphaenvmapmask and $normalmapalphaenvmapmask require $envmap!")
		}

		if( !usingSpecmap.isEmpty() )
		{
			if( usingBaseAlphaSpecmap || usingBumpmapAlphaSpecmap )
			{
				Error("$envmapmask does not work with $basealphaenvmapmask or $normalmapalphaenvmapmask!")

				usingBaseAlphaSpecmap = false;
				usingBumpmapAlphaSpecmap = false;
			}
		}
		else if(usingBaseAlphaSpecmap)
		{
			if(usingBumpmapAlphaSpecmap)
			{
				Error("$basealphaenvmapmask does not work with $normalmapalphaenvmapmask!")
			}

			usingBumpmapAlphaSpecmap = false;
		}

		if( !usingSpecmap.isEmpty() )
		{
			ui->lineEdit_specmap->setText(usingSpecmap);
			createReconvertAction(ui->lineEdit_specmap, usingSpecmap);

			if(!(value = vmt.parameters.take("$envmapmask2")).isEmpty()) {
				ui->lineEdit_specmap2->setText(utils::prepareTexture(value));
				createReconvertAction(ui->lineEdit_specmap2, value);
			}

			if(usingEnvmap)
			{
				ui->checkBox_basealpha->setDisabled(true);
				ui->checkBox_normalalpha->setDisabled(true);
			}
		}
		else if(usingBaseAlphaSpecmap)
		{
			ui->checkBox_basealpha->setChecked(true);

			ui->checkBox_phongBaseAlpha->setChecked(true);
			ui->checkBox_phongNormalAlpha->setDisabled(true);

			if(usingEnvmap)
			{
				ui->lineEdit_specmap->setDisabled(true);
				ui->lineEdit_specmap2->setDisabled(true);

				ui->checkBox_basealpha->setEnabled(true);
				ui->checkBox_basealpha->setChecked(true);

			}
		}
		else
		{
			ui->checkBox_normalalpha->setChecked(true);

			ui->checkBox_phongBaseAlpha->setDisabled(true);
			ui->checkBox_phongNormalAlpha->setChecked(true);

			if(usingEnvmap)
			{
				ui->lineEdit_specmap->setDisabled(true);
				ui->lineEdit_specmap2->setDisabled(true);

				ui->checkBox_normalalpha->setEnabled(true);
				ui->checkBox_normalalpha->setChecked(true);

			}
		}
	}

	//----------------------------------------------------------------------------------------//

	normalblend::parseParameters(ui, &vmt);
	treesway::parseParameters(ui, &vmt);
	layerblend::parseParameters(ui, &vmt);
	emissiveblend::parseParameters(ui, &vmt, this);

	if( !( value = vmt.parameters.take("$phongexponenttexture") ).isEmpty() )
	{
		const QString texture = validateTexture("preview_exponent", value,
			"$phongexponenttexture", realGameinfoDir);
		ui->lineEdit_exponentTexture->setText(texture);
		createReconvertAction(ui->lineEdit_exponentTexture, value);
	}

	if( !( value = vmt.parameters.take("$phongwarptexture") ).isEmpty() )
	{
		utils::parseTexture("$phongwarptexture", value, ui,
							ui->lineEdit_phongWarp, vmt);
		createReconvertAction(ui->lineEdit_phongWarp, value);
	}

	//----------------------------------------------------------------------------------------//

	// Self Illumination Groupbox

	bool usingSelfIllum = false;

	if( !( value = vmt.parameters.take("$selfillum") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$selfillum") )
		{
			ui->groupBox_selfIllumination->setChecked(true);

			usingSelfIllum = true;
		}

		showSelfIllumination = true;
	}

	if( !( value = vmt.parameters.take("$selfillummask") ).isEmpty() )
	{
		if (!usingSelfIllum)
			Error("$selfillummask only works with \"$selfillum 1\"!")

		utils::parseTexture("$selfillummask", value, ui,
			ui->lineEdit_maskTexture, vmt);

		showSelfIllumination = true;
	}

	if( !( value = vmt.parameters.take("$selfillum_envmapmask_alpha") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$selfillum_envmapmask_alpha") )
			ui->checkBox_envmapAlpha->setChecked(true);

		showSelfIllumination = true;
	}

	// ~BaseAlpha checkbox

	if( !( value = vmt.parameters.take("$selfillumtint") ).isEmpty() ) {

		if (!usingSelfIllum)
			Error("$selfillumtint only works with \"$selfillum 1\"!")

		applyColor("$selfillumtint", value,
			ui->toolButton_selfIllumTint,
			ui->doubleSpinBox_selfIllumTint, ui);

		showSelfIllumination = true;
	}

	if( !( value = vmt.parameters.take("$selfillumfresnelminmaxexp") ).isEmpty() )
	{
		if( !usingSelfIllum )
		{
			Error("$selfillumfresnelminmaxexp only works with \"$selfillum 1\"!")
		}

		QString ranges = value;
		ranges = ranges.simplified();

		if( ranges.startsWith('[') )
		{
			ranges.remove( 0, 1 );

			if( ranges.endsWith(']') )
			{
				ranges.chop(1);
				ranges = ranges.simplified();

				int index = ranges.indexOf(' ');
				if( index != -1 )
				{
					QString tmp = ranges.left( index );

					float x, y, z;

					bool ok;
					x = tmp.toFloat(&ok);

					if(ok)
					{
						ranges.remove(0, index + 1);

						index = ranges.indexOf(' ');
						if( index != -1 )
						{
							tmp = ranges.left( index );

							y = tmp.toFloat(&ok);

							if(ok)
							{
								ranges.remove(0, index + 1);

								z = ranges.toFloat(&ok);

								if(ok)
								{
									// Finally done, now validating

									if( !(x >= 0.0f && x < 1.0f) )
									{
										Error("$selfillumfresnelminmaxexp min value: \"" + Str(x) + "\" is not in the valid range of 0.0 to 1.0!")
									}
									else if( !(y >= 0.0f && y < 1.0f) )
									{
										Error("$selfillumfresnelminmaxexp max value: \"" + Str(y) + "\" is not in the valid range of 0.0 to 1.0!")
									}
									else if( !(z >= 0.0f && z < 1.0f) )
									{
										Error("$selfillumfresnelminmaxexp exp value: \"" + Str(z) + "\" is not in the valid range of 0.0 to 1.0!")
									}
									else
									{
										ui->doubleSpinBox_selfIllumFresnelMin->setValue(x);
										ui->doubleSpinBox_selfIllumFresnelMax->setValue(y);
										ui->doubleSpinBox_selfIllumFresnelExp->setValue(z);
									}
								}
								else
								{
									goto error;
								}
							}
							else
							{
								goto error;
							}
						}
						else
						{
							goto error;
						}
					}
					else
					{
						goto error;
					}
				}
				else
				{
					goto error;
				}
			}
			else
			{
				goto error;
			}
		}
		else
		{
			error:

			Error("$selfillumfresnelminmaxexp value: \"" + value + "\" is not in the valid format: \"[<min float> <max float> <exp float>]\"!")
		}

		vmt.state.showPhong = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$model") ).isEmpty() )
	{
		if (!(vmt.shaderName.compare("VertexLitGeneric", Qt::CaseInsensitive) ||
			  vmt.shaderName.compare("LightmappedGeneric", Qt::CaseInsensitive) ||
			  vmt.shaderName.compare("WorldVertexTransition", Qt::CaseInsensitive) ||
			  vmt.shaderName.compare("Refract", Qt::CaseInsensitive)))
		{
			Error("$model only works with the LightmappedGeneric, VertexLitGeneric, Refract or WorldVertexTransition shaders!")
		}

		if( loadBoolParameter( value, "$model") )
			ui->checkBox_model->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$vertexcolor") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$vertexcolor") )
			ui->checkBox_vertexColor->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$vertexalpha") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$vertexalpha") )
			ui->checkBox_vertexAlpha->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$ignorez") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$ignorez") )
			ui->checkBox_ignoreZ->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$nofog") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$nofog") )
			ui->checkBox_noFog->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$nolod") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$nolod") )
			ui->checkBox_noLod->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$disablecsmlookup") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$disablecsmlookup") )
			ui->checkBox_disableCsmLookup->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$nodecal") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$nodecal") )
			ui->checkBox_noDecal->setChecked(true);

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("$no_fullbright") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$no_fullbright") )
			ui->checkBox_noFullBright->setChecked(true);

		showMiscellaneous = true;
	}


	if( !( value = vmt.parameters.take("%keywords") ).isEmpty() )
	{
		ui->lineEdit_keywords->setText( value );

		showMiscellaneous = true;
	}

	if( !( value = vmt.parameters.take("%tooltexture") ).isEmpty() )
	{
		utils::parseTexture("%tooltexture", value, ui,
			ui->lineEdit_toolTexture, vmt);

		showMiscellaneous = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$spriteorientation") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Sprite", Qt::CaseInsensitive) )
		{
			Error("$spriteorientation only works with the Sprite shader!")
		}
		else
		{
			showSprite = true;
		}

		if( value == "parallel_upright" )
		{
			ui->comboBox_spriteOrientation->setCurrentIndex(0);
		}
		else if( value == "facing_upright" || value == "vp_facing_upright" )
		{
			ui->comboBox_spriteOrientation->setCurrentIndex(1);

			Warning("$spriteorientation facing_upright may not be working or is deprecated!")
		}
		else if( value == "vp_parallel" )
		{
			ui->comboBox_spriteOrientation->setCurrentIndex(2);
		}
		else if( value == "oriented" )
		{
			ui->comboBox_spriteOrientation->setCurrentIndex(3);
		}
		else if( value == "vp_parallel_oriented" )
		{
			ui->comboBox_spriteOrientation->setCurrentIndex(4);
		}
		else
		{
			Error("$spriteorientation contains unrecognized value: \"" + value + "\"!")
		}
	}

	if( !( value = vmt.parameters.take("$spriteorigin") ).isEmpty() )
	{
		if( vmt.shaderName != "Sprite" )
		{
			Error("$spriteorigin only works with the Sprite shader!")
		}
		else
		{
			showSprite = true;
		}

		QString tmpValue(value);
		value = value.simplified();

		if( value.startsWith('[') )
		{
			value.remove( 0, 1 );

			if( value.endsWith(']') )
			{
				value.chop(1);
				value = value.trimmed();

				int index = value.indexOf(' ');
				if( index != -1 )
				{
					float x, y;

					bool ok;
					x = value.left( index ).toFloat(&ok);

					if(ok)
					{
						value.remove(0, index + 1);

						y = value.toFloat(&ok);

						if(ok)
						{
							// Finally done, now validating

							if( !(x >= 0.0f && x <= 1.0f) )
							{
								Error("$spriteorigin x value: \"" + Str(x) + "\" is not in the valid range of 0.0 to 1.0!")
							}
							else if( !(y >= 0.0f && y <= 1.0f) )
							{
								Error("$spriteorigin y value: \"" + Str(y) + "\" is not in the valid range of 0.0 to 1.0!")
							}
							else
							{
								ui->doubleSpinBox_spriteX->setValue(x);
								ui->doubleSpinBox_spriteY->setValue(y);
							}
						}
						else
						{
							goto error2;
						}
					}
					else
					{
						goto error2;
					}
				}
				else
				{
					goto error2;
				}
			}
			else
			{
				goto error2;
			}
		}
		else
		{
			error2:

			Error("$spriteorigin value: \"" + tmpValue + "\" is not in the valid format: \"[<x float> <y float>]\"!")
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$basetexturetransform") ).isEmpty() )
	{
		value = value.simplified();

		QStringList values( value.split(" ") );
		if( values.size() == 11 )
		{
			if( values.at(0) == "center" && values.at(3) == "scale" && values.at(6) == "rotate" && values.at(8) == "translate" )
			{
				double centerX, centerY, scaleX, scaleY, angle, translateX, translateY;

				if( _toDouble( &centerX, values.at(1) ))
				{
					if( !(centerX >= 0.0 && centerX <= 1.0) )
					{
						Error("$basetexturetransform center X value: \"" + Str(centerX) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error3;
				}
				if( _toDouble( &centerY, values.at(2) ))
				{
					if( !(centerY >= 0.0 && centerY <= 1.0) )
					{
						Error("$basetexturetransform center Y value: \"" + Str(centerY) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error3;
				}
				if( _toDouble( &scaleX, values.at(4) ))
				{
					if( !(scaleX >= 0.0 && scaleX <= 255.0) )
					{
						Error("$basetexturetransform scale X value: \"" + Str(scaleX) + "\" is not in the valid range of 0.0 to 255.0!")
					}
				}
				else
				{
					goto error3;
				}
				if( _toDouble( &scaleY, values.at(5) ))
				{
					if( !(scaleY >= 0.0 && scaleY <= 255.0) )
					{
						Error("$basetexturetransform scale Y value: \"" + Str(scaleY) + "\" is not in the valid range of 0.0 to 255.0!")
					}
				}
				else
				{
					goto error3;
				}
				if( _toDouble( &angle, values.at(7) ))
				{
					if( !(angle >= -360.0 && angle <= 360.0) )
					{
						Error("$basetexturetransform rotate angle value \"" + Str(angle) + "\" is not in the valid range of -360.0 to 360.0!")
					}
				}
				else
				{
					goto error3;
				}
				if( _toDouble( &translateX, values.at(9) ))
				{
					if( !(translateX >= 0.0 && translateX <= 1.0) )
					{
						Error("$basetexturetransform translate X value: \"" + Str(translateX) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error3;
				}
				if( _toDouble( &translateY, values.at(10) ))
				{
					if( !(translateY >= 0.0 && translateY <= 1.0) )
					{
						Error("$basetexturetransform translate Y value \"" + Str(translateY) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error3;
				}

				//----------------------------------------------------------------------------------------//

				ui->doubleSpinBox_bt_centerX->setValue(centerX);
				ui->doubleSpinBox_bt_centerY->setValue(centerY);
				ui->doubleSpinBox_bt_scaleX->setValue(scaleX);
				ui->doubleSpinBox_bt_scaleY->setValue(scaleY);
				ui->doubleSpinBox_bt_angle->setValue(angle);
				ui->doubleSpinBox_bt_translateX->setValue(translateX);
				ui->doubleSpinBox_bt_translateY->setValue(translateY);

				if( !transformsModified(0) )
				{
					Info("$basetexturetransform contains the default value: \"center 0.5 0.5 scale 1 1 rotate 0 translate 0 0\"!")
				}
			}
			else
			{
				goto error3;
			}
		}
		else
		{
			error3:

			Error("$basetexturetransform value: \"" + value + "\" is not in the valid format: \"center <X float> <Y float> scale <X float> <Y float> rotate <angle int> translate <X float> <Y float>\"!")
		}

		showBaseTextureTransform = true;
	}

	if( !( value = vmt.parameters.take("$bumptransform") ).isEmpty() )
	{
		value = value.simplified();

		QStringList values( value.split(" ") );
		if( values.size() == 11 )
		{
			if( values.at(0) == "center" && values.at(3) == "scale" && values.at(6) == "rotate" && values.at(8) == "translate" )
			{
				double centerX, centerY, scaleX, scaleY, angle, translateX, translateY;

				if( _toDouble( &centerX, values.at(1) ))
				{
					if( !(centerX >= 0.0 && centerX <= 1.0) )
					{
						Error("$bumptransform center X value: \"" + Str(centerX) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error4;
				}
				if( _toDouble( &centerY, values.at(2) ))
				{
					if( !(centerY >= 0.0 && centerY <= 1.0) )
					{
						Error("$bumptransform center Y value: \"" + Str(centerX) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error4;
				}
				if( _toDouble( &scaleX, values.at(4) ))
				{
					if( !(scaleX >= 0.0 && scaleX <= 255.0) )
					{
						Error("$bumptransform scale X value: \"" + Str(scaleX) + "\" is not in the valid range of 0.0 to 255.0!")
					}
				}
				else
				{
					goto error4;
				}
				if( _toDouble( &scaleY, values.at(5) ))
				{
					if( !(scaleY >= 0.0 && scaleY <= 255.0) )
					{
						Error("$bumptransform scale Y value: \"" + Str(scaleY) + "\" is not in the valid range of 0.0 to 255.0!")
					}
				}
				else
				{
					goto error4;
				}
				if( _toDouble( &angle, values.at(7) ))
				{
					if( !(angle >= -360.0 && angle <= 360.0) )
					{
						Error("$bumptransform rotate angle value \"" + Str(angle) + "\" is not in the valid range of -360.0 to 360.0!")
					}
				}
				else
				{
					goto error4;
				}
				if( _toDouble( &translateX, values.at(9) ))
				{
					if( !(translateX >= 0.0 && translateX <= 1.0) )
					{
						Error("$bumptransform translate X value: \"" + Str(translateX) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error4;
				}
				if( _toDouble( &translateY, values.at(10) ))
				{
					if( !(translateY >= 0.0 && translateY <= 1.0) )
					{
						Error("$bumptransform translate Y value \"" + Str(translateY) + "\" is not in the valid range of 0.0 to 1.0!")
					}
				}
				else
				{
					goto error4;
				}

				//----------------------------------------------------------------------------------------//

				ui->doubleSpinBox_bm_centerX->setValue(centerX);
				ui->doubleSpinBox_bm_centerY->setValue(centerY);
				ui->doubleSpinBox_bm_scaleX->setValue(scaleX);
				ui->doubleSpinBox_bm_scaleY->setValue(scaleY);
				ui->doubleSpinBox_bm_angle->setValue(angle);
				ui->doubleSpinBox_bm_translateX->setValue(translateX);
				ui->doubleSpinBox_bm_translateY->setValue(translateY);

				if( !transformsModified(1) )
				{
					Info("$bumptransform contains the default value: \"center 0.5 0.5 scale 1 1 rotate 0 translate 0 0\"!")
				}
			}
			else
			{
				goto error4;
			}
		}
		else
		{
			error4:

			Error("$bumptransform value: \"" + value + "\" is not in the valid format: \"center <X float> <Y float> scale <X float> <Y float> rotate <angle int> translate <X float> <Y float>\"!")
		}

		showBumpmapTransform = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$refract") ).isEmpty() )
	{
		if( (vmt.shaderName != "Refract") && (vmt.shaderName != "Water") )
		{
			Error("$refract only works with the Refract or Water shaders!")
		}
	}

	if( !( value = vmt.parameters.take("$refracttinttexture") ).isEmpty() )
	{
		if( (vmt.shaderName != "Refract") && (vmt.shaderName != "Water") )
		{
			Error("$refracttinttexture only works with the Refract or Water shaders!")
		}

		if( vmt.shaderName == "Refract" ) {
			utils::parseTexture("$refracttinttexture", value, ui,
				ui->lineEdit_refractTexture, vmt);

		} else if( vmt.shaderName == "Water" ) {
			if( value.compare( "_rt_waterrefraction", Qt::CaseInsensitive ))
			{
				Warning("$refracttinttexture contains unsupported value: \"" + value + "\". Only \"_rt_waterrefraction\" is supported!")
			}
		}
	}

	if( !( value = vmt.parameters.take("$refracttint") ).isEmpty() ) {

		if( (vmt.shaderName != "Refract") && (vmt.shaderName != "Water") )
			Error("$refracttint only works with the Refract or Water shaders!")

		QToolButton* colorWidget = (vmt.shader == Shader::S_Water)
			? ui->toolButton_refractionTint
			: ui->toolButton_refractTint;

		applyBackgroundColor("$refracttint", value, colorWidget,
			ui->horizontalSlider_waterRefractColor, ui);

		if( vmt.shaderName == "Water" )
			showWaterRefraction = true;
	}

	if( !( value = vmt.parameters.take("$refractamount") ).isEmpty() )
	{
		if( (vmt.shaderName != "Refract") && (vmt.shaderName != "Water") )
		{
			Error("$refractamount only works with the Refract or Water shaders!")
		}

		double amount;
		if( vmt.shaderName == "Water" )
		{
			if( loadDoubleParameter( &amount, value, "$refractamount" ) )
				ui->doubleSpinBox_refractionAmount->setValue(amount);

			showWaterRefraction = true;
		}
		else
		{
			if( loadDoubleParameter( &amount, value, "$refractamount" ) )
				ui->doubleSpinBox_refractAmount->setValue(amount);
		}
	}

	if( !( value = vmt.parameters.take("$bluramount") ).isEmpty() )
	{
		if( vmt.shaderName != "Refract" )
		{
			Error("$bluramount only works with the Refract shader!")
		}

		double amount;

		if( loadDoubleParameter( &amount, value, "$bluramount", 0.0 ))
			ui->doubleSpinBox_refractBlur->setValue(amount);
	}

	if( !( value = vmt.parameters.take("$refracttexture") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$refracttexture only works with the Water shader!")
		else
			showWaterRefraction = true;

		if( value.compare("_rt_waterrefraction", Qt::CaseInsensitive) )
			Warning("$refracttexture contains unsupported value: \"" + value + "\". Only \"_rt_waterrefraction\" is supported at this time!")
	}

	//----------------------------------------------------------------------------------------//

	// Fog groupbox parameters

	if( !( value = vmt.parameters.take("$fogenable") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$fogenable only works with the Water shader!")
		}

		if( value == "1" )
		{
			ui->checkBox_volumetricFog->setChecked(true);
		}
		else
		{
			if( value != "0" )
			{
				Error("$fogenable has unrecognizable value: \"" + value + "\"!")
			}
		}

		showWaterFog = true;
	}

	if( !( value = vmt.parameters.take("$fogcolor") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$fogcolor only works with the Water shader!")

		applyBackgroundColor("$fogcolor", value,
			ui->toolButton_fogTint,
			ui->horizontalSlider_waterFogColor, ui);

		showWaterFog = true;
	}

	if( !( value = vmt.parameters.take("$fogstart") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$fogstart only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$fogstart" ) )
			ui->spinBox_fogStart->setValue(amount);

		showWaterFog = true;
	}

	if( !( value = vmt.parameters.take("$fogend") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$fogend only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$fogend" ) )
			ui->spinBox_fogEnd->setValue(amount);

		showWaterFog = true;
	}

	if( !( value = vmt.parameters.take("$flashlighttint") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$flashlighttint only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flashlighttint" ) )
			ui->doubleSpinBox_flashlightTint->setValue(amount);

		showWaterFog = true;
	}

	if( !( value = vmt.parameters.take("$lightmapwaterfog") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$lightmapwaterfog only works with the Water shader!")
		}

		if( loadBoolParameter( value, "$lightmapwaterfog" ))
			ui->checkBox_lightmapFog->setChecked(true);

		showWaterFog = true;
	}

	//----------------------------------------------------------------------------------------//

	// Water groupbox parameters

	bool isBottomWater = false;

	if( !( value = vmt.parameters.take("$abovewater") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$abovewater only works with the Water shader!")
		}

		if( value == "0" )
		{
			ui->checkBox_waterBottom->setChecked(true);
			isBottomWater = true;
		}
		else
		{
			if( value != "1" )
			{
				Error("$abovewater has unrecognizable value: \"" + value + "\"!")
			}
		}
	}

	if( !( value = vmt.parameters.take("$normalmap") ).isEmpty() ) {

		if( vmt.shaderName != "Refract" && vmt.shaderName != "Water" )
			Error("$normalmap only works with the Refract or Water shaders!")


		if (!bumpmap.isEmpty())
			Error("$bumpmap should not be used with Water or Refract. Use $normalmap instead!")

		QString texture = validateTexture( "preview_bumpmap1", value, "$normalmap", realGameinfoDir );

		if( !texture.isEmpty() ) {

			if( vmt.shaderName == "Refract" ) {
				ui->lineEdit_refractNormalMap->setText(texture);
				createReconvertAction(ui->lineEdit_refractNormalMap, texture);
			}
			else {
				ui->lineEdit_waterNormalMap->setText(texture);
				createReconvertAction(ui->lineEdit_waterNormalMap, texture);
			}
		}

	} else {

		if (!bumpmap.isEmpty()) {

			Error("$bumpmap should not be used with Water or Refract. Setting $normalmap to the existing $bumpmap value!")

			QString texture = validateTexture( "preview_bumpmap1", bumpmap, "$normalmap", realGameinfoDir );

			if( !texture.isEmpty() )
			{
				if( vmt.shaderName == "Refract" ) {
					ui->lineEdit_refractNormalMap->setText(texture);
					createReconvertAction(ui->lineEdit_refractNormalMap, texture);
				}
				else {
					ui->lineEdit_waterNormalMap->setText(texture);
					createReconvertAction(ui->lineEdit_waterNormalMap, texture);
				}
			}
		}
	}

	if( !( value = vmt.parameters.take("$normalmap2") ).isEmpty() ) {

		if( vmt.shaderName != "Refract" )
			Error("$normalmap2 only works with the Refract shader!")

		QString texture = validateTexture( "preview_bumpmap2", value, "$normalmap2", realGameinfoDir );

		if( !texture.isEmpty() ) {
			ui->lineEdit_refractNormalMap2->setText(texture);
			createReconvertAction(ui->lineEdit_refractNormalMap2, texture);
		}
	}

	if( !( value = vmt.parameters.take("$reflect2dskybox") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$reflect2dskybox only works with the Water shader!")

		//Who cares
		//if (getCurrentGame() != "Portal 2" && getCurrentGame() != "Counter-Strike: Global Offensive")
		//	Error("$reflect2dskybox is only supported in Portal 2 and Counter-Strike: Global Offensive!")

		if( loadBoolParameter( value, "$reflect2dskybox" ))
			ui->checkBox_reflect2dskybox->setChecked(true);

		showWaterReflection = true;
	}

	if( !( value = vmt.parameters.take("$reflect3dskybox") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$reflect3dskybox only works with the Water shader!")

		if( loadBoolParameter( value, "$reflect3dskybox" ))
			ui->checkBox_reflect3dskybox->setChecked(true);

		showWaterReflection = true;
	}

	if( !( value = vmt.parameters.take("$bottommaterial") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$bottommaterial only works with the Water shader!")

		if(isBottomWater)
			Error("$bottommaterial only works with \"$abovewater 1\"!")

		QStringList filenames( value.split("/") );

		bool error = false;
		for( int i = 0; i < filenames.size(); ++i )
		{
			if( filenames.at(i).contains( QRegExp("[\\\\:*?\"<>|/]") ))
			{
				Error("$bottommaterial contains invalid filename value: \"" + value + "\"!")
				error = true;

				break;
			}
		}

		if(!error)
		{
			if( !materialDirectory.isEmpty() )
			{
				materialDirectory.append( "/" + value  + ".vmt" );

				if( !QFile::exists(materialDirectory) )
				{
					Info("$bottommaterial VMT was not found in the regular materials directory. Validation can not be executed!")
				}
				else
				{
					// Time to silently validate the vmt

					VmtFile tmp = vmtParser->loadVmtFileSilently( materialDirectory );

					if( tmp.parameters.contains("$abovewater") )
					{
						if( tmp.parameters.value("$abovewater") != "0" )
						{
							Error("$bottommaterial VMT does not use \"$abovewater 0\"!")
						}
					}

					bool reflectFound = false;
					if( tmp.parameters.contains("$reflecttexture") )
					{
						reflectFound = true;
					}
					else if( tmp.parameters.contains("$reflectamount") )
					{
						reflectFound = true;
					}
					else if( tmp.parameters.contains("$reflect2dskybox") )
					{
						reflectFound = true;
					}
					else if( tmp.parameters.contains("$reflectentities") )
					{
						reflectFound = true;
					}
					else if( tmp.parameters.contains("$reflecttint") )
					{
						reflectFound = true;
					}

					if(reflectFound)
					{
						Error("$bottommaterial VMT contains reflect parameters (like reflecttint) which are not allowed!")
					}
				}
			}
			else
			{
				Info("$bottommaterial VMT was not found in the regular materials directory. Validation can not be executed")
			}

			ui->lineEdit_bottomMaterial->setText(value);
		}

		// Little fix
		//if( ui->comboBox_game->currentIndex() != 0 )
			//ui->toolButton_bottomMaterial->setEnabled(true);
	}

	if( !( value = vmt.parameters.take("$scroll1") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$scroll1 only works with the Water shader!")
		}

		loadScrollParameter( value, "$scroll1", 1 );

		showScroll = true;
	}

	if( !( value = vmt.parameters.take("$scroll2") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$scroll2 only work with the Water shader!")
		}

		loadScrollParameter( value, "$scroll2", 2 );

		showScroll = true;
	}

	bool forcingExpensive = false;
	if( !( value = vmt.parameters.take("$forceexpensive") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$forceexpensive only works with the Water shader!")
		}

		if( loadBoolParameter( value, "$forceexpensive" ))
		{
			ui->checkBox_expensive->setChecked(true);
			forcingExpensive = true;
		}
	}

	if( !( value = vmt.parameters.take("$forcecheap") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$forcecheap only works with the Water shader!")
		}

		if(forcingExpensive)
		{
			Error("A Water shader can be forced to be either cheap: \"$forcecheap 1\" or expensive: \"$forceexpensive 1\"!")
		}
		else
		{
			if( loadBoolParameter( value, "$forcecheap" ))
				ui->checkBox_cheap->setChecked(true);
		}
	}

	if( !( value = vmt.parameters.take("$bumpframe") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$bumpframe only works with the Water shader!")
		}

		int amount;
		if( loadIntParameter( &amount, value, "$bumpframe" ) )
		{
			if( amount < 0 )
			{
				Error("$bumpframe contains a negative value which is not allowed!")
			}
			else
			{
				ui->checkBox_bumpframe->setChecked(true);
				ui->spinBox_bumpframe->setValue(amount);
			}
		}
	}

	// Misc. parameters

	if( !( value = vmt.parameters.take("%compilewater") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("%compilewater only works with the Water shader!")
		}

		if( value == "0" )
		{
			Error("Setting %compilewater to 0 will not compile your Water shader. Changing value to 1!")
		}
		else
		{
			if( value != "1" )
			{
				Error("%compilewater has unrecognizable value: \"" + value + "\"!")
			}
		}
	}

	if( !( value = vmt.parameters.take("$flow_debug") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$flow_debug only works with the Water shader!")

		if( loadBoolParameter( value, "$flow_debug") )
			ui->checkBox_flowDebug->setChecked(true);

		showFlowmap = true;
	}

	//----------------------------------------------------------------------------------------//

	// Water - Reflection groupbox parameters

	if( !( value = vmt.parameters.take("$reflecttint") ).isEmpty() )
	{
		if (vmt.shaderName != "Water")
			Error("$reflecttint only works with the Water shader!")

		if (isBottomWater)
	 		Error("$reflecttint only works with \"$abovewater 1\"!")

		applyBackgroundColor("$reflecttint", value,
			ui->toolButton_reflectionTint,
			ui->horizontalSlider_waterReflectColor, ui);

		showWaterReflection = true;
	}

	if( !( value = vmt.parameters.take("$reflectamount") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$reflectamount only works with the Water shader!")
		}

		if(isBottomWater)
		{
			Error("$reflectamount only works with \"$abovewater 1\"!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$reflectamount" ) )
			ui->doubleSpinBox_reflectionAmount->setValue(amount);

		showWaterReflection = true;
	}

	if( !( value = vmt.parameters.take("$reflecttexture") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$reflecttexture only works with the Water shader!")

		if(isBottomWater)
			Error("$reflecttexture only works with \"$abovewater 1\"!")

		if( value.toLower() != "_rt_waterreflection" ) {

			utils::parseTexture("$reflecttexture", value, ui,
				ui->lineEdit_waterReflectTexture, vmt);
			//Warning("$reflecttexture contains unsupported value: \"" + value + "\". Only \"_rt_waterreflection\" is supported at this time!")

		} else {

			ui->checkBox_realTimeReflection->setChecked(true);
		}

		showWaterReflection = true;
	}

	if( !( value = vmt.parameters.take("$reflectentities") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$reflectentities only works with the Water shader!")

		if(isBottomWater)
			Error("$reflectentities only works with \"$abovewater 1\"!")

		if(!ui->checkBox_realTimeReflection->isChecked())
			Error("$reflectentities only works with \"$reflecttexture _rt_waterreflection\"!")

		if( loadBoolParameter( value, "$reflectentities" ))
			ui->checkBox_reflectEntities->setChecked(true);

		showWaterReflection = true;
	}

	if( !( value = vmt.parameters.take("$reflectonlymarkedentities") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$reflectonlymarkedentities only works with the Water shader!")

		if(isBottomWater)
			Error("$reflectonlymarkedentities only works with \"$abovewater 1\"!")

		if(!ui->checkBox_realTimeReflection->isChecked())
			Error("$reflectonlymarkedentities only works with \"$reflecttexture _rt_waterreflection\"!")

		if( loadBoolParameter( value, "$reflectonlymarkedentities" ))
			ui->checkBox_reflectMarkedEntities->setChecked(true);

		showWaterReflection = true;
	}


	if( !( value = vmt.parameters.take("$reflectskyboxonly") ).isEmpty() ) {

		if( vmt.shaderName != "Water" )
			Error("$reflectskyboxonly only work with the Water shader!")

		if(isBottomWater)
			Error("$reflectskyboxonly only works with \"$abovewater 1\"!")

		if(!ui->checkBox_realTimeReflection->isChecked())
			Error("$reflectskyboxonly only works with \"$reflecttexture _rt_waterreflection\"!")

		if( loadBoolParameter( value, "$reflectskyboxonly" ))
			ui->checkBox_skybox->setChecked(true);

		showWaterReflection = true;
	}

	//----------------------------------------------------------------------------------------//

	// Flow groupbox parameters

	if( !( value = vmt.parameters.take("$flowmap") ).isEmpty() )
	{
		if( vmt.shaderName != "Water" )
		{
			Error("$flowmap only works with the Water shader!")
		}

		utils::parseTexture("$flowmap", value, ui,
			ui->lineEdit_flowMap, vmt);
		createReconvertAction(ui->lineEdit_flowMap, value);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_noise_texture") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_noise_texture only works with the Water shader!")
		}

		utils::parseTexture("$flow_noise_texture", value, ui,
			ui->lineEdit_noiseTexture, vmt);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_normaluvscale") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_normaluvscale only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_normaluvscale" ) )
			ui->doubleSpinBox_normalUV->setValue(amount);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_worlduvscale") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_worlduvscale only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_worlduvscale" ) )
			ui->doubleSpinBox_worldUV->setValue(amount);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_uvscrolldistance") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_uvscrolldistance only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_uvscrolldistance" ) )
			ui->doubleSpinBox_scrollUV->setValue(amount);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_bumpstrength") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_bumpstrength only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_bumpstrength" ) )
			ui->doubleSpinBox_bumpStrength->setValue(amount);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_noise_scale") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_noise_scale only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_noise_scale" ) )
			ui->doubleSpinBox_noiseScale->setValue(amount);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_timescale") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_timescale only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_timescale" ) )
			ui->doubleSpinBox_timeScale->setValue(amount);

		showFlowmap = true;
	}

	if( !( value = vmt.parameters.take("$flow_timeintervalinseconds") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Water", Qt::CaseInsensitive) )
		{
			Error("$flow_timeintervalinseconds only works with the Water shader!")
		}

		double amount;
		if( loadDoubleParameter( &amount, value, "$flow_timeintervalinseconds" ) )
			ui->doubleSpinBox_timeInterval->setValue(amount);

		showFlowmap = true;
	}

	//----------------------------------------------------------------------------------------//

	bool rimLightWrongShader = false;

	if( !(value = vmt.parameters.value("$rimlight")).isEmpty() ) {

		if( vmt.shaderName.compare("VertexLitGeneric", Qt::CaseInsensitive) )
			rimLightWrongShader = true;
		else
			showRimLight = true;
	}

	if( !( value = vmt.parameters.take("$rimlightexponent") ).isEmpty() ) {

		if( vmt.shaderName.compare("VertexLitGeneric", Qt::CaseInsensitive) )
			rimLightWrongShader = true;
		else
			showRimLight = true;

		int amount;
		if( loadIntParameter( &amount, value, "$rimlightexponent" ) )
			ui->spinBox_rimLightExponent->setValue(amount);
	}

	if( !( value = vmt.parameters.take("$rimlightboost") ).isEmpty() )  {

		if( vmt.shaderName.compare("VertexLitGeneric", Qt::CaseInsensitive) )
			rimLightWrongShader = true;
		else
			showRimLight = true;

		double amount;
		if( loadDoubleParameter( &amount, value, "$rimlightboost" ) )
			ui->doubleSpinBox_rimLightBoost->setValue(amount);
	}

	if( !( value = vmt.parameters.take("$rimlightmask") ).isEmpty() ) {

		if( vmt.shaderName.compare("VertexLitGeneric", Qt::CaseInsensitive) )
			rimLightWrongShader = true;
		else
			showRimLight = true;

		if( loadBoolParameter( value, "$rimlightmask") )
			ui->checkBox_rimLightAlphaMask->setChecked(true);
	}

	if( showRimLight && ( vmt.parameters.take("$rimlight") ) != "1" ) // || ( vmt.parameters.take("$rimlightmask") ).compare("1") != 0 )
		Error("Parameters regarding rimlight require \"$rimlight 1\"!")

	if(rimLightWrongShader)
		Error("Parameters regarding rimlight only work with the VertexLitGeneric shader!")

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("$color") ).isEmpty() )
	{
		QString col1 = "$color";
		utils::applyColor(col1, value, ui->toolButton_color1, ui->doubleSpinBox_color1, ui, true);

		showColor = true;
	}

	if( !( value = vmt.parameters.take("$color2") ).isEmpty() )
	{
		QString col1 = "$color2";
		utils::applyColor(col1, value, ui->toolButton_color2, ui->doubleSpinBox_color2, ui);

		showColor = true;
	}

	if( !( value = vmt.parameters.take("$blendtintbybasealpha") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$blendtintbybasealpha") )
			ui->checkBox_blendTint->setChecked(true);

		showColor = true;
	}

	if( !( value = vmt.parameters.take("$tintmasktexture") ).isEmpty() )
	{
		utils::parseTexture("$tintmasktexture", value, ui,
			ui->lineEdit_tintMask, vmt);
		createReconvertAction(ui->lineEdit_tintMask, value);

		showColor = true;
	}

	if( !( value = vmt.parameters.take("$envmapmaskintintmasktexture") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$envmapmaskintintmasttexture") )
			ui->checkBox_tintSpecMask->setChecked(true);

		showColor = true;
	}

	if( !( value = vmt.parameters.take("$notint") ).isEmpty() )
	{
		if( loadBoolParameter( value, "$notint") )
			ui->checkBox_noTint->setChecked(true);

		showColor = true;
	}

	//----------------------------------------------------------------------------------------//

	if( !( value = vmt.parameters.take("include") ).isEmpty() )
	{
		if( vmt.shaderName.compare("Patch", Qt::CaseInsensitive) )
		{
			Error("include parameter only works with the Patch shader!")
		}
		else
		{
			ui->lineEdit_patchVmt->setText(value);
		}
	}

	//----------------------------------------------------------------------------------------//

	if (vmt.parameters.contains("$scale")) {

		if( vmt.shaderName != "Lightmapped_4WayBlend")
			Error("$scale is deprecated. Consider using $basetexturetransform or $bumptransform!")
	}

	//----------------------------------------------------------------------------------------//

	// Filling in additional parameters with the leftover parameter-value-map entries

	QMap< QString, QString >::const_iterator it = vmt.parameters.constBegin();
	uint counter = 0;
	while( it != vmt.parameters.constEnd() )
	{
		reinterpret_cast<ParameterLineEdit*>( ui->formLayout_2->itemAt(counter)->widget() )->setText( it.key() );
		reinterpret_cast<ValueLineEdit*>( ui->formLayout_3->itemAt(counter)->widget() )->setText( it.value() );

		++counter;
		++it;
	}

	//----------------------------------------------------------------------------------------//

	if(showBaseTexture && !ui->action_baseTexture->isChecked())
		ui->action_baseTexture->trigger();

	if(showBaseTexture2 && !ui->action_baseTexture2->isChecked())
		ui->action_baseTexture2->trigger();

	if(vmt.state.showDetail && !ui->action_detail->isChecked())
		ui->action_detail->trigger();

	if(showTransparency && !ui->action_transparency->isChecked()) {
		glWidget_diffuse1->setTransparencyVisible(true);
		ui->action_transparency->trigger();
	}

	if(showColor && !ui->action_color->isChecked()) {
		glWidget_diffuse1->setColorVisible(true);
		ui->action_color->trigger();
	}

	if(vmt.state.showNormalBlend && !ui->action_normalBlend->isChecked()) {
		ui->action_normalBlend->trigger();
	}

	if(vmt.state.showTreeSway && !ui->action_treeSway->isChecked()) {
		ui->action_treeSway->trigger();
	}

	if(vmt.state.showLayerBlend && !ui->action_layerBlend->isChecked()) {
		ui->action_layerBlend->trigger();
	}

	if(vmt.state.showEmissiveBlend && !ui->action_emissiveBlend->isChecked()) {
		ui->action_emissiveBlend->trigger();
	}

	if(showDecal && !ui->action_decal->isChecked())
		ui->action_decal->trigger();

	if(showShadingReflection && !ui->action_reflection->isChecked())
		ui->action_reflection->trigger();

	if(showSelfIllumination && !ui->action_selfIllumination->isChecked())
		ui->action_selfIllumination->trigger();

	if(showRimLight && !ui->action_rimLight->isChecked())
		ui->action_rimLight->trigger();

	if(showFlowmap && !ui->action_flowmap->isChecked())
		ui->action_flowmap->trigger();

	if(showWaterReflection && !ui->action_waterReflection->isChecked())
		ui->action_waterReflection->trigger();

	if(showWaterRefraction && !ui->action_refraction->isChecked())
		ui->action_refraction->trigger();

	if(showWaterFog && !ui->action_fog->isChecked())
		ui->action_fog->trigger();

	if(showBaseTextureTransform && !ui->action_baseTextureTransforms->isChecked())
		ui->action_baseTextureTransforms->trigger();

	if(showBumpmapTransform && !ui->action_bumpmapTransforms->isChecked())
		ui->action_bumpmapTransforms->trigger();

	if(showMiscellaneous && !ui->action_misc->isChecked())
		ui->action_misc->trigger();

	if(showOther && !ui->action_other->isChecked())
		ui->action_other->trigger();

	if(showSprite && !ui->groupBox_sprite->isVisible())
		ui->groupBox_sprite->setVisible(true);

	if(showScroll && !ui->action_scroll->isChecked())
		ui->action_scroll->trigger();

	//----------------------------------------------------------------------------------------//

	updateWindowTitle();
	refreshRequested();

	mParsingVMT = false;
}

VmtFile MainWindow::makeVMT()
{
	VmtFile vmtFile;

	vmtFile.shaderName = ui->comboBox_shader->currentText();


	QString tmp;

	if( !ui->lineEdit_patchVmt->isHidden() &&  !(tmp = ui->lineEdit_patchVmt->text().trimmed()).isEmpty() )
		vmtFile.parameters.insert( "include", tmp );


	if( !ui->groupBox_baseTexture->isHidden() ) {

		if( !( tmp = ui->lineEdit_diffuse->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$basetexture", tmp );

		if( !( tmp = ui->lineEdit_bumpmap->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$bumpmap", tmp );

		if( ui->checkBox_ssbump->isEnabled() && ui->checkBox_ssbump->isChecked() )
			vmtFile.parameters.insert( "$ssbump", "1" );

		if( ui->comboBox_shader->currentText() == "Lightmapped_4WayBlend" ) {

			if( ui->doubleSpinBox_lumstart1->value() != 0.0 )
				vmtFile.parameters.insert( "$texture1_lumstart", Str( ui->doubleSpinBox_lumstart1->value() ));

			if( ui->doubleSpinBox_lumend1->value() != 1.0 )
				vmtFile.parameters.insert( "$texture1_lumend", Str( ui->doubleSpinBox_lumend1->value() ));
		}
	}

	if( !ui->groupBox_normalBlend->isHidden() ) {
		vmtFile.parameters.insert( "$addbumpmaps", "1" );

		if( !( tmp = ui->lineEdit_bump2->text().trimmed() ).isEmpty() ) {
			vmtFile.parameters.insert( "$bumpmap2", tmp );
		}

		if( transformsModified(2) )
		{
			vmtFile.parameters.insert( "$bumptransform2", "center " + Str(ui->doubleSpinBox_bm_centerX_2->value()) + " " + Str(ui->doubleSpinBox_bm_centerY_2->value()) +
																" scale " + Str(ui->doubleSpinBox_bm_scaleX_2->value()) + " " + Str(ui->doubleSpinBox_bm_scaleY_2->value()) +
																" rotate " + Str(ui->doubleSpinBox_bm_angle_2->value())  +
																" translate " + Str(ui->doubleSpinBox_bm_translateX_2->value()) + " " + Str(ui->doubleSpinBox_bm_translateY_2->value()) );
		}

		if( ui->doubleSpinBox_bumpdetailscale->value() != 1.0 )
			vmtFile.parameters.insert( "$bumpdetailscale1", Str( ui->doubleSpinBox_bumpdetailscale->value() ));

		if( ui->doubleSpinBox_bumpdetailscale2->value() != 1.0 )
			vmtFile.parameters.insert( "$bumpdetailscale2", Str( ui->doubleSpinBox_bumpdetailscale2->value() ));

	}


	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_baseTexture2->isHidden() ) {

		if( ui->lineEdit_diffuse2->isEnabled()  && !( tmp = ui->lineEdit_diffuse2->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$basetexture2", tmp );

		if( !( tmp = ui->lineEdit_bumpmap2->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$bumpmap2", tmp );

		if( ui->comboBox_surface2->isEnabled() && !( tmp = ui->comboBox_surface2->currentText() ).isEmpty() )
			vmtFile.parameters.insert( "$surfaceprop2", tmp );

		if( ui->comboBox_shader->currentText() == "Lightmapped_4WayBlend" ) {

			if( ui->doubleSpinBox_uvscalex2->value() != 1.0 || ui->doubleSpinBox_uvscaley2->value() != 1.0 )
				vmtFile.parameters.insert( "$texture2_uvscale", "[" + Str(ui->doubleSpinBox_uvscalex2->value())
										   + " " + Str(ui->doubleSpinBox_uvscaley2->value()) + "]");

			if( ui->doubleSpinBox_lumstart2->value() != 0.0 )
				vmtFile.parameters.insert( "$texture2_lumstart", Str( ui->doubleSpinBox_lumstart2->value() ));

			if( ui->doubleSpinBox_lumend2->value() != 1.0 )
				vmtFile.parameters.insert( "$texture2_lumend", Str( ui->doubleSpinBox_lumend2->value() ));

			if( ui->doubleSpinBox_blendstart2->value() != 0.0 )
				vmtFile.parameters.insert( "$texture2_blendstart", Str( ui->doubleSpinBox_blendstart2->value() ));

			if( ui->doubleSpinBox_blendend2->value() != 1.0 )
				vmtFile.parameters.insert( "$texture2_blendend", Str( ui->doubleSpinBox_blendend2->value() ));

			if( ui->doubleSpinBox_blendfactor2->value() != 1.0 )
				vmtFile.parameters.insert( "$lumblendfactor2", Str( ui->doubleSpinBox_blendfactor2->value() ));

		} else if( ui->comboBox_shader->currentText() != "Lightmapped_4WayBlend") {

			if( !( tmp = ui->lineEdit_blendmodulate->text().trimmed() ).isEmpty() )
				vmtFile.parameters.insert( "$blendmodulatetexture", tmp.toLower() );
		}
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_baseTexture3->isHidden() ) {

		if( !( tmp = ui->lineEdit_diffuse3->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$basetexture3", tmp );

		if( ui->doubleSpinBox_uvscalex3->value() != 1.0 || ui->doubleSpinBox_uvscaley3->value() != 1.0 )
			vmtFile.parameters.insert( "$texture3_uvscale", "[" + Str(ui->doubleSpinBox_uvscalex3->value())
									   + " " + Str(ui->doubleSpinBox_uvscaley3->value()) + "]");

		if( ui->doubleSpinBox_lumstart3->value() != 0.0 )
			vmtFile.parameters.insert( "$texture3_lumstart", Str( ui->doubleSpinBox_lumstart3->value() ));

		if( ui->doubleSpinBox_lumend3->value() != 1.0 )
			vmtFile.parameters.insert( "$texture3_lumend", Str( ui->doubleSpinBox_lumend3->value() ));

		if( ui->doubleSpinBox_blendstart3->value() != 0.0 )
			vmtFile.parameters.insert( "$texture3_blendstart", Str( ui->doubleSpinBox_blendstart3->value() ));

		if( ui->doubleSpinBox_blendend3->value() != 1.0 )
			vmtFile.parameters.insert( "$texture3_blendend", Str( ui->doubleSpinBox_blendend3->value() ));

		if( ui->doubleSpinBox_blendfactor3->value() != 1.0 )
			vmtFile.parameters.insert( "$lumblendfactor3", Str( ui->doubleSpinBox_blendfactor3->value() ));
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_baseTexture4->isHidden() ) {

		if( !( tmp = ui->lineEdit_diffuse4->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$basetexture4", tmp );

		if( ui->doubleSpinBox_uvscalex4->value() != 1.0 || ui->doubleSpinBox_uvscaley4->value() != 1.0 )
			vmtFile.parameters.insert( "$texture4_uvscale", "[" + Str(ui->doubleSpinBox_uvscalex4->value())
									   + " " + Str(ui->doubleSpinBox_uvscaley4->value()) + "]");

		if( ui->doubleSpinBox_lumstart4->value() != 0.0 )
			vmtFile.parameters.insert( "$texture4_lumstart", Str( ui->doubleSpinBox_lumstart4->value() ));

		if( ui->doubleSpinBox_lumend4->value() != 1.0 )
			vmtFile.parameters.insert( "$texture4_lumend", Str( ui->doubleSpinBox_lumend4->value() ));

		if( ui->doubleSpinBox_blendstart4->value() != 0.0 )
			vmtFile.parameters.insert( "$texture4_blendstart", Str( ui->doubleSpinBox_blendstart4->value() ));

		if( ui->doubleSpinBox_blendend4->value() != 1.0 )
			vmtFile.parameters.insert( "$texture4_blendend", Str( ui->doubleSpinBox_blendend4->value() ));

		if( ui->doubleSpinBox_blendfactor4->value() != 1.0 )
			vmtFile.parameters.insert( "$lumblendfactor4", Str( ui->doubleSpinBox_blendfactor4->value() ));
	}

	//---------------------------------------------------------------------------------------//

	if (!ui->groupBox_unlitTwoTexture->isHidden()) {

		if( !(tmp = ui->lineEdit_unlitTwoTextureDiffuse->text().trimmed()).isEmpty() )
			vmtFile.parameters.insert( "$basetexture", tmp );

		if( !(tmp = ui->lineEdit_unlitTwoTextureDiffuse2->text().trimmed()).isEmpty() )
			vmtFile.parameters.insert( "$texture2", tmp );
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_transparency->isHidden() )
	{
		if( ui->doubleSpinBox_opacity->value() != 1.0 )
			vmtFile.parameters.insert( "$alpha", Str( ui->doubleSpinBox_opacity->value() ));

		if( ui->checkBox_transparent->isChecked() )
			vmtFile.parameters.insert( "$translucent", "1" );

		if( ui->checkBox_alphaTest->isChecked() )
			vmtFile.parameters.insert( "$alphatest", "1" );

		if( ui->doubleSpinBox_alphaTestRef->isEnabled() && ui->doubleSpinBox_alphaTestRef->value() != 0.7 )
			vmtFile.parameters.insert( "$alphatestreference", Str( ui->doubleSpinBox_alphaTestRef->value() ));

		if( ui->checkBox_additive->isChecked() )
			vmtFile.parameters.insert( "$additive", "1" );

		if( ui->checkBox_alphaToCoverage->isChecked() )
			vmtFile.parameters.insert( "$allowalphatocoverage", "1" );

		if( ui->checkBox_noCull->isChecked() )
			vmtFile.parameters.insert( "$nocull", "1" );

		if( ui->checkBox_decal->isChecked() )
			vmtFile.parameters.insert( "$decal", "1" );
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_detailTexture->isHidden() )
	{
		if( !ui->lineEdit_detail->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$detail", ui->lineEdit_detail->text().trimmed() );

		if( ui->comboBox_detailBlendMode->isEnabled() && ui->comboBox_detailBlendMode->currentIndex() != 0 )
			vmtFile.parameters.insert( "$detailblendmode", Str( ui->comboBox_detailBlendMode->currentIndex() ));

		if( ui->doubleSpinBox_detailAmount->isEnabled() && ui->doubleSpinBox_detailAmount->value() != 1.0 )
			vmtFile.parameters.insert( "$detailblendfactor", Str( ui->doubleSpinBox_detailAmount->value() ));

		if( ui->checkBox_detailScaleUniform->isChecked() ||
				ui->doubleSpinBox_detailScale->value() == ui->doubleSpinBox_detailScaleY->value() )
			utils::addOnUnequal("$detailscale", ui->doubleSpinBox_detailScale,
							4.0, &vmtFile);
		else {
			vmtFile.parameters.insert( "$detailscale", QString( "[" + Str( ui->doubleSpinBox_detailScale->value()) +
																" " + Str( ui->doubleSpinBox_detailScaleY->value()) + "]" ) );
		}

		if( ui->comboBox_shader->currentText() == "Lightmapped_4WayBlend" ) {

			if( ui->doubleSpinBox_detailAmount2->isEnabled() && ui->doubleSpinBox_detailAmount2->value() != 1.0 )
				vmtFile.parameters.insert( "$detailblendfactor2", Str( ui->doubleSpinBox_detailAmount2->value() ));

			if( ui->doubleSpinBox_detailAmount3->isEnabled() && ui->doubleSpinBox_detailAmount3->value() != 1.0 )
				vmtFile.parameters.insert( "$detailblendfactor3", Str( ui->doubleSpinBox_detailAmount3->value() ));

			if( ui->doubleSpinBox_detailAmount4->isEnabled() && ui->doubleSpinBox_detailAmount4->value() != 1.0 )
				vmtFile.parameters.insert( "$detailblendfactor4", Str( ui->doubleSpinBox_detailAmount4->value() ));
		}

		if( ui->comboBox_shader->currentText() == "WorldVertexTransition" ) {

			if( ui->doubleSpinBox_detailAmount2->isEnabled() && ui->doubleSpinBox_detailAmount2->value() != 1.0 )
				vmtFile.parameters.insert( "$detailblendfactor2", Str( ui->doubleSpinBox_detailAmount2->value() ));

			if( !ui->lineEdit_detail2->text().trimmed().isEmpty() )
				vmtFile.parameters.insert( "$detail2", ui->lineEdit_detail2->text().trimmed() );

			/*if( ui->comboBox_detailBlendMode2->isEnabled() && ui->comboBox_detailBlendMode2->currentIndex() != 0 )
				vmtFile.parameters.insert( "$detailblendmode2", Str( ui->comboBox_detailBlendMode2->currentIndex() ));*/

			if( ui->checkBox_detailScaleUniform2->isChecked() ||
					ui->doubleSpinBox_detailScale2->value() == ui->doubleSpinBox_detailScaleY2->value() )
				utils::addOnUnequal("$detailscale2", ui->doubleSpinBox_detailScale2,
								4.0, &vmtFile);
			else {
				vmtFile.parameters.insert( "$detailscale2", QString( "[" + Str( ui->doubleSpinBox_detailScale2->value()) +
																	" " + Str( ui->doubleSpinBox_detailScaleY2->value()) + "]" ) );
			}
		}
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_textureDecal->isHidden() )
	{
		if( !ui->lineEdit_decal->text().trimmed().isEmpty() ) {
			vmtFile.parameters.insert( "$decaltexture", ui->lineEdit_decal->text().trimmed() );
			vmtFile.parameters.insert( "$decalblendmode", Str( ui->comboBox_decalBlendMode->currentIndex() ));
		}
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_textureOther->isHidden() )
	{
		if( !( tmp = ui->lineEdit_lightWarp->text().trimmed() ).isEmpty() && ui->lineEdit_lightWarp->isEnabled() )
			vmtFile.parameters.insert( "$lightwarptexture", tmp.toLower() );

		if( ui->doubleSpinBox_seamlessScale->isEnabled() && ui->doubleSpinBox_seamlessScale->value() != 0.0 )
			vmtFile.parameters.insert( "$seamless_scale", Str( ui->doubleSpinBox_seamlessScale->value() ));

		tmp = toParameterBig(utils::getBG(ui->toolButton_reflectivity),
							 ui->doubleSpinBox_reflectivity->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$reflectivity", tmp );

		tmp = toParameterBig(utils::getBG(ui->toolButton_reflectivity_2),
							 ui->doubleSpinBox_reflectivity_2->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$reflectivity2", tmp );
	}

	shadingreflection::insertParametersFromViews(&vmtFile, ui);

	if( !ui->groupBox_selfIllumination->isHidden() ) {

		vmtFile.parameters.insert( "$selfillum", "1" );

		if( !ui->lineEdit_maskTexture->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$selfillummask", ui->lineEdit_maskTexture->text().trimmed() );

		if( ui->checkBox_envmapAlpha->isChecked() )
			vmtFile.parameters.insert( "$selfillum_envmapmask_alpha", "1" );

		tmp = toParameterBig(utils::getBG(ui->toolButton_selfIllumTint),
							 ui->doubleSpinBox_selfIllumTint->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$selfillumtint", tmp );

		if( ui->doubleSpinBox_selfIllumFresnelMin->value() != 0.0 ||
			ui->doubleSpinBox_selfIllumFresnelMax->value() != 1.0 ||
			ui->doubleSpinBox_selfIllumFresnelExp->value() != 1.0 )
		{
			vmtFile.parameters.insert( "$selfillumfresnelminmaxexp", QString( "[" + Str(ui->doubleSpinBox_selfIllumFresnelMin->value()) +
																	   " " + Str(ui->doubleSpinBox_selfIllumFresnelMax->value()) +
																	   " " + Str(ui->doubleSpinBox_selfIllumFresnelExp->value()) + "]" ) );
		}
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_phong->isHidden() )
	{
		vmtFile.parameters.insert( "$phong", "1" );

		if( ui->doubleSpinBox_boost->value() != 1.0 )
			vmtFile.parameters.insert( "$phongboost", Str( ui->doubleSpinBox_boost->value() ));

		if( ui->doubleSpinBox_albedoBoost->value() != 1.0 )
			vmtFile.parameters.insert( "$phongalbedoboost", Str( ui->doubleSpinBox_albedoBoost->value() ));

		/*if( ui->doubleSpinBox_fresnelRangesX->value() != 0.0 ||
			ui->doubleSpinBox_fresnelRangesY->value() != 0.5 ||
			ui->doubleSpinBox_fresnelRangesZ->value() != 1.0 )*/
		vmtFile.parameters.insert( "$phongfresnelranges", QString( "[" + Str(ui->doubleSpinBox_fresnelRangesX->value()) +
																   " " + Str(ui->doubleSpinBox_fresnelRangesY->value()) +
																   " " + Str(ui->doubleSpinBox_fresnelRangesZ->value()) + "]" ) );

		// Note that we always place $phongexponent in the VMT
		// regardless if it has the default value 5
		if (ui->spinBox_exponent->isEnabled()) {
			vmtFile.parameters.insert("$phongexponent",
				Str(ui->spinBox_exponent->value()));
		}

		tmp = toParameter(utils::getBG(ui->toolButton_phongTint));
		if( ui->toolButton_phongTint->isEnabled() && tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$phongtint", tmp );

		if( ui->checkBox_halfLambert->isChecked() )
			vmtFile.parameters.insert( "$halflambert", "1" );

		if( ui->checkBox_disableHalfLambert->isChecked() )
			vmtFile.parameters.insert( "$phongdisablehalflambert", "1" );

		if( ui->lineEdit_exponentTexture->isEnabled() && !ui->lineEdit_exponentTexture->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$phongexponenttexture", ui->lineEdit_exponentTexture->text().trimmed() );

		if( !ui->lineEdit_phongWarp->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$phongwarptexture", ui->lineEdit_phongWarp->text().trimmed() );

		if( ui->checkBox_baseLuminanceMask->isChecked() )
			vmtFile.parameters.insert( "$basemapluminancephongmask", "1" );

		if( ui->checkBox_exponentBaseAlpha->isChecked() )
			vmtFile.parameters.insert( "$basemapalphaphongmask", "1" );

		if( ui->checkBox_albedoTint->isChecked() )
			vmtFile.parameters.insert( "$phongalbedotint", "1" );
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_phongBrush->isHidden() )
	{
		vmtFile.parameters.insert( "$phong", "1" );
		vmtFile.parameters.insert( "$phongexponent", Str( ui->spinBox_exponent2->value() ) );

		float multiplier = ui->doubleSpinBox_phongAmount->value();
		const QColor &color = (utils::getBG(ui->toolButton_phongAmount));
		QString red = QString::number(color.redF() * multiplier, 'f', 2);
		QString green = QString::number(color.greenF() * multiplier, 'f', 2);
		QString blue = QString::number(color.blueF() * multiplier, 'f', 2);

		if(red.endsWith('0'))
			red.chop(1);
		if(green.endsWith('0'))
			green.chop(1);
		if(blue.endsWith('0'))
			blue.chop(1);

		if(red.endsWith(".0"))
			red.chop(2);
		if(green.endsWith(".0"))
			green.chop(2);
		if(blue.endsWith(".0"))
			blue.chop(2);

		if( red == "0.0")
			red = "0";
		if( green == "0.0" )
			green = "0";
		if( blue == "0.0" )
			blue = "0.0";

		vmtFile.parameters.insert( "$phongamount", QString("[" + red + " " + green + " " + blue + " " + Str( ui->doubleSpinBox_phongAmountAlpha->value()) + "]" ));

		if ( ui->doubleSpinBox_maskContrast->value() != 1.0 || ui->doubleSpinBox_maskBrightness->value() != 1 ) {
			vmtFile.parameters.insert( "$phongmaskcontrastbrightness", QString( "[" + Str( ui->doubleSpinBox_maskContrast->value()) +
																				" " + Str( ui->doubleSpinBox_maskBrightness->value()) + "]" ) );
		}

		if( !ui->label_spec_amount2->isHidden() ) {

			if ( ui->spinBox_exponent2->value() != ui->spinBox_spec_exponent2->value() )
				vmtFile.parameters.insert( "$phongexponent2", Str( ui->spinBox_spec_exponent2->value() ) );

			if ( ui->doubleSpinBox_maskContrast->value() != ui->doubleSpinBox_spec_maskContrast2->value() || ui->doubleSpinBox_maskBrightness->value() != ui->doubleSpinBox_spec_maskBrightness2->value() ) {
				vmtFile.parameters.insert( "$phongmaskcontrastbrightness2", QString( "[" + Str( ui->doubleSpinBox_spec_maskContrast2->value()) +
																					" " + Str( ui->doubleSpinBox_spec_maskBrightness2->value()) + "]" ) );
			}

			float multiplier2 = ui->doubleSpinBox_spec_amount2->value();
			if ( multiplier2 != multiplier || utils::getBG(ui->toolButton_phongAmount) != utils::getBG(ui->toolButton_spec_amount2) ) {

				const QColor color2 = (utils::getBG(ui->toolButton_spec_amount2));
				QString red2 = QString::number(color2.redF() * multiplier2, 'f', 2);
				QString green2 = QString::number(color2.greenF() * multiplier2, 'f', 2);
				QString blue2 = QString::number(color2.blueF() * multiplier2, 'f', 2);

				if(red2.endsWith('0'))
					red2.chop(1);
				if(green2.endsWith('0'))
					green2.chop(1);
				if(blue2.endsWith('0'))
					blue2.chop(1);

				if(red2.endsWith(".0"))
					red2.chop(2);
				if(green2.endsWith(".0"))
					green2.chop(2);
				if(blue2.endsWith(".0"))
					blue2.chop(2);

				if( red2 == "0.0")
					red2 = "0";
				if( green2 == "0.0" )
					green2 = "0";
				if( blue2 == "0.0" )
					blue2 = "0.0";

				vmtFile.parameters.insert( "$phongamount2", QString("[" + red2 + " " + green2 + " " + blue2 + " " + Str( ui->doubleSpinBox_spec_amountAlpha2->value()) + "]" ));
			}
		}


		if( ui->checkBox_phongBaseAlpha->isChecked() )
			vmtFile.parameters.insert( "$basealphaenvmapmask", "1" );

		if( ui->checkBox_phongNormalAlpha->isChecked() )
			vmtFile.parameters.insert( "$normalmapalphaenvmapmask", "1" );
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_rimLight->isHidden() )
	{
		vmtFile.parameters.insert( "$phong", "1" ); // Needed for rimlight to work
		vmtFile.parameters.insert( "$rimlight", "1" );

		vmtFile.parameters.insert( "$rimlightexponent", Str(ui->spinBox_rimLightExponent->value()) );

		vmtFile.parameters.insert( "$rimlightboost", Str(ui->doubleSpinBox_rimLightBoost->value()) );

		if( ui->checkBox_rimLightAlphaMask->isChecked() )
			vmtFile.parameters.insert( "$rimlightmask", "1" );
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_layerblend->isHidden() )
	{
		if( ui->checkBox_newLayerBlend->isChecked() )
			vmtFile.parameters.insert( "$newlayerblending", "1" );

		if( ui->doubleSpinBox_layerBlendSoftness->value() != 0.5 )
			vmtFile.parameters.insert( "$blendsoftness",
									   Str( ui->doubleSpinBox_layerBlendSoftness->value() ));

		tmp = toParameterBig(utils::getBG(ui->toolButton_layer1tint),
							 ui->doubleSpinBox_layer1tint->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$layertint1", tmp );

		tmp = toParameterBig(utils::getBG(ui->toolButton_layer2tint),
							 ui->doubleSpinBox_layer2tint->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$layertint2", tmp );

		tmp = toParameterBig(utils::getBG(ui->toolButton_layerBorderTint),
							 ui->doubleSpinBox_layerBorderTint->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$layerbordertint", tmp );

		if( ui->doubleSpinBox_layerBorderOffset->value() != 0.0 )
			vmtFile.parameters.insert( "$layerborderoffset",
									   Str( ui->doubleSpinBox_layerBorderOffset->value() ));

		if( ui->doubleSpinBox_layerBorderSoftness->value() != 0.5 )
			vmtFile.parameters.insert( "$layerbordersoftness",
									   Str( ui->doubleSpinBox_layerBorderSoftness->value() ));

		if( ui->doubleSpinBox_layerBorderStrength->value() != 0.5 )
			vmtFile.parameters.insert( "$layerborderstrength",
									   Str( ui->doubleSpinBox_layerBorderStrength->value() ));

		if( ui->doubleSpinBox_layerEdgeOffset->value() != 0.0 )
			vmtFile.parameters.insert( "$layeredgeoffset",
									   Str( ui->doubleSpinBox_layerEdgeOffset->value() ));

		if( ui->doubleSpinBox_layerEdgeSoftness->value() != 0.5 )
			vmtFile.parameters.insert( "$layeredgesotfness",
									   Str( ui->doubleSpinBox_layerEdgeSoftness->value() ));

		if( ui->doubleSpinBox_layerEdgeStrength->value() != 0.5 )
			vmtFile.parameters.insert( "$layeredgestrength",
									   Str( ui->doubleSpinBox_layerEdgeStrength->value() ));


		if( ui->checkBox_layerEdgeNormal->isChecked() )
			vmtFile.parameters.insert( "$layeredgenormal", "1" );

		if( ui->checkBox_layerEdgePunchin->isChecked() )
			vmtFile.parameters.insert( "$layeredgepunchin", "1" );

	}

	if( !ui->groupBox_emissiveBlend->isHidden() )
	{
		vmtFile.parameters.insert( "$emissiveblendenabled", "1" );

		if( !ui->lineEdit_emissiveBlendTexture->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$emissiveblendtexture", ui->lineEdit_emissiveBlendTexture->text().trimmed() );

		if( !ui->lineEdit_emissiveBlendBaseTexture->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$emissiveblendbasetexture", ui->lineEdit_emissiveBlendBaseTexture->text().trimmed() );

		if( !ui->lineEdit_emissiveBlendFlowTexture->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$emissiveblendflowtexture", ui->lineEdit_emissiveBlendFlowTexture->text().trimmed() );

		tmp = toParameterBig(utils::getBG(ui->toolButton_emissiveBlendTint),
							 ui->doubleSpinBox_emissiveBlendTint->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$emissiveblendtint", tmp );

		if( ui->doubleSpinBox_emissiveBlendStrength->value() != 1.0 )
			vmtFile.parameters.insert( "$emissiveblendstrength",
									   Str( ui->doubleSpinBox_emissiveBlendStrength->value() ));

		vmtFile.parameters.insert( "$emissiveblendscrollvector", QString( "[" + Str(ui->doubleSpinBox_emissiveBlendScrollX->value()) +
																		  " " + Str(ui->doubleSpinBox_emissiveBlendScrollY->value()) + "]" ) );
	}

	if( !ui->groupBox_treeSway->isHidden() )
	{
		vmtFile.parameters.insert( "$treesway", "1" );

		vmtFile.parameters.insert( "$treeswayheight", Str( ui->spinBox_treeswayHeight->value() ));

		vmtFile.parameters.insert( "$treeswayradius", Str( ui->spinBox_treeswayRadius->value() ));

		vmtFile.parameters.insert( "$treeswayspeedlerpend", Str( ui->spinBox_treeswaySpeedLerpEnd->value() ));

		vmtFile.parameters.insert( "$treeswayspeedlerpstart", Str( ui->spinBox_treeswaySpeedLerpStart->value() ));

		vmtFile.parameters.insert( "$treeswayscrumblefrequency", Str( ui->spinBox_treeswayScrumbleFrequency->value() ));

		vmtFile.parameters.insert( "$treeswayfalloffexp", Str( ui->spinBox_treeswayFalloff->value() ));

		vmtFile.parameters.insert( "$treeswayscrumblefalloffexp", Str( ui->spinBox_treeswayScrumbleFalloff->value() ));

		vmtFile.parameters.insert( "$treeswaystartheight", Str( ui->doubleSpinBox_treeswayStartHeight->value() ));

		vmtFile.parameters.insert( "$treeswaystartradius", Str( ui->doubleSpinBox_treeswayStartRadius->value() ));

		vmtFile.parameters.insert( "$treeswaystrength", Str( ui->doubleSpinBox_treeswayStrength->value() ));

		vmtFile.parameters.insert( "$treeswayspeedhighwindmultiplier", Str( ui->doubleSpinBox_treeswayspeedHighWind->value() ));

		vmtFile.parameters.insert( "$treeswayscrumblestrength", Str( ui->doubleSpinBox_treeswayScrumbleStrength->value() ));

		vmtFile.parameters.insert( "$treeswayspeed", Str( ui->doubleSpinBox_treeswaySpeed->value() ));

		vmtFile.parameters.insert( "$treeswayscrumblespeed", Str( ui->doubleSpinBox_treeswayScrumbleSpeed->value() ));
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_misc->isHidden() )
	{
		if( ui->checkBox_model->isEnabled() && ui->checkBox_model->isChecked() )
			vmtFile.parameters.insert( "$model", "1" );

		if( ui->checkBox_vertexColor->isChecked() )
			vmtFile.parameters.insert( "$vertexcolor", "1" );

		if( ui->checkBox_vertexAlpha->isChecked() )
			vmtFile.parameters.insert( "$vertexalpha", "1" );

		if( ui->checkBox_ignoreZ->isChecked() )
			vmtFile.parameters.insert( "$ignorez", "1" );

		if( ui->checkBox_noFog->isChecked() )
			vmtFile.parameters.insert( "$nofog", "1" );

		if( ui->checkBox_noLod->isChecked() )
			vmtFile.parameters.insert( "$nolod", "1" );

		if( ui->checkBox_noDecal->isChecked() )
			vmtFile.parameters.insert( "$nodecal", "1" );

		if( ui->checkBox_noFullBright->isChecked() )
			vmtFile.parameters.insert( "$no_fullbright", "1" );

		if( ui->checkBox_disableCsmLookup->isChecked() )
			vmtFile.parameters.insert( "$disablecsmlookup", "1" );

		if( !ui->lineEdit_keywords->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "%keywords", ui->lineEdit_keywords->text().trimmed() );

		if( !ui->lineEdit_toolTexture->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "%tooltexture", ui->lineEdit_toolTexture->text().trimmed() );
	}

	if( !ui->groupBox_color->isHidden() ) {

		tmp = toParameterBig(utils::getBG(ui->toolButton_color1),
							 ui->doubleSpinBox_color1->value(), true);
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$color", tmp );

		tmp = toParameterBig(utils::getBG(ui->toolButton_color2),
							  ui->doubleSpinBox_color2->value());
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$color2", tmp );

		if( ui->checkBox_blendTint->isChecked() )
			vmtFile.parameters.insert( "$blendtintbybasealpha", "1" );

		if( !ui->lineEdit_tintMask->text().trimmed().isEmpty() )
			vmtFile.parameters.insert( "$tintmasktexture", ui->lineEdit_tintMask->text().trimmed() );

		if( ui->checkBox_noTint->isChecked() )
			vmtFile.parameters.insert( "$notint", "1" );
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_baseTextureTransforms->isHidden() )
	{
		if( transformsModified(0) )
		{
			vmtFile.parameters.insert( "$basetexturetransform", "center " + Str(ui->doubleSpinBox_bt_centerX->value()) + " " + Str(ui->doubleSpinBox_bt_centerY->value()) +
																" scale " + Str(ui->doubleSpinBox_bt_scaleX->value()) + " " + Str(ui->doubleSpinBox_bt_scaleY->value()) +
																" rotate " + Str(ui->doubleSpinBox_bt_angle->value())  +
																" translate " + Str(ui->doubleSpinBox_bt_translateX->value()) + " " + Str(ui->doubleSpinBox_bt_translateY->value()) );
		}
	}

	if( !ui->groupBox_bumpmapTransforms->isHidden() )
	{
		if( transformsModified(1) )
		{
			vmtFile.parameters.insert( "$bumptransform", "center " + Str(ui->doubleSpinBox_bm_centerX->value()) + " " + Str(ui->doubleSpinBox_bm_centerY->value()) +
														 " scale " + Str(ui->doubleSpinBox_bm_scaleX->value()) + " " + Str(ui->doubleSpinBox_bm_scaleY->value()) +
														 " rotate " + Str(ui->doubleSpinBox_bm_angle->value())  +
														 " translate " + Str(ui->doubleSpinBox_bm_translateX->value()) + " " + Str(ui->doubleSpinBox_bm_translateY->value()) );
		}
	}

	//---------------------------------------------------------------------------------------//

	if( !ui->groupBox_sprite->isHidden() )
	{
		switch( ui->comboBox_spriteOrientation->currentIndex() )
		{
		case 0:

			vmtFile.parameters.insert( "$spriteorientation", "parallel_upright" );
			break;

		case 1:

			vmtFile.parameters.insert( "$spriteorientation", "facing_upright" );
			break;

		case 2:

			vmtFile.parameters.insert( "$spriteorientation", "vp_parallel" );
			break;

		case 3:

			vmtFile.parameters.insert( "$spriteorientation", "oriented" );
			break;

		case 4:

			vmtFile.parameters.insert( "$spriteorientation", "vp_parallel_oriented" );
			break;
		}

		vmtFile.parameters.insert( "$spriteorigin", "[" + Str( ui->doubleSpinBox_spriteX->value() ) + " " +
														  Str( ui->doubleSpinBox_spriteY->value() ) + "]");
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_refract->isHidden() )
	{
		vmtFile.parameters.insert( "$refract", "1" );

		if( !( tmp = ui->lineEdit_refractNormalMap->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$normalmap", tmp );

		if( !( tmp = ui->lineEdit_refractNormalMap2->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$normalmap2", tmp );

		if( !( tmp = ui->lineEdit_refractTexture->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$refracttinttexture", tmp );

		tmp = toParameter(utils::getBG(ui->toolButton_refractTint));
		if( tmp != "[1 1 1]" )
			vmtFile.parameters.insert( "$refracttint", tmp );

		if( ui->doubleSpinBox_refractAmount->value() != 0.0 )
			vmtFile.parameters.insert( "$refractamount", Str( ui->doubleSpinBox_refractAmount->value() ));

		if( ui->doubleSpinBox_refractBlur->value() != 0.0 )
			vmtFile.parameters.insert( "$bluramount", Str( ui->doubleSpinBox_refractBlur->value() ));
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_water->isHidden() ) {

		if(getCurrentGame() != "") {

			QString selectedGame = getCurrentGame();

			if( !(selectedGame == "Left 4 Dead 2" ||
				  selectedGame == "Portal 2" ||
				  selectedGame == "Alien Swarm" ||
				  selectedGame == "Dota 2" ||
				  selectedGame == "Counter-Strike: Global Offensive" )) {

			}

		}

		if( !( tmp = ui->lineEdit_waterNormalMap->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$normalmap", tmp );

		if( !( tmp = ui->lineEdit_bottomMaterial->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$bottommaterial", tmp );

		vmtFile.parameters.insert( "$abovewater", Str(!ui->checkBox_waterBottom->isChecked()) );

		if( ui->checkBox_expensive->isChecked() )
			vmtFile.parameters.insert( "$forceexpensive", "1" );

		if( ui->checkBox_cheap->isChecked() )
			vmtFile.parameters.insert( "$forcecheap", "1" );

		if( ui->checkBox_bumpframe->isChecked() )
			vmtFile.parameters.insert( "$bumpframe", Str( ui->spinBox_bumpframe->value() ));

		//----------------------------------------------------------------------------------------//

		vmtFile.parameters.insert( "$surfaceprop", ui->comboBox_waterSurface->currentText() );

		vmtFile.parameters.insert( "%compilewater", "1" );

	} else {

		if( !ui->groupBox_refract->isHidden() && !( tmp = ui->comboBox_refractSurface->currentText() ).isEmpty() ) {

			vmtFile.parameters.insert( "$surfaceprop", tmp );

		} else if( !ui->groupBox_baseTexture->isHidden() && !( tmp = ui->comboBox_surface->currentText() ).isEmpty() ) {

			vmtFile.parameters.insert( "$surfaceprop", tmp );

		} else if( !ui->groupBox_unlitTwoTexture->isHidden() && !( tmp = ui->comboBox_unlitTwoTextureSurface->currentText() ).isEmpty()) {

			vmtFile.parameters.insert( "$surfaceprop", tmp );
		}
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_scroll->isHidden() )
	{
		if( ui->doubleSpinBox_scrollX1->value() != 0.0 || ui->doubleSpinBox_scrollY1->value() != 0.0 )
			vmtFile.parameters.insert( "$scroll1", "[" + Str(ui->doubleSpinBox_scrollX1->value()) + " " + Str(ui->doubleSpinBox_scrollY1->value()) + "]");

		if( ui->doubleSpinBox_scrollX2->value() != 0.0 || ui->doubleSpinBox_scrollY2->value() != 0.0 )
			vmtFile.parameters.insert( "$scroll2", "[" + Str(ui->doubleSpinBox_scrollX2->value()) + " " + Str(ui->doubleSpinBox_scrollY2->value()) + "]");
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_waterFlow->isHidden() )
	{
		if( !( tmp = ui->lineEdit_flowMap->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$flowmap", tmp );

		if( !( tmp = ui->lineEdit_noiseTexture->text().trimmed() ).isEmpty() )
			vmtFile.parameters.insert( "$flow_noise_texture", tmp );

		if( ui->doubleSpinBox_normalUV->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_normaluvscale", Str( ui->doubleSpinBox_normalUV->value() ));

		if( ui->doubleSpinBox_worldUV->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_worlduvscale", Str( ui->doubleSpinBox_worldUV->value() ));

		if( ui->doubleSpinBox_scrollUV->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_uvscrolldistance", Str( ui->doubleSpinBox_scrollUV->value() ));

		if( ui->doubleSpinBox_bumpStrength->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_bumpstrength", Str( ui->doubleSpinBox_bumpStrength->value() ));

		if( ui->doubleSpinBox_noiseScale->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_noise_scale", QString::number( ui->doubleSpinBox_noiseScale->value(), 'f', 5));

		if( ui->doubleSpinBox_timeScale->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_timescale", Str( ui->doubleSpinBox_timeScale->value() ));

		if( ui->doubleSpinBox_timeInterval->value() != 0.0 )
			vmtFile.parameters.insert( "$flow_timeintervalinseconds", Str( ui->doubleSpinBox_timeInterval->value() ));

		if( ui->checkBox_flowDebug->isChecked() )
			vmtFile.parameters.insert( "$flow_debug", "1" );
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_waterReflection->isHidden() ) {

		tmp = toWaterParameter(utils::getBG(ui->toolButton_reflectionTint));
		if( tmp != "{255 255 255}" )
			vmtFile.parameters.insert( "$reflecttint", tmp );

		if( ui->doubleSpinBox_reflectionAmount->value() != 0.0 )
			vmtFile.parameters.insert( "$reflectamount", Str( ui->doubleSpinBox_reflectionAmount->value() ));

		if (ui->checkBox_reflect2dskybox->isVisible() && ui->checkBox_reflect2dskybox->isChecked())
			vmtFile.parameters.insert( "$reflect2dskybox", "1" );

		if (ui->checkBox_reflect3dskybox->isVisible() && ui->checkBox_reflect3dskybox->isChecked())
			vmtFile.parameters.insert( "$reflect3dskybox", "1" );

		if( ui->checkBox_realTimeReflection->isChecked() ) {

			vmtFile.parameters.insert( "$reflecttexture", "_rt_waterreflection" );

		} else if( !( tmp = ui->lineEdit_waterReflectTexture->text().trimmed() ).isEmpty() && ui->lineEdit_waterReflectTexture->isEnabled() ) {

			vmtFile.parameters.insert( "$reflecttexture", tmp );
			//vmtFile.parameters.insert( "$envmap", "env_cubemap" );
		}

		if( ui->checkBox_skybox->isChecked() && ui->checkBox_skybox->isEnabled() )
			vmtFile.parameters.insert( "$reflectskyboxonly", "1" );

		if( ui->checkBox_reflectMarkedEntities->isChecked() && ui->checkBox_reflectMarkedEntities->isEnabled() )
			vmtFile.parameters.insert( "$reflectonlymarkedentities", "1" );

		if( ui->checkBox_reflectEntities->isChecked() && ui->checkBox_reflectEntities->isEnabled() )
			vmtFile.parameters.insert( "$reflectentities", "1" );
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_waterRefraction->isHidden() ) {

		vmtFile.parameters.insert("$refract", "1");

		vmtFile.parameters.insert("$refracttexture", "_rt_waterrefraction");

		tmp = toWaterParameter(utils::getBG(ui->toolButton_refractionTint));
		if( tmp != "{255 255 255}" )
			vmtFile.parameters.insert( "$refracttint", tmp );

		if( ui->doubleSpinBox_refractionAmount->value() != 0.0 )
			vmtFile.parameters.insert( "$refractamount", Str( ui->doubleSpinBox_refractionAmount->value() ));
	}

	//----------------------------------------------------------------------------------------//

	if( !ui->groupBox_waterFog->isHidden() ) {

		tmp = toWaterParameter(utils::getBG(ui->toolButton_fogTint));
		if( tmp != "{255 255 255}" )
			vmtFile.parameters.insert( "$fogcolor", tmp );

		vmtFile.parameters.insert( "$fogstart", Str( ui->spinBox_fogStart->value() ));

		vmtFile.parameters.insert( "$fogend", Str( ui->spinBox_fogEnd->value() ));

		if( ui->checkBox_lightmapFog->isChecked() )
			vmtFile.parameters.insert( "$lightmapwaterfog", "1" );

		if( ui->checkBox_volumetricFog->isChecked() )
			vmtFile.parameters.insert( "$fogenable", "1" );
		else
			vmtFile.parameters.insert( "$fogenable", "0" );

		if( ui->doubleSpinBox_flashlightTint->value() != 0.0 )
			vmtFile.parameters.insert( "$flashlighttint", Str( ui->doubleSpinBox_flashlightTint->value() ));
	}

	//----------------------------------------------------------------------------------------//

	QString tmp2, tmp3;
	for (int i = 0; i < ui->formLayout_2->count(); ++i ) {
		tmp2 = reinterpret_cast<QLineEdit*>( ui->formLayout_2->itemAt(i)->widget() )->text().trimmed();
		tmp3 = reinterpret_cast<QLineEdit*>( ui->formLayout_3->itemAt(i)->widget() )->text().trimmed();

		if (!tmp2.isEmpty() && !tmp3.isEmpty()) {
			vmtFile.parameters.insert(tmp2, tmp3);
		}
	}

	//----------------------------------------------------------------------------------------//

	vmtFile.subGroups = vmtParser->formatSubGroups( ui->textEdit_proxies->toPlainText(), 0 );

	vmtFile.directory = vmtParser->lastVMTFile().directory;
	vmtFile.fileName = vmtParser->lastVMTFile().fileName;

	return vmtFile;
}

void MainWindow::resetWidgets() {

	mIsResetting = true;

	mVMTLoaded = false;

	updateWindowTitle();

	fresnelYStart = 0.5;

	hideParameterGroupboxes();

	ui->vmtPreviewTextEdit->clear();

	ui->textEdit_proxies->clear();

	mLogger->clear();

	texturesToCopy.clear();

	QDir dir( QDir::currentPath() + "/Cache/Move/");
		dir.setNameFilters( QStringList() << "*.vtf" );
		dir.setFilter(QDir::Files);

	foreach(QString dirFile, dir.entryList())
		dir.remove(dirFile);

	//----------------------------------------------------------------------------------------//

	foreach (GLWidget *glWidget, glWidgets) {
		glWidget->setVisible(false);
	}
	glWidget_diffuse1->reset();
	glWidget_diffuse2->reset();
	glWidget_envmap->setVisible(false);
	glWidget_spec->setVisible(false);

	//----------------------------------------------------------------------------------------//

	ui->lineEdit_diffuse->setEnabled(true);
	ui->lineEdit_diffuse2->setEnabled(true);
	ui->lineEdit_diffuse3->setEnabled(true);
	ui->lineEdit_diffuse4->setEnabled(true);
	ui->lineEdit_detail->setEnabled(true);
	ui->lineEdit_detail2->setEnabled(true);
	ui->lineEdit_exponentTexture->setEnabled(true);
	ui->lineEdit_bumpmap->setEnabled(true);
	ui->lineEdit_bumpmap2->setEnabled(true);

	ui->label_bumpmapAlpha->setVisible(false);
	ui->lineEdit_bumpmapAlpha->setVisible(false);
	ui->toolButton_bumpmapAlpha->setVisible(false);

	ui->label_diffuseAlpha->setVisible(false);
	ui->lineEdit_diffuseAlpha->setVisible(false);
	ui->toolButton_diffuseAlpha->setVisible(false);

	//----------------------------------------------------------------------------------------//

	ui->label_exponent->setEnabled(true);
	ui->spinBox_exponent->setEnabled(true);
	ui->horizontalSlider_exponent->setEnabled(true);

	//----------------------------------------------------------------------------------------//

	clearLineEditAction(ui->lineEdit_diffuse);
	clearLineEditAction(ui->lineEdit_diffuse2);
	clearLineEditAction(ui->lineEdit_diffuse3);
	clearLineEditAction(ui->lineEdit_diffuse4);
	clearLineEditAction(ui->lineEdit_bumpmap);
	clearLineEditAction(ui->lineEdit_bumpmap2);
	clearLineEditAction(ui->lineEdit_blendmodulate);
	clearLineEditAction(ui->lineEdit_bump2);
	clearLineEditAction(ui->lineEdit_detail);
	clearLineEditAction(ui->lineEdit_detail2);
	clearLineEditAction(ui->lineEdit_exponentTexture);
	clearLineEditAction(ui->lineEdit_specmap);
	clearLineEditAction(ui->lineEdit_unlitTwoTextureDiffuse);
	clearLineEditAction(ui->lineEdit_unlitTwoTextureDiffuse2);
	clearLineEditAction(ui->lineEdit_waterNormalMap);
	clearLineEditAction(ui->lineEdit_decal);
	clearLineEditAction(ui->lineEdit_phongWarp);
	clearLineEditAction(ui->lineEdit_diffuseAlpha);
	clearLineEditAction(ui->lineEdit_bumpmapAlpha);
	clearLineEditAction(ui->lineEdit_specmap2);
	clearLineEditAction(ui->lineEdit_tintMask);
	clearLineEditAction(ui->lineEdit_emissiveBlendTexture);
	clearLineEditAction(ui->lineEdit_emissiveBlendBaseTexture);
	clearLineEditAction(ui->lineEdit_emissiveBlendFlowTexture);

	//----------------------------------------------------------------------------------------//

	setCurrentGame(mSettings->lastGame);

	ui->menu_water->setDisabled(true);
	ui->action_phong->setDisabled(true);
	ui->action_phongBrush->setDisabled(true);
	ui->action_rimLight->setDisabled(true);
	ui->action_refract->setDisabled(true);

	ui->comboBox_surface->setCurrentIndex(0);
	ui->comboBox_shader->setCurrentIndex(ui->comboBox_shader->findText("LightmappedGeneric"));
		this->shaderChanged();
	ui->comboBox_surface2->setCurrentIndex(0);

	ui->lineEdit_diffuse->clear();
	ui->lineEdit_bumpmap->clear();
	ui->lineEdit_diffuse2->clear();
	ui->lineEdit_bumpmap2->clear();
	ui->lineEdit_detail->clear();
	ui->lineEdit_detail2->clear();

	ui->lineEdit_decal->clear();
	ui->comboBox_decalBlendMode->setCurrentIndex(0);

	ui->lineEdit_blendmodulate->clear();
	ui->lineEdit_lightWarp->clear();

	ui->label_seamlessScale->setEnabled(true);
	ui->doubleSpinBox_seamlessScale->setEnabled(true);

	ui->checkBox_ssbump->setChecked(false);
	ui->checkBox_transparent->setChecked(false);
	ui->checkBox_noCull->setChecked(false);

	ui->doubleSpinBox_seamlessScale->setValue(0.0);
	ui->doubleSpinBox_opacity->setValue(1.0);
	ui->doubleSpinBox_alphaTestRef->setValue(0.7);

	ui->checkBox_alphaTest->setChecked(false);

	ui->checkBox_transparent->setEnabled(true);
	ui->checkBox_alphaTest->setEnabled(true);

	ui->label_23->setDisabled(true);
	ui->doubleSpinBox_alphaTestRef->setDisabled(true);
	ui->checkBox_alphaToCoverage->setChecked(false);
	ui->checkBox_alphaToCoverage->setDisabled(true);

	ui->comboBox_detailBlendMode->setCurrentIndex(0);
	ui->comboBox_detailBlendMode2->setCurrentIndex(0);

	detailtexture::reset(ui);

	ui->label_detailAmount->setDisabled(true);
	ui->label_detailScale->setDisabled(true);
	ui->doubleSpinBox_detailAmount->setValue(1.0);
	ui->doubleSpinBox_detailAmount->setDisabled(true);

	ui->doubleSpinBox_detailAmount2->setValue(1.0);
	ui->doubleSpinBox_detailAmount2->setDisabled(true);
	ui->doubleSpinBox_detailAmount3->setValue(1.0);
	ui->doubleSpinBox_detailAmount4->setValue(1.0);

	//----------------------------------------------------------------------------------------//

	// Other

	ui->doubleSpinBox_reflectivity->setValue(1.0);
	ui->toolButton_reflectivity->setStyleSheet( "background-color: rgb(255, 255, 255)" );

	ui->doubleSpinBox_reflectivity_2->setValue(1.0);
	ui->toolButton_reflectivity_2->setStyleSheet( "background-color: rgb(255, 255, 255)" );


	//----------------------------------------------------------------------------------------//

	ui->doubleSpinBox_lumstart1->setValue(0.0);
	ui->doubleSpinBox_lumend1->setValue(1.0);

	ui->doubleSpinBox_uvscalex2->setValue(1.0);
	ui->doubleSpinBox_uvscaley2->setValue(1.0);
	ui->doubleSpinBox_lumstart2->setValue(0.0);
	ui->doubleSpinBox_lumend2->setValue(1.0);
	ui->doubleSpinBox_blendstart2->setValue(0.0);
	ui->doubleSpinBox_blendend2->setValue(1.0);
	ui->doubleSpinBox_blendfactor2->setValue(1.0);

	ui->doubleSpinBox_uvscalex3->setValue(1.0);
	ui->doubleSpinBox_uvscaley3->setValue(1.0);
	ui->doubleSpinBox_lumstart3->setValue(0.0);
	ui->doubleSpinBox_lumend3->setValue(1.0);
	ui->doubleSpinBox_blendstart3->setValue(0.0);
	ui->doubleSpinBox_blendend3->setValue(1.0);
	ui->doubleSpinBox_blendfactor3->setValue(1.0);

	ui->doubleSpinBox_uvscalex4->setValue(1.0);
	ui->doubleSpinBox_uvscaley4->setValue(1.0);
	ui->doubleSpinBox_lumstart4->setValue(0.0);
	ui->doubleSpinBox_lumend4->setValue(1.0);
	ui->doubleSpinBox_blendstart4->setValue(0.0);
	ui->doubleSpinBox_blendend4->setValue(1.0);
	ui->doubleSpinBox_blendfactor4->setValue(1.0);

	ui->lineEdit_diffuse3->setText("");
	ui->lineEdit_diffuse4->setText("");

	//----------------------------------------------------------------------------------------//

	ui->lineEdit_unlitTwoTextureDiffuse->setText("");
	ui->lineEdit_unlitTwoTextureDiffuse2->setText("");
	ui->comboBox_unlitTwoTextureSurface->setCurrentIndex(0);

	//----------------------------------------------------------------------------------------//

	ui->checkBox_cubemap->setEnabled(true);
	ui->label_envmap->setEnabled(true);
	ui->lineEdit_envmap->setEnabled(true);
	ui->toolButton_envmap->setEnabled(true);

	ui->lineEdit_envmap->setEnabled(true);
	ui->lineEdit_envmap->clear();
	ui->checkBox_cubemap->setChecked(false);

	//----------------------------------------------------------------------------------------//

	ui->lineEdit_specmap->clear();
	ui->lineEdit_specmap->setDisabled(true);
	ui->toolButton_specmap->setDisabled(true);

	ui->lineEdit_specmap2->clear();
	ui->lineEdit_specmap2->setDisabled(true);
	ui->toolButton_specmap2->setDisabled(true);


	//----------------------------------------------------------------------------------------//

	// Reflection

	ui->label_saturation->setDisabled(true);
	ui->doubleSpinBox_saturation->setValue(1.0);
	ui->doubleSpinBox_saturation->setDisabled(true);
	ui->horizontalSlider_saturation->setDisabled(true);

	ui->label_contrast->setDisabled(true);
	ui->doubleSpinBox_contrast->setValue(0.0);
	ui->doubleSpinBox_contrast->setDisabled(true);
	ui->horizontalSlider_contrast->setDisabled(true);
	ui->checkBox_tintSpecMask->setChecked(false);

	ui->label_envmapTint->setDisabled(true);
	ui->doubleSpinBox_envmapTint->setDisabled(true);
	ui->doubleSpinBox_envmapTint->setValue(1.0);
	ui->horizontalSlider_envmapTint->setDisabled(true);
	ui->toolButton_envmapTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_envmapTint->setDisabled(true);

	ui->checkBox_basealpha->setDisabled(true);
	ui->checkBox_basealpha->setChecked(false);

	ui->checkBox_normalalpha->setDisabled(true);
	ui->checkBox_normalalpha->setChecked(false);

	ui->label_fresnelReflection->setDisabled(true);
	ui->horizontalSlider_fresnelReflection->setDisabled(true);
	ui->doubleSpinBox_fresnelReflection->setValue(0.0);
	ui->doubleSpinBox_fresnelReflection->setDisabled(true);

	ui->label_lightinfluence->setDisabled(true);
	ui->horizontalSlider_envmapLight->setDisabled(true);
	ui->doubleSpinBox_envmapLight->setValue(0.0);
	ui->doubleSpinBox_envmapLight->setDisabled(true);
	ui->doubleSpinBox_envmapLightMin->setValue(0.0);
	ui->doubleSpinBox_envmapLightMin->setDisabled(true);
	ui->doubleSpinBox_envmapLightMax->setValue(1.0);
	ui->doubleSpinBox_envmapLightMax->setDisabled(true);

	ui->label_anisotropy->setDisabled(true);
	ui->horizontalSlider_envmapAniso->setDisabled(true);
	ui->doubleSpinBox_envmapAniso->setValue(0.0);
	ui->doubleSpinBox_envmapAniso->setDisabled(true);

	//----------------------------------------------------------------------------------------//

	phong::resetWidgets(ui);
	normalblend::resetWidgets(ui);
	treesway::resetWidgets(ui);
	layerblend::resetWidgets(ui);
	emissiveblend::resetWidgets(ui);

	//----------------------------------------------------------------------------------------//

	ui->spinBox_rimLightExponent->setValue(5);
	ui->doubleSpinBox_rimLightBoost->setValue(1.0);

	ui->checkBox_rimLightAlphaMask->setChecked(false);

	//----------------------------------------------------------------------------------------//

	ui->checkBox_model->setChecked(false);
	ui->checkBox_vertexColor->setChecked(false);
	ui->checkBox_vertexAlpha->setChecked(false);
	ui->checkBox_ignoreZ->setChecked(false);
	ui->checkBox_additive->setChecked(false);
	ui->checkBox_noFog->setChecked(false);
	ui->checkBox_decal->setChecked(false);
	ui->checkBox_noLod->setChecked(false);
	ui->checkBox_noFullBright->setChecked(false);
	ui->checkBox_noDecal->setChecked(false);

	ui->lineEdit_keywords->clear();

	ui->lineEdit_toolTexture->clear();

	//----------------------------------------------------------------------------------------//

	ui->doubleSpinBox_bt_centerX->setValue(0.5);
	ui->doubleSpinBox_bt_centerY->setValue(0.5);
	ui->doubleSpinBox_bt_scaleX->setValue(1.0);
	ui->doubleSpinBox_bt_scaleY->setValue(1.0);
	ui->doubleSpinBox_bt_angle->setValue(0.0);
	ui->doubleSpinBox_bt_translateX->setValue(0.0);
	ui->doubleSpinBox_bt_translateY->setValue(0.0);

	ui->doubleSpinBox_bm_centerX->setValue(0.5);
	ui->doubleSpinBox_bm_centerY->setValue(0.5);
	ui->doubleSpinBox_bm_scaleX->setValue(1.0);
	ui->doubleSpinBox_bm_scaleY->setValue(1.0);
	ui->doubleSpinBox_bm_angle->setValue(0.0);
	ui->doubleSpinBox_bm_translateX->setValue(0.0);
	ui->doubleSpinBox_bm_translateY->setValue(0.0);

	//----------------------------------------------------------------------------------------//

	ui->comboBox_spriteOrientation->setCurrentIndex(0);

	ui->doubleSpinBox_spriteX->setValue(0.5);
	ui->doubleSpinBox_spriteY->setValue(0.5);

	//----------------------------------------------------------------------------------------//

	ui->lineEdit_waterNormalMap->clear();
	ui->lineEdit_bottomMaterial->clear();

	ui->comboBox_waterSurface->setCurrentIndex(0);

	ui->checkBox_flowDebug->setChecked(false);
	ui->checkBox_reflect2dskybox->setChecked(false);
	ui->checkBox_reflect3dskybox->setChecked(false);

	ui->doubleSpinBox_scrollX1->setValue(0.0);
	ui->doubleSpinBox_scrollX2->setValue(0.0);
	ui->doubleSpinBox_scrollY1->setValue(0.0);
	ui->doubleSpinBox_scrollY2->setValue(0.0);

	ui->checkBox_waterBottom->setChecked(false);
	ui->checkBox_cheap->setChecked(false);
	ui->checkBox_expensive->setChecked(false);

	ui->checkBox_cheap->setEnabled(true);
	ui->checkBox_expensive->setEnabled(true);

	ui->spinBox_bumpframe->setValue(0);

	//----------------------------------------------------------------------------------------//

	ui->lineEdit_flowMap->clear();
	ui->lineEdit_noiseTexture->clear();

	ui->doubleSpinBox_normalUV->setValue(0.0);
	ui->doubleSpinBox_worldUV->setValue(0.0);
	ui->doubleSpinBox_scrollUV->setValue(0.0);
	ui->doubleSpinBox_bumpStrength->setValue(0.0);
	ui->doubleSpinBox_noiseScale->setValue(0.0);
	ui->doubleSpinBox_timeScale->setValue(0.0);
	ui->doubleSpinBox_timeInterval->setValue(0.0);

	//----------------------------------------------------------------------------------------//

	ui->toolButton_reflectionTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );

	ui->doubleSpinBox_reflectionAmount->setValue(0.0);

	ui->checkBox_realTimeReflection->setChecked(false);
	ui->checkBox_reflectEntities->setChecked(false);
	ui->checkBox_skybox->setChecked(false);

	//----------------------------------------------------------------------------------------//

	// Selfillum

	ui->lineEdit_maskTexture->setText("");
	ui->checkBox_envmapAlpha->setChecked(false);
	ui->doubleSpinBox_selfIllumTint->setValue(1.0);
	ui->toolButton_selfIllumTint->setStyleSheet("background-color: rgb(255, 255, 255)");
	ui->doubleSpinBox_selfIllumFresnelMin->setValue(0.0);
	ui->doubleSpinBox_selfIllumFresnelMax->setValue(1.0);
	ui->doubleSpinBox_selfIllumFresnelExp->setValue(1.0);

	//----------------------------------------------------------------------------------------//

	ui->comboBox_refractSurface->setCurrentIndex(0);

	ui->lineEdit_refractNormalMap->clear();
	ui->lineEdit_refractNormalMap2->clear();

	ui->toolButton_refractionTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );

	ui->doubleSpinBox_refractionAmount->setValue(0.0);

	//----------------------------------------------------------------------------------------//

	ui->toolButton_fogTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );

	ui->spinBox_fogStart->setValue(0.0);
	ui->spinBox_fogEnd->setValue(0.0);

	ui->doubleSpinBox_flashlightTint->setValue(0.0);

	ui->checkBox_lightmapFog->setChecked(false);

	//----------------------------------------------------------------------------------------//

	ui->lineEdit_refractTexture->clear();

	ui->toolButton_refractTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );

	ui->doubleSpinBox_refractAmount->setValue(0.0);
	ui->doubleSpinBox_refractBlur->setValue(0.0);

	//----------------------------------------------------------------------------------------//

	ui->doubleSpinBox_color1->setValue(1.0);
	ui->doubleSpinBox_color2->setValue(1.0);
	ui->toolButton_color1->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->toolButton_color2->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	ui->lineEdit_tintMask->clear();

	ui->checkBox_blendTint->setChecked(false);

	//----------------------------------------------------------------------------------------//

	QLayoutItem *child;
	while ((child = ui->formLayout_2->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}

	while ((child = ui->formLayout_3->takeAt(0)) != 0) {

		delete child->widget();
		 delete child;
	}

	ui->formLayout_2->addRow( new ParameterLineEdit(ui->scrollAreaWidgetContents) );
	ui->formLayout_3->addRow( new ValueLineEdit(ui->scrollAreaWidgetContents) );

	//----------------------------------------------------------------------------------------//

	mChildWidgetChanged = false;

	mIsResetting = false;
}

void MainWindow::action_New() {

	mLoading = true;

	bool actionAccepted = true;

	if( mChildWidgetChanged ) {

		switch( _displaySaveMessage() ) {

			case QMessageBox::Save:

				action_Save();
				if( !mChildWidgetChanged)
					break;

			case QMessageBox::Cancel:
			case QMessageBox::Escape:

				actionAccepted = false;

				break;
		}
	}

	if(actionAccepted) {
		resetWidgets();
		updateWindowTitle();

		QList<QAction*> actions = ui->action_games->actions();
		foreach(QAction* action, actions) {
			action->setEnabled(true);
		}

		refreshRequested();

		setCurrentGame( mSettings->saveLastGame ? mSettings->lastGame : "");
	}

	mLoading = false;
}

void MainWindow::action_Open() {

	if(mChildWidgetChanged) {
		int msgBoxResult = _displaySaveMessage();

		if(msgBoxResult == QMessageBox::Save)
			action_Save();
		else if(msgBoxResult == QMessageBox::Cancel)
			return;
	}

	QString fileName;

	if( getCurrentGame() != "" ) {

		QString lastGameVMTDirectory = QDir::toNativeSeparators(mIniSettings->value( "lastSaveAsDir" ).toString());

		fileName = QFileDialog::getOpenFileName( this,
												 "Open Valve Material",
												 lastGameVMTDirectory,
												 "VMT (*.vmt)" );
		if( fileName.isEmpty() )
			return;

		mIniSettings->setValue( "lastSaveAsDir", QDir::toNativeSeparators(fileName).left( QDir::toNativeSeparators(fileName).lastIndexOf("\\")) );

	} else {

		fileName = QFileDialog::getOpenFileName( this,
												 "Open Valve Material",
												 mIniSettings->value("lastSaveAsDir").toString(),
												 "VMT (*.vmt)" );
		if( fileName.isEmpty() )
			return;

		mIniSettings->setValue( "lastSaveAsDir", QDir::toNativeSeparators(fileName).left( QDir::toNativeSeparators(fileName).lastIndexOf("\\")) );
	}

	loadVMT(fileName);
}

void MainWindow::action_Save() {

	QString dir;

	mCursor = ui->vmtPreviewTextEdit->textCursor();
	mCursorPos = mCursor.position();

	if(mVMTLoaded) {

		if(mPreviewChanged)
			vmtPreviewParse();
		else
			refreshRequested();

		const QString directory = vmtParser->lastVMTFile().directory;
		const QString fileName = vmtParser->lastVMTFile().fileName;

		dir = directory + '/';

		vmtParser->saveVmtFile( ui->vmtPreviewTextEdit->toPlainText(), directory + "/" + fileName );

		processTexturesToCopy(dir);

		mChildWidgetChanged = false;

		updateWindowTitle();

	} else {

		action_saveAs();
	}

	mLoading = true;

	refreshRequested();

	//vmtParser->saveVmtFile( ui->vmtPreviewTextEdit->toPlainText(), vmtParser->lastVMTFile().directory + "/" + vmtParser->lastVMTFile().fileName );

	mLoading = false;

	mCursor.setPosition(mCursorPos);
	ui->vmtPreviewTextEdit->setTextCursor(mCursor);
}

QString MainWindow::action_saveAs() {

	QString fileName;

	if(mVMTLoaded) {

		if( getCurrentGame() == "" ) {

			fileName = QFileDialog::getSaveFileName( this, "Save Valve Material",
													 vmtParser->lastVMTFile().fileName, "VMT (*.vmt)" );
		} else {

			fileName = QFileDialog::getSaveFileName( this, "Save Valve Material",
													 vmtParser->lastVMTFile().directory + "/" + vmtParser->lastVMTFile().fileName,
													 "VMT (*.vmt)" );
		}

	} else {

		QString lastSaveAsDir = QDir::toNativeSeparators(mIniSettings->value("lastSaveAsDir").toString());
		QString currentGameDir = QDir::toNativeSeparators(currentGameMaterialDir());
		QString vtfName;
		if ( ui->lineEdit_diffuse->text().isEmpty() ) vtfName = "\\untitled.vmt";
		else {
			QString vtfLong = ui->lineEdit_diffuse->text();

			QString tmp;
			if ( vtfLong.contains("/")) {
				tmp = vtfLong.section("/", -1);
			} else {
				tmp = vtfLong.section("\\", -1);
			}

			if ( tmp.contains(".") )  {
				vtfName = "\\" + tmp.section(".", 0, 0);
			} else {
				vtfName = "\\" + tmp;
			}

			if( vtfName.endsWith("_diffuse") ) {
				vtfName.chop(8);
			}

		}

		if( getCurrentGame() == "" ) {

			fileName = QFileDialog::getSaveFileName( this, tr("Save Valve Material"), lastSaveAsDir + vtfName,
													 tr("VMT (*.vmt)") );

		} else {

			if( lastSaveAsDir.startsWith( currentGameDir, Qt::CaseInsensitive ) ) {

				fileName = QFileDialog::getSaveFileName(this, tr("Save Valve Material"),
														lastSaveAsDir + vtfName, "VMT (*.vmt)");

			} else {

				fileName = QFileDialog::getSaveFileName(this, tr("Save Valve Material"),
														QDir::toNativeSeparators(currentGameMaterialDir()) + vtfName, "VMT (*.vmt)");
			}
		}
	}

	if( !fileName.isEmpty() ) {

		mIniSettings->setValue("lastSaveAsDir", QDir::toNativeSeparators(fileName).left( QDir::toNativeSeparators(fileName).lastIndexOf('\\') ));

		if(mPreviewChanged)
			vmtPreviewParse();
		else
			refreshRequested();

		setCurrentFile( fileName );

		vmtParser->saveVmtFile( ui->vmtPreviewTextEdit->toPlainText(), fileName );

		processTexturesToCopy( fileName.left( fileName.lastIndexOf('/') + 1 ) );

		mChildWidgetChanged = false;

		mVMTLoaded = true;

		action_Save();

	} else {

		// Preventing a quit
		mExitRequested = false;
	}

	updateWindowTitle();
	refreshRequested();

	return fileName;
}

void MainWindow::saveAsTemplate()
{
	const auto suggestion = QDir::toNativeSeparators(
		QDir::current().filePath("templates/untitled.vmt"));
	const auto fileName = QFileDialog::getSaveFileName(this,
		"Save Valve Material Template", suggestion, "VMT (*.vmt)" );

	if (fileName.isEmpty())
		return;

	refreshRequested();
	vmtParser->saveVmtFile(ui->vmtPreviewTextEdit->toPlainText(),
		fileName);
	mChildWidgetChanged = false;
	mVMTLoaded = true;

	updateWindowTitle();
	refreshRequested();
	action_RefreshTemplateList();
}

void MainWindow::action_RefreshTemplateList() {

	//QDir* templatesDir = new QDir(QDir::currentPath() + "/templates");
	QStringList templates = QDir(QDir::currentPath() + "/templates").entryList(QStringList()<<"*.vmt");
	for ( int i = 0; i < templates.size(); i++ ) {
		templates[i].prepend(QDir::currentPath() + "/templates/");
	}
	//qDebug()<<templates;

	int numTemplates = qMin( templates.size(), (int)MaxTemplates );

	for( int i = 0; i < numTemplates; ++i )
	{
	   QString text = tr("&%1").arg( QFileInfo(templates[i]).fileName() );
	   templateActions[i]->setText(text);
	   templateActions[i]->setData(templates[i]);
	   templateActions[i]->setVisible(true);
	}


	for( int j = numTemplates; j < MaxTemplates; ++j )
	{
		templateActions[j]->setVisible(false);
	}

	if( numTemplates > 0 )
	{
		ui->menuTemplates->insertAction( ui->actionRefresh_List, separatorTemp );
		separatorTemp->setVisible(true);
	}
	else
	{
		separatorTemp->setVisible(false);
	}
}

void MainWindow::toggleTransparency() {

	if(ui->action_transparency->isEnabled()) {

		glWidget_diffuse1->setTransparencyVisible( !ui->groupBox_transparency->isVisible() );

		if( !ui->groupBox_transparency->isVisible() ) {

			ui->groupBox_transparency->show(); \
			if (!mParsingVMT)
				ui->groupBox_transparency->fadeShow(this);

			ui->groupBox_transparency->repaint();
			ui->action_transparency->setChecked(true);



		} else {

			ui->groupBox_transparency->fadeHide(this);
			ui->action_transparency->setChecked(false);
		}
	}
}

void MainWindow::toggleDetailTexture()
{
	utils::toggle(this, ui->action_detail, ui->groupBox_detailTexture);
}

void MainWindow::toggleColor()
{
	glWidget_diffuse1->setColorVisible( !ui->groupBox_color->isVisible() );
	colorChanged();
	utils::toggle(this, ui->action_color, ui->groupBox_color);
}

void MainWindow::toggleOther()
{
	utils::toggle(this, ui->action_other, ui->groupBox_textureOther);
}

void MainWindow::toggleMisc()
{
	utils::toggle(this, ui->action_misc, ui->groupBox_misc);
}

void MainWindow::togglePhong() {

	if(ui->action_phong->isEnabled()) {

		if(!ui->groupBox_phong->isVisible()) {

			if( ui->checkBox_exponentBaseAlpha->isChecked() &&
				ui->checkBox_exponentBaseAlpha->isEnabled() )
				previewTexture( 4, ui->lineEdit_diffuse->text() );
			else
				previewTexture( 4, ui->lineEdit_bumpmap->text() );

			if( ui->checkBox_basealpha->isChecked() && ui->checkBox_basealpha->isEnabled() )
				previewTexture( 1, ui->lineEdit_diffuse->text() );

		} else {
			previewTexture( 4, "" );
			if( ui->checkBox_basealpha->isChecked() && ui->checkBox_basealpha->isEnabled() )
				previewTexture( 0, ui->lineEdit_diffuse->text() );
		}
	}

	utils::toggle(this, ui->action_phong, ui->groupBox_phong);
}

void MainWindow::togglePhongBrush() {
	if(ui->action_phongBrush->isEnabled()) {
		utils::Preview preview = phong::toggle(ui);
		previewTexture(preview.mode, preview.texture);
	}
	utils::toggle(this, ui->action_phongBrush, ui->groupBox_phongBrush);
}

void MainWindow::toggleReflection() {

	if( ui->groupBox_shadingReflection->isVisible() )
		previewTexture( 0, "" );

	else {

		if( ui->checkBox_basealpha->isChecked() && ui->checkBox_basealpha->isEnabled() && ui->action_phong->isEnabled() )
			previewTexture( 1, ui->lineEdit_diffuse->text() );

		else if( ui->checkBox_basealpha->isChecked() && ui->checkBox_basealpha->isEnabled() && !ui->action_phong->isEnabled())
			previewTexture( 0, ui->lineEdit_diffuse->text() );

		else if( ui->checkBox_normalalpha->isChecked() && ui->checkBox_normalalpha->isEnabled() )
			previewTexture( 1, ui->lineEdit_bumpmap->text() );
	}

	utils::toggle(this, ui->action_reflection, ui->groupBox_shadingReflection);
}

void MainWindow::toggleSelfIllumination() {
	utils::toggle(this, ui->action_selfIllumination,
				  ui->groupBox_selfIllumination);
}

void MainWindow::toggleFlowmap() {
	utils::toggle(this, ui->action_flowmap,
				  ui->groupBox_waterFlow);
}
void MainWindow::toggleWaterReflection() {
	utils::toggle(this, ui->action_waterReflection,
				  ui->groupBox_waterReflection);
}
void MainWindow::toggleWaterRefraction() {
	utils::toggle(this, ui->action_refraction,
				  ui->groupBox_waterRefraction);
}
void MainWindow::toggleWaterFog() {
	utils::toggle(this, ui->action_fog,
				  ui->groupBox_waterFog);
}

QString MainWindow::currentGameMaterialDir() {

	QString game = getCurrentGame();

	if( game.isEmpty() )
		return "";

	QString dir = mAvailableGames.value(game);

	dir.replace( 0, 1, dir.at(0).toUpper() );

	if( !QDir(dir).exists("materials") )
		return dir;

	dir.append("/materials");

	return dir;
}

QString MainWindow::validateTexture(QString objectName, QString vtf, const QString& command, QDir gameInfoDir )
{
	vtf.replace( "\\", "/" );

	if( vtf.endsWith(".vtf") ) {

		Warning("" + command + " should not end with .vtf!")

		if( !( gameInfoDir.exists( "materials/" + vtf ))) {

			if(mGameSelected)
				Info("" + command + " vtf file: \"" + vtf + ".vtf\" cannot be found!")

		} else {

			if( !getCurrentGame().isEmpty() ) {

				if( objectName == "preview_basetexture1" || objectName == "preview_bumpmap1" ) {

					glWidget_diffuse1->setVisible(true);
					previewTexture(objectName, vtf, true, false, false, false);

				} else if( objectName == "preview_basetexture2" || objectName == "preview_bumpmap2" ) {

					glWidget_diffuse2->setVisible(true);
					previewTexture(objectName, vtf, true, false, false, false);

				} else {

					foreach(GLWidget* glWidget, glWidgets) {

						if( objectName == glWidget->objectName() ) {

							glWidget->setVisible(true);
							previewTexture(objectName, vtf.left( vtf.size() - 4 ), true, false, false, false);
							break;
						}
					}
				}
			}
		}

		return vtf.left( vtf.size() - 4 );

	} else {

		if( !( gameInfoDir.exists( "materials/" + vtf + ".vtf" ))) {

			if(mGameSelected)
				Info("" + command + " vtf file: \"" + vtf + ".vtf\" cannot be found!")

		} else {

			if( !getCurrentGame().isEmpty() ) {

				if( objectName == "preview_basetexture1" || objectName == "preview_bumpmap1" ) {

					glWidget_diffuse1->setVisible(true);
					previewTexture(objectName, vtf, true, false, false, false);

				} else if( objectName == "preview_basetexture2" || objectName == "preview_bumpmap2" ) {

					glWidget_diffuse2->setVisible(true);
					previewTexture(objectName, vtf, true, false, false, false);

				} else {

					foreach(GLWidget* glWidget, glWidgets) {

						if( objectName == glWidget->objectName() ) {

							glWidget->setVisible(true);
							previewTexture(objectName, vtf, true, false, false, false);
							break;
						}
					}
				}
			}
		}

		return vtf;
	}
}

void MainWindow::checkCacheSize()
{
	QFileInfoList files = QDir(QDir::currentPath() + "/Cache").entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

	qint64 directorySize = 0;

	for (int i = 0; i < files.size(); ++i) {

		if(files.at(i).completeSuffix().toLower() == "png")
			directorySize += files.at(i).size();
	}

	if(directorySize / 1024.0 / 1024.0 > static_cast<double>(mSettings->cacheSize))
		clearCacheFolder();
}

bool MainWindow::isGroupboxChanged(MainWindow::GroupBoxes groupBox)
{
	switch(groupBox)
	{
	case BaseTexture:

		return (ui->lineEdit_diffuse->text() != "" ||
				ui->lineEdit_bumpmap->text() != "" ||
				ui->comboBox_surface->currentIndex() > 0 ||
				ui->checkBox_ssbump->isChecked() ||
				ui->doubleSpinBox_lumstart1->value() != 0.0 ||
				ui->doubleSpinBox_lumend1->value() != 1.0);

	case NormalBlend:
		return normalblend::hasChanged(ui);

	case BaseTexture2:

		return (ui->lineEdit_diffuse2->text() != "" ||
				ui->lineEdit_bumpmap2->text() != "" ||
				ui->lineEdit_blendmodulate->text() != "" ||
				ui->comboBox_surface2->currentIndex() > 0 ||
				ui->doubleSpinBox_uvscalex2->value() != 1.0 ||
				ui->doubleSpinBox_uvscaley2->value() != 1.0 ||
				ui->doubleSpinBox_lumstart2->value() != 0.0 ||
				ui->doubleSpinBox_lumend2->value() != 1.0 ||
				ui->doubleSpinBox_blendstart2->value() != 0.0 ||
				ui->doubleSpinBox_blendend2->value() != 1.0 ||
				ui->doubleSpinBox_blendfactor2->value() != 1.0);

	case BaseTexture3:

		return (ui->lineEdit_diffuse3->text() != "" ||
				ui->doubleSpinBox_uvscalex3->value() != 1.0 ||
				ui->doubleSpinBox_uvscaley3->value() != 1.0 ||
				ui->doubleSpinBox_lumstart3->value() != 0.0 ||
				ui->doubleSpinBox_lumend3->value() != 1.0 ||
				ui->doubleSpinBox_blendstart3->value() != 0.0 ||
				ui->doubleSpinBox_blendend3->value() != 1.0 ||
				ui->doubleSpinBox_blendfactor3->value() != 1.0);

	case BaseTexture4:

		return (ui->lineEdit_diffuse4->text() != "" ||
				ui->doubleSpinBox_uvscalex4->value() != 1.0 ||
				ui->doubleSpinBox_uvscaley4->value() != 1.0 ||
				ui->doubleSpinBox_lumstart4->value() != 0.0 ||
				ui->doubleSpinBox_lumend4->value() != 1.0 ||
				ui->doubleSpinBox_blendstart4->value() != 0.0 ||
				ui->doubleSpinBox_blendend4->value() != 1.0 ||
				ui->doubleSpinBox_blendfactor4->value() != 1.0);

	case Transparency:

		return (ui->doubleSpinBox_opacity->value() != 1.0 ||
				ui->checkBox_transparent->isChecked() ||
				ui->checkBox_alphaTest->isChecked() ||
				ui->checkBox_alphaToCoverage->isChecked() ||
				ui->doubleSpinBox_alphaTestRef->value() != 0.7 ||
				ui->checkBox_additive->isChecked() ||
				ui->checkBox_noCull->isChecked());

	case DetailTexture:
		return detailtexture::hasChanged(ui);

	case Color:

		return (ui->doubleSpinBox_color1->value() != 1.0 ||
				ui->doubleSpinBox_color2->value() != 1.0 ||
				ui->lineEdit_tintMask->text() != "" ||
				ui->checkBox_blendTint->text() != "" ||
				ui->checkBox_noTint->text() != "" ||
				utils::getBG(ui->toolButton_color1) != QColor(255, 255, 255) ||
				utils::getBG(ui->toolButton_color2) != QColor(255, 255, 255));

	case OtherTexture:

		return (ui->lineEdit_lightWarp->text() != "" ||
				ui->doubleSpinBox_seamlessScale->value() != 0.0 ||
				ui->doubleSpinBox_reflectivity->value() != 1.0 ||
				ui->doubleSpinBox_reflectivity_2->value() != 1.0 ||
				utils::getBG(ui->toolButton_reflectivity) != QColor(255, 255, 255) ||
				utils::getBG(ui->toolButton_reflectivity_2) != QColor(255, 255, 255));

	case Phong:
	case PhongBrush:
		return phong::hasChanged(groupBox, ui);

	case TreeSway:
		return treesway::hasChanged(ui);

	case LayerBlend:
		return layerblend::hasChanged(ui);

	case EmissiveBlend:
		return emissiveblend::hasChanged(ui);

	case Reflection:

		return (ui->lineEdit_envmap->text() != "" ||
				ui->checkBox_cubemap->isChecked() ||
				ui->lineEdit_specmap->text() != "" ||
				ui->lineEdit_specmap2->text() != "" ||
				ui->checkBox_basealpha->isChecked() ||
				ui->checkBox_normalalpha->isChecked() ||
				ui->checkBox_tintSpecMask->isChecked() ||
				ui->doubleSpinBox_envmapTint->value() != 1.0 ||
				utils::getBG(ui->toolButton_reflectionTint) != QColor(255, 255, 255) ||
				ui->doubleSpinBox_saturation->value() != 1.0 ||
				ui->doubleSpinBox_contrast->value() != 0.0 ||
				ui->doubleSpinBox_fresnelReflection->value() != 1.0) ||
				ui->doubleSpinBox_envmapLight->value() != 0.0 ||
				ui->doubleSpinBox_envmapAniso->value() != 0.0;

	case SelfIllumination:

		return (ui->lineEdit_maskTexture->text() != "" ||
				ui->checkBox_envmapAlpha->isChecked() ||
				//ui->checkBox_baseAlpha->isChecked() ||
				ui->doubleSpinBox_selfIllumTint->value() != 1.0 ||
				utils::getBG(ui->toolButton_selfIllumTint) != QColor(255, 255, 255) ||
				ui->doubleSpinBox_selfIllumFresnelMin->value() != 0.0 ||
				ui->doubleSpinBox_selfIllumFresnelMax->value() != 1.0 ||
				ui->doubleSpinBox_selfIllumFresnelExp->value() != 1.0);

	case RimLight:

		return (ui->spinBox_rimLightExponent->value() != 5 ||
				ui->doubleSpinBox_rimLightBoost->value() != 1.0 ||
				ui->checkBox_rimLightAlphaMask->isChecked());

	case Water:

		return (ui->lineEdit_waterNormalMap->text() != "" ||
				ui->lineEdit_bottomMaterial->text() != "" ||
				ui->comboBox_waterSurface->currentIndex() != 0 ||
				ui->checkBox_waterBottom->isChecked() ||
				ui->checkBox_cheap->isChecked() ||
				ui->checkBox_expensive->isChecked() ||
				ui->checkBox_bumpframe->isChecked());

	case WaterFlowmap:

		return (ui->lineEdit_flowMap->text() != "" ||
				ui->lineEdit_noiseTexture->text() != "" ||
				ui->doubleSpinBox_normalUV->value() != 0.0 ||
				ui->doubleSpinBox_noiseScale->value() != 0.0 ||
				ui->doubleSpinBox_worldUV->value() != 0.0 ||
				ui->doubleSpinBox_timeScale->value() != 0.0 ||
				ui->doubleSpinBox_scrollUV->value() != 0.0 ||
				ui->doubleSpinBox_timeInterval->value() != 0.0 ||
				ui->doubleSpinBox_bumpStrength->value() != 0.0 ||
				ui->checkBox_flowDebug->isChecked());

	case WaterReflection:

		return (utils::getBG(ui->toolButton_reflectionTint) != QColor(255, 255, 255) ||
				ui->doubleSpinBox_reflectionAmount->value() != 0.0 ||
				ui->checkBox_skybox->isChecked() ||
				ui->checkBox_reflectEntities->isChecked() ||
				ui->checkBox_reflect2dskybox->isChecked() ||
				ui->checkBox_reflect3dskybox->isChecked());

	case WaterRefraction:

		return (utils::getBG(ui->toolButton_refractionTint) != QColor(255, 255, 255) ||
				ui->doubleSpinBox_refractionAmount->value() != 0.0);

	case Fog:

		return (utils::getBG(ui->toolButton_fogTint) != QColor(255, 255, 255) ||
				ui->spinBox_fogStart->value() != 0 ||
				ui->spinBox_fogEnd->value() != 0 ||
				ui->doubleSpinBox_flashlightTint->value() != 0.0 ||
				ui->checkBox_volumetricFog->isChecked() ||
				ui->checkBox_lightmapFog->isChecked());

	case Scroll:

		return (ui->doubleSpinBox_scrollX1->value() != 0.0 ||
				ui->doubleSpinBox_scrollY1->value() != 0.0 ||
				ui->doubleSpinBox_scrollX1->value() != 0.0 ||
				ui->doubleSpinBox_scrollY2->value() != 0.0);

	case BaseTextureTransforms:

		return (ui->doubleSpinBox_bt_centerX->value() != 0.5 ||
				ui->doubleSpinBox_bt_centerY->value() != 0.5 ||
				ui->doubleSpinBox_bt_scaleX->value() != 1.0 ||
				ui->doubleSpinBox_bt_scaleY->value() != 1.0 ||
				ui->doubleSpinBox_bt_angle->value() != 0.0 ||
				ui->doubleSpinBox_bt_translateX->value() != 0.0 ||
				ui->doubleSpinBox_bt_translateY->value() != 0.0);

	case BumpMapTransforms:

		return (ui->doubleSpinBox_bm_centerX->value() != 0.5 ||
				ui->doubleSpinBox_bm_centerY->value() != 0.5 ||
				ui->doubleSpinBox_bm_scaleX->value() != 1.0 ||
				ui->doubleSpinBox_bm_scaleY->value() != 1.0 ||
				ui->doubleSpinBox_bm_angle->value() != 0.0 ||
				ui->doubleSpinBox_bm_translateX->value() != 0.0 ||
				ui->doubleSpinBox_bm_translateY->value() != 0.0);

	case Misc:

		return (ui->lineEdit_toolTexture->text() != "" ||
				ui->lineEdit_keywords->text() != "" ||
				ui->checkBox_model->isChecked() ||
				ui->checkBox_decal->isChecked() ||
				ui->checkBox_noFog->isChecked() ||
				ui->checkBox_vertexAlpha->isChecked() ||
				ui->checkBox_noDecal->isChecked() ||
				ui->checkBox_noLod->isChecked() ||
				ui->checkBox_vertexColor->isChecked() ||
				ui->checkBox_ignoreZ->isChecked() ||
				ui->checkBox_noFullBright->isChecked());

	case Refract:

		return (ui->lineEdit_refractNormalMap->text() != "" ||
				ui->lineEdit_refractNormalMap2->text() != "" ||
				ui->lineEdit_refractTexture->text() != "" ||
				ui->comboBox_refractSurface->currentIndex() != 0 ||
				// OLD: ui->lineEdit_refractTint->text() != "" ||
				ui->doubleSpinBox_refractAmount->value() != 0 ||
				ui->doubleSpinBox_refractBlur->value() != 0);

	case Sprite:

		return (ui->comboBox_spriteOrientation->currentIndex() != 0  ||
				ui->doubleSpinBox_spriteX->value() != 0.5 ||
				ui->doubleSpinBox_spriteY->value() != 0.5);

	case UnlitTwoTexture:

		return (ui->lineEdit_unlitTwoTextureDiffuse->text() != "" ||
				ui->lineEdit_unlitTwoTextureDiffuse2->text() != "");

	case Patch:

		return (ui->lineEdit_patchVmt->text() != "");
	}

	return false;
}

bool MainWindow::loadBoolParameter( const QString& value, const QString& parameter ) {

	if( value == "1" )
		return true;
	else if( value == "0" )
		Info("" + parameter + " has default value: 0")
	else
		Error("" + parameter + " has unrecognizable value: \"" + value + "\"")

	return false;
}

bool MainWindow::loadIntParameter( int* intValue, const QString& value, const QString& parameter )
{
	bool ok;
	*intValue = value.toInt(&ok);

	if(ok)
	{
		return true;
	}
	else
	{
		Error("" + parameter + " value \"" + value + "\" has caused an error while parsing!")

		return false;
	}
}

bool MainWindow::loadDoubleParameter( double* doubleValue, const QString& value, const QString& parameter )
{
	bool ok;
	*doubleValue = value.toDouble(&ok);

	if(ok)
	{
		return true;
	}
	else
	{
		Error("" + parameter + " value \"" + value + "\" has caused an error while parsing!")

		return false;
	}
}

bool MainWindow::loadDoubleParameter( double* doubleValue, const QString& value, const QString& parameter, double defaultValue )
{
	bool ok;
	*doubleValue = value.toDouble(&ok);

	if(ok)
	{
		if( *doubleValue != defaultValue )
		{
			return true;
		}

		Info("" + parameter + " has default value: \"" + Str(defaultValue) + "\"")

		return false;
	}
	else
	{
		Error("" + parameter + " value \"" + value + "\" has caused an error while parsing!")

		return false;
	}
}

void MainWindow::loadScrollParameter( QString value, const QString& command, uint index )
{
	bool ok;
	float tmp1, tmp2, tmp3;

	value = value.simplified();
	if( value.startsWith('[') )
	{
		value.remove( 0, 1 );
		value = value.trimmed();

		if( value.endsWith(']') )
		{
			value.chop(1);
			value = value.trimmed();

			QStringList values = value.split(' ');

			switch( values.size() )
			{
			case 3:

				tmp3 = values.at(2).toFloat(&ok);

				if( !ok || tmp3 < -1.0f || tmp3 > 1.0f )
					goto error;

			case 2:

				tmp1 = values.at(0).toFloat(&ok);

				if( !ok || tmp1 < -1.0f || tmp1 > 1.0f )
					goto error;


				tmp2 = values.at(1).toFloat(&ok);

				if( !ok || tmp2 < -1.0f || tmp2 > 1.0f )
					goto error;

				break;

			default:

				goto error;
			}

			if( index == 1)
			{
				ui->doubleSpinBox_scrollX1->setValue(tmp1);
				ui->doubleSpinBox_scrollY1->setValue(tmp2);
			}
			else
			{
				ui->doubleSpinBox_scrollX2->setValue(tmp1);
				ui->doubleSpinBox_scrollY2->setValue(tmp2);
			}

			return;
		}
	}

	error:

	Error( command + " (" + value + ") is not in the valid format of: \"[<normal X> <normal Y>]\"!")
}

void MainWindow::widgetChanged()
{
	if(!mLoading) {

		if(mChildWidgetChanged == false) {

			mChildWidgetChanged = true;

			updateWindowTitle();


		}

		if(mSettings->autoRefresh) {
			const auto c = sender();
			if (c == nullptr) {
				refreshRequested();
			} else {
				if(qobject_cast<QWidget*>(c)->objectName() != "textEdit_proxies" ) {
					refreshRequested();
				}
			}
			
		}
		if(mSettings->autoSave) {
			if (mVMTLoaded)
				action_Save();
		}
	}

}

void MainWindow::refreshRequested() {

	VmtFile tmp3 = makeVMT();

	mPreviewChanged = false;

	ui->vmtPreviewTextEdit->blockSignals(true);

	if( !ui->textEdit_proxies->toPlainText().isEmpty() ) {

		QString tmp;
		QString tmp2 = VmtParser::parseSubGroups( tmp3.subGroups, &tmp );

		if( tmp2.isEmpty() ) {

			tmp3.subGroups = VmtParser::formatSubGroups(tmp, 1);

			ui->vmtPreviewTextEdit->setPlainText( vmtParser->convertVmt( tmp3,
																			   mSettings->parameterSortStyle == Settings::Grouped,
																			   mSettings->useQuotesForTexture,
																			   mSettings->useIndentation ));
		} else {

			tmp3.subGroups = "";

			ui->vmtPreviewTextEdit->setPlainText( vmtParser->convertVmt( tmp3,
																			   mSettings->parameterSortStyle == Settings::Grouped,
																			   mSettings->useQuotesForTexture,
																			   mSettings->useIndentation ));

			Info( "Proxies parsing error: " + tmp2 )
		}

	} else {

		ui->vmtPreviewTextEdit->setPlainText( vmtParser->convertVmt( tmp3,
																		   mSettings->parameterSortStyle == Settings::Grouped,
																		   mSettings->useQuotesForTexture,
																		   mSettings->useIndentation ));
	}

	ui->vmtPreviewTextEdit->blockSignals(false);
}

void MainWindow::clearMessageLog() {
	mLogger->clear();
}

void MainWindow::paste() {

	ui->vmtPreviewTextEdit->moveCursor(QTextCursor::End,
									   QTextCursor::MoveAnchor);
	ui->vmtPreviewTextEdit->moveCursor(QTextCursor::Left,
									   QTextCursor::MoveAnchor);	
	ui->vmtPreviewTextEdit->paste();

	ui->vmtPreviewTextEdit->textCursor().insertText("\n");

	//vmtPreviewParse();
}

void MainWindow::sortDroppedTextures(const QMimeData* mimeData ) {

	if (mimeData->hasUrls())
	{
		foreach (const QUrl& url, mimeData->urls())
		{
			QString filePath = url.toLocalFile();
			if (!filePath.isEmpty())
			{
				QString fileName = filePath.section("/", -1).section(".", 0, 0);
				const QString shader = ui->comboBox_shader->currentText();

				if (fileName.endsWith("_normal") ||
					fileName.endsWith("_n") ||
					fileName.endsWith("nrm") ) {

					processVtf("preview_bumpmap1", filePath, ui->lineEdit_bumpmap);

				} else if (fileName.endsWith("_specular") ||
						   fileName.endsWith("_s") ||
						   fileName.endsWith("spec") ) {

					if (ui->lineEdit_bumpmapAlpha->isVisible())
						processVtf("", filePath, ui->lineEdit_bumpmapAlpha);
					else
						processVtf("", filePath, ui->lineEdit_specmap);

				} else if (shader == "VertexLitGeneric" &&
							(fileName.endsWith("_glossiness") ||
							 fileName.endsWith("_g") ||
							 fileName.endsWith("gloss")) ) {

					processVtf("", filePath, ui->lineEdit_exponentTexture);

				} else if (shader == "VertexLitGeneric" &&
							(fileName.endsWith("tintmask") ||
							 fileName.endsWith("colormask") ||
							 fileName.endsWith("_cm") ||
							 fileName.endsWith("_tm")) ) {

					processVtf("", filePath, ui->lineEdit_tintMask);

				} else {

					processVtf("preview_basetexture1", filePath, ui->lineEdit_diffuse);
				}

				//Info("got image " + fileName);
			}
		}
	}
}

void MainWindow::handleTextureDrop(const QString& filePath)
{
	const auto name = qobject_cast<QWidget *>(sender())->objectName();
	//Info(name + " got " + filePath);
	
	// TODO: Only handle when a game is selected?

	if (name == "lineEdit_diffuse")
		processVtf("preview_basetexture1", filePath, ui->lineEdit_diffuse);

	else if (name == "lineEdit_bumpmap" )
		processVtf( "preview_bumpmap1", filePath, ui->lineEdit_bumpmap );

	else if (name == "lineEdit_diffuse2" )
		processVtf( "preview_basetexture2", filePath, ui->lineEdit_diffuse2 );

	else if (name == "lineEdit_bumpmap2" )
		processVtf( "preview_bumpmap2", filePath, ui->lineEdit_bumpmap2 );

	else if (name == "lineEdit_diffuse3" )
		processVtf( "preview_basetexture3", filePath, ui->lineEdit_diffuse3 );

	else if (name == "lineEdit_diffuse4" )
		processVtf( "preview_basetexture4", filePath, ui->lineEdit_diffuse4 );

	else if (name == "lineEdit_detail" )
		processVtf( "preview_detail", filePath, ui->lineEdit_detail );

	else if (name == "lineEdit_detail2" )
		processVtf( "", filePath, ui->lineEdit_detail2 );

	else if (name == "lineEdit_refractNormalMap" )
		processVtf( "preview_bumpmap1", filePath, ui->lineEdit_refractNormalMap );
	else if (name == "lineEdit_refractNormalMap2" )
		processVtf( "preview_bumpmap2", filePath, ui->lineEdit_refractNormalMap2 );
	else if (name == "lineEdit_refractTexture" )
		processVtf( "", filePath, ui->lineEdit_refractTexture );

	else if (name == "lineEdit_waterNormalMap" )
		processVtf( "preview_bumpmap1", filePath, ui->lineEdit_waterNormalMap );

	else if (name == "lineEdit_unlitTwoTextureDiffuse" )
		processVtf( "preview_basetexture1", filePath, ui->lineEdit_unlitTwoTextureDiffuse );
	else if (name == "lineEdit_unlitTwoTextureDiffuse2" )
		processVtf( "preview_basetexture2", filePath, ui->lineEdit_unlitTwoTextureDiffuse2 );

	else if (name == "lineEdit_blendmodulate" )
		processVtf( "preview_blendmod", filePath, ui->lineEdit_blendmodulate );
	else if (name == "lineEdit_lightWarp" )
		processVtf( "", filePath, ui->lineEdit_lightWarp );

	else if (name == "lineEdit_envmap" )
		processVtf( "", filePath, ui->lineEdit_envmap );
	else if (name == "lineEdit_specmap" )
		processVtf( "", filePath, ui->lineEdit_specmap );
	else if (name == "lineEdit_specmap2" )
		processVtf( "", filePath, ui->lineEdit_specmap2 );

	else if (name == "lineEdit_exponentTexture" )
		processVtf( "preview_exponent", filePath, ui->lineEdit_exponentTexture );

	else if (name == "lineEdit_maskTexture" )
		processVtf( "", filePath, ui->lineEdit_maskTexture );

	else if (name == "lineEdit_flowMap" )
		processVtf( "", filePath, ui->lineEdit_flowMap );
	else if (name == "lineEdit_noiseTexture" )
		processVtf( "", filePath, ui->lineEdit_noiseTexture );

	else if (name == "lineEdit_toolTexture" )
		processVtf( "", filePath, ui->lineEdit_toolTexture );

	else if (name == "lineEdit_bump2" )
		processVtf( "preview_bumpmap2", filePath, ui->lineEdit_bump2 );

	else if (name == "lineEdit_waterReflectTexture" )
		processVtf( "", filePath, ui->lineEdit_waterReflectTexture );

	else if (name == "lineEdit_decal" )
		processVtf( "", filePath, ui->lineEdit_decal );

	else if (name == "lineEdit_phongWarp" )
		processVtf( "", filePath, ui->lineEdit_phongWarp );

	else if (name == "lineEdit_bumpmapAlpha" )
		processVtf( "", filePath, ui->lineEdit_bumpmapAlpha );

	else if (name == "lineEdit_diffuseAlpha" )
		processVtf( "", filePath, ui->lineEdit_diffuseAlpha );

	else if (name == "lineEdit_tintMask" )
		processVtf( "", filePath, ui->lineEdit_tintMask );

	else if (name == "lineEdit_emissiveBlendTexture" )
		processVtf( "", filePath, ui->lineEdit_emissiveBlendTexture );

	else if (name == "lineEdit_emissiveBlendBaseTexture" )
		processVtf( "", filePath, ui->lineEdit_emissiveBlendBaseTexture );

	else if (name == "lineEdit_emissiveBlendFlowTexture" )
		processVtf( "", filePath, ui->lineEdit_emissiveBlendFlowTexture );

}

void MainWindow::finishedLoading()
{
	TextureThread* thread = reinterpret_cast<TextureThread*>(sender());

	if( thread->object == "preview_basetexture1" ) {

		glWidget_diffuse1->loadTexture( "Cache/" + thread->output + ".png", glWidget_diffuse1->getBumpmap() );

		if( ui->groupBox_shadingReflection->isVisible() ) {

			if( ui->checkBox_basealpha->isChecked() && ui->groupBox_phong->isVisible() )
				previewTexture( 1, ui->lineEdit_diffuse->text() );
			else if ( ui->checkBox_basealpha->isChecked() && !ui->groupBox_phong->isVisible())
				previewTexture( 0, ui->lineEdit_diffuse->text() );

		}

		if( ui->groupBox_phong->isVisible() ) {

			if( ui->checkBox_exponentBaseAlpha->isChecked() )
				previewTexture( 4, ui->lineEdit_diffuse->text() );
			else
				previewTexture( 4, ui->lineEdit_bumpmap->text() );

		}

	} else if( thread->object == "preview_bumpmap1" ) {

		glWidget_diffuse1->loadTexture( glWidget_diffuse1->getDiffuse(), "Cache/" + thread->output + ".png" );

		if( ui->groupBox_shadingReflection->isVisible() ) {

			if( ui->checkBox_normalalpha->isChecked() )
				previewTexture( 1, ui->lineEdit_bumpmap->text() );

		}
		if( ui->groupBox_phong->isVisible() ) {

			if( ui->checkBox_exponentBaseAlpha->isChecked() )
				previewTexture( 4, ui->lineEdit_diffuse->text() );
			else
				previewTexture( 4, ui->lineEdit_bumpmap->text() );
		}

	} else if( thread->object == "preview_basetexture2" ) {

		glWidget_diffuse2->loadTexture( "Cache/" + thread->output + ".png", glWidget_diffuse2->getBumpmap()  );

	} else if( thread->object == "preview_bumpmap2" )  {

		glWidget_diffuse2->loadTexture( glWidget_diffuse2->getDiffuse(), "Cache/" + thread->output + ".png" );

	} else if( thread->object == "preview_envmap1" ) {

		glWidget_envmap->updateValues(thread->mode, "Cache/" + thread->output + ".png" );

	} else if( thread->object == "preview_spec1" ) {

		glWidget_spec->updateValues(thread->mode, "Cache/" + thread->output + ".png" );

	} else {

		foreach(GLWidget* glWidget, glWidgets) {

			if( glWidget->objectName() == thread->object ) {

				glWidget->loadTexture( "Cache/" + thread->output + ".png");
				break;
			}
		}
	}

	QApplication::restoreOverrideCursor();
	QApplication::restoreOverrideCursor();
	repaint();
}

void MainWindow::previewTexture()
{
	QWidget* caller = qobject_cast<QWidget *>( sender() );

	if( caller->objectName() == "lineEdit_diffuse" ) {
		previewTexture( "preview_basetexture1",
						ui->lineEdit_diffuse->text(),
						true,
						ui->checkBox_transparent->isChecked() || ui->checkBox_alphaTest->isChecked() || ui->checkBox_normalalpha->isChecked(),
						ui->checkBox_alphaTest->isChecked(),
						false );
	} else if( caller->objectName() == "lineEdit_bumpmap" )
		previewTexture( "preview_bumpmap1", ui->lineEdit_bumpmap->text(), false, false, false, false );
	else if( caller->objectName() == "lineEdit_bumpmap2" )
		previewTexture( "preview_bumpmap2", ui->lineEdit_bumpmap2->text(), false, false, false, false );
	else if( caller->objectName() == "lineEdit_diffuse2" )
		previewTexture( "preview_basetexture2", ui->lineEdit_diffuse2->text(), false, false, false, false );
	else if( caller->objectName() == "lineEdit_diffuse3" )
		previewTexture( "preview_basetexture3", ui->lineEdit_diffuse3->text(), false, false, false, false );
	else if( caller->objectName() == "lineEdit_diffuse4" )
		previewTexture( "preview_basetexture4", ui->lineEdit_diffuse4->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_detail" )
		previewTexture( "preview_detail", ui->lineEdit_detail->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_refractNormalMap" )
		previewTexture( "preview_normalmap1", ui->lineEdit_refractNormalMap->text(), false, false, false, false );
	else if( caller->objectName() == "lineEdit_refractNormalMap2" )
		previewTexture( "preview_normalmap2", ui->lineEdit_refractNormalMap2->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_waterNormalMap" )
		previewTexture( "preview_normalmap1", ui->lineEdit_waterNormalMap->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_unlitTwoTextureDiffuse" )
		previewTexture( "preview_basetexture1", ui->lineEdit_unlitTwoTextureDiffuse->text(), false, false, false, false );
	else if( caller->objectName() == "lineEdit_unlitTwoTextureDiffuse2" )
		previewTexture( "preview_basetexture2", ui->lineEdit_unlitTwoTextureDiffuse2->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_bump2" )
		previewTexture( "preview_bumpmap2", ui->lineEdit_bump2->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_blendmodulate" )
		previewTexture( "preview_blendmod", ui->lineEdit_blendmodulate->text(), false, false, false, false );

	else if( caller->objectName() == "lineEdit_exponentTexture" )
		previewTexture( "preview_exponent", ui->lineEdit_exponentTexture->text(), false, false, false, false );

	repaint();
}

bool MainWindow::previewTexture( const QString& object, const QString& texture, bool baseTexture, bool alpha, bool alphaTest, bool alphaOnly, bool ignoreCache ) {

	checkCacheSize();

	if( getCurrentGame().isEmpty() ) {

		// Error( "No preview can be generated because no game is selected!")

		return false;

	} else {
		QString texturePath;
		if( texture.endsWith(".") ) {
			texturePath = texture;
			texturePath.chop(1);
		} else {
			texturePath = ( currentGameMaterialDir() + "/" + texture );
		}
		QFile vtfFile( texturePath + ".vtf" );
		if( !vtfFile.exists() ) {

			if( object == "preview_basetexture1" ) {

				glWidget_diffuse1->loadTexture("", glWidget_diffuse1->getBumpmap());

				if( ui->groupBox_shadingReflection->isVisible() && ui->checkBox_basealpha->isChecked() )
					previewTexture( 0, "" );
				if( ui->groupBox_phong->isVisible() && ui->checkBox_exponentBaseAlpha->isChecked() )
					previewTexture( 4, "" );

			} else if( object == "preview_bumpmap1" ) {

				glWidget_diffuse1->loadTexture(glWidget_diffuse1->getDiffuse(), "");

				if( ui->groupBox_shadingReflection->isVisible() && ui->checkBox_normalalpha->isChecked() )
					previewTexture( 0, "" );
				if( ui->groupBox_phong->isVisible() && !ui->checkBox_exponentBaseAlpha->isChecked() )
					previewTexture( 4, "" );

			}

			else if( object == "preview_basetexture2" )
				glWidget_diffuse2->loadTexture("", glWidget_diffuse2->getBumpmap());
			else if( object == "preview_bumpmap2" )
				glWidget_diffuse2->loadTexture(glWidget_diffuse2->getDiffuse(), "");

			else {

				foreach(GLWidget* glWidget, glWidgets) {

					if( glWidget->objectName() == object ) {

						glWidget->setVisible(false);
						break;
					}
				}

			}

			return false;

		} else {

			if( object == "preview_basetexture1" )
				glWidget_diffuse1->loadTexture("", glWidget_diffuse1->getBumpmap());
			else if( object == "preview_bumpmap1" )
				glWidget_diffuse1->loadTexture(glWidget_diffuse1->getDiffuse(), "");

			else if( object == "preview_basetexture2" )
				glWidget_diffuse2->loadTexture("", glWidget_diffuse2->getBumpmap());
			else if( object == "preview_bumpmap2" )
				glWidget_diffuse2->loadTexture(glWidget_diffuse2->getDiffuse(), "");

			else {

				foreach(GLWidget* glWidget, glWidgets) {

					 if( glWidget->objectName() == object ) {

						glWidget->setVisible(true);
						break;
					}
				}
			}
		}

		TextureThread* textureThread = new TextureThread(this);
		connect(textureThread, SIGNAL(finished()), this, SLOT(finishedLoading()));

		textureThread->alpha = alpha;
		textureThread->alphaTest = alphaTest;
		textureThread->baseTexture = baseTexture;
		textureThread->alphaOnly = alphaOnly;
		textureThread->object = object;

		textureThread->output = Str( qHash( QFileInfo(texturePath + ".vtf").fileName() + Str( vtfFile.size() )));

		QFile cacheFile( "Cache/" + textureThread->output + ".png" );
		if( ignoreCache ) {
			if( cacheFile.exists() )
				cacheFile.remove();
		}

		if( cacheFile.exists() )
		{
			if( object == "preview_basetexture1" ) {

				glWidget_diffuse1->loadTexture( "Cache/" + textureThread->output, glWidget_diffuse1->getBumpmap() );

				if( ui->groupBox_shadingReflection->isVisible() ) {

					if( ui->checkBox_basealpha->isChecked() && ui->groupBox_phong->isVisible() )
						previewTexture( 1, ui->lineEdit_diffuse->text() );
					else if ( ui->checkBox_basealpha->isChecked() && !ui->groupBox_phong->isVisible())
						previewTexture( 0, ui->lineEdit_diffuse->text() );

				}

				if( ui->groupBox_phong->isVisible() ) {

					if( ui->checkBox_exponentBaseAlpha->isChecked() )
						previewTexture( 4, ui->lineEdit_diffuse->text() );
					else
						previewTexture( 4, ui->lineEdit_bumpmap->text() );

				}

			} else if( object == "preview_bumpmap1" ) {

				glWidget_diffuse1->loadTexture( glWidget_diffuse1->getDiffuse(), "Cache/" + textureThread->output );

				if( ui->groupBox_shadingReflection->isVisible() ) {

					if( ui->checkBox_normalalpha->isChecked() )
						previewTexture( 1, ui->lineEdit_bumpmap->text() );

				}

				if( ui->groupBox_phong->isVisible() ) {

					if( ui->checkBox_exponentBaseAlpha->isChecked() )
						previewTexture( 4, ui->lineEdit_diffuse->text() );
					else
						previewTexture( 4, ui->lineEdit_bumpmap->text() );
				}
			}

			else if( object == "preview_basetexture2" )
				glWidget_diffuse2->loadTexture( "Cache/" + textureThread->output, glWidget_diffuse2->getBumpmap() );
			else if( object == "preview_bumpmap2" )
				glWidget_diffuse2->loadTexture( glWidget_diffuse2->getDiffuse(), "Cache/" + textureThread->output );

			else {

				foreach(GLWidget* glWidget, glWidgets) {

					if( glWidget->objectName() == object ) {

						glWidget->loadTexture( "Cache/" + textureThread->output );
						break;
					}
				}

			}
		} else {

			QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

			textureThread->vtfFile = QFileInfo(texturePath + ".png").fileName();
			textureThread->input = "vtfcmd.exe -file \"" + texturePath.replace("/", "\\") + ".vtf\" -output \"" +
					QDir::currentPath().replace("/", "\\") + "\\Cache\" " + "-exportformat \"png\"";

			textureThread->start();
		}

		return true;
	}

	return false;
}

bool MainWindow::previewTexture( const int type, const QString& texture ) {

	/* types:
	 * 0 = ENVMAP inverted
	 * 1 = ENVMAP normal
	 *
	 * 3 = ENVMAP lineedit
	 * 4 = PHONG
	 *
	 */

	checkCacheSize();

	GLWidget_Spec* widget;
	QString preview;

	if (type <= 3) {
		widget = glWidget_envmap;
		preview = "preview_envmap1";
	} else {
		widget = glWidget_spec;
		preview = "preview_spec1";
	}

	GLWidget_Spec::Mode mode;

	if (type == 0) {
		mode = GLWidget_Spec::Diffuse;
	} else if (type == 3) {
		mode = GLWidget_Spec::Mask;
	} else {
		mode = GLWidget_Spec::Bumpmap;
	}

	if( getCurrentGame().isEmpty() ) {

		// Error( "No preview can be generated because no game is selected!")

		return false;

	} else {

		QString texturePath( currentGameMaterialDir() + "/" + texture );

		QFile vtfFile( texturePath + ".vtf" );
		if( !vtfFile.exists() ) {

			widget->updateValues(mode, texture);

			return false;

		} else {

			widget->updateValues(mode, texture);
		}

		TextureThread* textureThread = new TextureThread(this);
		connect(textureThread, SIGNAL(finished()), this, SLOT(finishedLoading()));

		textureThread->object = preview;
		textureThread->mode = mode;

		textureThread->output = Str( qHash( QFileInfo(texturePath + ".vtf").fileName() + Str( vtfFile.size() )));

		QFile cacheFile( "Cache/" + textureThread->output + ".png" );
		if( cacheFile.exists() ) {

			widget->updateValues(mode, "Cache/" + textureThread->output);

		} else {

			QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

			textureThread->vtfFile = QFileInfo(texturePath + ".png").fileName();
			textureThread->input = "vtfcmd.exe -file \"" + texturePath.replace("/", "\\") + ".vtf\" -output \"" +
					QDir::currentPath().replace("/", "\\") + "\\Cache\" " + "-exportformat \"png\"";

			textureThread->start();
		}

		return true;
	}

	return false;
}

void MainWindow::previewTexture(const QString& object)
{	
	QString cacheFile = "";
	const QString cacheFilePng = QString("Cache/%1.png").arg(object);
	const QString cacheFileTga = QString("Cache/%1.tga").arg(object);
	QFile png(cacheFilePng);
	QFile tga(cacheFileTga);

	if (png.exists())
		cacheFile = cacheFilePng;
	else if (tga.exists())
		cacheFile = cacheFileTga;

	if (object == "preview_basetexture1") {
		glWidget_diffuse1->loadTexture(cacheFile, glWidget_diffuse1->getBumpmap());

	} else if (object == "preview_bumpmap1") {
		glWidget_diffuse1->loadTexture(glWidget_diffuse1->getDiffuse(), cacheFile);

	} else if(object == "preview_basetexture2") {
		glWidget_diffuse2->loadTexture(cacheFile, glWidget_diffuse2->getBumpmap());

	} else if (object == "preview_bumpmap2") {
		glWidget_diffuse2->loadTexture(glWidget_diffuse2->getDiffuse(), cacheFile);

	} else {
		foreach (GLWidget* glWidget, glWidgets) {
			if (glWidget->objectName() == object) {
				glWidget->loadTexture(cacheFile);
				break;
			}
		}
	}
}

QString MainWindow::steamAppsDirectory()
{
#if defined(Q_OS_WIN)
	QSettings settings("HKEY_CURRENT_USER\\Software\\Valve\\Steam",
		QSettings::NativeFormat);
	QString steamDirString = settings.value("SteamPath").toString();

	if( !steamDirString.isEmpty() ) {
		QDir steamDir(steamDirString);

		if( steamDir.cd("steamapps") ) {
			mSteamInstalled = true;
			return steamDir.absolutePath();
		}
	}

	return "";
#elif defined(Q_OS_LINUX)
	QDir dir(QString("/home/%1/.local/share/Steam/")
		.arg(QString(qgetenv("USER"))));
	if (dir.exists() && dir.cd("steamapps"))
		mSteamInstalled = true;
	return dir.absolutePath();
#else
	return "";
#endif
}

void MainWindow::gameChanged( const QString& game )
{
	bool wasLoading = false;

	if(mLoading)
		wasLoading = true;
	else
		mLoading = true;

	QString surfaceSelection = ui->comboBox_surface->currentText();
	QString surfaceSelection2 = ui->comboBox_surface2->currentText();
	QString surfaceSelection3 = ui->comboBox_unlitTwoTextureSurface->currentText();
	QString surfaceSelection4 = ui->comboBox_refractSurface->currentText();

	ui->comboBox_surface->clear();
	ui->comboBox_surface2->clear();
	ui->comboBox_unlitTwoTextureSurface->clear();
	ui->comboBox_refractSurface->clear();

	ui->comboBox_surface->insertItem(0, "" );
	ui->comboBox_surface2->insertItem(0, "" );
	ui->comboBox_unlitTwoTextureSurface->insertItem(0, "");
	ui->comboBox_refractSurface->insertItem(0, "");

	// ui->checkBox_reflect2dskybox->setVisible(game == "Portal 2" || game == "Counter-Strike: Global Offensive" || game == "");

	// index == 0 alone won't trigger the first time (for some unknown reason), so -1 was added
	if(game == "")
	{
		ui->toolButton_diffuse->setDisabled(true);
		ui->toolButton_bumpmap->setDisabled(true);

		ui->toolButton_diffuse2->setDisabled(true);
		ui->toolButton_bumpmap2->setDisabled(true);

		ui->toolButton_detail->setDisabled(true);

		ui->toolButton_blendmodulate->setDisabled(true);

		ui->toolButton_envmap->setDisabled(true);

		ui->toolButton_specmap->setDisabled(true);

		ui->toolButton_specmap2->setDisabled(true);

		ui->toolButton_tintMask->setDisabled(true);

		ui->toolButton_exponentTexture->setDisabled(true);

		ui->toolButton_refractNormalMap->setDisabled(true);
		ui->toolButton_refractNormalMap2->setDisabled(true);

		ui->toolButton_waterNormalMap->setDisabled(true);
		ui->toolButton_flowMap->setDisabled(true);
		ui->toolButton_noiseTexture->setDisabled(true);

		ui->toolButton_refractTexture->setDisabled(true);

		ui->toolButton_bottomMaterial->setDisabled(true);

		ui->toolButton_toolTexture->setDisabled(true);

		ui->toolButton_lightWarp->setDisabled(true);

		ui->toolButton_unlitTwoTextureDiffuse->setDisabled(true);
		ui->toolButton_unlitTwoTextureDiffuse2->setDisabled(true);

		ui->toolButton_maskTexture->setDisabled(true);

		ui->toolButton_emissiveBlendTexture->setDisabled(true);
		ui->toolButton_emissiveBlendBaseTexture->setDisabled(true);
		ui->toolButton_emissiveBlendFlowTexture->setDisabled(true);

		QStringList defaultSurfaces = extractLines(":/surfaces/default");
		ui->comboBox_surface->insertItems(1, defaultSurfaces);
		ui->comboBox_surface2->insertItems(1, defaultSurfaces);
		ui->comboBox_unlitTwoTextureSurface->insertItems(1, defaultSurfaces);
		ui->comboBox_refractSurface->insertItems(1, defaultSurfaces);

		mGameSelected = false;
	}
	else
	{
		if( ui->comboBox_shader->currentText() != "Water" ) {

			ui->toolButton_diffuse->setEnabled(true);
			ui->toolButton_diffuse2->setEnabled(true);
		}

		ui->toolButton_bumpmap->setEnabled(true);

		ui->toolButton_bumpmap2->setEnabled(true);

		ui->toolButton_detail->setEnabled(true);

		ui->toolButton_maskTexture->setEnabled(true);

		ui->toolButton_blendmodulate->setEnabled(true);

		if( !ui->checkBox_cubemap->isChecked() )
			ui->toolButton_envmap->setEnabled(true);

		if( ui->lineEdit_specmap->isEnabled() )
			ui->toolButton_specmap->setEnabled(true);
		//	ui->lineEdit_specmap2->setEnabled(true);
		//	ui->toolButton_specmap2->setEnabled(true);

		if( !ui->checkBox_albedoTint->isChecked() )
			ui->toolButton_exponentTexture->setEnabled(true);

		ui->toolButton_refractNormalMap->setEnabled(true);
		ui->toolButton_refractNormalMap2->setEnabled(true);

		ui->toolButton_waterNormalMap->setEnabled(true);

		ui->toolButton_flowMap->setEnabled(true);
		ui->toolButton_noiseTexture->setEnabled(true);

		ui->toolButton_refractTexture->setEnabled(true);

		if( ui->checkBox_waterBottom->isChecked() )
			ui->toolButton_bottomMaterial->setDisabled(true);

		ui->toolButton_toolTexture->setEnabled(true);

		ui->toolButton_lightWarp->setEnabled(true);

		ui->toolButton_unlitTwoTextureDiffuse->setEnabled(true);
		ui->toolButton_unlitTwoTextureDiffuse2->setEnabled(true);

		ui->toolButton_emissiveBlendTexture->setEnabled(true);
		ui->toolButton_emissiveBlendBaseTexture->setEnabled(true);
		ui->toolButton_emissiveBlendFlowTexture->setEnabled(true);

		QStringList tmp(extractLines(":/surfaces/default"));

		if( getCurrentGame() == "Alien Swarm" )
		{
			tmp.append(extractLines(":/surfaces/alienSwarm"));
		}
		else if( getCurrentGame() == "Half-Life 2: Episode Two" )
		{
			tmp.append(extractLines(":/surfaces/ep2"));
		}
		else if( getCurrentGame() == "Half-Life 2" )
		{
			tmp.append( extractLines(":/surfaces/hl2") );
		}
		else if( getCurrentGame() == "Left 4 Dead" )
		{
			tmp.append( extractLines(":/surfaces/l4d") );
		}
		else if( getCurrentGame() == "Left 4 Dead 2" )
			tmp.append( extractLines(":/surfaces/l4d") + extractLines(":/surfaces/l4d2"));

		else if( getCurrentGame() == "Counter-Strike: Global Offensive" )
			tmp.append( extractLines(":/surfaces/csgo") );

		tmp.sort();

		ui->comboBox_surface->insertItems(1, tmp);
		ui->comboBox_surface2->insertItems(1, tmp);
		ui->comboBox_unlitTwoTextureSurface->insertItems(1, tmp);
		ui->comboBox_refractSurface->insertItems(1, tmp);

		ui->comboBox_surface->setCurrentIndex( ui->comboBox_surface->findText( surfaceSelection ));
		ui->comboBox_surface2->setCurrentIndex( ui->comboBox_surface2->findText( surfaceSelection2 ));
		ui->comboBox_unlitTwoTextureSurface->setCurrentIndex( ui->comboBox_unlitTwoTextureSurface->findText( surfaceSelection3 ));
		ui->comboBox_refractSurface->setCurrentIndex( ui->comboBox_refractSurface->findText(surfaceSelection4) );
		mGameSelected = true;
	}

	if(!wasLoading)
		mLoading = false;
}


#define UNCHECK_MENU(menu) { \
	for( int i = 0; i < menu->actions().count(); ++i ) \
	{ \
		menu->actions().at(i)->setChecked(false); \
	} \
}

#define ALLOW_MENU(menu) { \
	menu->setEnabled(true); \
	for( int i = 0; i < menu->actions().count(); ++i ) \
	{ \
		menu->actions().at(i)->setEnabled(true); \
	} \
}

void MainWindow::shaderChanged()
{
	bool luminanceEnabled = ui->comboBox_shader->currentText() == "Lightmapped_4WayBlend" || ui->comboBox_shader->currentText() == "Patch";
	bool isBlend = ui->comboBox_shader->currentText() == "WorldVertexTransition";

	ui->frame_texture->setVisible(luminanceEnabled);
	ui->frame_texture2->setVisible(luminanceEnabled);
	ui->frame_texture3->setVisible(luminanceEnabled);
	ui->frame_texture4->setVisible(luminanceEnabled);

	ui->label_detailAmount2->setVisible(luminanceEnabled);
	ui->label_detailAmount3->setVisible(luminanceEnabled);
	ui->label_detailAmount4->setVisible(luminanceEnabled);

	ui->doubleSpinBox_detailAmount2->setVisible(luminanceEnabled);
	ui->doubleSpinBox_detailAmount3->setVisible(luminanceEnabled);
	ui->doubleSpinBox_detailAmount4->setVisible(luminanceEnabled);

	ui->horizontalSlider_detailAmount2->setVisible(luminanceEnabled);
	ui->horizontalSlider_detailAmount3->setVisible(luminanceEnabled);
	ui->horizontalSlider_detailAmount4->setVisible(luminanceEnabled);

	//----------------------------------------------------------------------------------------//

	bool wasLoading = false;

	if (mLoading)
		wasLoading = true;
	else
		mLoading = true;

	if (!wasLoading) mLoading = false;

	if (mIgnoreShaderChanged)
		return;

	const QString shader = ui->comboBox_shader->currentText();
	// TODO: Use lowercased shader everywhere
	const QString s = shader.toLower();

	if (mSettings->showShaderNameInTab && ui->comboBox_shader->count() != 0)
		ui->tabWidget->setTabText( 0, "Parameters (" + shader + ")" );
	else
		ui->tabWidget->setTabText( 0, "Parameters" );

	//----------------------------------------------------------------------------------------//

	// Will hide groupboxes and uncheck actions for groups which have all their
	// default values. We therefore declutter the UI after a shader change as we
	// only displayed modified groups.
	for(int i = 0; i <= 24; ++i) {

		if(!isGroupboxChanged(static_cast<GroupBoxes>(i))) {

			switch(static_cast<GroupBoxes>(i)) {

			case BaseTexture: ui->groupBox_baseTexture->setVisible(false);ui->action_baseTexture->setChecked(false);break;
			case NormalBlend:
				normalblend::resetAction(ui);
				break;
			case BaseTexture2: ui->groupBox_baseTexture2->setVisible(false);ui->action_baseTexture2->setChecked(false);break;
			case Transparency: ui->groupBox_transparency->setVisible(false);ui->action_transparency->setChecked(false);break;
			case DetailTexture: ui->groupBox_detailTexture->setVisible(false);ui->action_detail->setChecked(false);break;
			case Color: ui->groupBox_color->setVisible(false);ui->action_color->setChecked(false);break;
			case OtherTexture: ui->groupBox_textureOther->setVisible(false);ui->action_other->setChecked(false);break;
			case Phong: ui->groupBox_phong->setVisible(false);ui->action_phong->setChecked(false);break;
			case PhongBrush:
				phong::resetAction(ui);
				break;
			case TreeSway:
				treesway::resetAction(ui);
				break;
			case LayerBlend:
				layerblend::resetAction(ui);
				break;
			case EmissiveBlend:
				emissiveblend::resetAction(ui);
				break;
			case Reflection: ui->groupBox_shadingReflection->setVisible(false);ui->action_reflection->setChecked(false);break;
			case SelfIllumination: ui->groupBox_selfIllumination->setVisible(false);ui->action_selfIllumination->setChecked(false);break;
			case RimLight: ui->groupBox_rimLight->setVisible(false);ui->action_rimLight->setChecked(false);break;
			case Water: ui->groupBox_water->setVisible(false);ui->action_water->setChecked(false);break;
			case WaterFlowmap: ui->groupBox_waterFlow->setVisible(false);ui->action_flowmap->setChecked(false);break;
			case WaterReflection: ui->groupBox_waterReflection->setVisible(false);ui->action_waterReflection->setChecked(false);break;
			case WaterRefraction: ui->groupBox_waterRefraction->setVisible(false);ui->action_refraction->setChecked(false);break;
			case Fog: ui->groupBox_waterFog->setVisible(false);ui->action_fog->setChecked(false);break;
			case Scroll: ui->groupBox_scroll->setVisible(false);ui->action_scroll->setChecked(false);break;
			case BaseTextureTransforms: ui->groupBox_baseTextureTransforms->setVisible(false);ui->action_baseTextureTransforms->setChecked(false);break;
			case BumpMapTransforms: ui->groupBox_bumpmapTransforms->setVisible(false);ui->action_bumpmapTransforms->setChecked(false);break;
			case Misc: ui->groupBox_misc->setVisible(false);ui->action_misc->setChecked(false);break;
			case Patch: ui->groupBox_patch->setVisible(false);ui->action_patch->setChecked(false);break;
			case Refract: ui->groupBox_refract->setVisible(false);ui->action_refract->setChecked(false);break;
			case Sprite: ui->groupBox_sprite->setVisible(false);ui->action_sprite->setChecked(false);break;
			case UnlitTwoTexture: ui->groupBox_unlitTwoTexture->setVisible(false);ui->action_unlitTwoTexture->setChecked(false);break;
			case BaseTexture3: ui->groupBox_baseTexture3->setVisible(false);ui->action_baseTexture3->setChecked(false);break;
			case BaseTexture4: ui->groupBox_baseTexture4->setVisible(false);ui->action_baseTexture4->setChecked(false);break;
			}
		}
	}

	//----------------------------------------------------------------------------------------//

	if (gShaders.contains(shader)) {

		ui->groupBox_waterReflection->setTitle("Reflection");

		if (shader == "Patch") {

			ui->action_patch->setEnabled(false);
			ui->action_patch->setChecked(true);
			ui->groupBox_patch->setVisible(true);

			ui->menu_custom->setEnabled(true);

			ui->checkBox_model->setEnabled(true);

			ALLOW_MENU(ui->menu_texture)
			ALLOW_MENU(ui->menu_shading)
			ALLOW_MENU(ui->menu_water)

		} else {

			const auto isVertexLitGeneric =
				(shader == "VertexLitGeneric");

			ui->action_baseTexture3->setVisible(luminanceEnabled);
			ui->action_baseTexture4->setVisible(luminanceEnabled);

			ui->action_CreateBlendTexture->setVisible(isBlend || luminanceEnabled);

			//----------------------------------------------------------------------------------------//

			ui->action_patch->setEnabled(false);
			ui->action_patch->setChecked(false);
			ui->groupBox_patch->setVisible(false);

			ui->menu_custom->setDisabled(true);

			//----------------------------------------------------------------------------------------//

			int shaderIndex = mSettings->customShaders.count() - 1;
			for (; shaderIndex >= 0; --shaderIndex ) {

				if (mSettings->customShaders.at(shaderIndex).name == shader) {

					break;
				}
			}

			if (ui->comboBox_shader->count() != 0) {

				for (int i = 0; i < mSettings->customShaders.at(shaderIndex).groups.count(); ++i) {

					processShaderGroups(mSettings->customShaders.at(shaderIndex).groups.at(i));
				}
			}

			QVector<Shader::Groups> remainingGroups;
			for (int i = 0; i <= 24; ++i) {

				remainingGroups.push_back(static_cast<Shader::Groups>(i));
			}

			for (int i = 0; i < mSettings->customShaders.at(shaderIndex).groups.count(); ++i) {

				if (mSettings->customShaders.at(shaderIndex).groups.contains(static_cast<Shader::Groups>(i))) {

					remainingGroups.remove(remainingGroups.indexOf(static_cast<Shader::Groups>(i)));
				}
			}

			//----------------------------------------------------------------------------------------//

			ui->checkBox_model->setEnabled( shader == "VertexLitGeneric"   ||
											shader == "LightmappedGeneric" ||
											shader == "WorldVertexTransition" ||
											shader == "Refract" );

			ui->action_normalBlend->setEnabled( shader == "LightmappedGeneric" );
			ui->action_normalBlend->setVisible( shader == "LightmappedGeneric" );
			ui->action_rimLight->setVisible( shader == "VertexLitGeneric" );
			ui->action_rimLight->setVisible( shader == "VertexLitGeneric" );

			ui->action_treeSway->setVisible(isVertexLitGeneric);

			ui->checkBox_tintSpecMask->setVisible(isVertexLitGeneric);

			ui->action_decal->setVisible(isVertexLitGeneric);
			if (!isVertexLitGeneric) {
				ui->action_treeSway->setChecked(false);
				ui->action_decal->setChecked(false);
				ui->groupBox_treeSway->setVisible(false);
				ui->groupBox_textureDecal->setVisible(false);
			}

			if(shader != "LightmappedGeneric") {
				ui->groupBox_normalBlend->setVisible(false);
				ui->action_normalBlend->setChecked(false);
			}
			ui->action_layerBlend->setVisible(isBlend);

			ui->action_emissiveBlend->setVisible(isVertexLitGeneric);

			ui->horizontalSlider_reflectivity_2->setVisible( shader == "WorldVertexTransition" );
			ui->doubleSpinBox_reflectivity_2->setVisible( shader == "WorldVertexTransition" );
			ui->toolButton_reflectivity_2->setVisible( shader == "WorldVertexTransition" );
			ui->toolButton_reflectivity_2->setVisible( shader == "WorldVertexTransition" );
			ui->label_reflectivity2->setVisible( shader == "WorldVertexTransition" );

			ui->lineEdit_specmap2->setVisible( shader == "WorldVertexTransition" );
			ui->toolButton_specmap2->setVisible( shader == "WorldVertexTransition" );
			ui->label_specmap2->setVisible( shader == "WorldVertexTransition" );

			ui->frame_detail2->setVisible(isBlend);
			ui->comboBox_detailBlendMode2->setVisible(false);

			ui->label_detailAmount2->setVisible(isBlend || luminanceEnabled);
			ui->doubleSpinBox_detailAmount2->setVisible(isBlend || luminanceEnabled);
			ui->horizontalSlider_detailAmount2->setVisible(isBlend || luminanceEnabled);


			//----------------------------------------------------------------------------------------//

			ui->action_baseTexture->setChecked(shader == "Lightmapped_4WayBlend");
			ui->action_baseTexture2->setChecked(shader == "Lightmapped_4WayBlend");

			ui->action_baseTexture->setEnabled(shader != "Lightmapped_4WayBlend");
			ui->action_baseTexture2->setEnabled(shader != "Lightmapped_4WayBlend");

			ui->groupBox_baseTexture->setVisible(shader == "Lightmapped_4WayBlend");
			ui->groupBox_baseTexture2->setVisible(shader == "Lightmapped_4WayBlend");

			if(shader == "Lightmapped_4WayBlend") {
				ui->action_baseTexture3->setEnabled(false);
				ui->action_baseTexture4->setEnabled(false);

				ui->action_baseTexture3->setChecked(true);
				ui->action_baseTexture4->setChecked(true);
			}

			ui->groupBox_baseTexture3->setVisible(shader == "Lightmapped_4WayBlend");
			ui->groupBox_baseTexture4->setVisible(shader == "Lightmapped_4WayBlend");

			//----------------------------------------------------------------------------------------//

			// Base Texture not allowed
			if (shader == "Refract" || shader == "UnlitTwoTexture" || shader == "Water") {
				ui->action_baseTexture->setChecked(false);
				ui->groupBox_baseTexture->setVisible(false);

			} else { // Base Texture enforced

				ui->action_baseTexture->setChecked(true);
				ui->groupBox_baseTexture->setVisible(true);
			}

			ui->action_baseTexture->setDisabled(true);

			//----------------------------------------------------------------------------------------//

			// Base Texture 2 enforced
			if (shader == "WorldVertexTransition") {

				ui->action_baseTexture2->setChecked(true);
				ui->groupBox_baseTexture2->setVisible(true);

			} else { // Base Texture 2 not allowed

				ui->action_baseTexture2->setChecked(false);
				ui->groupBox_layerblend->setVisible(false);
			}

			ui->label_blendmodulate->setVisible( shader != "Lightmapped_4WayBlend" );
			ui->toolButton_blendmodulate->setVisible( shader != "Lightmapped_4WayBlend" );
			ui->lineEdit_blendmodulate->setVisible( shader != "Lightmapped_4WayBlend" );

			ui->action_baseTexture2->setDisabled(true);

			//----------------------------------------------------------------------------------------//

			// Water enforced
			// Flowmap, Reflection, Refraction, Fog, Scroll allowed
			// Transparency, Detail, Color, Other not allowed
			if (shader == "Water") {

				ui->groupBox_water->setVisible(true);

				//----------------------------------------------------------------------------------------//

				ui->action_transparency->setChecked(false);
				ui->groupBox_transparency->setVisible(false);

				ui->action_detail->setChecked(false);
				ui->groupBox_detailTexture->setVisible(false);

				ui->action_color->setChecked(false);
				ui->groupBox_color->setVisible(false);

				ui->action_reflection->setChecked(false);
				ui->groupBox_shadingReflection->setVisible(false);

				ui->action_other->setChecked(false);
				ui->groupBox_textureOther->setVisible(false);

				//----------------------------------------------------------------------------------------//

				ui->menu_texture->setDisabled(true);
				ui->menu_shading->setEnabled(true);
				ui->menu_water->setEnabled(true);

			} else { // Transparency, Detail, Color, Other allowed
					 // Water, Flowmap, Reflection, Refraction, Fog, Scroll  not allowed

				ui->groupBox_water->setVisible(false);

				ui->action_flowmap->setChecked(false);
				ui->groupBox_waterFlow->setVisible(false);

				ui->action_waterReflection->setChecked(false);
				ui->groupBox_waterReflection->setVisible(false);

				ui->action_refraction->setChecked(false);
				ui->groupBox_waterRefraction->setVisible(false);

				ui->action_fog->setChecked(false);
				ui->groupBox_waterFog->setVisible(false);

				ui->action_scroll->setChecked(false);
				ui->groupBox_scroll->setVisible(false);

				//----------------------------------------------------------------------------------------//

				ui->menu_texture->setEnabled(true);
				ui->menu_shading->setEnabled(true);
				ui->menu_water->setDisabled(true);
			}

			//----------------------------------------------------------------------------------------//

			shaders::handlePhongBrushRim(ui, s);

			//----------------------------------------------------------------------------------------//

			// Reflection, Self Illumination not allowed
			if (shader == "Water" || shader == "UnlitTwoTexture" ) {

				ui->menu_shading->setDisabled(true);

				ui->action_reflection->setEnabled(false);
				ui->action_reflection->setChecked(false);
				ui->groupBox_shadingReflection->setVisible(false);

				ui->action_selfIllumination->setEnabled(false);
				ui->action_selfIllumination->setChecked(false);
				ui->groupBox_selfIllumination->setVisible(false);

			} else if (shader == "UnlitGeneric" ) {

				ui->menu_shading->setDisabled(false);

				ui->action_reflection->setEnabled(true);

				ui->action_selfIllumination->setEnabled(false);
				ui->action_selfIllumination->setChecked(false);
				ui->groupBox_selfIllumination->setVisible(false);

			} else { // Reflection, Self Illumination allowed

				ui->menu_shading->setEnabled(true);

				ui->action_reflection->setVisible(true);
				ui->action_reflection->setEnabled(true);

				ui->action_selfIllumination->setVisible(true);
				ui->action_selfIllumination->setEnabled(true);
			}

			if (shader == "Water") {

				ui->menu_shading->setEnabled(true);
				ui->action_reflection->setVisible(true);
				ui->action_reflection->setEnabled(true);
			}

			ui->groupBox_refract->setVisible(shader == "Refract");
			ui->groupBox_sprite->setVisible(shader == "Sprite");
			ui->groupBox_unlitTwoTexture->setVisible(shader == "UnlitTwoTexture");
		}

		if (ui->comboBox_shader->count() != 0) {

			for (int i = mSettings->customShaders.count() - 1; i >= 0; --i ) {

				if (mSettings->customShaders.at(i).name == shader) {

					//QVector<Shader::Groups> groups;// = Shader::transformGroups(Shader::S_Custom, mSettings->customShaders.at(i).groups);

					for (int j = 0; j < mSettings->customShaders.at(i).groups.count(); ++j) {

						processShaderGroups(mSettings->customShaders.at(i).groups.at(j));
					}

					break;
				}
			}
		}

	} else {
		ui->groupBox_waterReflection->setTitle("Water - Reflection");

		ui->menu_custom->setEnabled(true);

		ui->action_patch->setEnabled(true);

		ui->action_baseTexture->setEnabled(true);
		ui->action_baseTexture2->setEnabled(true);

		ui->action_baseTexture->setVisible(true);
		ui->action_baseTexture2->setVisible(true);

		ui->action_CreateBlendTexture->setVisible(true);

		ui->action_phongBrush->setEnabled(false);

		ui->action_phong->setVisible(true);
		ui->action_phong->setEnabled(true);

		ui->action_rimLight->setEnabled(true);
		ui->action_waterReflection->setEnabled(true);
		ui->action_reflection->setEnabled(true);

		ui->menu_texture->setEnabled(true);
		ui->menu_shading->setEnabled(true);
		ui->menu_water->setEnabled(true);

		if (ui->comboBox_shader->count() != 0) {

			for (int i = mSettings->customShaders.count() - 1; i >= 0; --i ) {

				if (mSettings->customShaders.at(i).name == shader) {

					//QVector<Shader::Groups> groups;// = Shader::transformGroups(Shader::S_Custom, mSettings->customShaders.at(i).groups);

					for (int j = 0; j < mSettings->customShaders.at(i).groups.count(); ++j) {

						processShaderGroups(mSettings->customShaders.at(i).groups.at(j));
					}

					break;
				}
			}
		}
	}

	ui->menu_texture->menuAction()->setVisible( ui->menu_texture->isEnabled() );
	ui->menu_shading->menuAction()->setVisible( ui->menu_shading->isEnabled() );
	ui->menu_water->menuAction()->setVisible( ui->menu_water->isEnabled() );

	ui->menu_custom->menuAction()->setVisible( ui->menu_custom->isEnabled() );

	ui->action_baseTexture->setVisible( ui->action_baseTexture->isEnabled() );
	ui->action_baseTexture2->setVisible( ui->action_baseTexture2->isEnabled() );
	ui->action_transparency->setVisible( ui->action_transparency->isEnabled() );
	ui->action_detail->setVisible( ui->action_detail->isEnabled() );
	ui->action_color->setVisible( ui->action_color->isEnabled() );
	ui->action_other->setVisible( ui->action_other->isEnabled() );

	ui->action_phong->setVisible( ui->action_phong->isEnabled() );
	ui->action_phongBrush->setVisible( ui->action_phongBrush->isEnabled() );
	ui->action_reflection->setVisible( ui->action_reflection->isEnabled() );
	ui->action_selfIllumination->setVisible( ui->action_selfIllumination->isEnabled() );
	ui->action_rimLight->setVisible( ui->action_rimLight->isEnabled() );

	ui->action_normalBlend->setVisible( ui->action_normalBlend->isEnabled() );
	ui->action_treeSway->setVisible( ui->action_treeSway->isEnabled() );
	ui->action_decal->setVisible( ui->action_decal->isEnabled() );
	ui->action_layerBlend->setVisible( ui->action_layerBlend->isEnabled() );
	ui->action_emissiveBlend->setVisible( ui->action_emissiveBlend->isEnabled() );

	//----------------------------------------------------------------------------------------//

	transparencyAction->setVisible( ui->action_transparency->isEnabled() && ui->menu_texture->isEnabled() );
	detailAction->setVisible( ui->action_detail->isEnabled() && ui->menu_texture->isEnabled()  );
	colorAction->setVisible( ui->action_color->isEnabled() && ui->menu_texture->isEnabled() );
	otherAction->setVisible( ui->action_other->isEnabled() && ui->menu_texture->isEnabled() );

	reflectionAction->setVisible( ui->action_reflection->isEnabled() && ui->menu_shading->isEnabled() );
	selfillumAction->setVisible( ui->action_selfIllumination->isEnabled() && ui->menu_shading->isEnabled() );
	phongAction->setVisible( ui->action_phong->isEnabled() && ui->menu_shading->isEnabled() );

	phongBrushAction->setVisible( ui->action_phongBrush->isEnabled() && ui->menu_shading->isEnabled() );

	waterFlowmapAction->setVisible( ui->action_flowmap->isEnabled() && ui->menu_water->isEnabled() );
	waterReflectionAction->setVisible( ui->action_waterReflection->isEnabled() && ui->menu_water->isEnabled() );
	waterRefractionAction->setVisible( ui->action_refraction->isEnabled() && ui->menu_water->isEnabled() );
	waterFogAction->setVisible( ui->action_fog->isEnabled() && ui->menu_water->isEnabled() );

	updateWindowTitle();

    if (mSettings->autoRefresh)
        refreshRequested();

	if (!wasLoading) mLoading = false;
}

#define TRIGGER_IF(x) if (!x->isChecked() && x->isEnabled()) x->trigger(); break;

void MainWindow::closeEvent( QCloseEvent* event )
{
	if( mChildWidgetChanged )
	{
		mExitRequested = true;

		switch( _displaySaveMessage() )
		{
		case QMessageBox::Save:

			action_Save();

			if(mExitRequested != false)
			{
				saveSettings();

				if(mSettings->deleteCacheAfterApplicationExit)
					clearCacheFolder();

				event->accept();
			}
			else
			{
				event->ignore();
			}

			break;

		case QMessageBox::Discard:

			saveSettings();

			if(mSettings->deleteCacheAfterApplicationExit)
				clearCacheFolder();

			event->accept();

			break;

		case QMessageBox::Cancel:
		case QMessageBox::Escape:

			event->ignore();

			break;
		}

		mExitRequested = false;

		return;
	}

	saveSettings();

	if(mSettings->deleteCacheAfterApplicationExit)
		clearCacheFolder();
}

void MainWindow::showEvent(QShowEvent *event)
{
	Q_UNUSED(event)
	if (!initialFile.isEmpty()) {
		loadVMT(initialFile);
		initialFile = "";
	}


	if (mOpenConvertDialog) {
		QTimer::singleShot(0, this, SLOT( displayConversionDialog() ));
	}
}

void MainWindow::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	// check for our needed mime type, here a file or a list of files
	if (mimeData->hasUrls())
	{
		QString str = mimeData->urls().at(0).toLocalFile();

		if(QFileInfo(str).suffix() == "vmt") {

			if( mChildWidgetChanged )
			{
				switch( _displaySaveMessage() )
				{
				case QMessageBox::Save:

					action_Save();

				case QMessageBox::Discard:

					break;

				case QMessageBox::Cancel:
				case QMessageBox::Escape:

					return;
				}
			}

			loadVMT(mimeData->urls().at(0).toLocalFile());
		} else {
			sortDroppedTextures (mimeData);
			//Info("has images");
		}
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls())
	{
		foreach (const QUrl& url, event->mimeData()->urls())
		{
			QString str = url.toLocalFile();
			if (!str.isEmpty())
			{
				if (QFileInfo(str).suffix() == "vmt" ||
					QFileInfo(str).suffix() == "vtf" ||
					QFileInfo(str).suffix() == "png" ||
					QFileInfo(str).suffix() == "tga")
				{
					event->acceptProposedAction();
				}
			}
		}
	}
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void MainWindow::setGameState(bool enabled)
{
	QList<QAction*> actions = ui->action_games->actions();

	foreach(QAction* action, actions) {

		if( !(action->text() == "Manage games..." || action->text() == "Browse to game") ) {

			action->setEnabled(enabled);
		}
	}
}

QString MainWindow::getCurrentGame()
{
	QList<QAction*> actions = ui->action_games->actions();

	foreach(QAction* action, actions) {

		if(action->isChecked())
			return action->text();
	}

	return "";
}

void MainWindow::setGames( const QStringList& gameList )
{
	QList<QAction*> actions;

	foreach(QString gameName, gameList) {

		QAction* action = new QAction( gameName, this );
		action->setCheckable(true);

		actions.append(action);
		connect(action, SIGNAL(triggered(bool)), this, SLOT(gameTriggered(bool)));
	}

	ui->action_games->clear();
	ui->action_games->addActions(actions);
	ui->action_games->addSeparator();
		actions.clear();
			QAction* action = new QAction( "Browse to game", this );
			connect(action, SIGNAL(triggered()), this, SLOT(browseToGameDirectory()));
		actions.append(action);
			action = new QAction( "Manage games...", this );
			connect(action, SIGNAL(triggered()), this, SLOT(showEditGamesDialog()));
		actions.append(action);
	ui->action_games->addActions(actions);
}

void MainWindow::setCurrentGame(const QString& game)
{
	QList<QAction*> actions = ui->action_games->actions();

	foreach(QAction* action, actions) {
		action->setChecked(action->text() == game);
	}

	gameChanged(game);
}

void MainWindow::saveSettings()
{
	mIniSettings->setValue("mainWindowGeometry", saveGeometry());
	mIniSettings->setValue("mainWindowState", saveState());

	mIniSettings->setValue("saveLastGame", mSettings->saveLastGame ? "1" : "0");

	if(mSettings->saveLastGame)
		mIniSettings->setValue("lastGame", getCurrentGame());
}

void MainWindow::readSettings()
{
	mSettings->saveLastGame = setKey("saveLastGame", true, mIniSettings);
	mSettings->fullPathFilenameInWindowTitle =
		setKey("fullPathFilenameInWindowTitle", false, mIniSettings);
	mSettings->showShaderNameInWindowTitle =
		setKey("showShaderNameInWindowTitle", true, mIniSettings);

	mSettings->autoRefresh = setKey("autoRefresh", true, mIniSettings);
	mSettings->autoSave = setKey("autoSave", false, mIniSettings);

	mSettings->templateNew = setKey("templateNew", false, mIniSettings);
	mSettings->checkForUpdates = setKey("checkForUpdates", true, mIniSettings);
	mSettings->removeSuffix = setKey("removeSuffix", false, mIniSettings);
	mSettings->removeAlpha = setKey("removeAlpha", false, mIniSettings);
	mSettings->changeName = setKey("changeName", false, mIniSettings);

	mSettings->noNormalSharpen = setKey("noNormalSharpen", false, mIniSettings);
	mSettings->uncompressedNormal = setKey("uncompressedNormal", false, mIniSettings);
	mSettings->noGlossMip = setKey("noGlossMip", false, mIniSettings);

	mSettings->useIndentation =
		setKey("useIndentation", true, mIniSettings);
	mSettings->useQuotesForTexture =
		setKey("useQuotesForTexture", true, mIniSettings);
	mSettings->deleteCacheAfterApplicationExit =
		setKey("clearCacheAfterExit", false, mIniSettings);


	mSettings->recentFileListStyle =
		setKey("recentFileListStyle", false, mIniSettings)
			? Settings::RecentFileMenu : Settings::FileMenu;

	if (mSettings->recentFileListStyle == Settings::FileMenu)
		ui->menuRecentFiles->menuAction()->setVisible(false);


	mSettings->parameterSortStyle =
		setKey("parameterSortStyle", false, mIniSettings)
			? Settings::AlphabeticallySorted : Settings::Grouped;


	mSettings->showShaderNameInTab =
		setKey("showShaderNameInTab", false, mIniSettings);

	const QString tabTitle = mSettings->showShaderNameInTab
		? "Parameters (" + ui->comboBox_shader->currentText() + ")"
		: "Parameters";

	ui->tabWidget->setTabText(0, tabTitle);

	// Note that if you just pass "" for the def, because then the boolean
	// overloaded method is used for some random reason
	mSettings->lastGame = setKey("lastGame", QString(), mIniSettings);

	mSettings->diffuseSuffix = setKey("diffuseSuffix", QString("_d"), mIniSettings);
	mSettings->bumpSuffix = setKey("bumpSuffix", QString("_n"), mIniSettings);
	mSettings->specSuffix = setKey("specSuffix", QString("_s"), mIniSettings);
	mSettings->glossSuffix = setKey("glossSuffix", QString("_g"), mIniSettings);
	mSettings->tintmaskSuffix = setKey("tintmaskSuffix", QString("_tintmask"), mIniSettings);

	mSettings->mipmapFilter = setKey("mipmapFilter", QString("Box"), mIniSettings);
	mSettings->mipmapSharpenFilter = setKey("mipmapSharpenFilter", QString("Sharpen Soft"), mIniSettings);

	QFile defaultShaderFile(":/files/defaultShaders");
	defaultShaderFile.open(QFile::ReadOnly | QFile::Text);

	while (!defaultShaderFile.atEnd()) {
		QString line = defaultShaderFile.readLine();
		QStringList options( line.split("?") );

		QVector< Shader::Groups > groups;

		for( int i = 2; i < options.count(); ++i )
		{
			groups.append( static_cast< Shader::Groups >( options.at(i).toInt() ));
		}

		mSettings->defaultShaders.append( Shader( options.at(0), options.at(1) == "1", groups ));
	}

	defaultShaderFile.close();


	QStringList shaderList( mIniSettings->value("shaders").toString().split( "??", QString::SkipEmptyParts ));

	for( int i = 0; i < shaderList.count(); ++i )
	{
		QStringList shaderParameterList( shaderList.at(i).split( "?", QString::SkipEmptyParts ));

		switch( shaderParameterList.count() )
		{
		case 1:

			mSettings->customShaders.push_back( Shader( shaderParameterList.at(0) ));

			break;

		case 2:

			mSettings->customShaders.push_back( Shader( shaderParameterList.at(0), shaderParameterList.at(1) == "1" ));

			break;

		default:

			QVector< Shader::Groups > groups;

			for( int j = 2; j < shaderParameterList.count(); ++j )
			{
				groups.push_back( static_cast< Shader::Groups>( shaderParameterList.at(j).toUInt() ));
			}

			mSettings->customShaders.push_back( Shader( shaderParameterList.at(0), shaderParameterList.at(1) == "1", groups ));

			break;
		}
	}


	QString tmp(setKey( "cacheSize", "100", mIniSettings ));
	mSettings->cacheSize = ( tmp.toUInt() >= 1 && tmp.toUInt() <= 16384 ) ? tmp.toUInt() : 100;


	QString addToIniSettings;

	addToIniSettings.append( addDefaultShader("LightmappedGeneric",	   true, mSettings, QVector< Shader::Groups >() << Shader::G_Base_Texture ) );
	addToIniSettings.append( addDefaultShader("UnlitGeneric",		   true, mSettings, QVector< Shader::Groups >() << Shader::G_Base_Texture ) );
	addToIniSettings.append( addDefaultShader("VertexLitGeneric",	   true, mSettings, QVector< Shader::Groups >() << Shader::G_Base_Texture ) );
	addToIniSettings.append( addDefaultShader("WorldVertexTransition", true, mSettings, QVector< Shader::Groups >() << Shader::G_Base_Texture << Shader::G_Base_Texture2 ) );
	addToIniSettings.append( addDefaultShader("UnlitTwoTexture",	   true, mSettings, QVector< Shader::Groups >() ) );
	addToIniSettings.append( addDefaultShader("Lightmapped_4WayBlend", true, mSettings,
											  QVector< Shader::Groups >() << Shader::G_Base_Texture  << Shader::G_Base_Texture2
																		  << Shader::G_Base_Texture3 << Shader::G_Base_Texture4 ) );

	// Sprite, Refract and Water have enforced shader groups
	addToIniSettings.append( addDefaultShader("Cable",		     false, mSettings ) );
	addToIniSettings.append( addDefaultShader("Decal",		     true,  mSettings ) );
	addToIniSettings.append( addDefaultShader("DecalModulate",   true,  mSettings ) );
	addToIniSettings.append( addDefaultShader("Infected",	     false, mSettings ) );
	addToIniSettings.append( addDefaultShader("Modulate",	     false, mSettings ) );
	addToIniSettings.append( addDefaultShader("MonitorScreen",   false, mSettings ) );
	addToIniSettings.append( addDefaultShader("Patch",		     true,  mSettings ) );
	addToIniSettings.append( addDefaultShader("Predator",	     false, mSettings ) );
	addToIniSettings.append( addDefaultShader("Refract",	     true,  mSettings ) );
	addToIniSettings.append( addDefaultShader("Shattered Glass", false, mSettings ) );
	addToIniSettings.append( addDefaultShader("Sprite",		     true,  mSettings ) );
	addToIniSettings.append( addDefaultShader("SpriteCard",	     false, mSettings ) );
	addToIniSettings.append( addDefaultShader("Water",		     true,  mSettings ) );

	if( !addToIniSettings.isEmpty() )
		mIniSettings->setValue( "shaders", mIniSettings->value("shaders").toString() + addToIniSettings );

	for (int i = 0; i < mSettings->customShaders.size(); ++i) {

		if (mSettings->customShaders.at(i).enabled)
			ui->comboBox_shader->addItem(mSettings->customShaders.at(i).name);
	}

	sortShaderComboBox();

	//----------------------------------------------------------------------------------------//

	if (mIniSettings->contains("mainWindowGeometry"))
		restoreGeometry(mIniSettings->value("mainWindowGeometry").toByteArray());
	else
		resize(100, 100);

	if (mIniSettings->contains("mainWindowState"))
		restoreState(mIniSettings->value("mainWindowState").toByteArray());
}

void MainWindow::changeOption( Settings::Options option, const QString& value )
{
	switch(option)
	{
		case Settings::_FullPathFilenameInWindowTitle:
		case Settings::_ShowShaderNameInWindowTitle:

			updateWindowTitle();

			break;

		case Settings::_RecentFileListStyle:

			if( value == "0" )
			{
				for( uint i = 0; i < MaxRecentFiles; ++i )
				{
					ui->menuRecentFiles->removeAction( recentFileActions[i] );
					ui->menuFile->insertAction( ui->actionExit, recentFileActions[i] );
				}

				ui->menuRecentFiles->menuAction()->setVisible(false);

				ui->menuFile->insertAction( ui->actionExit, separatorAct );
			}
			else
			{
				for( uint i = 0; i < MaxRecentFiles; ++i )
				{
					ui->menuFile->removeAction( recentFileActions[i] );
					ui->menuRecentFiles->insertAction( NULL, recentFileActions[i] );
				}

				ui->menuRecentFiles->menuAction()->setVisible(true);
			}

			break;

		case Settings::_RecentFileEntryStyle:

			updateRecentFileActions( value == "1" );

			break;

		case Settings::_UseQuotesForTexture:
		case Settings::_ParameterSortStyle:

			if( !ui->vmtPreviewTextEdit->toPlainText().isEmpty() )
				refreshRequested();

			break;

		case Settings::_UseIndentation:

			refreshRequested();

			break;

		case Settings::_ShowShaderNameInTab:

			ui->tabWidget->setTabText( 0, "Parameters" + ((value == "1") ? " (" + ui->comboBox_shader->currentText() + ")" : "") );

			break;

		case Settings::_CacheSize:
			checkCacheSize();
			break;

		case Settings::_AutoRefresh:
			refreshRequested();
			break;

		case Settings::_RemoveSuffix:
			break;

		case Settings::_AutoSave:
			break;

		case Settings::_RemoveAlpha:
			break;

		case Settings::_TemplateNew:
			break;

		case Settings::_CheckForUpdates:
			break;

		case Settings::_ChangeName:
			break;

		case Settings::_CustomShaders:
			// TODO: Move into own function

			QString currentShader = ui->comboBox_shader->currentText();

			ui->comboBox_shader->clear();

			for (int i = 0; i < mSettings->customShaders.size(); ++i) {

				if (mSettings->customShaders.at(i).enabled)
					ui->comboBox_shader->addItem(mSettings->customShaders.at(i).name);
			}

			sortShaderComboBox();

			ui->comboBox_shader->setCurrentIndex( ui->comboBox_shader->findText(currentShader) );

			break;
	}
}

void MainWindow::toggledDetailUniformScale(bool checked)
{
	const auto object = qobject_cast<QWidget *>(sender());

	if(object == ui->checkBox_detailScaleUniform)
		detailtexture::toggledUniformScale(checked, ui);
	else if(object == ui->checkBox_detailScaleUniform2)
		detailtexture::toggledUniformScale(checked, ui, true);
}


void MainWindow::updateTextureOnUi(
	const QString& objectName, const QString& relativeFilePath)
{
	previewTexture(objectName, relativeFilePath, true, false, false, false, true);
}

void MainWindow::openRecentFile()
{
	QAction* action = qobject_cast<QAction*>(sender());

	if(action)
	{
		if( mChildWidgetChanged )
		{
			switch( _displaySaveMessage() )
			{
			case QMessageBox::Save:

				action_Save();

			case QMessageBox::Cancel:
			case QMessageBox::Escape:

				return;
			}
		}

		if( QDir().exists( action->data().toString() ))
		{
			loadVMT( action->data().toString() );
		}
		else
		{
			QStringList files = mIniSettings->value("recentFileList").toStringList();

			files.removeAll( action->data().toString() );

			mIniSettings->setValue("recentFileList", files);

			updateRecentFileActions( mSettings->recentFileEntryStyle == Settings::FullPath ? true : false );

#ifdef Q_OS_WIN
			SetForegroundWindow( (HWND)winId() );
#endif

			MsgBox msgBox(this);
				msgBox.setWindowTitle("VMT Editor");
				msgBox.setStandardButtons( QMessageBox::Ok );
				msgBox.setIcon( QMessageBox::Warning );
				msgBox.setText( "\"" + action->data().toString() + "\"\n\ndoes not exist. Removing from recent file list!" );

			msgBox.exec();
		}
	}
}

void MainWindow::openTemplate() {

	mLoading = true;

	QAction* action = qobject_cast<QAction*>(sender());

	if (mSettings->templateNew) {

		if (mChildWidgetChanged) {
			switch( _displaySaveMessage() ) {

			case QMessageBox::Save:
				action_Save();
			case QMessageBox::Cancel:
			case QMessageBox::Escape:
				return;
			}
		}

		resetWidgets();
		updateWindowTitle();

		QList<QAction*> actions = ui->action_games->actions();
		foreach(QAction* action, actions) {
			action->setEnabled(true);
		}

		setCurrentGame(mSettings->saveLastGame ? mSettings->lastGame : "");
	}

	VmtFile vmt = vmtParser->loadVmtFile( action->data().toString(), true );

	if (!ui->textEdit_proxies->toPlainText().isEmpty() &&
		!vmt.subGroups.isEmpty() ) {

		ui->textEdit_proxies->moveCursor(QTextCursor::End,
										 QTextCursor::MoveAnchor);

		ui->textEdit_proxies->textCursor().deletePreviousChar();
		ui->textEdit_proxies->insertPlainText(
				vmt.subGroups.replace("    ", "\t")
					.replace("Proxies\n{\n", "\n") );

	} else {
		ui->textEdit_proxies->insertPlainText(
				vmt.subGroups.replace("    ", "\t") );
	}

	parseVMT(vmt, true);

	mLoading = false;

	refreshRequested();
}


void MainWindow::setCurrentFile( const QString& fileName )
{
	QStringList files = mIniSettings->value("recentFileList").toStringList();

	files.removeAll(fileName);
	files.prepend(fileName);

	while( files.size() > MaxRecentFiles )
		files.removeLast();

	mIniSettings->setValue("recentFileList", files);

#ifdef Q_OS_WIN
	auto item = new QWinJumpListItem(QWinJumpListItem::Link);
	item->setFilePath(QCoreApplication::applicationFilePath());
	item->setWorkingDirectory(QCoreApplication::applicationDirPath());
	item->setTitle(QFileInfo(fileName).fileName());
	item->setIcon(QIcon(":/icons/vmt_192_flat"));
	item->setDescription(fileName);
	item->setArguments(QStringList(fileName));

	QWinJumpList().recent()->addItem(item);
#endif

	updateRecentFileActions(mSettings->recentFileEntryStyle == Settings::FullPath);
}

void MainWindow::updateRecentFileActions( bool fullPath )
{
	 QStringList files = mIniSettings->value("recentFileList").toStringList();

	 int numRecentFiles = qMin( files.size(), (int)MaxRecentFiles );

	 if(fullPath)
	 {
		 for( int i = 0; i < numRecentFiles; ++i )
		 {
			QString text = tr("&%1").arg( QFileInfo(files[i]).absoluteFilePath() );
			recentFileActions[i]->setText(text);
			recentFileActions[i]->setData(files[i]);
			recentFileActions[i]->setVisible(true);
		 }
	 }
	 else
	 {
		 for( int i = 0; i < numRecentFiles; ++i )
		 {
			QString text = tr("&%1").arg( QFileInfo(files[i]).fileName() );
			recentFileActions[i]->setText(text);
			recentFileActions[i]->setData(files[i]);
			recentFileActions[i]->setVisible(true);
		 }
	 }


	 for( int j = numRecentFiles; j < MaxRecentFiles; ++j )
	 {
		 recentFileActions[j]->setVisible(false);
	 }


	 if( numRecentFiles > 0 )
	 {
		 ui->menuFile->insertAction( ui->actionExit, separatorAct );

		 separatorAct->setVisible(true);
	 }
	 else
	 {
		 separatorAct->setVisible(false);
	 }
}

void MainWindow::changeShader()
{
	const auto objectName = qobject_cast<QWidget *>(sender())->objectName();
	int index = 0;

	if (objectName == "toolButton_Lightmapped") {
		index = ui->comboBox_shader->findText("LightmappedGeneric", Qt::MatchFixedString);
	} else if (objectName == "toolButton_WorldVertex") {
		index = ui->comboBox_shader->findText("WorldVertexTransition", Qt::MatchFixedString);
	} else if (objectName == "toolButton_VertexLit") {
		index = ui->comboBox_shader->findText("VertexLitGeneric", Qt::MatchFixedString);
	} else if (objectName == "toolButton_Unlit") {
	index = ui->comboBox_shader->findText("UnlitGeneric", Qt::MatchFixedString);
	}

	int currentIndex = ui->comboBox_shader->currentIndex();
	if (index != currentIndex) {
		ui->comboBox_shader->setCurrentIndex(index);
		shaderChanged();
		mChildWidgetChanged = true;
		updateWindowTitle();
	}
}

void MainWindow::browseVTF()
{
	const auto name = qobject_cast<QWidget *>(sender())->objectName();

	if (name == "toolButton_diffuse")
		processVtf( "preview_basetexture1", "", ui->lineEdit_diffuse );

	else if (name == "toolButton_bumpmap" )
		processVtf( "preview_bumpmap1", "", ui->lineEdit_bumpmap );

	else if (name == "toolButton_diffuse2" )
		processVtf( "preview_basetexture2", "", ui->lineEdit_diffuse2 );

	else if (name == "toolButton_bumpmap2" )
		processVtf( "preview_bumpmap2", "", ui->lineEdit_bumpmap2 );

	else if (name == "toolButton_diffuse3" )
		processVtf( "preview_basetexture3", "", ui->lineEdit_diffuse3 );

	else if (name == "toolButton_diffuse4" )
		processVtf( "preview_basetexture4", "", ui->lineEdit_diffuse4 );

	else if (name == "toolButton_detail" )
		processVtf( "preview_detail", "", ui->lineEdit_detail );

	else if (name == "toolButton_detail2" )
		processVtf( "", "", ui->lineEdit_detail2 );

	else if (name == "toolButton_refractNormalMap" )
		processVtf( "preview_bumpmap1", "", ui->lineEdit_refractNormalMap );
	else if (name == "toolButton_refractNormalMap2" )
		processVtf( "preview_bumpmap2", "", ui->lineEdit_refractNormalMap2 );
	else if (name == "toolButton_refractTexture" )
		processVtf( "", "", ui->lineEdit_refractTexture );

	else if (name == "toolButton_waterNormalMap" )
		processVtf( "preview_bumpmap1", "", ui->lineEdit_waterNormalMap );

	else if (name == "toolButton_unlitTwoTextureDiffuse" )
		processVtf( "preview_basetexture1", "", ui->lineEdit_unlitTwoTextureDiffuse );
	else if (name == "toolButton_unlitTwoTextureDiffuse2" )
		processVtf( "preview_basetexture2", "", ui->lineEdit_unlitTwoTextureDiffuse2 );

	else if (name == "toolButton_blendmodulate" )
		processVtf( "preview_blendmod", "", ui->lineEdit_blendmodulate );
	else if (name == "toolButton_lightWarp" )
		processVtf( "", "", ui->lineEdit_lightWarp );

	else if (name == "toolButton_envmap" )
		processVtf( "", "", ui->lineEdit_envmap );
	else if (name == "toolButton_specmap" )
		processVtf( "preview_envmap1", "", ui->lineEdit_specmap );

	else if (name == "toolButton_exponentTexture" )
		processVtf( "preview_exponent", "", ui->lineEdit_exponentTexture );

	else if (name == "toolButton_maskTexture" )
		processVtf( "", "", ui->lineEdit_maskTexture );

	else if (name == "toolButton_flowMap" )
		processVtf( "", "", ui->lineEdit_flowMap );
	else if (name == "toolButton_noiseTexture" )
		processVtf( "", "", ui->lineEdit_noiseTexture );

	else if (name == "toolButton_toolTexture" )
		processVtf( "", "", ui->lineEdit_toolTexture );

	else if (name == "toolButton_bump2" )
		processVtf( "preview_bumpmap2", "", ui->lineEdit_bump2 );

	else if (name == "toolButton_waterReflectTexture" )
		processVtf( "", "", ui->lineEdit_waterReflectTexture );

	else if (name == "toolButton_decal" )
		processVtf( "", "", ui->lineEdit_decal );

	else if (name == "toolButton_phongWarp" )
		processVtf( "", "", ui->lineEdit_phongWarp );

	else if (name == "toolButton_bumpmapAlpha" )
		processVtf( "", "", ui->lineEdit_bumpmapAlpha );

	else if (name == "toolButton_diffuseAlpha" )
		processVtf( "", "", ui->lineEdit_diffuseAlpha );

	else if (name == "toolButton_tintMask" )
		processVtf( "", "", ui->lineEdit_tintMask );

	else if (name == "toolButton_specmap2" )
		processVtf( "", "", ui->lineEdit_specmap2 );

	else if (name == "toolButton_emissiveBlendTexture" )
		processVtf( "", "", ui->lineEdit_emissiveBlendTexture );

	else if (name == "toolButton_emissiveBlendBaseTexture" )
		processVtf( "", "", ui->lineEdit_emissiveBlendBaseTexture );

	else if (name == "toolButton_emissiveBlendFlowTexture" )
		processVtf( "", "", ui->lineEdit_emissiveBlendFlowTexture );
}

QString MainWindow::launchBrowseVtfDialog(QLineEdit* lineEdit)
{
	const QString lastTextureDirectory = QDir::toNativeSeparators(
		mIniSettings->value("lastTextureBrowseDir", "").toString());

	QString dir;
	if (lastTextureDirectory.startsWith(QDir::toNativeSeparators(currentGameMaterialDir()), Qt::CaseInsensitive )) {
		dir = lastTextureDirectory;
	} else {
		if (QDir(currentGameMaterialDir() + "/" + lineEdit->text() + ".vtf").exists()) {
			dir = currentGameMaterialDir() + "/" + lineEdit->text();
		} else {
			dir = currentGameMaterialDir();
		}
	}
	const auto tex = 
		tr("Textures (*.vtf *.bmp *.dds *.gif *.jpg *.png *.tga)");
	return QFileDialog::getOpenFileName(
		this, tr("Open Valve Texture File"), dir, tex);
}

void MainWindow::processVtf(const QString& objectName,
	const QString& textureFileName, QLineEdit* lineEdit)
{
	QString fileName;

	if (lineEdit == ui->lineEdit_bumpmapAlpha || lineEdit == ui->lineEdit_diffuseAlpha) {
		if (textureFileName.isEmpty()) {

			QString dir = QDir::toNativeSeparators(
						mIniSettings->value("lastTextureBrowseDir", "").toString());
			const auto tex = tr("Textures (*.bmp *.dds *.gif *.jpg *.png *.tga)");
			fileName = QFileDialog::getOpenFileName(
						this, tr("Open Texture File"), dir, tex);
			if (fileName.isEmpty())
				return;

		} else if (textureFileName.section(".", -1) != "vtf") {
			fileName = textureFileName;
		} else {
			return;
		}


		lineEdit->setText(fileName);
		clearLineEditAction(lineEdit);

		QAction *clear = lineEdit->addAction(QIcon(":/icons/clear"), QLineEdit::TrailingPosition);
		clear->setToolTip("Clear");
		connect (clear, SIGNAL(triggered()), SLOT(clearLineEdit()));

		if(mVMTLoaded) {

			if (lineEdit == ui->lineEdit_bumpmapAlpha)
				ui->lineEdit_bumpmap->actions()[0]->trigger();
			else if (lineEdit == ui->lineEdit_diffuseAlpha)
				ui->lineEdit_diffuse->actions()[0]->trigger();

		} else if (lineEdit == ui->lineEdit_bumpmapAlpha) {
			if (combineMaps(ui->lineEdit_bumpmap, ui->lineEdit_bumpmapAlpha)) {
				InfoReconvert("Converting \"bumpmap_alpha_combine.png\"...");
				QString convertedFile = QDir::currentPath() + "/Cache/bumpmap_alpha_combine.png";
				QString mipmapFilter = outputParameters(2, false);
				ConversionThread* conversionThread = new ConversionThread(this);
				conversionThread->fileName = convertedFile;
				conversionThread->newFileName = "lineEdit_bumpmap_" + texturesToCopy.value(ui->lineEdit_bumpmap) + ".vtf";
				conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\" " + mipmapFilter;
				conversionThread->start();
			}

		} else if (lineEdit == ui->lineEdit_diffuseAlpha) {
			if (combineMaps(ui->lineEdit_diffuse, ui->lineEdit_diffuseAlpha)) {
				InfoReconvert("Converting \"diffuse_alpha_combine.png\"...");
				QString convertedFile = QDir::currentPath() + "/Cache/diffuse_alpha_combine.png";
				QString mipmapFilter = outputParameters(1, false);
				ConversionThread* conversionThread = new ConversionThread(this);
				conversionThread->fileName = convertedFile;
				conversionThread->newFileName = "lineEdit_diffuse_" + texturesToCopy.value(ui->lineEdit_diffuse) + ".vtf";
				conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\" " + mipmapFilter;
				conversionThread->start();
			}
		}

		return;

	}

	if (textureFileName.isEmpty()) {
		fileName = launchBrowseVtfDialog(lineEdit);
		if (fileName.isEmpty())
			return;
	} else {
		fileName = textureFileName;
	}

	clearLineEditAction(lineEdit);

	bool updateLastTextureDirectory = true;
	const QString fileType = fileName.right( fileName.size() - fileName.lastIndexOf(".") );

	if (lineEdit == ui->lineEdit_bumpmap ) {
		ui->label_bumpmapAlpha->setVisible(false);
		ui->lineEdit_bumpmapAlpha->setVisible(false);
		ui->toolButton_bumpmapAlpha->setVisible(false);
	} else if (lineEdit == ui->lineEdit_diffuse ) {
		ui->label_diffuseAlpha->setVisible(false);
		ui->lineEdit_diffuseAlpha->setVisible(false);
		ui->toolButton_diffuseAlpha->setVisible(false);
	}

	if( fileName.startsWith(currentGameMaterialDir(), Qt::CaseInsensitive) &&
			fileType.toLower() == ".vtf" ) {

		lineEdit->setEnabled(true);
		texturesToCopy.remove(lineEdit);
		goto updateLineEdit;

	} else {

		lineEdit->setEnabled(true);
		texturesToCopy.remove(lineEdit);

		updateLastTextureDirectory = false;

		QString nameWithExtension( fileName.right( fileName.size() - fileName.lastIndexOf("/") ));

		if( fileType.toLower() == ".vtf" ) {

			if( mVMTLoaded ) {

				QAction *reconvert = lineEdit->addAction(QIcon(":/icons/reconvert"), QLineEdit::TrailingPosition);
				lineEdit->setToolTip(fileName);
				connect(reconvert, SIGNAL(triggered()), SLOT(reconvertTexture()));

				QString dir = QDir::toNativeSeparators(mIniSettings->value("lastSaveAsDir").toString());
				QString fullNewName = dir + nameWithExtension;

				if (QFile::exists(fullNewName)) {
					MsgBox msgBox(this);
					msgBox.setWindowTitle("File already exists!");
					QPushButton* overwriteButton = msgBox.addButton( "Overwrite", QMessageBox::YesRole );
					msgBox.addButton( QMessageBox::Cancel );
					msgBox.setDefaultButton( overwriteButton );
					msgBox.setIconPixmap(QPixmap(":/icons/info_warning"));;

					msgBox.setText( QDir::toNativeSeparators(nameWithExtension) + " already exists. Do you want to overwrite it?"  );

					if(  msgBox.exec() != QMessageBox::Cancel ) {

						if( QFile::remove(fullNewName) ) {

							if( QFile::copy(fileName, fullNewName) ) {

								Info( "File \"" + fileName + "\" successfully copied");

								fileName = fullNewName;

								goto updateLineEdit;

							} else {
								Error("\"" + fileName + "\" could not be copied to: \"" + fullNewName + "\"")
							}

						} else {
							Error("\"" + fileName + "\" could not be deleted!")
						}
					}

				} else {

					if( QFile::copy(fileName, fullNewName) ) {

						fileName = fullNewName;

						Info( "File \"" + fileName + "\" successfully copied");

						goto updateLineEdit;

					} else {
						Error("\"" + fileName + "\" could not be copied to: \"" + fullNewName + "\"")
					}
				}

			//VMT not saved
			} else {

				lineEdit->setText(fileName.right( fileName.length() - fileName.lastIndexOf('/', fileName.lastIndexOf('/') - 1) ));
				QString tempName = QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + lineEdit->objectName() + "_" + QDir::toNativeSeparators(fileName).section("\\", -1);
				QFile::copy(fileName, tempName);
				texturesToCopy.insert(lineEdit, QDir::toNativeSeparators(fileName).section("\\", -1).section(".", 0, 0) );
				lineEdit->setDisabled(true);

				QAction *reconvert = lineEdit->addAction(QIcon(":/icons/reconvert"), QLineEdit::TrailingPosition);
				lineEdit->setToolTip(fileName);
				connect(reconvert, SIGNAL(triggered()), SLOT(reconvertTexture()));

				previewTexture( objectName, fileName.section(".", 0, -2) + ".", true, false, false, false, true );
			}

		} else {

			// Image files, fileType != ".vtf"
			bool noAlpha = true;
			int type = 0;

			if( lineEdit == ui->lineEdit_diffuse ) {
				type = 1;
				if (ui->checkBox_basealpha->isChecked() ||
					ui->groupBox_selfIllumination->isVisible() ||
					ui->checkBox_alphaTest->isChecked() ||
					ui->checkBox_transparent->isChecked() ||
					ui->checkBox_blendTint->isChecked() ||
					ui->checkBox_phongBaseAlpha->isChecked() ||
					ui->checkBox_exponentBaseAlpha->isChecked() )
					noAlpha = false;

				ui->label_diffuseAlpha->setVisible(true);
				ui->lineEdit_diffuseAlpha->setVisible(true);
				ui->toolButton_diffuseAlpha->setVisible(true);
			}
			else if( lineEdit == ui->lineEdit_bumpmap ) {
				type = 2;
				if (ui->checkBox_normalalpha->isChecked() ||
					ui->groupBox_phong->isVisible() ||
					ui->checkBox_phongNormalAlpha->isChecked() )
					noAlpha = false;

				ui->label_bumpmapAlpha->setVisible(true);
				ui->lineEdit_bumpmapAlpha->setVisible(true);
				ui->toolButton_bumpmapAlpha->setVisible(true);
			}
			else if( lineEdit == ui->lineEdit_bumpmap2 ) {
				type = 2;
				if (ui->checkBox_normalalpha->isChecked() ||
					ui->groupBox_phong->isVisible() ||
					ui->checkBox_phongNormalAlpha->isChecked() )
					noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_diffuse2 ) {
				if (ui->checkBox_basealpha->isChecked() )
					noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_diffuse3 ) {
				if (ui->checkBox_basealpha->isChecked() )
					noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_diffuse4 ) {
				if (ui->checkBox_basealpha->isChecked() )
					noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_unlitTwoTextureDiffuse ) {
				noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_unlitTwoTextureDiffuse2 ) {
				noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_specmap ) {
				type = 3;
				if (ui->checkBox_envmapAlpha->isChecked() )
					noAlpha = false;
			}
			else if( lineEdit == ui->lineEdit_decal )
				noAlpha = false;

			else if( lineEdit == ui->lineEdit_exponentTexture )
				type = 4;

			else if( lineEdit == ui->lineEdit_tintMask )
				type = 8;

			if( texturesToCopy.contains(lineEdit) ) {

				QString toDelete = "Cache\\Move\\" + lineEdit->objectName() + "_" + texturesToCopy.value(lineEdit);

				if( !QFile::remove(toDelete) )
					Error("Error while removing \"" + toDelete + "\"")

				texturesToCopy.remove(lineEdit);
			}

			if ( lineEdit == ui->lineEdit_bumpmap ||
				 lineEdit == ui->lineEdit_bumpmap2 ||
				 lineEdit == ui->lineEdit_bump2 ||
				 lineEdit == ui->lineEdit_refractNormalMap ||
				 lineEdit == ui->lineEdit_refractNormalMap2 ||
				 lineEdit == ui->lineEdit_waterNormalMap )
				type = 2;

			QString mipmapFilter = outputParameters(type, noAlpha);

			if(mVMTLoaded) {

				QString newFile = removeSuffix(fileName.section("/", -1).section(".", 0, 0), type);
				QString dir = QDir::toNativeSeparators(mIniSettings->value("lastSaveAsDir").toString() + "/");
				QString relativeFilePath = QDir( currentGameMaterialDir() ).relativeFilePath(dir + newFile);

				if( QFile::exists(dir + newFile + ".vtf") ) {
					MsgBox msgBox(this);
					msgBox.setWindowTitle("File already exists!");
					QPushButton* overwriteButton = msgBox.addButton( "Overwrite", QMessageBox::YesRole );
					msgBox.addButton( QMessageBox::Cancel );
					msgBox.setDefaultButton( overwriteButton );
					msgBox.setIconPixmap(QPixmap(":/icons/info_warning"));;

					msgBox.setText( QDir::toNativeSeparators(newFile + ".vtf") + " already exists. Do you want to overwrite it?"  );

					if(  msgBox.exec() != QMessageBox::Cancel ) {

						if( !QFile::remove( dir + newFile + ".vtf" ) ) {
							Error( "Error removing \"" + dir + newFile + ".vtf\"" );
							return;
						}
					} else {
						return;
					}
				}

				InfoReconvert("Converting \"" + fileName.replace("\\", "/").section("/", -1) + "\"...");

				QAction *reconvert = lineEdit->addAction(QIcon(":/icons/reconvert"), QLineEdit::TrailingPosition);
				lineEdit->setToolTip(fileName);
				connect(reconvert, SIGNAL(triggered()), SLOT(reconvertTexture()));

				QAction *reconvertHalf = lineEdit->addAction(QIcon(":/icons/reconvert_half"), QLineEdit::TrailingPosition);
				connect(reconvertHalf, SIGNAL(triggered()), SLOT(reconvertTextureHalf()));

				QAction *openConvertDialog = lineEdit->addAction(QIcon(":/icons/reconvert_dialog"), QLineEdit::TrailingPosition);
				connect(openConvertDialog, SIGNAL(triggered()), SLOT(openReconvertDialogAction()));

				mIniPaths->setValue(relativeFilePath, fileName);

				ConversionThread* conversionThread = new ConversionThread(this);
					conversionThread->fileName = fileName;
					conversionThread->objectName = objectName;
					conversionThread->relativeFilePath = relativeFilePath;
					conversionThread->newFileName = "";
					conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\" " + mipmapFilter;
					conversionThread->moveFile = true;
					conversionThread->newFile = newFile;
					conversionThread->newFileDir = dir;
					conversionThread->start();

				lineEdit->setText(relativeFilePath);


			} else {

				QString outputFile = fileName.right( fileName.length() - fileName.lastIndexOf('/') - 1 );
				outputFile = outputFile.left( outputFile.indexOf('.') );

				texturesToCopy.insert(lineEdit, outputFile);

				lineEdit->setText(fileName.right( fileName.length() - fileName.lastIndexOf('/', fileName.lastIndexOf('/') - 1) ));
				lineEdit->setDisabled(true);

				QAction *reconvert = lineEdit->addAction(QIcon(":/icons/reconvert"), QLineEdit::TrailingPosition);
				lineEdit->setToolTip(fileName);
				connect(reconvert, SIGNAL(triggered()), SLOT(reconvertTexture()));

				QAction *reconvertHalf = lineEdit->addAction(QIcon(":/icons/reconvert_half"), QLineEdit::TrailingPosition);
				connect(reconvertHalf, SIGNAL(triggered()), SLOT(reconvertTextureHalf()));

				QAction *openConvertDialog = lineEdit->addAction(QIcon(":/icons/reconvert_dialog"), QLineEdit::TrailingPosition);
				connect(openConvertDialog, SIGNAL(triggered()), SLOT(openReconvertDialogAction()));

				InfoReconvert("Converting \"" + fileName.replace("\\", "/").section("/", -1) + "\"...");

				ConversionThread* conversionThread = new ConversionThread(this);
					conversionThread->fileName = fileName;
					conversionThread->newFileName = lineEdit->objectName() + "_" + texturesToCopy.value(lineEdit) + ".vtf";
					conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\" " + mipmapFilter;
					conversionThread->start();

				fileName.chop(4);

				QString fromFile = fileName.left( fileName.lastIndexOf("/") ) + nameWithExtension;
				QString toFile = QDir::currentPath() + "/Cache/" + objectName;

				if( QFile::exists(toFile + ".png") )
					QFile::remove(toFile + ".png");
				if( QFile::exists(toFile + ".tga") )
					QFile::remove(toFile + ".tga");

				QFile::copy(fromFile, toFile + fileType);

				previewTexture(objectName);
			}
		}
	}

	return;

	//----------------------------------------------------------------------------------------//

	updateLineEdit:

	fileName = fileName.left( fileName.size() - 4 );

	if(updateLastTextureDirectory) {
		mIniSettings->setValue("lastTextureBrowseDir",
			QDir::toNativeSeparators(fileName).left( QDir::toNativeSeparators(fileName).lastIndexOf("\\")));
	}

	QString relativeFilePath = QDir( currentGameMaterialDir() ).relativeFilePath(fileName);
	lineEdit->setText(relativeFilePath);
	createReconvertAction(lineEdit, relativeFilePath);

	previewTexture( objectName, relativeFilePath, true, false, false, false );
}

void MainWindow::loadExternalVmt()
{
	QFile tmp( currentGameMaterialDir() + "/" + ui->lineEdit_bottomMaterial->text() + ".vmt" );

	if( tmp.exists() ) {

		QProcess* process = new QProcess();
		process->start( "\"" + QCoreApplication::applicationFilePath() + "\" \"" + currentGameMaterialDir() + "/" + ui->lineEdit_bottomMaterial->text() + ".vmt\"" );
	}
}

void MainWindow::browseToGameDirectory() {

	QString currGame = getCurrentGame();

	if(currGame != "") {

		QString directory = mAvailableGames[currGame];

#ifdef Q_OS_WIN
		ShellExecuteA(NULL, 0, "explorer.exe", directory.replace("/", "\\").toStdString().c_str(), NULL, SW_NORMAL);
#endif
	}
}

void MainWindow::browseVMT()
{
	QString fileName;
	int ret = QMessageBox::Retry;

	do
	{
		QString dir;

		if(mVMTLoaded) {

			if( getCurrentGame() == "" )
				dir = vmtParser->lastVMTFile().fileName;
			else
				dir = vmtParser->lastVMTFile().directory + "/" + vmtParser->lastVMTFile().fileName;

		} else {

			dir = currentGameMaterialDir();
		}


		fileName = QFileDialog::getOpenFileName( this, tr("Open Valve Material"), dir, tr("VMT (*.VMT)") );
		if( !fileName.isEmpty() )
		{
			if( fileName.startsWith( currentGameMaterialDir(), Qt::CaseInsensitive) )
			{
				fileName = fileName.left( fileName.size() - 4 );

				QDir tmp( currentGameMaterialDir() );

				ui->lineEdit_bottomMaterial->setText( tmp.relativeFilePath(fileName) );

				break; // Needed
			}
			else
			{
				ret = MsgBox::warning( this, "Unsupported directory specified!",
												"Game directory was not found\n\nGame: " + getCurrentGame() +
												"\nDirectory: " + currentGameMaterialDir().left(currentGameMaterialDir().size() - 10),
												QMessageBox::Retry | QMessageBox::Abort, QMessageBox::Retry);
			}
		}
		else
		{
			break;
		}

	} while( ret == QMessageBox::Retry );
}

void MainWindow::alphaTestReferenceChanged()
{
	glWidget_diffuse1->setAlphaTestReference( ui->doubleSpinBox_alphaTestRef->cleanText().toDouble() );
}

void MainWindow::detailTextureChanged(QString text)
{
	const auto lineEdit = qobject_cast<QWidget *>(sender());
	if (lineEdit == ui->lineEdit_detail)
		detailtexture::processDetailTextureChange(text, ui);
	else if (lineEdit == ui->lineEdit_detail2)
		detailtexture::processDetailTextureChange(text, ui, true);
}

void MainWindow::requestedCubemap( bool enabled )
{
	ui->lineEdit_envmap->setDisabled( enabled );
	ui->toolButton_envmap->setEnabled( !(enabled) && (getCurrentGame() != "") );
}

void MainWindow::changeColor(QToolButton* colorField , TintSlider *slider) {

#ifdef Q_OS_WIN
	QString bytes = mIniSettings->value( "customColors" ).toByteArray();

	// Setting white as the default preview color when settings.ini doesn't exist or the key CustomColors wasn't created yet
	if( bytes.isEmpty() )
		bytes = "16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215";

	QStringList values = bytes.split(';', QString::SkipEmptyParts);

	COLORREF acrCustClr[16];

	for(int i = 0; i < values.size(); ++i)
		acrCustClr[i] = values.at(i).toInt();

	CHOOSECOLOR cc;

	static DWORD rgbCurrent;        // initial color selection

	//----------------------------------------------------------------------------------------////

	QColor c = utils::getBG(colorField);

	rgbCurrent = RGB( c.red(), c.green(), c.blue() );

	//----------------------------------------------------------------------------------------////

	ZeroMemory( &cc, sizeof(cc) );
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = (HWND)this->winId();
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if( ChooseColor(&cc) != 0 )
	{
		rgbCurrent = cc.rgbResult;

		QColor color;
		color.setRed(   GetRValue(rgbCurrent) );
		color.setGreen( GetGValue(rgbCurrent) );
		color.setBlue(  GetBValue(rgbCurrent) );

		slider->initialize(colorField, color);

		colorField->setStyleSheet( QString("background-color: rgb(%1, %2, %3)")
								   .arg( QString::number( qMin( 255, color.red()   )))
								   .arg( QString::number( qMin( 255, color.green() )))
								   .arg( QString::number( qMin( 255, color.blue()  ))));

		//----------------------------------------------------------------------------------------//

		bytes = QByteArray();

		for( uint i = 0; i < 16; ++i )
		{
			bytes.append( Str( acrCustClr[i] ) + ";" );
		}

		mIniSettings->setValue( "customColors", bytes );
		widgetChanged();
	}
#else
	Q_UNUSED(colorField)
	Q_UNUSED(slider)
#endif
}

void MainWindow::changeColor(QToolButton* colorField)
{
#ifdef Q_OS_WIN
	QString bytes = mIniSettings->value( "customColors" ).toByteArray();

	// Setting white as the default preview color when settings.ini doesn't exist or the key CustomColors wasn't created yet
	if( bytes.isEmpty() )
		bytes = "16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215;16777215";

	QStringList values = bytes.split(';', QString::SkipEmptyParts);

	COLORREF acrCustClr[16];

	for(int i = 0; i < values.size(); ++i)
		acrCustClr[i] = values.at(i).toInt();

	CHOOSECOLOR cc;

	static DWORD rgbCurrent;        // initial color selection

	//----------------------------------------------------------------------------------------////

	QColor c = utils::getBG(colorField);

	rgbCurrent = RGB( c.red(), c.green(), c.blue() );

	//----------------------------------------------------------------------------------------////

	ZeroMemory( &cc, sizeof(cc) );
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = (HWND)this->winId();
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if( ChooseColorW(&cc) != 0 )
	{
		rgbCurrent = cc.rgbResult;

		QColor color;
		color.setRed(   GetRValue(rgbCurrent) );
		color.setGreen( GetGValue(rgbCurrent) );
		color.setBlue(  GetBValue(rgbCurrent) );

		colorField->setStyleSheet( QString("background-color: rgb(%1, %2, %3)")
								   .arg( QString::number( qMin( 255, color.red()   )))
								   .arg( QString::number( qMin( 255, color.green() )))
								   .arg( QString::number( qMin( 255, color.blue()  ))));

		//----------------------------------------------------------------------------------------//

		bytes = QByteArray();

		for( uint i = 0; i < 16; ++i )
		{
			bytes.append( Str( acrCustClr[i] ) + ";" );
		}

		mIniSettings->setValue( "customColors", bytes );
		widgetChanged();
	}
#else
	Q_UNUSED(colorField)
#endif
}

void MainWindow::changedColor() {

	QWidget* caller = qobject_cast<QWidget *>( sender() );

	if( caller->objectName() == "toolButton_envmapTint" )
		changeColor(ui->toolButton_envmapTint);

	else if( caller->objectName() == "toolButton_phongTint" )
		changeColor(ui->toolButton_phongTint, ui->horizontalSlider_phongTint);

	else if( caller->objectName() == "toolButton_refractTint" )
		changeColor(ui->toolButton_refractTint);

	else if( caller->objectName() == "toolButton_refractionTint" )
		changeColor(ui->toolButton_refractionTint, ui->horizontalSlider_waterRefractColor);

	else if( caller->objectName() == "toolButton_reflectionTint" )
		changeColor(ui->toolButton_reflectionTint, ui->horizontalSlider_waterReflectColor);

	else if( caller->objectName() == "toolButton_fogTint" )
		changeColor(ui->toolButton_fogTint, ui->horizontalSlider_waterFogColor);

	else if( caller->objectName() == "toolButton_color1" ) {
		changeColor(ui->toolButton_color1);
		colorChanged();
	}
	else if( caller->objectName() == "toolButton_color2" ) {
		changeColor(ui->toolButton_color2);
		colorChanged();
	}
	else if( caller->objectName() == "toolButton_reflectivity" )
		changeColor(ui->toolButton_reflectivity);

	else if( caller->objectName() == "toolButton_reflectivity_2" )
		changeColor(ui->toolButton_reflectivity_2);

	else if( caller->objectName() == "toolButton_selfIllumTint" )
		changeColor(ui->toolButton_selfIllumTint);

	else if( caller->objectName() == "toolButton_phongAmount" )
		changeColor(ui->toolButton_phongAmount);

	else if( caller->objectName() == "toolButton_spec_amount2" )
		changeColor(ui->toolButton_spec_amount2);

	else if( caller->objectName() == "toolButton_layer1tint" )
		changeColor(ui->toolButton_layer1tint);

	else if( caller->objectName() == "toolButton_layer2tint" )
		changeColor(ui->toolButton_layer2tint);

	else if( caller->objectName() == "toolButton_layerBorderTint" )
		changeColor(ui->toolButton_layerBorderTint);

	else if( caller->objectName() == "toolButton_emissiveBlendTint" )
		changeColor(ui->toolButton_emissiveBlendTint);
}

void MainWindow::resetColor()
{
	// Clearing the lineedit is handled within Qt Designer signals

	QWidget* caller = qobject_cast<QWidget *>( sender() );

	if( caller->objectName() == "pushButton_envmapTint" )
	{
		ui->toolButton_envmapTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	}
	else if( caller->objectName() == "pushButton_phongTint" )
	{
		ui->toolButton_phongTint->setStyleSheet( "background-color: rgb(255, 255, 255)" );
	}
}

void MainWindow::modifiedLineEdit( QString text )
{
	if( mIsResetting )
		return;

	QWidget* caller = qobject_cast<QWidget *>( sender() );

	if( caller->objectName() == "lineEdit_diffuse" ) {

		if( ui->checkBox_basealpha->isChecked() && ui->groupBox_phong->isVisible() )
			previewTexture( 1, text );
		else if ( ui->checkBox_basealpha->isChecked() && !ui->groupBox_phong->isVisible())
			previewTexture( 0, text );
		if( ui->checkBox_exponentBaseAlpha->isChecked() && ui->groupBox_phong->isVisible() )
			previewTexture( 4, text );

	} else if( caller->objectName() == "lineEdit_bumpmap" ) {

		if( ui->checkBox_normalalpha->isChecked() )
			previewTexture( 1, text );
		if( ui->groupBox_phong->isVisible() )
			previewTexture( 4, text );

	} else if( caller->objectName() == "lineEdit_envmap" ) {

		if( text.isEmpty() )
		{
			ui->checkBox_cubemap->setEnabled(true);

			ui->label_saturation->setDisabled(true);
			ui->horizontalSlider_saturation->setDisabled(true);
			ui->doubleSpinBox_saturation->setDisabled(true);

			ui->label_contrast->setDisabled(true);
			ui->horizontalSlider_contrast->setDisabled(true);
			ui->doubleSpinBox_contrast->setDisabled(true);

			ui->label_fresnelReflection->setDisabled(true);
			ui->horizontalSlider_fresnelReflection->setDisabled(true);
			ui->doubleSpinBox_fresnelReflection->setDisabled(true);

			ui->label_envmapTint->setDisabled(true);
			ui->horizontalSlider_envmapTint->setDisabled(true);
			ui->doubleSpinBox_envmapTint->setDisabled(true);
			ui->toolButton_envmapTint->setDisabled(true);

			ui->label_specmap->setDisabled(true);
			ui->lineEdit_specmap->setDisabled(true);
			ui->toolButton_specmap->setDisabled(true);

			ui->label_specmap2->setDisabled(true);
			ui->lineEdit_specmap2->setDisabled(true);
			ui->toolButton_specmap2->setDisabled(true);


			ui->checkBox_basealpha->setDisabled(true);
			ui->checkBox_normalalpha->setDisabled(true);
			ui->checkBox_tintSpecMask->setDisabled(true);

			ui->label_lightinfluence->setDisabled(true);
			ui->horizontalSlider_envmapLight->setDisabled(true);
			ui->doubleSpinBox_envmapLight->setDisabled(true);
			ui->doubleSpinBox_envmapLightMin->setDisabled(true);
			ui->doubleSpinBox_envmapLightMax->setDisabled(true);

			ui->label_anisotropy->setDisabled(true);
			ui->horizontalSlider_envmapAniso->setDisabled(true);
			ui->doubleSpinBox_envmapAniso->setDisabled(true);
		}
		else
		{
			ui->checkBox_cubemap->setDisabled(true);

			ui->label_saturation->setEnabled(true);
			ui->horizontalSlider_saturation->setEnabled(true);
			ui->doubleSpinBox_saturation->setEnabled(true);

			ui->label_contrast->setEnabled(true);
			ui->horizontalSlider_contrast->setEnabled(true);
			ui->doubleSpinBox_contrast->setEnabled(true);

			ui->label_fresnelReflection->setEnabled(true);
			ui->horizontalSlider_fresnelReflection->setEnabled(true);
			ui->doubleSpinBox_fresnelReflection->setEnabled(true);

			ui->label_envmapTint->setEnabled(true);
			ui->horizontalSlider_envmapTint->setEnabled(true);
			ui->doubleSpinBox_envmapTint->setEnabled(true);
			ui->toolButton_envmapTint->setEnabled(true);

			ui->label_lightinfluence->setEnabled(true);
			ui->horizontalSlider_envmapLight->setEnabled(true);
			ui->doubleSpinBox_envmapLight->setEnabled(true);
			ui->doubleSpinBox_envmapLightMin->setEnabled(true);
			ui->doubleSpinBox_envmapLightMax->setEnabled(true);

			ui->label_anisotropy->setEnabled(true);
			ui->horizontalSlider_envmapAniso->setEnabled(true);
			ui->doubleSpinBox_envmapAniso->setEnabled(true);

			ui->label_specmap->setEnabled(true);
			ui->label_specmap2->setEnabled(true);

			if( !ui->lineEdit_specmap->text().isEmpty() )
			{
				ui->lineEdit_specmap->setEnabled(true);
				ui->lineEdit_specmap2->setEnabled(true);

				if( getCurrentGame() != "" )
					ui->toolButton_specmap->setEnabled(true);
					ui->toolButton_specmap2->setEnabled(true);

				ui->checkBox_basealpha->setDisabled(true);
				ui->checkBox_normalalpha->setDisabled(true);
				ui->checkBox_tintSpecMask->setDisabled(true);
			}
			else if( ui->checkBox_basealpha->isChecked() ||  ui->checkBox_phongBaseAlpha->isChecked() )
			{
				ui->lineEdit_specmap->setDisabled(true);
				ui->toolButton_specmap->setDisabled(true);

				ui->lineEdit_specmap2->setDisabled(true);
				ui->toolButton_specmap2->setDisabled(true);

				ui->checkBox_basealpha->setEnabled(true);
				ui->checkBox_normalalpha->setDisabled(true);
				ui->checkBox_tintSpecMask->setDisabled(true);

				ui->checkBox_phongNormalAlpha->setDisabled(true);
			}
			else if( ui->checkBox_normalalpha->isChecked() || ui->checkBox_phongNormalAlpha->isChecked() )
			{
				ui->lineEdit_specmap->setDisabled(true);
				ui->toolButton_specmap->setDisabled(true);

				ui->lineEdit_specmap2->setDisabled(true);
				ui->toolButton_specmap2->setDisabled(true);

				ui->checkBox_basealpha->setDisabled(true);
				ui->checkBox_normalalpha->setEnabled(true);
				ui->checkBox_tintSpecMask->setDisabled(true);

				ui->checkBox_phongBaseAlpha->setDisabled(true);
			}
			else
			{
				ui->lineEdit_specmap->setEnabled(true);
				ui->lineEdit_specmap2->setEnabled(true);

				if( getCurrentGame() != "" )
					ui->toolButton_specmap->setEnabled(true);
					ui->toolButton_specmap2->setEnabled(true);

				ui->checkBox_basealpha->setEnabled(true);
				ui->checkBox_normalalpha->setEnabled(true);

				ui->checkBox_phongBaseAlpha->setEnabled(true);
				ui->checkBox_phongNormalAlpha->setEnabled(true);
				ui->checkBox_tintSpecMask->setEnabled(true);
			}
		}
	}
	else if( caller->objectName() == "lineEdit_specmap" )
	{
		if( text.isEmpty() )
		{
			ui->checkBox_basealpha->setEnabled(true);
			ui->checkBox_normalalpha->setEnabled(true);

			previewTexture(3, "");
		}
		else
		{
			ui->checkBox_basealpha->setDisabled(true);
			ui->checkBox_normalalpha->setDisabled(true);

			previewTexture(3, text);
		}
	}
	else if( caller->objectName() == "lineEdit_exponentTexture" ) {

		bool enabled = ui->lineEdit_exponentTexture->text().isEmpty();

		ui->label_exponent->setEnabled(enabled);
		ui->spinBox_exponent->setEnabled(enabled);
		ui->horizontalSlider_exponent->setEnabled(enabled);
	}
}

void MainWindow::modifiedCheckBox( bool enabled )
{
	//if( mIsResetting )
	//	return;

	QWidget* caller = qobject_cast<QWidget *>( sender() );

	if( caller->objectName() == "checkBox_cubemap" )
	{
		ui->label_envmap->setDisabled(enabled);
		ui->lineEdit_envmap->setDisabled(enabled);
		ui->toolButton_envmap->setDisabled(enabled);

		if(enabled)
		{
			ui->label_saturation->setEnabled(true);
			ui->doubleSpinBox_saturation->setEnabled(true);
			ui->horizontalSlider_saturation->setEnabled(true);

			ui->label_contrast->setEnabled(true);
			ui->doubleSpinBox_contrast->setEnabled(true);
			ui->horizontalSlider_contrast->setEnabled(true);

			ui->checkBox_tintSpecMask->setEnabled(true);

			ui->label_fresnelReflection->setEnabled(true);
			ui->doubleSpinBox_fresnelReflection->setEnabled(true);
			ui->horizontalSlider_fresnelReflection->setEnabled(true);

			ui->label_envmapTint->setEnabled(true);
			ui->horizontalSlider_envmapTint->setEnabled(true);
			ui->doubleSpinBox_envmapTint->setEnabled(true);

			ui->toolButton_envmapTint->setEnabled(true);

			ui->label_specmap->setEnabled(true);
			ui->label_specmap2->setEnabled(true);

			ui->label_lightinfluence->setEnabled(true);
			ui->horizontalSlider_envmapLight->setEnabled(true);
			ui->doubleSpinBox_envmapLight->setEnabled(true);
			ui->doubleSpinBox_envmapLightMin->setEnabled(true);
			ui->doubleSpinBox_envmapLightMax->setEnabled(true);

			ui->label_anisotropy->setEnabled(true);
			ui->horizontalSlider_envmapAniso->setEnabled(true);
			ui->doubleSpinBox_envmapAniso->setEnabled(true);

			if( !ui->lineEdit_specmap->text().isEmpty() )
			{
				ui->lineEdit_specmap->setEnabled(true);
				ui->lineEdit_specmap2->setEnabled(true);

				if( getCurrentGame() != "" )
					ui->toolButton_specmap->setEnabled(true);
					ui->toolButton_specmap2->setEnabled(true);

				ui->checkBox_basealpha->setDisabled(true);
				ui->checkBox_normalalpha->setDisabled(true);
			}
			else if( ui->checkBox_basealpha->isChecked() )
			{
				ui->lineEdit_specmap->setDisabled(true);
				ui->toolButton_specmap->setDisabled(true);

				ui->label_specmap->setDisabled(true);
				ui->label_specmap2->setDisabled(true);

				ui->lineEdit_specmap2->setDisabled(true);
				ui->toolButton_specmap2->setDisabled(true);

				ui->checkBox_basealpha->setEnabled(true);
				ui->checkBox_normalalpha->setDisabled(true);
			}
			else if( ui->checkBox_normalalpha->isChecked() )
			{
				ui->lineEdit_specmap->setDisabled(true);
				ui->toolButton_specmap->setDisabled(true);

				ui->label_specmap->setDisabled(true);
				ui->label_specmap2->setDisabled(true);

				ui->lineEdit_specmap2->setDisabled(true);
				ui->toolButton_specmap2->setDisabled(true);

				ui->checkBox_basealpha->setDisabled(true);
				ui->checkBox_normalalpha->setEnabled(true);
			}
			else
			{
				ui->lineEdit_specmap->setEnabled(true);
				ui->lineEdit_specmap2->setEnabled(true);

				ui->label_specmap->setEnabled(true);
				ui->label_specmap2->setEnabled(true);

				if( getCurrentGame() != "" )
					ui->toolButton_specmap->setEnabled(true);
					ui->toolButton_specmap2->setEnabled(true);

				ui->checkBox_basealpha->setEnabled(true);
				ui->checkBox_normalalpha->setEnabled(true);
			}
		}
		else
		{
			if( ui->lineEdit_envmap->text().isEmpty() )
			{
				ui->label_saturation->setDisabled(true);
				ui->doubleSpinBox_saturation->setDisabled(true);
				ui->horizontalSlider_saturation->setDisabled(true);

				ui->label_contrast->setDisabled(true);
				ui->doubleSpinBox_contrast->setDisabled(true);
				ui->horizontalSlider_contrast->setDisabled(true);

				ui->checkBox_tintSpecMask->setDisabled(true);

				ui->label_fresnelReflection->setDisabled(true);
				ui->doubleSpinBox_fresnelReflection->setDisabled(true);
				ui->horizontalSlider_fresnelReflection->setDisabled(true);

				ui->label_envmapTint->setDisabled(true);
				ui->horizontalSlider_envmapTint->setDisabled(true);
				ui->doubleSpinBox_envmapTint->setDisabled(true);
				ui->toolButton_envmapTint->setDisabled(true);

				ui->label_specmap->setDisabled(true);
				ui->lineEdit_specmap->setDisabled(true);
				ui->toolButton_specmap->setDisabled(true);

				ui->label_specmap2->setDisabled(true);
				ui->lineEdit_specmap2->setDisabled(true);
				ui->toolButton_specmap2->setDisabled(true);

				ui->checkBox_basealpha->setDisabled(true);
				ui->checkBox_normalalpha->setDisabled(true);

				ui->label_lightinfluence->setDisabled(true);
				ui->horizontalSlider_envmapLight->setDisabled(true);
				ui->doubleSpinBox_envmapLight->setDisabled(true);
				ui->doubleSpinBox_envmapLightMin->setDisabled(true);
				ui->doubleSpinBox_envmapLightMax->setDisabled(true);

				ui->label_anisotropy->setDisabled(true);
				ui->horizontalSlider_envmapAniso->setDisabled(true);
				ui->doubleSpinBox_envmapAniso->setDisabled(true);
			}
		}
	}
	else if( caller->objectName() == "checkBox_basealpha" )
	{
		if(enabled)
		{
			ui->lineEdit_specmap->setDisabled(true);
			ui->toolButton_specmap->setDisabled(true);

			ui->label_specmap->setDisabled(true);
			ui->label_specmap2->setDisabled(true);

			ui->lineEdit_specmap2->setDisabled(true);
			ui->toolButton_specmap2->setDisabled(true);

			ui->checkBox_normalalpha->setDisabled(true);

			ui->checkBox_phongNormalAlpha->setDisabled(true);

			if ( ui->groupBox_phong->isVisible() )
				previewTexture( 1, ui->lineEdit_diffuse->text() );
			else if  ( !ui->groupBox_phong->isVisible() ) {
				previewTexture( 0, ui->lineEdit_diffuse->text() );
			}

		}
		else
		{
			ui->lineEdit_specmap->setEnabled(true);
			ui->checkBox_normalalpha->setEnabled(true);

			ui->label_specmap->setEnabled(true);
			ui->label_specmap2->setEnabled(true);

			ui->lineEdit_specmap2->setEnabled(true);

			ui->checkBox_phongNormalAlpha->setEnabled(true);

			ui->toolButton_specmap->setEnabled( getCurrentGame() != "" );
			ui->toolButton_specmap2->setEnabled( getCurrentGame() != "" );

			previewTexture( 0, "" );
		}
	}
	else if( caller->objectName() == "checkBox_normalalpha" )
	{
		if(enabled)
		{
			ui->lineEdit_specmap->setDisabled(true);
			ui->toolButton_specmap->setDisabled(true);
			ui->lineEdit_specmap2->setDisabled(true);
			ui->toolButton_specmap2->setDisabled(true);

			ui->label_specmap->setDisabled(true);
			ui->label_specmap2->setDisabled(true);

			ui->checkBox_basealpha->setDisabled(true);

			ui->checkBox_phongBaseAlpha->setDisabled(true);

			previewTexture( 1, ui->lineEdit_bumpmap->text() );
		}
		else
		{
			ui->lineEdit_specmap->setEnabled(true);
			ui->checkBox_basealpha->setEnabled(true);
			ui->lineEdit_specmap2->setEnabled(true);

			ui->label_specmap->setEnabled(true);
			ui->label_specmap2->setEnabled(true);

			ui->checkBox_phongBaseAlpha->setEnabled(true);

			ui->toolButton_specmap->setEnabled( getCurrentGame() != "" );
			ui->toolButton_specmap2->setEnabled( getCurrentGame() != "" );

			previewTexture( 0, "" );
		}
	}
	else if( caller->objectName() == "checkBox_exponentBaseAlpha" )
	{
		if(enabled)
		{
			previewTexture( 4, ui->lineEdit_diffuse->text() );
		}
		else
		{
			previewTexture( 4, ui->lineEdit_bumpmap->text() );
		}
	}
	else if( caller->objectName() == "checkBox_expensive" )
	{
		if(enabled)
		{
			ui->checkBox_cheap->setDisabled(true);
		}
		else
		{
			ui->checkBox_cheap->setEnabled(true);
		}
	}
	else if( caller->objectName() == "checkBox_cheap" )
	{
		if(enabled)
		{
			ui->checkBox_expensive->setDisabled(true);
		}
		else
		{
			ui->checkBox_expensive->setEnabled(true);
		}
	}
	else if( caller->objectName() == "checkBox_waterBottom" )
	{
		if(enabled)
		{
			ui->action_waterReflection->setDisabled(true);
			ui->action_waterReflection->setChecked(false);

			ui->groupBox_waterReflection->hide();

			ui->toolButton_bottomMaterial->setDisabled(true);
		}
		else
		{
			ui->action_waterReflection->setEnabled(true);

			if( getCurrentGame() != "" )
				ui->toolButton_bottomMaterial->setEnabled(true);
		}
	}
	else if( caller->objectName() == "checkBox_halfLambert" )
	{
		ui->label_lightWarp->setDisabled(enabled);
		ui->lineEdit_lightWarp->setDisabled(enabled);
		ui->toolButton_lightWarp->setDisabled(enabled);
	}
	else if( caller->objectName() == "checkBox_realTimeReflection" ) {

		ui->checkBox_reflectEntities->setEnabled(enabled);
		ui->checkBox_skybox->setEnabled(enabled);
		ui->checkBox_reflect2dskybox->setEnabled(enabled);
		ui->checkBox_reflect3dskybox->setEnabled(enabled);
		ui->checkBox_reflectMarkedEntities->setEnabled(enabled);

		ui->lineEdit_waterReflectTexture->setDisabled(enabled);
		ui->toolButton_waterReflectTexture->setDisabled(enabled);

	} else if( caller->objectName() == "checkBox_transparent" ) {

		glWidget_diffuse1->setAlphaVisible(enabled);

	} else if( caller->objectName() == "checkBox_alphaTest" ) {

		glWidget_diffuse1->setEnableAlphaTest(enabled);
	}
}

bool MainWindow::transformsModified( uint index )
{
	if( index == 0 )
	{
		return( ui->doubleSpinBox_bt_centerX->value() != 0.5    ||
				ui->doubleSpinBox_bt_centerY->value() != 0.5    ||
				ui->doubleSpinBox_bt_scaleX->value() != 1.0     ||
				ui->doubleSpinBox_bt_scaleY->value() != 1.0     ||
				ui->doubleSpinBox_bt_angle->value() != 0.0      ||
				ui->doubleSpinBox_bt_translateX->value() != 0.0 ||
				ui->doubleSpinBox_bt_translateY->value() != 0.0 );
	}
	else if( index == 1 )
	{
		return( ui->doubleSpinBox_bm_centerX->value() != 0.5    ||
				ui->doubleSpinBox_bm_centerY->value() != 0.5    ||
				ui->doubleSpinBox_bm_scaleX->value() != 1.0     ||
				ui->doubleSpinBox_bm_scaleY->value() != 1.0     ||
				ui->doubleSpinBox_bm_angle->value() != 0.0      ||
				ui->doubleSpinBox_bm_translateX->value() != 0.0 ||
				ui->doubleSpinBox_bm_translateY->value() != 0.0 );
	}
	else if( index == 2 )
	{
		return( ui->doubleSpinBox_bm_centerX_2->value() != 0.5    ||
				ui->doubleSpinBox_bm_centerY_2->value() != 0.5    ||
				ui->doubleSpinBox_bm_scaleX_2->value() != 1.0     ||
				ui->doubleSpinBox_bm_scaleY_2->value() != 1.0     ||
				ui->doubleSpinBox_bm_angle_2->value() != 0.0      ||
				ui->doubleSpinBox_bm_translateX_2->value() != 0.0 ||
				ui->doubleSpinBox_bm_translateY_2->value() != 0.0 );
	}

	return false;
}

void MainWindow::addGLWidget(
	const QString& textureOverlay, QVBoxLayout* layout,
	const QString& widgetObjectName)
{
	GLWidget* glWidget = new GLWidget(textureOverlay, widgetObjectName,
			glWidgets.length() > 0 ? glWidgets.at(0) : NULL);
	glWidgets.append(glWidget);

	layout->addWidget(glWidget);
	glWidget->setVisible(false);
}

void MainWindow::loadScale( const QString& value, const QString& parameter,
							QDoubleSpinBox* spinBox1, QDoubleSpinBox* spinBox2 )
{
	utils::DoubleTuple r = utils::toDoubleTuple(value, 2);

	if (!r.valid) {
		ERROR(doubleTupleBadFormat(parameter, value))
		return;
	}

	bool valid = utils::isTupleBetween(r, 0.0, 100.0);
	if (valid) {
		double mc = r.values.at(0);
		double mb = r.values.at(1);
		spinBox1->setValue(mc);
		spinBox2->setValue(mb);
	} else {
		const QString error = doubleTupleBadBetween(parameter, value,
			0.0, 100.0);
		ERROR(error)
	}
}

void MainWindow::setBackgroundColor(const QColor& color, QPlainTextEdit* colorWidget) {

	colorWidget->setStyleSheet("background-color: rgb(" + Str(color.red()) + "," + Str(color.green()) + "," + Str(color.blue()) + ")");
}

void MainWindow::loadVMT( const QString& vmtPath )
{
	mLoading = true;

	mLogger->clear();

	VmtFile vmt = vmtParser->loadVmtFile(vmtPath);

	ui->tabWidget->setCurrentIndex(0);
	resetWidgets();

	ui->textEdit_proxies->setPlainText( vmt.subGroups.replace("    ", "\t") );

	mIniSettings->setValue("lastSaveAsDir",
		QDir::toNativeSeparators(vmtPath).left(QDir::toNativeSeparators(vmtPath).lastIndexOf('\\')));

	//----------------------------------------------------------------------------------------//

	// Finding game directory by backtracking and searching for gameinfo.txt
	// Placed in this code position to avoid clearing the surfaceprop comboboxes afterwards and lose the parsed entry

	QDir gameinfoDir = vmt.directory;
	bool gameinfoFound = false;

	while( gameinfoDir.cdUp() )
	{
		if( gameinfoDir.exists("gameinfo.txt") )
		{
			gameinfoFound = true;

			break;
		}
	}

	if(!gameinfoFound) {

		setCurrentGame("");
		Info("gameinfo.txt not found in VMT parent directory! No game selected. Texture browse and preview will not work")

	} else {

		QMap< QString, QString >::const_iterator it = mAvailableGames.constBegin();
		while( it != mAvailableGames.constEnd() ) {

			if( it.value().compare( gameinfoDir.absolutePath(), Qt::CaseInsensitive ) == 0 ) {

				setCurrentGame(it.key());
				setGameState(false);

				break;
			}

			++it;
		}
	}

	parseVMT(vmt);

	//----------------------------------------------------------------------------------------//

	setCurrentFile(vmtPath);

	mVMTLoaded = true;
	mLoading = false;

	updateWindowTitle();
}

//----------------------------------------------------------------------------------------////////

bool MainWindow::_toInt( int* value, QString input )
{
	QString text(input);

	bool ok;
	int toReturn = text.toInt(&ok);

	if(ok)
	{
		*value = toReturn;

		return true;
	}
	else
	{
		return false;
	}
}

bool MainWindow::_toDouble( double* value, QString input )
{
	QString text(input);

	bool ok;
	double toReturn = text.toDouble(&ok);

	if(ok)
	{
		*value = toReturn;

		return true;
	}
	else
	{
		return false;
	}
}

int MainWindow::_displaySaveMessage()
{
#ifdef Q_OS_WIN
	SetForegroundWindow( (HWND)winId() );
#endif

	MsgBox msgBox(this);
		msgBox.setWindowTitle("VMT Editor");
		msgBox.setStandardButtons( QMessageBox::Save | QMessageBox::Cancel );
		msgBox.addButton( "Don't Save", QMessageBox::DestructiveRole );
		msgBox.setDefaultButton( QMessageBox::Save );
		msgBox.setIconPixmap(QPixmap(":/icons/information"));
		msgBox.setText("Do you want to save changes to " + (mVMTLoaded ? vmtParser->lastVMTFile().fileName : "untitled") + "?");

	return msgBox.exec();
}

void MainWindow::hideParameterGroupboxes()
{
	for( int i = 0; i < ui->verticalLayout_parameterLeft->count(); ++i )
	{
		QWidget* widget = ui->verticalLayout_parameterLeft->itemAt(i)->widget();

		if( widget != NULL )
			widget->hide();
	}

	QString shader = ui->comboBox_shader->currentText();

	ui->groupBox_patch->setVisible( shader == "Patch" );
	ui->groupBox_sprite->setVisible( shader == "Sprite" );
	ui->groupBox_baseTexture->setVisible( shader == "Lightmapped_4WayBlend" || shader == "VertexLitGeneric" || shader == "WorldVertexTransition" || shader == "UnlitGeneric" || shader == "LightmappedGeneric" );
	ui->groupBox_refract->setVisible( shader == "Refract" );
	ui->groupBox_unlitTwoTexture->setVisible( shader == "UnlitTwoTexture" );

	ui->groupBox_baseTexture2->setVisible(shader == "Lightmapped_4WayBlend");
	ui->groupBox_baseTexture3->setVisible(shader == "Lightmapped_4WayBlend");
	ui->groupBox_baseTexture4->setVisible(shader == "Lightmapped_4WayBlend");

	if( shader == "Water" )
	{
		ui->groupBox_water->show();

	}

	UNCHECK_MENU( ui->menu_texture )
	UNCHECK_MENU( ui->menu_shading )
	UNCHECK_MENU( ui->menu_water )

	if( shader == "WorldVertexTransition" )
	{
		ui->groupBox_baseTexture2->setVisible( shader == "WorldVertexTransition" );

		ui->action_baseTexture2->setChecked(true);
	}
}

void MainWindow::refreshGameListSoft()
{
	if(!mSteamInstalled) {

		if(steamAppsDirectory().isEmpty())
			return;
	}

	QString oldCurrentGame( getCurrentGame() );

	QDir tmp(mSteamAppsDir);
	QStringList games;

	if( tmp.cd("common") ) {

		if( QFile::exists( tmp.absolutePath() + "/left 4 dead 2/left4dead2/gameinfo.txt"))
			mAvailableGames.insert("Left 4 Dead 2", tmp.absolutePath() + "/left 4 dead 2/left4dead2" );

		if( QFile::exists( tmp.absolutePath() + "/portal 2/portal2/gameinfo.txt"))
			mAvailableGames.insert("Portal 2", tmp.absolutePath() + "/portal 2/portal2" );

		if( QFile::exists( tmp.absolutePath() + "/left 4 dead/left4dead/gameinfo.txt"))
			mAvailableGames.insert("Left 4 Dead", tmp.absolutePath() + "/left 4 dead/left4dead" );

		if( QFile::exists( tmp.absolutePath() + "/alien swarm/swarm/gameinfo.txt") )
			mAvailableGames.insert("Alien Swarm", tmp.absolutePath() + "/alien swarm/swarm" );

//		if( QFile::exists( tmp.absolutePath() + "/dota 2 beta/dota/gameinfo.txt") )
//			mAvailableGames.insert("Dota 2", tmp.absolutePath() + "/dota 2 beta/dota" );

		if( QFile::exists( tmp.absolutePath() + "/counter-strike global offensive/csgo/gameinfo.txt") )
			mAvailableGames.insert("Counter-Strike: Global Offensive", tmp.absolutePath() + "/counter-strike global offensive/csgo" );

		if( QFile::exists( tmp.absolutePath() + "/counter-strike source/cstrike/gameinfo.txt") )
			mAvailableGames.insert("Counter-Strike: Source", tmp.absolutePath() + "/counter-strike source/cstrike");

		if( QFile::exists( tmp.absolutePath() + "/day of defeat source/dod/gameinfo.txt") )
			mAvailableGames.insert("Day of Defeat: Source", tmp.absolutePath() + "/day of defeat source/dod");

		if( QFile::exists( tmp.absolutePath() + "/garrysmod/garrysmod/gameinfo.txt") )
			mAvailableGames.insert("Garry's Mod", tmp.absolutePath() + "/garrysmod/garrysmod");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2/hl2/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2", tmp.absolutePath() + "/half-life 2/hl2");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2 episode one/episodic/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2: Episode One", tmp.absolutePath() + "/half-life 2 episode one/episodic");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2 episode two/ep2/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2: Episode Two", tmp.absolutePath() + "/half-life 2 episode two/ep2");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2 lostcoast/lostcoast/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2: Lost Coast", tmp.absolutePath() + "/half-life 2 lostcoast/lostcoast");

		if( QFile::exists( tmp.absolutePath() + "/team fortress 2/tf/gameinfo.txt") )
			mAvailableGames.insert("Team Fortress 2", tmp.absolutePath() + "/team fortress 2/tf");

		if( QFile::exists( tmp.absolutePath() + "/portal/portal/gameinfo.txt") )
			mAvailableGames.insert("Portal", tmp.absolutePath() + "/portal/portal");

		if( QFile::exists( tmp.absolutePath() + "/dayofinfamy/doi/gameinfo.txt") )
			mAvailableGames.insert("Day of Infamy", tmp.absolutePath() + "/dayofinfamy/doi");

		if( QFile::exists( tmp.absolutePath() + "/insurgency2/insurgency/gameinfo.txt") )
			mAvailableGames.insert("Insurgency", tmp.absolutePath() + "/insurgency2/insurgency");

		if( QFile::exists( tmp.absolutePath() + "/black mesa/bms/gameinfo.txt") )
			mAvailableGames.insert("Black Mesa", tmp.absolutePath() + "/black mesa/bms");
	}

	//----------------------------------------------------------------------------------------//

	QString gamesInSettings;
	QMap<QString, QString>::const_iterator it = mAvailableGames.constBegin();
	while( it != mAvailableGames.constEnd() ) {

		gamesInSettings.append( it.key() + "|" + it.value() + "?" );

		games.append( it.key() );

		++it;
	}

	games.sort();

	setGames(games);

	mIniSettings->setValue( "availableGames", gamesInSettings );

	setCurrentGame(oldCurrentGame);
}

void MainWindow::refreshGameList() {

	if(!mSteamInstalled) {

		if(steamAppsDirectory().isEmpty())
			return;
	}

	QString oldCurrentGame( getCurrentGame() );

	mAvailableGames.clear();

	QDir tmp(mSteamAppsDir);
	QStringList games;

	if( tmp.cd("common") ) {

		if( QFile::exists( tmp.absolutePath() + "/left 4 dead 2/left4dead2/gameinfo.txt"))
			mAvailableGames.insert("Left 4 Dead 2", tmp.absolutePath() + "/left 4 dead 2/left4dead2" );

		if( QFile::exists( tmp.absolutePath() + "/portal 2/portal2/gameinfo.txt"))
			mAvailableGames.insert("Portal 2", tmp.absolutePath() + "/portal 2/portal2" );

		if( QFile::exists( tmp.absolutePath() + "/left 4 dead/left4dead/gameinfo.txt"))
			mAvailableGames.insert("Left 4 Dead", tmp.absolutePath() + "/left 4 dead/left4dead" );

		if( QFile::exists( tmp.absolutePath() + "/alien swarm/swarm/gameinfo.txt") )
			mAvailableGames.insert("Alien Swarm", tmp.absolutePath() + "/alien swarm/swarm" );

//		if( QFile::exists( tmp.absolutePath() + "/dota 2 beta/dota/gameinfo.txt") )
//			mAvailableGames.insert("Dota 2", tmp.absolutePath() + "/dota 2 beta/dota" );

		if( QFile::exists( tmp.absolutePath() + "/Counter-Strike Global Offensive/csgo/gameinfo.txt") )
			mAvailableGames.insert("Counter-Strike: Global Offensive", tmp.absolutePath() + "/Counter-Strike Global Offensive/csgo" );

		if( QFile::exists( tmp.absolutePath() + "/counter-strike source/cstrike/gameinfo.txt") )
			mAvailableGames.insert("Counter-Strike: Source", tmp.absolutePath() + "/counter-strike source/cstrike");

		if( QFile::exists( tmp.absolutePath() + "/day of defeat source/dod/gameinfo.txt") )
			mAvailableGames.insert("Day of Defeat: Source", tmp.absolutePath() + "/day of defeat source/dod");

		if( QFile::exists( tmp.absolutePath() + "/garrysmod/garrysmod/gameinfo.txt") )
			mAvailableGames.insert("Garry's Mod", tmp.absolutePath() + "/garrysmod/garrysmod");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2/hl2/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2", tmp.absolutePath() + "/half-life 2/hl2");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2 episode one/episodic/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2: Episode One", tmp.absolutePath() + "/half-life 2 episode one/episodic");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2 episode two/ep2/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2: Episode Two", tmp.absolutePath() + "/half-life 2 episode two/ep2");

		if( QFile::exists( tmp.absolutePath() + "/half-life 2 lostcoast/lostcoast/gameinfo.txt") )
			mAvailableGames.insert("Half-Life 2: Lost Coast", tmp.absolutePath() + "/half-life 2 lostcoast/lostcoast");

		if( QFile::exists( tmp.absolutePath() + "/team fortress 2/tf/gameinfo.txt") )
			mAvailableGames.insert("Team Fortress 2", tmp.absolutePath() + "/team fortress 2/tf");

		if( QFile::exists( tmp.absolutePath() + "/portal/portal/gameinfo.txt") )
			mAvailableGames.insert("Portal", tmp.absolutePath() + "/portal/portal");

		if( QFile::exists( tmp.absolutePath() + "/dayofinfamy/doi/gameinfo.txt") )
			mAvailableGames.insert("Day of Infamy", tmp.absolutePath() + "/dayofinfamy/doi");

		if( QFile::exists( tmp.absolutePath() + "/insurgency2/insurgency/gameinfo.txt") )
			mAvailableGames.insert("Insurgency", tmp.absolutePath() + "/insurgency2/insurgency");

		if( QFile::exists( tmp.absolutePath() + "/black mesa/bms/gameinfo.txt") )
			mAvailableGames.insert("Black Mesa", tmp.absolutePath() + "/black mesa/bms");
	}

	//----------------------------------------------------------------------------------------//

	QString gamesInSettings;
	QMap<QString, QString>::const_iterator it = mAvailableGames.constBegin();
	while( it != mAvailableGames.constEnd() ) {

		gamesInSettings.append( it.key() + "|" + it.value() + "?" );

		games.append( it.key() );

		++it;
	}

	games.sort();

	setGames(games);

	mIniSettings->setValue( "availableGames", gamesInSettings );

	if(mAvailableGames.isEmpty())
		Error("No games found. Manually add games with Games > Manage games dialog.")


	setCurrentGame(oldCurrentGame);
}

void MainWindow::clearCacheFolder()
{
	QDir cacheFolder;
	if (cacheFolder.cd("Cache")) {

		QFileInfoList files = cacheFolder.entryInfoList( QDir::Files );

		for (int i = 0; i < files.size(); ++i) {

			if (files.at(i).completeSuffix().toLower() == "png")
				cacheFolder.remove( files.at(i).fileName() );
		}

		cacheFolder.cdUp();
		cacheFolder.rmdir( "Cache" );
	}
}

void MainWindow::sortShaderComboBox()
{
	QStringList entries;

	for (int i = 0; i < ui->comboBox_shader->count(); ++i) {

		entries.append( ui->comboBox_shader->itemText(i));
	}

	entries.sort();

	ui->comboBox_shader->clear();
	ui->comboBox_shader->addItems(entries);
}

void MainWindow::processTexturesToCopy( const QString& dir ) {

	QMap<QLineEdit*, QString>::iterator it = texturesToCopy.begin();
	while( it != texturesToCopy.end() ) {

		int type = 0;
		if( it.key() == ui->lineEdit_diffuse )
			type = 1;
		else if( it.key() == ui->lineEdit_bumpmap )
			type = 2;
		else if( it.key() == ui->lineEdit_specmap )
			type = 3;
		else if( it.key() == ui->lineEdit_specmap2 )
			type = 3;
		else if( it.key() == ui->lineEdit_exponentTexture )
			type = 4;
		else if( it.key() == ui->lineEdit_tintMask )
			type = 8;

		QString fileName = removeSuffix(it.value(), type);

		if( QFile::exists(dir + fileName + ".vtf") ) {

			MsgBox msgBox(this);
				msgBox.setWindowTitle("File already exists!");
				QPushButton* overwriteButton = msgBox.addButton( "Overwrite", QMessageBox::YesRole );
				msgBox.addButton( QMessageBox::Cancel );
				msgBox.setDefaultButton( overwriteButton );
				msgBox.setIconPixmap(QPixmap(":/icons/info_warning"));

			msgBox.setText( dir.right( dir.length() - dir.lastIndexOf("/") - 1) + fileName +
							".vtf already exists. Do you want to overwrite it?"  );

			if( msgBox.exec() == QMessageBox::Cancel ) {

				++it;
				continue;
			}

			if( !QFile::remove( dir + fileName + ".vtf" ) ) {

				Error( "Error removing \"" + dir + fileName + ".vtf\"" )
				++it;
				continue;
			}
		}

		if( !QFile::rename( QDir::currentPath() + "/Cache/Move/" + it.key()->objectName() + "_" + it.value() + ".vtf",
					   dir + fileName + ".vtf" ) ) {

			Error( "Error moving file from \"" + QDir::currentPath() + "/Cache/Move/" + it.key()->objectName() + "_" + fileName + ".vtf\" to " +
				dir + fileName + ".vtf")
			++it;
			continue;
		}

		QString fileNameConvert = it.key()->toolTip();
		QString relativeFilePath = QDir( currentGameMaterialDir() ).relativeFilePath(dir + fileName);

		mIniPaths->setValue (relativeFilePath, fileNameConvert);
		if (it.key() == ui->lineEdit_bumpmap && !ui->lineEdit_bumpmapAlpha->text().isEmpty())
			mIniPaths->setValue(relativeFilePath + "_alpha_combine", ui->lineEdit_bumpmapAlpha->text());
		else if (it.key() == ui->lineEdit_diffuse && !ui->lineEdit_diffuseAlpha->text().isEmpty())
			mIniPaths->setValue(relativeFilePath + "_alpha_combine", ui->lineEdit_diffuseAlpha->text());

		it.key()->setEnabled(true);
		it.key()->setText( dir.right( dir.length() - dir.lastIndexOf("materials/") - QString("materials/").length() ) + fileName );

		it = texturesToCopy.erase(it);
	}
}

QString MainWindow::addDefaultShader(const QString &shaderName, bool enabled, Settings *settings, QVector<Shader::Groups> groups)
{
	bool found = false;
	for( int i = 0; i < settings->customShaders.count(); ++i )
	{
		if( settings->customShaders.at(i).name == shaderName )
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		settings->customShaders.push_back( Shader( shaderName, enabled, groups ));

		QString groupText;
		for( int i = 0; i < groups.count(); ++i )
		{
			groupText.append( Str( groups.at(i) ) + "?" );
		}

		return QString("%1?%2?%3?").arg( shaderName, enabled ? "1" : "0", groupText );
	}

	return "";
}

// Sometimes I wish I wouldn't be doing this kind of stuff
#define PROCESSSHADERGROUPS(x, y) { if (!x->isChecked()) x->setChecked(true); y->setVisible(true); break; }

void MainWindow::processShaderGroups( Shader::Groups groups )
{
	switch (groups) {

		case Shader::G_Base_Texture: PROCESSSHADERGROUPS(ui->action_baseTexture, ui->groupBox_baseTexture)
		case Shader::G_Base_Texture2: PROCESSSHADERGROUPS(ui->action_baseTexture2, ui->groupBox_baseTexture2)
		case Shader::G_Transparency: PROCESSSHADERGROUPS(ui->action_transparency, ui->groupBox_transparency)
		case Shader::G_Detail: PROCESSSHADERGROUPS(ui->action_detail, ui->groupBox_detailTexture)
		case Shader::G_Color: PROCESSSHADERGROUPS(ui->action_color, ui->groupBox_color)
		case Shader::G_Other: PROCESSSHADERGROUPS(ui->action_other, ui->groupBox_textureOther)
		case Shader::G_Phong: PROCESSSHADERGROUPS(ui->action_phong, ui->groupBox_phong)
		case Shader::G_PhongBrush: PROCESSSHADERGROUPS(ui->action_phongBrush, ui->groupBox_phongBrush)
		case Shader::G_Reflection: PROCESSSHADERGROUPS(ui->action_reflection, ui->groupBox_shadingReflection)
		case Shader::G_Self_Illumination: PROCESSSHADERGROUPS(ui->action_selfIllumination, ui->groupBox_selfIllumination)
		case Shader::G_Rim_Light: PROCESSSHADERGROUPS(ui->action_rimLight, ui->groupBox_rimLight)
		case Shader::G_Water: PROCESSSHADERGROUPS(ui->action_water, ui->groupBox_water)
		case Shader::G_Flowmap: PROCESSSHADERGROUPS(ui->action_flowmap, ui->groupBox_waterFlow)
		case Shader::G_WaterReflection: PROCESSSHADERGROUPS(ui->action_waterReflection, ui->groupBox_waterReflection)
		case Shader::G_Refraction: PROCESSSHADERGROUPS(ui->action_refraction, ui->groupBox_waterRefraction)
		case Shader::G_Fog: PROCESSSHADERGROUPS(ui->action_fog, ui->groupBox_waterFog)
		case Shader::G_Scroll: PROCESSSHADERGROUPS(ui->action_scroll, ui->groupBox_scroll)
		case Shader::G_Base_Texture_Texture_Transforms: PROCESSSHADERGROUPS(ui->action_baseTextureTransforms, ui->groupBox_baseTextureTransforms)
		case Shader::G_Bumpmap_Texture_Transforms: PROCESSSHADERGROUPS(ui->action_bumpmapTransforms, ui->groupBox_bumpmapTransforms)
		case Shader::G_Miscellaneous: PROCESSSHADERGROUPS(ui->action_misc, ui->groupBox_misc)
		case Shader::G_Refract: PROCESSSHADERGROUPS(ui->action_refract, ui->groupBox_refract)
		case Shader::G_Patch: MsgBox::warning(NULL, "Invalid parameter passed!", "MainWindow::shaderChanged():\n\nG_Patch should not be used for a custom shader!"); break;
		case Shader::G_Sprite: PROCESSSHADERGROUPS(ui->action_sprite, ui->groupBox_sprite)
		case Shader::G_UnlitTwoTexture: PROCESSSHADERGROUPS(ui->action_unlitTwoTexture, ui->groupBox_unlitTwoTexture)
		case Shader::G_Base_Texture3: PROCESSSHADERGROUPS(ui->action_baseTexture3, ui->groupBox_baseTexture3)
		case Shader::G_Base_Texture4: PROCESSSHADERGROUPS(ui->action_baseTexture4, ui->groupBox_baseTexture4)
	}
}

void MainWindow::updateWindowTitle() {

	QString title(mChildWidgetChanged ? "*" : "");

	if(mVMTLoaded) {

		title.append(mSettings->fullPathFilenameInWindowTitle ? vmtParser->lastVMTFile().directory /*+ "/"*/ : "");

		if(mSettings->fullPathFilenameInWindowTitle && !title.endsWith("/"))
			title.append("/");

		title.append(vmtParser->lastVMTFile().fileName);

	} else {

		title.append("untitled");
	}

	title.append( (mSettings->showShaderNameInWindowTitle ? " - " + ui->comboBox_shader->currentText() : "") + " - VMT Editor");

	setWindowTitle(title);
}

void MainWindow::acceptedEditGames()
{
	QStringList games;
	QString gamesInSettings;
	QMap<QString, QString>::const_iterator it = mAvailableGames.constBegin();
	while( it != mAvailableGames.constEnd() ) {

		gamesInSettings.append( it.key() + "|" + it.value() + "?" );

		games.append( it.key() );

		++it;
	}

	mIniSettings->setValue( "availableGames", gamesInSettings );

	games.sort();

	setGames(games);

	setCurrentGame(gameBeforeEditGames);
}

void MainWindow::deniedEditGames()
{
	mAvailableGames = mAvailableGamesSave;
}

void MainWindow::displayOptionsDialog()
{
	OptionsDialog dialog(this);
	dialog.parseSettings( mIniSettings, mSettings );

	connect( &dialog, SIGNAL( optionChanged(Settings::Options, QString) ), this, SLOT( changeOption(Settings::Options, QString) ));

	dialog.show();

	switch( dialog.exec() )
	{
	case QDialog::Accepted:

		QString currentShader = ui->comboBox_shader->currentText();

		dialog.updateCustomShaders();

		mIgnoreShaderChanged = true;

		ui->comboBox_shader->clear();

		for (int i = 0; i < mSettings->customShaders.size(); ++i) {

			if (mSettings->customShaders.at(i).enabled)
				ui->comboBox_shader->addItem(mSettings->customShaders.at(i).name);
		}

		sortShaderComboBox();

		//mIgnoreShaderChanged = false;

		int index = ui->comboBox_shader->findText(currentShader);
		if ( index != -1 )
			ui->comboBox_shader->setCurrentIndex(index);
		else
			ui->comboBox_shader->setCurrentIndex(0);

		mIgnoreShaderChanged = false;
	}
}

void MainWindow::displayConversionDialog()
{
#ifdef Q_OS_WIN
	if( QDir().exists("vtfcmd.exe") ) {

		ConversionDialog* dialog = new ConversionDialog( mIniSettings );

		if (fileToConvert != "") {
			dialog->addFile( fileToConvert );
			fileToConvert = "";
		}
		dialog->setModal(false);
		dialog->show();
		//dialog.exec();

	} else {

		MsgBox::warning(this, "VMT Editor - Application Missing", "vtfcmd.exe is needed for the batch process and was not found in the working directory!");
	}
#else
	ConversionDialog dialog(mIniSettings, this);
	dialog.show();
	dialog.exec();
#endif
}

void MainWindow::displayConversionDialogTexture(QString file)
{
#ifdef Q_OS_WIN
	if( QDir().exists("vtfcmd.exe") ) {

		QString dir = mIniSettings->value("lastSaveAsDir").toString();

		mIniSettings->setValue("lastTextureConvertDir", dir);
		ConversionDialog* dialog = new ConversionDialog( mIniSettings );

		if (file != "") {
			dialog->addFile( file );
		}
		dialog->setModal(false);
		dialog->show();
		//dialog.exec();

	} else {

		MsgBox::warning(this, "VMT Editor - Application Missing", "vtfcmd.exe is needed for the batch process and was not found in the working directory!");
	}
#else
	ConversionDialog dialog(mIniSettings, this);
	dialog.show();
	dialog.exec();
#endif
}

void MainWindow::displayBatchDialog() {

	BatchDialog dialog( mAvailableGames, makeVMT(), mIniSettings, this );

	dialog.show();
	dialog.exec();
}

void MainWindow::gameTriggered( bool triggered )
{
	QAction* action = reinterpret_cast<QAction*>(QObject::sender());

	if(triggered) {

		QList<QAction*> actions = ui->action_games->actions();

		foreach(QAction* action_, actions) {

			action_->setChecked(false);

			if(action_->text() == action->text()) {

				action_->setChecked(true);
			}
		}

		gameChanged(action->text());

	} else {

		QList<QAction*> actions = ui->action_games->actions();

		foreach(QAction* action_, actions) {

			action_->setChecked(false);
		}

		gameChanged("");
	}
}

void MainWindow::openReconvertDialogAction() {
	const auto lineEdit =
		qobject_cast<QLineEdit*>(qobject_cast<QObject*>(sender())->parent());
	const auto tooltip = lineEdit->toolTip();

	displayConversionDialogTexture(tooltip);
}


void MainWindow::reconvertTextureHalf() {
	const auto lineEdit =
		qobject_cast<QLineEdit*>(qobject_cast<QObject*>(sender())->parent());
	const auto objectName = lineEdit->objectName();
	const auto tooltip = lineEdit->toolTip();

	QImage texture;
	if (!texture.load(tooltip)) {
		Error("Texture size could not be determined");
		return;
	}

	QString resize = "-rwidth " + Str(texture.width() / 2) +
			" -rheight " + Str(texture.height() / 2) +
			" -rfilter BOX";

	reconvertTexture(lineEdit, objectName, tooltip, resize);
}

void MainWindow::reconvertTexture() {
	const auto lineEdit =
		qobject_cast<QLineEdit*>(qobject_cast<QObject*>(sender())->parent());
	const auto objectName = lineEdit->objectName();
	const auto tooltip = lineEdit->toolTip();

	reconvertTexture(lineEdit, objectName, tooltip, "");
}

void MainWindow::reconvertTexture(QLineEdit* lineEdit,
								  const QString& objectName,
								  const QString& tooltip,
								  const QString& resize)
{
	bool noAlpha = true;
	bool combine = false;
	int type = 0;
	QString preview;

	if( objectName == "lineEdit_diffuse" ) {
		type = 1;
		preview = "preview_basetexture1";
		if (ui->checkBox_basealpha->isChecked() ||
			ui->groupBox_selfIllumination->isVisible() ||
			ui->checkBox_alphaTest->isChecked() ||
			ui->checkBox_transparent->isChecked() ||
			ui->checkBox_blendTint->isChecked() ||
			ui->checkBox_phongBaseAlpha->isChecked() ||
			ui->checkBox_exponentBaseAlpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_bumpmap" ) {
		preview = "preview_bumpmap1";
		if (ui->checkBox_normalalpha->isChecked() ||
			ui->groupBox_phong->isVisible() ||
			ui->checkBox_phongNormalAlpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_diffuse2" ) {
		preview = "preview_basetexture2";
		if (ui->checkBox_basealpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_bumpmap2" ) {
		preview = "preview_bumpmap2";
		if (ui->checkBox_normalalpha->isChecked() ||
			ui->groupBox_phong->isVisible() ||
			ui->checkBox_phongNormalAlpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_diffuse3" ) {
		preview = "preview_basetexture3";
		if (ui->checkBox_basealpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_diffuse4" ) {
		preview = "preview_basetexture4";
		if (ui->checkBox_basealpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_detail" )
		preview = "preview_detail";
	else if( objectName == "lineEdit_refractNormalMap" )
		preview = "preview_bumpmap1";
	else if( objectName == "lineEdit_refractNormalMap2" )
		preview = "preview_bumpmap2";
	else if( objectName == "lineEdit_waterNormalMap" )
		preview = "preview_bumpmap1";
	else if( objectName == "lineEdit_unlitTwoTextureDiffuse" ) {
		preview = "preview_basetexture1";
		noAlpha = false;
	}
	else if( objectName == "lineEdit_unlitTwoTextureDiffuse2" ) {
		preview = "preview_basetexture2";
		noAlpha = false;
	}
	else if( objectName == "lineEdit_bump2" )
		preview = "preview_bumpmap2";
	else if( objectName == "lineEdit_specmap" ) {
		preview = "preview_spec1";
		type = 3;
		if (ui->checkBox_envmapAlpha->isChecked() )
			noAlpha = false;
	}
	else if( objectName == "lineEdit_specmap2" )
		type = 3;
	else if( objectName == "lineEdit_decal" )
		noAlpha = false;

	if( objectName == "lineEdit_exponentTexture" ) {
		preview = "preview_exponent";
		type = 4;
	}

	else if( objectName == "lineEdit_blendmodulate" )
		preview = "preview_blendmod";

	if ( objectName == "lineEdit_bumpmap" ||
		 objectName == "lineEdit_bumpmap2" ||
		 objectName == "lineEdit_bump2" ||
		 objectName == "lineEdit_refractNormalMap" ||
		 objectName == "lineEdit_refractNormalMap2" ||
		 objectName == "lineEdit_waterNormalMap" )
		type = 2;

	if( (objectName == "lineEdit_bumpmap" &&
		 ui->lineEdit_bumpmapAlpha->isVisible() &&
		 !ui->lineEdit_bumpmapAlpha->text().isEmpty()) ||
		(objectName == "lineEdit_diffuse" &&
		 ui->lineEdit_diffuseAlpha->isVisible() &&
		 !ui->lineEdit_diffuseAlpha->text().isEmpty()) ) {
		combine = true;
		noAlpha = false;
	}

	QString mipmapFilter = outputParameters(type, noAlpha);

	QString lineEditText = lineEdit->text();

	//QString dir = QDir::toNativeSeparators(mIniSettings->value("lastSaveAsDir").toString() + "/");
	QString dir = QDir::toNativeSeparators(currentGameMaterialDir() + "/" +
										   lineEditText.section("/", 0, -2) + "/");

	QString fileName = tooltip;
	QString extension = fileName.section(".", -1);
	QString newFile = lineEditText.section("/", -1).section(".", 0, 0);

	QString relativeFilePath = QDir( currentGameMaterialDir() ).relativeFilePath(dir + newFile);

	/*if( QFile::exists(dir + newFile + ".vtf") ) {

		if( !QFile::remove( dir + newFile + ".vtf" ) ) {

			Error( "Error removing \"" + dir + newFile + ".vtf\"" );
			return;
		}
	}*/

	mIniPaths->setValue(relativeFilePath, fileName);

	if (combine && objectName == "lineEdit_bumpmap") {
		if (combineMaps(ui->lineEdit_bumpmap, ui->lineEdit_bumpmapAlpha)) {
			mIniPaths->setValue(relativeFilePath + "_alpha_combine", ui->lineEdit_bumpmapAlpha->text());
			fileName = QDir::currentPath() + "/Cache/bumpmap_alpha_combine.png";
		}
	} else if (combine && objectName == "lineEdit_diffuse") {
		if (combineMaps(ui->lineEdit_diffuse, ui->lineEdit_diffuseAlpha)) {
			mIniPaths->setValue(relativeFilePath + "_alpha_combine", ui->lineEdit_diffuseAlpha->text());
			fileName = QDir::currentPath() + "/Cache/diffuse_alpha_combine.png";
		}
	}

	if (extension != "vtf") {
		InfoReconvert("Converting \"" + fileName.replace("\\", "/").section("/", -1) + "\"...");

		ConversionThread* conversionThread = new ConversionThread(this);
		conversionThread->fileName = fileName;
		conversionThread->objectName = preview;
		conversionThread->relativeFilePath = relativeFilePath;
		conversionThread->newFileName = "";
		conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\" " + mipmapFilter + " " + resize;
		conversionThread->moveFile = true;
		conversionThread->newFile = newFile;
		conversionThread->newFileDir = dir;
		conversionThread->start();

	} else {
		QFile::copy(fileName, QDir::currentPath() + "/Cache/Move/" + objectName + "_" + newFile + ".vtf");

		if( !QFile::rename( QDir::currentPath() + "/Cache/Move/" + objectName + "_" + newFile + ".vtf",
							dir + newFile + ".vtf" ) ) {

			Error( "Error moving file to " + dir)
		} else {
			Info( "File \"" + fileName + "\" successfully copied");
		}
	}

	if (extension == "vtf")
		previewTexture( preview, relativeFilePath, true, false, false, false, true );

	lineEdit->setText(relativeFilePath);
}

void MainWindow::clearLineEdit()
{
	const auto lineEdit =
		qobject_cast<QLineEdit*>(qobject_cast<QObject*>(sender())->parent());
	lineEdit->clear();
	clearLineEditAction(lineEdit);
}

void MainWindow::createReconvertAction(QLineEdit* lineEdit, QString fileName) {
	QString value = mIniPaths->value(fileName).toString();

	if (value != "") {
		QDir dir;
		if (dir.exists(value)) {
			QAction *reconvert = lineEdit->addAction(QIcon(":/icons/reconvert"), QLineEdit::TrailingPosition);
			lineEdit->setToolTip(value);
			connect(reconvert, SIGNAL(triggered()), SLOT(reconvertTexture()));

			QAction *reconvertHalf = lineEdit->addAction(QIcon(":/icons/reconvert_half"), QLineEdit::TrailingPosition);
			connect(reconvertHalf, SIGNAL(triggered()), SLOT(reconvertTextureHalf()));

			QAction *openConvertDialog = lineEdit->addAction(QIcon(":/icons/reconvert_dialog"), QLineEdit::TrailingPosition);
			connect(openConvertDialog, SIGNAL(triggered()), SLOT(openReconvertDialogAction()));


			if (lineEdit == ui->lineEdit_bumpmap) {
				ui->label_bumpmapAlpha->setVisible(true);
				ui->lineEdit_bumpmapAlpha->setVisible(true);
				ui->toolButton_bumpmapAlpha->setVisible(true);
				QString value_alpha = mIniPaths->value(fileName + "_alpha_combine").toString();
				if (value_alpha != "") {
					clearLineEditAction(ui->lineEdit_bumpmapAlpha);
					ui->lineEdit_bumpmapAlpha->setText(value_alpha);
					QAction *clear = ui->lineEdit_bumpmapAlpha->addAction(QIcon(":/icons/clear"), QLineEdit::TrailingPosition);
					clear->setToolTip("Clear");
					connect (clear, SIGNAL(triggered()), SLOT(clearLineEdit()));
				}

			} else if (lineEdit == ui->lineEdit_diffuse) {
				ui->label_diffuseAlpha->setVisible(true);
				ui->lineEdit_diffuseAlpha->setVisible(true);
				ui->toolButton_diffuseAlpha->setVisible(true);
				QString value_alpha = mIniPaths->value(fileName + "_alpha_combine").toString();
				if (value_alpha != "") {
					clearLineEditAction(ui->lineEdit_diffuseAlpha);
					ui->lineEdit_diffuseAlpha->setText(value_alpha);
					QAction *clear = ui->lineEdit_diffuseAlpha->addAction(QIcon(":/icons/clear"), QLineEdit::TrailingPosition);
					clear->setToolTip("Clear");
					connect (clear, SIGNAL(triggered()), SLOT(clearLineEdit()));
				}
			}
		}
	}
}

void MainWindow::reconvertAll() {

	if(mVMTLoaded) {
		triggerLineEditAction(ui->lineEdit_diffuse);
		triggerLineEditAction(ui->lineEdit_diffuse2);
		triggerLineEditAction(ui->lineEdit_diffuse3);
		triggerLineEditAction(ui->lineEdit_diffuse4);
		triggerLineEditAction(ui->lineEdit_bumpmap);
		triggerLineEditAction(ui->lineEdit_bumpmap2);
		triggerLineEditAction(ui->lineEdit_bump2);
		triggerLineEditAction(ui->lineEdit_detail);
		triggerLineEditAction(ui->lineEdit_detail2);
		triggerLineEditAction(ui->lineEdit_exponentTexture);
		triggerLineEditAction(ui->lineEdit_specmap);
		triggerLineEditAction(ui->lineEdit_specmap2);
		triggerLineEditAction(ui->lineEdit_unlitTwoTextureDiffuse);
		triggerLineEditAction(ui->lineEdit_unlitTwoTextureDiffuse2);
		triggerLineEditAction(ui->lineEdit_waterNormalMap);
		triggerLineEditAction(ui->lineEdit_decal);
		triggerLineEditAction(ui->lineEdit_phongWarp);
		triggerLineEditAction(ui->lineEdit_blendmodulate);
		triggerLineEditAction(ui->lineEdit_tintMask);
		triggerLineEditAction(ui->lineEdit_emissiveBlendTexture);
		triggerLineEditAction(ui->lineEdit_emissiveBlendBaseTexture);
		triggerLineEditAction(ui->lineEdit_emissiveBlendFlowTexture);

	}
}

void MainWindow::refreshInGame() {
	QProcess process;
	QString game = getCurrentGame();

	if (game.isEmpty()) {
		Error("No game selected!");
		return;
	}

	if (!mVMTLoaded) {
		Error("No file opened!");
		return;
	}

	action_Save();

	QDir dir(mAvailableGames.value(game));
	dir.cdUp();

	QStringList nameFilter("*.exe");
	QStringList exes = dir.entryList(nameFilter);

	qDebug() << exes;

	QString exe = QDir::toNativeSeparators(dir.absoluteFilePath(exes[0]));

	QString relativeFilePath = QDir(currentGameMaterialDir())
				.relativeFilePath(vmtParser->lastVMTFile().directory + "/"
								  + vmtParser->lastVMTFile().fileName)
				.section(".", 0, -2);

	QString arg = "\"" + exe + "\" -hijack +mat_reloadmaterial \"" + relativeFilePath + "\"";
	qDebug() << arg;

	process.startDetached(arg);
	Info("Reloading material \"" + relativeFilePath + "\"");

}

bool MainWindow::combineMaps(QLineEdit *lineEditBase, QLineEdit *lineEditAlpha) {
	QString basePath = lineEditBase->toolTip();
	QString alphaPath = lineEditAlpha->text().trimmed();
	QImage base;
	QImage alpha;
	if (!base.load(basePath)) {
		qDebug() << "Could not load " << basePath;
		return false;
	}
	if (!alpha.load(alphaPath)) {
		Error("Could not load alpha texture \"" + alphaPath + "\"");
		return false;
	}
	if ((base.height() != alpha.height()) || (base.width() != alpha.width())) {
		alpha = alpha.scaled( base.width(), base.height(),
							  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
	base = base.convertToFormat(QImage::Format_ARGB32);
	alpha = alpha.convertToFormat(QImage::Format_ARGB32);

	QColor rgb, a, pix;
	for (int i = 0; i < base.width(); ++i) {

		for (int j = 0; j < base.height(); ++j) {
			rgb = base.pixel(i, j);
			a = alpha.pixel(i, j);
			pix.setRedF(   rgb.redF() );
			pix.setGreenF( rgb.greenF() );
			pix.setBlueF(  rgb.blueF() );
			pix.setAlphaF( a.redF() );

			base.setPixel(i, j, pix.rgba());
		}
	}
	
	QString fileName;
	if (lineEditBase == ui->lineEdit_bumpmap) {
		fileName = QDir::currentPath() + "/Cache/bumpmap_alpha_combine.png";
	} else if (lineEditBase == ui->lineEdit_diffuse) {
		fileName = QDir::currentPath() + "/Cache/diffuse_alpha_combine.png";
	}
	
	if (QFile::exists(fileName)) {
		if (!QFile::remove(fileName)) {
			Error("Error removing \"" + fileName + "\"");
			return false;
		}
	}

	if (base.save(fileName, "PNG")) {
		qDebug() << "File succesfully combined";
		return true;
	}

	Error("Combining failed!");
	return false;
}

void MainWindow::createBlendToolTexture()
{
	bool is4Way =  ui->comboBox_shader->currentText() == "Lightmapped_4WayBlend";
	bool blendmod = true;

	if(!mVMTLoaded) {
		Error( "VMT must be saved before creating blend tool texture");
		return;
	}

	QString vtf1Path = currentGameMaterialDir() + "/" +
					   ui->lineEdit_diffuse->text();
	QString vtf2Path = currentGameMaterialDir() + "/" +
					   ui->lineEdit_diffuse2->text();
	QString blendmodPath = currentGameMaterialDir() + "/" +
					   ui->lineEdit_blendmodulate->text();


	QFile vtf1File (vtf1Path + ".vtf");
	QFile vtf2File (vtf2Path + ".vtf");
	QFile blendmodFile (blendmodPath + ".vtf");

	//4way
	QString vtf3Path = currentGameMaterialDir() + "/" +
					   ui->lineEdit_diffuse3->text();
	QString vtf4Path = currentGameMaterialDir() + "/" +
					   ui->lineEdit_diffuse4->text();

	QFile vtf3File (vtf3Path + ".vtf");
	QFile vtf4File (vtf4Path + ".vtf");



	QString texture1File = Str( qHash( QFileInfo(vtf1Path + ".vtf").fileName() +
						   Str( vtf1File.size() )));
	QString texture2File = Str( qHash( QFileInfo(vtf2Path + ".vtf").fileName() +
						   Str( vtf2File.size() )));

	QString modulateFile = Str( qHash( QFileInfo(blendmodPath + ".vtf").fileName() +
						   Str( blendmodFile.size() )));

	QString texture3File = Str( qHash( QFileInfo(vtf3Path + ".vtf").fileName() +
						   Str( vtf3File.size() )));
	QString texture4File = Str( qHash( QFileInfo(vtf4Path + ".vtf").fileName() +
						   Str( vtf4File.size() )));

	QImage texture1;
	QImage texture2;

	QImage modulate;

	QImage texture3;
	QImage texture4;

	int size = 256;

	if (!texture1.load(QDir::currentPath() + "/Cache/" + texture1File + ".png")) {
		Error( "Error loading Diffuse texture" )
		return;
	}

	size = texture1.width();

	if (!texture2.load(QDir::currentPath() + "/Cache/" + texture2File + ".png")) {
		Error( "Error loading Diffuse 2 texture" )
		return;
	}

	if (is4Way) {
		if (!texture3.load(QDir::currentPath() + "/Cache/" + texture3File + ".png")) {
			Error( "Error loading Diffuse 3 texture" )
			return;
		}
		if (!texture4.load(QDir::currentPath() + "/Cache/" + texture4File + ".png")) {
			Error( "Error loading Diffuse 4 texture" )
			return;
		}
	} else {
		if (!modulate.load(QDir::currentPath() + "/Cache/" + modulateFile + ".png")) {
			qDebug() << "No blend modulate texture";
			blendmod = false;
		} else {
		modulate = modulate.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
	}

	texture1 = texture1.convertToFormat(QImage::Format_RGB32);
	texture1 = texture1.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	texture2 = texture2.convertToFormat(QImage::Format_RGB32);
	texture2 = texture2.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	if (is4Way) {
		texture3 = texture3.convertToFormat(QImage::Format_RGB32);
		texture3 = texture3.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		texture4 = texture4.convertToFormat(QImage::Format_RGB32);
		texture4 = texture4.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		QColor t1, t2, t3, t4, pix;
		for (int i = 0; i < size; ++i) {

			for (int j = 0; j < size; ++j) {
				t1 = texture1.pixel(i, j);
				t2 = texture2.pixel(i, j);
				t3 = texture3.pixel(i, j);
				t4 = texture4.pixel(i, j);

				double distance; //= ((i - size + j) + 24) / 48.0;
				double blend;
				if (i < 96) {
					distance = i - 88 + j / 4;
					blend = qBound(0.0, distance, 1.0);
					pix.setRgbF(t2.redF() * blend + t1.redF() * (1.0 - blend),
								t2.greenF() * blend + t1.greenF() * (1.0 - blend),
								t2.blueF() * blend + t1.blueF() * (1.0 - blend));
				} else if (i < 160) {
					distance = i - 160 + j / 4;
					blend = qBound(0.0, distance, 1.0);
					pix.setRgbF(t3.redF() * blend + t2.redF() * (1.0 - blend),
								t3.greenF() * blend + t2.greenF() * (1.0 - blend),
								t3.blueF() * blend + t2.blueF() * (1.0 - blend));
				} else {
					distance = i - 232 + j / 4;
					blend = qBound(0.0, distance, 1.0);
					pix.setRgbF(t4.redF() * blend + t3.redF() * (1.0 - blend),
								t4.greenF() * blend + t3.greenF() * (1.0 - blend),
								t4.blueF() * blend + t3.blueF() * (1.0 - blend));
				}
				pix.setAlphaF(1.0);

				texture1.setPixel(i, j, pix.rgba());
			}
		}

	} else {

		QColor tint1 = utils::getBG(ui->toolButton_layer1tint);
		QColor tint2 = utils::getBG(ui->toolButton_layer2tint);

		const double mult1 = ui->doubleSpinBox_layer1tint->value();
		const double mult2 = ui->doubleSpinBox_layer2tint->value();

		double r1 = 1.0, r2 = 1.0, g1 = 1.0, g2 = 1.0, b1 = 1.0, b2 = 1.0;

		if (ui->groupBox_layerblend->isVisible()) {
			r1 = tint1.redF() * mult1;
			r2 = tint2.redF() * mult2;
			g1 = tint1.greenF() * mult1;
			g2 = tint2.greenF() * mult2;
			b1 = tint1.blueF() * mult1;
			b2 = tint2.blueF() * mult2;
		}

		QColor t1, t2, mod, pix;
		for (int i = 0; i < size; ++i) {

			for (int j = 0; j < size; ++j) {
				t1 = texture1.pixel(i, j);
				t2 = texture2.pixel(i, j);

				double distance = ((i - size + j) + 8) / 16.0;
				double blend = qBound(0.0, distance, 1.0);

				if (blendmod) {
					distance = ((i - size + j) + 48) / 96.0;
					blend = qBound(0.0, distance, 1.0);
					mod = modulate.pixel(i, j);
					double minb = qMax(0.0, mod.greenF() - mod.redF());
					double maxb = qMin(1.0, mod.greenF() + mod.redF());
					blend = smoothstep(minb, maxb, blend);
				}

				pix.setRgbF(qBound(0.0, t2.redF() * blend * r2 + t1.redF() * (1.0 - blend) * r1, 1.0),
							qBound(0.0, t2.greenF() * blend * g2 + t1.greenF() * (1.0 - blend) * g1, 1.0),
							qBound(0.0, t2.blueF() * blend * b2 + t1.blueF() * (1.0 - blend) * b1, 1.0));
				pix.setAlphaF(1.0);

				texture1.setPixel(i, j, pix.rgba());
			}
		}
	}

	QString fileName = QDir::currentPath() + "/Cache/blend_tooltexture.png";

	if (QFile::exists(fileName)) {
		if(!QFile::remove(fileName)) {
			Error( "Error removing \"" + fileName + ".vtf\"" );
			return;
		}
	}

	if (texture1.save(fileName, "PNG")) {
		qDebug() << "File succesfully combined";
	} else {
		qDebug() << "Something fucked up";
		return;
	}

	QString newFile = vmtParser->lastVMTFile().fileName.section(".", 0, 0) + "_tooltexture";
	QString dir = QDir::toNativeSeparators(mIniSettings->value("lastSaveAsDir").toString() + "/");

	ConversionThread* conversionThread = new ConversionThread(this);
	conversionThread->fileName = fileName;
	conversionThread->outputParameter = "-output \"" + QDir::currentPath().replace("\\", "\\\\") + "\\Cache\\Move\\" + "\" -format DXT1 -alphaformat DXT1";
	conversionThread->moveFile = true;
	conversionThread->newFile = newFile;
	conversionThread->newFileDir = dir;
	conversionThread->start();

	QString relativeFilePath = QDir( currentGameMaterialDir() ).relativeFilePath(dir + newFile);

	ui->lineEdit_toolTexture->setText(relativeFilePath);

	if (!ui->groupBox_misc->isVisible()) {
		ui->groupBox_misc->setChecked(true);
		utils::toggle(this, true, ui->groupBox_misc, mParsingVMT);
	}

}

QString MainWindow::removeSuffix( const QString fileName, int type)
{
	QString newName = fileName;
	QString vmtName = vmtParser->lastVMTFile().fileName;
	vmtName.chop(4);

	if (mSettings->removeSuffix) {

		if( fileName.endsWith("_diffuse") ){
			newName.chop(8);
			newName = newName + mSettings->diffuseSuffix;

		} else if(fileName.endsWith("_normal") ) {
			newName.chop(7);
			newName = newName + mSettings->bumpSuffix;

		} else if(fileName.endsWith("_specular") ) {
			newName.chop(9);
			newName = newName + mSettings->specSuffix;

		} else if(fileName.endsWith("_glossiness") ) {
			newName.chop(11);
			newName = newName + mSettings->glossSuffix;
		}
	}

	if (mSettings->changeName) {
		if (type == 1) {
			newName = vmtName + mSettings->diffuseSuffix;
		}
		if (type == 2) {
			newName = vmtName + mSettings->bumpSuffix;
		}
		if (type == 3) {
			newName = vmtName + mSettings->specSuffix;
		}
		if (type == 4) {
			newName = vmtName + mSettings->glossSuffix;
		}
		if (type == 8) {
			newName = vmtName + mSettings->tintmaskSuffix;
		}
	}
	return newName;

}

QString MainWindow::outputParameters( int type, bool noAlpha )
{
	QMultiMap<QString, QString> arguments;

	QString tmp;
	if (type == 2 && mSettings->uncompressedNormal) {
		if (noAlpha && mSettings->removeAlpha) {
			arguments.insert("-format", "BGR888");
			arguments.insert("-alphaformat", "BGR888");
		} else {
			arguments.insert("-format", "BGR888");
			arguments.insert("-alphaformat", "BGRA8888");
		}
	} else {
		if (noAlpha && mSettings->removeAlpha)
			arguments.insert("-alphaformat", "DXT1");
		else
			arguments.insert("-alphaformat", "DXT5");
	}

	tmp = mSettings->mipmapFilter;
	if(tmp != "Box") {
		if(tmp == "Point") arguments.insert("-mfilter", "POINT");
		else if(tmp == "Triangle") arguments.insert("-mfilter", "TRIANGLE");
		else if(tmp == "Quadratic") arguments.insert("-mfilter", "QUADRATIC");
		else if(tmp == "Cubic") arguments.insert("-mfilter", "CUBIC");
		else if(tmp == "Catrom") arguments.insert("-mfilter", "CATROM");
		else if(tmp == "Mitchell") arguments.insert("-mfilter", "MITCHELL");
		else if(tmp == "Gaussian") arguments.insert("-mfilter", "GAUSSIAN");
		else if(tmp == "Sinc") arguments.insert("-mfilter", "SINC");
		else if(tmp == "Bessel") arguments.insert("-mfilter", "BESSEL");
		else if(tmp == "Hanning") arguments.insert("-mfilter", "HANNING");
		else if(tmp == "Hamming") arguments.insert("-mfilter", "HAMMING");
		else if(tmp == "Blackman") arguments.insert("-mfilter", "BLACKMAN");
		else if(tmp == "Kaiser") arguments.insert("-mfilter", "KAISER");
	}

	tmp = mSettings->mipmapSharpenFilter;
	if (!(type == 2 && mSettings->noNormalSharpen)) {
		if(tmp != "None") {
			if(tmp == "Negative") arguments.insert("-msharpen", "NEGATIVE");
			else if(tmp == "Lighter") arguments.insert("-msharpen", "LIGHTER");
			else if(tmp == "Darker") arguments.insert("-msharpen", "DARKER");
			else if(tmp == "More Contrast") arguments.insert("-msharpen", "CONTRASTMORE");
			else if(tmp == "Less Contrast") arguments.insert("-msharpen", "CONTRASTLESS");
			else if(tmp == "Smoothen") arguments.insert("-msharpen", "SMOOTHEN");
			else if(tmp == "Soft") arguments.insert("-msharpen", "GAUSSIAN");
			else if(tmp == "Sharpen Soft") arguments.insert("-msharpen", "SHARPENSOFT");
			else if(tmp == "Sharpen Medium") arguments.insert("-msharpen", "SHARPENMEDIUM");
			else if(tmp == "Sharpen Strong") arguments.insert("-msharpen", "SHARPENSTRONG");
			else if(tmp == "Find Edges") arguments.insert("-msharpen", "FINDEDGES");
			else if(tmp == "Contour") arguments.insert("-msharpen", "CONTOUR");
			else if(tmp == "Detect Edges") arguments.insert("-msharpen", "EDGEDETECT");
			else if(tmp == "Detect Edges - Soft") arguments.insert("-msharpen", "EDGEDETECTSOFT");
			else if(tmp == "Emboss") arguments.insert("-msharpen", "EMBOSS");
			else if(tmp == "Mean Removal") arguments.insert("-msharpen", "MEANREMOVAL");
			else if(tmp == "Unsharp") arguments.insert("-msharpen", "UNSHARP");
			else if(tmp == "XSharpen") arguments.insert("-msharpen", "XSHARPEN");
			else if(tmp == "Warpsharp") arguments.insert("-msharpen", "WARPSHARP");
		}
	}

	if (type == 4 && mSettings->noGlossMip)
		arguments.insert("-nomipmaps", "");

	QMap<QString, QString>::iterator it = arguments.begin();

	QString argumentString("");

	while(it != arguments.end()) {

		if(it.value() == "") {

			argumentString.append(" " + it.key());

		} else {

			argumentString.append(" " + it.key() + " " + it.value());
		}

		++it;
	}

	qDebug() << argumentString;
	return argumentString;

}

void MainWindow::checkForUpdates()
{
	auto v = checkForNewVersion();
	if (v.major == -1) {
		Error("Failed to fetch latest version!");

	} else if (v.major == 0) {
		mIniSettings->setValue("latestVersion", getCurrentVersion());
		Info(QString("You have the latest version: %1")
			.arg(removeTrailingVersionZero(getCurrentVersion())));
		ui->menuHelp->setTitle("Help");

	} else {
		const auto vs = versionToString(v);
		mIniSettings->setValue("latestVersion", vs);

		MsgBox msgBox(this);
		msgBox.setWindowTitle("New version available!");

		msgBox.setIconPixmap(QPixmap(":/icons/info_warning"));

		QPushButton* dlButton = msgBox.addButton("Download", QMessageBox::YesRole);
		msgBox.addButton(QMessageBox::Cancel);
		msgBox.setDefaultButton(dlButton);

		msgBox.setText(QString("A new version is available: %1")
			.arg(removeTrailingVersionZero(vs)));

		if (msgBox.exec() != QMessageBox::Cancel) {
			QDesktopServices::openUrl(QString(
				"https://github.com/Gira-X/VMT-Editor/releases/latest"));
		}
	}
}

void MainWindow::checkForUpdatesSilent()
{
	CheckVersionThread* thread = new CheckVersionThread();
	connect(thread, SIGNAL(notifyOnNewVersion(QString)),
		this,  SLOT(notifyOnNewVersion(QString)));
	connect(thread, SIGNAL(finished()),
		thread, SLOT(deleteLater()));
	thread->start();
}

void MainWindow::notifyOnNewVersion(QString version)
{
	Info(QString("New version available: %1").arg(removeTrailingVersionZero(version)));
	mIniSettings->setValue("latestVersion", version);
}

void MainWindow::showEditGamesDialog()
{
	gameBeforeEditGames = getCurrentGame();
	mAvailableGamesSave = mAvailableGames;

	EditGamesDialog dialog(&mAvailableGames, mIniSettings, this);

	connect(&dialog, SIGNAL(refreshGames()), this, SLOT(refreshGameListSoft()));
	connect(&dialog, SIGNAL(accepted()), this, SLOT(acceptedEditGames()));
	connect(&dialog, SIGNAL(rejected()), this, SLOT(deniedEditGames()));

	dialog.show();

	dialog.exec();
}

void MainWindow::opacityChanged( double value ) {

	glWidget_diffuse1->setAlpha(value);
}

void MainWindow::colorChanged() {

	QColor color1 = utils::getBG(ui->toolButton_color1);
	QColor color2 = utils::getBG(ui->toolButton_color2);

	double multiplier1 = ui->doubleSpinBox_color1->value();
	double multiplier2 = ui->doubleSpinBox_color2->value();

	double r1 = color1.redF() * multiplier1;
	double g1 = color1.greenF() * multiplier1;
	double b1 = color1.blueF() * multiplier1;

	double r2 = color2.redF() * multiplier2;
	double g2 = color2.greenF() * multiplier2;
	double b2 = color2.blueF() * multiplier2;

	double r = r1 * r2;
	double g = g1 * g2;
	double b = b1 * b2;

	glWidget_diffuse1->setColor(r,g,b);
}

void MainWindow::fresnelSliderEdited( int a ) {

	double value = double(a) / 100.0;
	ignoreFresnelY = true;
	if (inverseFresnelY) {
		ui->doubleSpinBox_fresnelRangesY->setValue( fresnelYStart * value );
	} else {
		ui->doubleSpinBox_fresnelRangesY->setValue( fresnelYStart + (1.0 - fresnelYStart) * value );
	}
}

void MainWindow::fresnelYEdited( double value ) {

	double fresnelX = ui->doubleSpinBox_fresnelRangesX->value();

	if(!ignoreFresnelY) {

		if(fresnelX > value) {
			fresnelYStart = (value / fresnelX);
			inverseFresnelY = true;
		} else {
			fresnelYStart = (value - fresnelX) / (1.000001 - fresnelX);
			inverseFresnelY = false;
		}
	}
	else
		ignoreFresnelY = false;
}

void MainWindow::displayAboutDialog()
{
	AboutDialog dialog(this);

	dialog.show();
	dialog.exec();
}

void MainWindow::openUserGuide()
{
	QDesktopServices::openUrl(QUrl("https://gira-x.github.io/VMT-Editor/documentation/#/"));
}


//----------------------------------------------------------------------------------------//

ParameterLineEdit::ParameterLineEdit(QWidget* parent) :
	QLineEdit(parent),
	completer(new QCompleter(vmtParameters_))
{
	setMinimumWidth(250);
	setAttribute(Qt::WA_DeleteOnClose);

	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setFilterMode(Qt::MatchContains);
	setCompleter(completer);

	connect(this, SIGNAL(textChanged(QString)), this, SLOT(_editingFinished()));
}

void ParameterLineEdit::_editingFinished()
{
	if( QApplication::activeWindow() != NULL )
		reinterpret_cast<MainWindow*>(QApplication::activeWindow() )->widgetChanged();

	Ui::MainWindow* ui = gUi;

	QFormLayout* grid2 = ui->formLayout_2;
	QFormLayout* grid3 = ui->formLayout_3;

	int index = grid2->indexOf(this);
	ValueLineEdit* valueLineEdit = reinterpret_cast<ValueLineEdit*>( grid3->itemAt(index)->widget() );

	if( text().isEmpty() )
	{
		if( !valueLineEdit->text().isEmpty() )
		{
			if( grid2->indexOf(this) != grid2->count() - 1 )
			{
				ParameterLineEdit* tmp = reinterpret_cast<ParameterLineEdit*>( grid2->itemAt( grid2->count() - 1 )->widget() );
				ValueLineEdit* tmp2 = reinterpret_cast<ValueLineEdit*>( grid3->itemAt( grid3->count() - 1 )->widget() );

				if( tmp->text().isEmpty() && tmp2->text().isEmpty() )
				{
					tmp->close();
					tmp2->close();
				}
			}
		}
		else
		{
			if( index != grid2->count() - 1 )
			{
				grid3->itemAt( grid2->indexOf(this) )->widget()->close();
				close();
			}
		}
	}
	else
	{
		if( grid2->indexOf(this) == grid2->count() - 1 )
		{
			if( valueLineEdit->text().isEmpty() )
				return;
		}

		bool createNew = true;

		for( int i = 1; i < grid2->count(); ++i )
		{
			if( reinterpret_cast<ParameterLineEdit*>( grid2->itemAt(i)->widget() )->text().isEmpty() )
			{
				createNew = false;
				break;
			}

			if( reinterpret_cast<ValueLineEdit*>( grid3->itemAt(i)->widget() )->text().isEmpty() )
			{
				createNew = false;
				break;
			}
		}

		if(createNew) {
			grid2->addRow(new ParameterLineEdit(ui->scrollAreaWidgetContents));
			grid3->addRow( new ValueLineEdit(ui->scrollAreaWidgetContents) );

			for (int i = 0; i < grid2->count(); ++i) {
				QWidget::setTabOrder(
					grid2->itemAt(i)->widget(),
					grid3->itemAt(i)->widget());

				if( (i + 1) < grid2->count() )
					QWidget::setTabOrder( grid3->itemAt(i)->widget(), grid2->itemAt(i + 1)->widget() );
			}
		}
	}
}

//----------------------------------------------------------------------------------------//

ValueLineEdit::ValueLineEdit(QWidget* parent) :
	QLineEdit(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	connect( this, SIGNAL( textChanged(QString) ), this, SLOT( _editingFinished() ));
}

void ValueLineEdit::_editingFinished()
{
	if( QApplication::activeWindow() != NULL )
		reinterpret_cast<MainWindow*>( QApplication::activeWindow() )->widgetChanged();

	Ui::MainWindow* ui = gUi;

	QFormLayout* grid2 = ui->formLayout_2;
	QFormLayout* grid3 = ui->formLayout_3;

	int index = grid3->indexOf(this);
	ParameterLineEdit* parameterLineEdit = reinterpret_cast<ParameterLineEdit*>( grid2->itemAt(index)->widget() );

	if( text().isEmpty() )
	{
		if( !parameterLineEdit->text().isEmpty() )
		{
			if( grid3->indexOf(this) != grid3->count() - 1 )
			{
				ParameterLineEdit* tmp = reinterpret_cast<ParameterLineEdit*>( grid2->itemAt( grid2->count() - 1 )->widget() );
				ValueLineEdit* tmp2 = reinterpret_cast<ValueLineEdit*>( grid3->itemAt( grid3->count() - 1 )->widget() );

				if( tmp->text().isEmpty() && tmp2->text().isEmpty() )
				{
					tmp->close();
					tmp2->close();
				}
			}
		}
		else
		{
			if( index != grid3->count() - 1 )
			{
				grid2->itemAt( grid3->indexOf(this) )->widget()->close();
				close();
			}
		}
	}
	else
	{
		if( grid3->indexOf(this) == grid3->count() - 1 )
		{
			if( parameterLineEdit->text().isEmpty() )
				return;
		}

		bool createNew = true;

		for( int i = 1; i < grid3->count(); ++i )
		{
			if( reinterpret_cast<ParameterLineEdit*>( grid2->itemAt(i)->widget() )->text().isEmpty() )
			{
				createNew = false;
				break;
			}

			if( reinterpret_cast<ValueLineEdit*>( grid3->itemAt(i)->widget() )->text().isEmpty() )
			{
				createNew = false;
				break;
			}
		}

		if(createNew)
		{
			grid2->addRow( new ParameterLineEdit(ui->scrollAreaWidgetContents) );
			grid3->addRow( new ValueLineEdit(ui->scrollAreaWidgetContents) );

			for( int i = 0; i < grid2->count(); ++i )
			{
				QWidget::setTabOrder( grid2->itemAt(i)->widget(), grid3->itemAt(i)->widget() );

				if( (i + 1) < grid2->count() )
				{
					QWidget::setTabOrder( grid3->itemAt(i)->widget(), grid2->itemAt(i + 1)->widget() );
				}
			}
		}
	}
}

// convenience macro for handling menu action triggers (see below for usage)
#define HANDLE_ACTION(groupBox) { \
	utils::toggle(this, checked, groupBox, mParsingVMT); \
}

void MainWindow::on_action_baseTexture2_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_baseTexture2)
}

void MainWindow::on_action_transparency_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_transparency)
}

void MainWindow::on_action_normalBlend_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_normalBlend)
}

void MainWindow::on_action_treeSway_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_treeSway)
}

void MainWindow::on_action_layerBlend_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_layerblend)
}

void MainWindow::on_action_emissiveBlend_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_emissiveBlend)
}

void MainWindow::on_action_decal_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_textureDecal)
}

void MainWindow::on_action_detail_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_detailTexture)
}

void MainWindow::on_action_other_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_textureOther)
}

void MainWindow::on_action_reflection_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_shadingReflection)

	if(!ui->groupBox_shadingReflection->isVisible()) {
		previewTexture( 0, "" );
	}
}

void MainWindow::on_action_phong_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_phong)

	if(ui->groupBox_phong->isVisible()) {

		if( ui->checkBox_exponentBaseAlpha->isChecked() )
			previewTexture( 4, ui->lineEdit_diffuse->text() );
		else
			previewTexture( 4, ui->lineEdit_bumpmap->text() );

	} else {
		previewTexture( 4, "" );
	}

}

void MainWindow::on_action_phongBrush_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_phongBrush)
}

void MainWindow::on_action_selfIllumination_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_selfIllumination)
}

void MainWindow::on_action_rimLight_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_rimLight)
}

void MainWindow::on_action_flowmap_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_waterFlow)
}

void MainWindow::on_action_waterReflection_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_waterReflection)
}

void MainWindow::on_action_refraction_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_waterRefraction)
}

void MainWindow::on_action_fog_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_waterFog)
}

void MainWindow::on_action_baseTextureTransforms_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_baseTextureTransforms)
}

void MainWindow::on_action_bumpmapTransforms_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_bumpmapTransforms)
}

void MainWindow::on_action_color_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_color)
}

void MainWindow::on_action_misc_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_misc)
}

void MainWindow::on_action_scroll_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_scroll)
}

void MainWindow::on_action_baseTexture_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_baseTexture)
}

void MainWindow::on_action_refract_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_refract)
}

void MainWindow::on_action_sprite_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_sprite)
}

void MainWindow::on_action_unlitTwoTexture_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_unlitTwoTexture)
}

void MainWindow::on_action_water_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_water)
}

void MainWindow::on_action_refract_2_triggered(bool checked)
{
	HANDLE_ACTION(ui->groupBox_refract)
}
