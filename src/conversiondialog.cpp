#include "conversiondialog.h"
#include "ui_conversiondialog.h"

#include "utilities.h"
#include "messagebox.h"

#include <QFileDialog>
#include <QProcess>

#ifdef Q_OS_WIN
#   include <Windows.h>
#endif

ConversionDialog::ConversionDialog( QSettings* iniSettings, QWidget* parent )
	: DialogWithoutHelpButton(parent),
	ui(new Ui::ConversionDialog),
	settings(iniSettings)
{
	ui->setupUi(this);

	connect( ui->pushButton_convert, SIGNAL(pressed()), this, SLOT(convertRequested()) );

	connect( ui->pushButton_browseImage, SIGNAL(pressed()), this, SLOT(addRequested()) );

	connect( ui->pushButton_removeImage, SIGNAL(pressed()), this, SLOT(removeRequested()) );

	connect( ui->pushButton_clearList, SIGNAL(pressed()), this, SLOT(clearRequested()) );

	ui->checkBox_resizeToPowerOfTwo->setChecked(true);

	switch( iniSettings->value("convertAskMode", 0).toInt() ) {

	case 0:

		ui->radioButton_ask->setChecked(true);

		break;

	case 1:

		ui->radioButton_overwrite->setChecked(true);

		break;

	case 2:

		ui->radioButton_skip->setChecked(true);
	}

	connect( ui->radioButton_ask, SIGNAL(toggled(bool)), this, SLOT(convertAskModeChanged()));
	connect( ui->radioButton_overwrite, SIGNAL(toggled(bool)), this, SLOT(convertAskModeChanged()));
	connect( ui->radioButton_skip, SIGNAL(toggled(bool)), this, SLOT(convertAskModeChanged()));
}

ConversionDialog::~ConversionDialog() {

	delete ui;
}

void ConversionDialog::addFile( QString fileName ) {

	settings->setValue( "lastConvertAddDirectory", fileName.left( fileName.lastIndexOf("\\") ));

	ui->listWidget_textures->addItem(fileName);

	listEntriesWithDirectories.insert( fileName, false );

	int counter = 0;
	QMap<QString, bool>::const_iterator it = listEntriesWithDirectories.constBegin();
	while( it != listEntriesWithDirectories.constEnd() ) {

		if( it.value() )
			ui->listWidget_textures->item(counter)->setToolTip( "Converted: " + it.key() );
		else
			ui->listWidget_textures->item(counter)->setToolTip( it.key() );

		++it;
		++counter;
	}

}

int ConversionDialog::countImagesToConvert() {

	int count = 0;

	for( int i = 0; i < ui->listWidget_textures->count(); ++i ) {

		if( !ui->listWidget_textures->item(i)->toolTip().startsWith("Converted: ") )
			++count;
	}

	return count;
}

void ConversionDialog::convertRequested() {

	if( countImagesToConvert() == 0 ) {

		MsgBox::information(this, "Nothing to convert",
							"There is nothing to convert.\nUse the Add... button to add images!");
		return;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QMultiMap<QString, QString> arguments;

	QString tmp = ui->comboBox_noAlphaTextures->currentText();
	if( tmp != "DXT1") {

		if(tmp == "RGBA 8888") arguments.insert("-format", "RGBA8888");
		else if(tmp == "ABGR 8888") arguments.insert("-format", "ABGR8888");
		else if(tmp == "RGB 888") arguments.insert("-format", "RGB888");
		else if(tmp == "BGR 888") arguments.insert("-format", "BGR888");
		else if(tmp == "RGB 565") arguments.insert("-format", "RGB565");
		else if(tmp == "I8") arguments.insert("-format", "I8");
		else if(tmp == "IA 88") arguments.insert("-format", "IA88");
		else if(tmp == "A 8") arguments.insert("-format", "A8");
		else if(tmp == "RGB 888 Bluescreen") arguments.insert("-format", "RGB888_BLUESCREEN");
		else if(tmp == "BGR 888 Bluescreen") arguments.insert("-format", "BGR888_BLUESCREEN");
		else if(tmp == "BGRA 8888") arguments.insert("-format", "BGRA8888");
		else if(tmp == "DXT3") arguments.insert("-format", "DXT3");
		else if(tmp == "DXT5") arguments.insert("-format", "DXT5");
		else if(tmp == "BGRX 8888") arguments.insert("-format", "BGRX8888");
		else if(tmp == "BGR 565") arguments.insert("-format", "BGR565");
		else if(tmp == "BGRX 5551") arguments.insert("-format", "BGRX5551");
		else if(tmp == "BGRA 4444") arguments.insert("-format", "BGRA4444");
		else if(tmp == "DXT1 one bit alpha") arguments.insert("-format", "DXT1_ONEBITALPHA");
		else if(tmp == "BGRA 5551") arguments.insert("-format", "BGRA5551");
		else if(tmp == "UV 88") arguments.insert("-format", "UV88");
		else if(tmp == "UVWQ 8888") arguments.insert("-format", "UVWQ8888");
		else if(tmp == "RGBA 16161616F") arguments.insert("-format", "RGBA16161616F");
		else if(tmp == "RGBA 16161616") arguments.insert("-format", "RGBA16161616");
		else if(tmp == "UVLX 8888") arguments.insert("-format", "UVLX8888");
	}

	//----------------------------------------------------------------------------------------//

	tmp = ui->comboBox_alphaTextures->currentText();
	if(tmp != "DXT5") {

		if(tmp == "RGBA 8888") arguments.insert("-alphaformat", "RGBA8888");
		else if(tmp == "ABGR 8888") arguments.insert("-alphaformat", "ABGR8888");
		else if(tmp == "RGB 888") arguments.insert("-alphaformat", "RGB888");
		else if(tmp == "BGR 888") arguments.insert("-alphaformat", "BGR888");
		else if(tmp == "RGB 565") arguments.insert("-alphaformat", "RGB565");
		else if(tmp == "I8") arguments.insert("-alphaformat", "I8");
		else if(tmp == "IA 88") arguments.insert("-alphaformat", "IA88");
		else if(tmp == "A 8") arguments.insert("-alphaformat", "A8");
		else if(tmp == "RGB 888 Bluescreen") arguments.insert("-alphaformat", "RGB888_BLUESCREEN");
		else if(tmp == "BGR 888 Bluescreen") arguments.insert("-alphaformat", "BGR888_BLUESCREEN");
		else if(tmp == "BGRA 8888") arguments.insert("-alphaformat", "BGRA8888");
		else if(tmp == "DXT1") arguments.insert("-alphaformat", "DXT1");
		else if(tmp == "DXT3") arguments.insert("-alphaformat", "DXT3");
		else if(tmp == "DXT5") arguments.insert("-alphaformat", "DXT5");
		else if(tmp == "BGRX 8888") arguments.insert("-alphaformat", "BGRX8888");
		else if(tmp == "BGR 565") arguments.insert("-alphaformat", "BGR565");
		else if(tmp == "BGRX 5551") arguments.insert("-alphaformat", "BGRX5551");
		else if(tmp == "BGRA 4444") arguments.insert("-alphaformat", "BGRA4444");
		else if(tmp == "DXT1 one bit alpha") arguments.insert("-alphaformat", "DXT1_ONEBITALPHA");
		else if(tmp == "BGRA 5551") arguments.insert("-alphaformat", "BGRA5551");
		else if(tmp == "UV 88") arguments.insert("-alphaformat", "UV88");
		else if(tmp == "UVWQ 8888") arguments.insert("-alphaformat", "UVWQ8888");
		else if(tmp == "RGBA 16161616F") arguments.insert("-alphaformat", "RGBA16161616F");
		else if(tmp == "RGBA 16161616") arguments.insert("-alphaformat", "RGBA16161616");
		else if(tmp == "UVLX 8888") arguments.insert("-alphaformat", "UVLX8888");
	}

	//----------------------------------------------------------------------------------------//

	if(ui->checkBox_resizeToPowerOfTwo->isChecked())
		arguments.insert("-resize", "");

	//----------------------------------------------------------------------------------------//

	tmp = ui->comboBox_resizeMethod->currentText();
	if(tmp != "Nearest Power of 2") {

		if(tmp == "Biggest Power of 2") arguments.insert("-rmethod", "BIGGEST");
		else if(tmp == "Smallest Power of 2") arguments.insert("-rmethod", "SMALLEST");
	}

	//----------------------------------------------------------------------------------------//

	tmp = ui->comboBox_resizeFilter->currentText();
	if(tmp != "Triangle") {

		if(tmp == "Point") arguments.insert("-rfilter", "POINT");
		else if(tmp == "Box") arguments.insert("-rfilter", "BOX");
		else if(tmp == "Quadratic") arguments.insert("-rfilter", "QUADRATIC");
		else if(tmp == "Cubic") arguments.insert("-rfilter", "CUBIC");
		else if(tmp == "Catrom") arguments.insert("-rfilter", "CATROM");
		else if(tmp == "Mitchell") arguments.insert("-rfilter", "MITCHELL");
		else if(tmp == "Gaussian") arguments.insert("-rfilter", "GAUSSIAN");
		else if(tmp == "Sinc") arguments.insert("-rfilter", "SINC");
		else if(tmp == "Bessel") arguments.insert("-rfilter", "BESSEL");
		else if(tmp == "Hanning") arguments.insert("-rfilter", "HANNING");
		else if(tmp == "Hamming") arguments.insert("-rfilter", "HAMMING");
		else if(tmp == "Blackman") arguments.insert("-rfilter", "BLACKMAN");
		else if(tmp == "Kaiser") arguments.insert("-rfilter", "KAISER");
	}

	tmp = ui->comboBox_vtfVersion->currentText();
	if(tmp != "7.3") {

		if(tmp == "7.4") arguments.insert("-version", "7.4");
		else if(tmp == "7.5") arguments.insert("-version", "7.5");
		else if(tmp == "7.2") arguments.insert("-version", "7.2");
		else if(tmp == "7.1") arguments.insert("-version", "7.1");
		else if(tmp == "7.0") arguments.insert("-version", "7.0");
	}

	//----------------------------------------------------------------------------------------//

	tmp = ui->comboBox_resizeSharpenFilter->currentText();
	if(tmp != "None") {

		if(tmp == "Negative") arguments.insert("-rsharpen", "NEGATIVE");
		else if(tmp == "Lighter") arguments.insert("-rsharpen", "LIGHTER");
		else if(tmp == "Darker") arguments.insert("-rsharpen", "DARKER");
		else if(tmp == "More Contrast") arguments.insert("-rsharpen", "CONTRASTMORE");
		else if(tmp == "Less Contrast") arguments.insert("-rsharpen", "CONTRASTLESS");
		else if(tmp == "Smoothen") arguments.insert("-rsharpen", "SMOOTHEN");
		else if(tmp == "Soft") arguments.insert("-rsharpen", "GAUSSIAN");
		else if(tmp == "Sharpen Soft") arguments.insert("-rsharpen", "SHARPENSOFT");
		else if(tmp == "Sharpen Medium") arguments.insert("-rsharpen", "SHARPENMEDIUM");
		else if(tmp == "Sharpen Strong") arguments.insert("-rsharpen", "SHARPENSTRONG");
		else if(tmp == "Find Edges") arguments.insert("-rsharpen", "FINDEDGES");
		else if(tmp == "Contour") arguments.insert("-rsharpen", "CONTOUR");
		else if(tmp == "Detect Edges") arguments.insert("-rsharpen", "EDGEDETECT");
		else if(tmp == "Detect Edges - Soft") arguments.insert("-rsharpen", "EDGEDETECTSOFT");
		else if(tmp == "Emboss") arguments.insert("-rsharpen", "EMBOSS");
		else if(tmp == "Mean Removal") arguments.insert("-rsharpen", "MEANREMOVAL");
		else if(tmp == "Unsharp") arguments.insert("-rsharpen", "UNSHARP");
		else if(tmp == "XSharpen") arguments.insert("-rsharpen", "XSHARPEN");
		else if(tmp == "Warpsharp") arguments.insert("-rsharpen", "WARPSHARP");
	}

	//----------------------------------------------------------------------------------------//

	tmp = Str( ui->spinBox_specificWidth->value() );
	if(tmp != "0")
		arguments.insert("-rwidth", tmp);

	//----------------------------------------------------------------------------------------//

	tmp = Str( ui->spinBox_specificHeight->value() );
	if(tmp != "0")
		arguments.insert("-rHeight", tmp);

	//----------------------------------------------------------------------------------------//

	tmp = Str( ui->spinBox_maximumWidth->value() );
	if(tmp != "4096")
		arguments.insert("-rclampwidth", tmp);

	//----------------------------------------------------------------------------------------//

	tmp = Str( ui->spinBox_maximumHeight->value() );
	if(tmp != "4096")
		arguments.insert("-rclampheight", tmp);

	//----------------------------------------------------------------------------------------//

	if(ui->checkBox_convertToNormalMap->isChecked()) {

		arguments.insert("-normal", "");

		tmp = ui->comboBox_generationKernel->currentText();
		if(tmp != "3x3") {

			if(tmp == "4x") arguments.insert("-nkernel", "4X");
			else if(tmp == "5x5") arguments.insert("-nkernel", "5X5");
			else if(tmp == "7x7") arguments.insert("-nkernel", "7X7");
			else if(tmp == "9x9") arguments.insert("-nkernel", "9X9");
			else if(tmp == "DUDV") arguments.insert("-nkernel", "DUDV");
		}

		//----------------------------------------------------------------------------------------//

		tmp = ui->comboBox_heightCalculation->currentText();
		if(tmp != "Average RGB") {

			if(tmp == "Alpha") arguments.insert("-nheight", "ALPHA");
			else if(tmp == "Biased RGB") arguments.insert("-nheight", "BIASEDRGB");
			else if(tmp == "Red") arguments.insert("-nheight", "RED");
			else if(tmp == "Green") arguments.insert("-nheight", "GREEN");
			else if(tmp == "Blue") arguments.insert("-nheight", "BLUE");
			else if(tmp == "Max RGB") arguments.insert("-nheight", "MAXRGB");
			else if(tmp == "Colorspace") arguments.insert("-nheight", "COLORSPACE");
		}

		//----------------------------------------------------------------------------------------//

		tmp = ui->comboBox_alphaResult->currentText();
		if(tmp != "White") {

			if(tmp == "No change") arguments.insert("-nalpha", "NOCHANGE");
			else if(tmp == "Height") arguments.insert("-nalpha", "HEIGHT");
			else if(tmp == "Black") arguments.insert("-nalpha", "BLACK");
		}

		//----------------------------------------------------------------------------------------//

		tmp = Str( ui->doubleSpinBox_normalMapScale->value() );
		if(tmp != "2")
			arguments.insert("-nscale", tmp);

		//----------------------------------------------------------------------------------------//

		if(ui->checkBox_wrapForTiledTextures->isChecked())
			arguments.insert("-nwrap", "");
	}

	//----------------------------------------------------------------------------------------//

	if(ui->checkBox_disableMipmaps->isChecked()) {

		arguments.insert("-nomipmaps", "");

	} else {

		tmp = ui->comboBox_mipmapFilter->currentText();
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

		//----------------------------------------------------------------------------------------//

		tmp = ui->comboBox_mipmapSharpenFilter->currentText();
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

	//----------------------------------------------------------------------------------------//

	tmp = Str( ui->doubleSpinBox_bumpMappingScale->value() );
	if(tmp != "1")
		arguments.insert("-bumpscale", tmp);

	//----------------------------------------------------------------------------------------//

	if( !ui->checkBox_thumbnailImage->isChecked() )
		arguments.insert("-nothumbnail", "");

	//----------------------------------------------------------------------------------------//

	if( !ui->checkBox_reflectivity->isChecked() )
		arguments.insert("-noreflectivity", "");

	//----------------------------------------------------------------------------------------//

	if( ui->checkBox_gammaCorrection->isChecked() ) {

		arguments.insert("-gamma", "");

		tmp = Str( ui->doubleSpinBox_gammaCorrection->value() );
		if(tmp != "2")
			arguments.insert("-gcorrection", tmp);
	}

	//----------------------------------------------------------------------------------------//

	QStringList flags;
	if(ui->checkBox_pointsample->isChecked()) flags.append( "POINTSAMPLE");
	if(ui->checkBox_trilinear->isChecked()) flags.append( "TRILINEAR");
	if(ui->checkBox_clamps->isChecked()) flags.append( "CLAMPS");
	if(ui->checkBox_clampt->isChecked()) flags.append( "CLAMPT");
	if(ui->checkBox_anisotropic->isChecked()) flags.append( "ANISOTROPIC");
	if(ui->checkBox_hintDXT5->isChecked()) flags.append( "HINT_DXT5");
	if(ui->checkBox_normal->isChecked()) flags.append( "NORMAL");
	if(ui->checkBox_noMIP->isChecked()) flags.append( "NOMIP");
	if(ui->checkBox_noLOD->isChecked()) flags.append( "NOLOD");
	if(ui->checkBox_minMIP->isChecked()) flags.append( "MINMIP");
	if(ui->checkBox_depthRendertarget->isChecked()) flags.append( "DEPTHRENDERTARGET");
	if(ui->checkBox_noDebugOverride->isChecked()) flags.append( "NODEBUGOVERRIDE");
	if(ui->checkBox_singleCopy->isChecked()) flags.append( "SINGLECOPY");
	if(ui->checkBox_noDepthbuffer->isChecked()) flags.append( "NODEPTHBUFFER");
	if(ui->checkBox_clampU->isChecked()) flags.append( "CLAMPU");
	if(ui->checkBox_vertexTexture->isChecked()) flags.append( "VERTEXTEXTURE");
	if(ui->checkBox_ssbump->isChecked()) flags.append( "SSBUMP");
	if(ui->checkBox_border->isChecked()) flags.append( "BORDER");
	if(ui->checkBox_rendertarget->isChecked()) flags.append( "RENDETARGET");
	if(ui->checkBox_procedural->isChecked()) flags.append( "PROCEDURAL");

	foreach(const QString& flag, flags) {
		arguments.insert("-flag", flag);
	}

	//----------------------------------------------------------------------------------------//

	QMap<QString, QString>::iterator it = arguments.begin();

	QString argumentString("vtfcmd.exe");

	while(it != arguments.end()) {

		if(it.value() == "") {

			argumentString.append(" " + it.key());

		} else {

			argumentString.append(" " + it.key() + " " + it.value());
		}

		++it;
	}

	//----------------------------------------------------------------------------------------//

	int rememberOverwrite = -1;

	for( int i = 0; i < ui->listWidget_textures->count(); ++i ) {

		QString fileName = ui->listWidget_textures->item(i)->toolTip();
			fileName.chop(4);

		bool skipEntry = false;

		QMap<QString, bool>::iterator it = listEntriesWithDirectories.begin();
		while( it != listEntriesWithDirectories.constEnd() ) {

			if( it.key() == fileName ) {

				if( it.value() )
					skipEntry = true;

				break;
			}

			++it;
		}

		if(skipEntry)
			continue;

		anotherTry:

		QString filePath = ui->listWidget_textures->item(i)->toolTip();

		QString test = filePath.left( filePath.size() - 3 ).append("vtf");

		// TODO:
		if( QDir(test.replace("\\", "/")).exists(test) ) {

			switch( settings->value("convertAskMode", 0).toInt() ) {

				case 0: {

					if( rememberOverwrite != i ) {

						MsgBox msgBox(this);
							msgBox.setWindowTitle("File already exists!");
							msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
							msgBox.setDefaultButton( QMessageBox::No );
							msgBox.setIcon( QMessageBox::Warning );

						msgBox.setText( "A file with the name of: \n\n\"" + test + "\"\n\n already exists."
										" Would you like to overwrite it?"  );

						QApplication::restoreOverrideCursor();

						switch( msgBox.exec() ) {

							case QMessageBox::Abort:

								goto done;

							case QMessageBox::No:

								QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

								ui->listWidget_textures->item(i)->setIcon(QIcon(":/icons/failed"));

								continue;

							case QMessageBox::Yes:

								rememberOverwrite = i;
						}
					}

					break;

				}

				case 2:

					ui->listWidget_textures->item(i)->setIcon(QIcon(":/icons/success"));

					continue;
			}
		}

		QString processString(argumentString + " -file \"" + filePath + "\"");

		QProcess process;
			process.start(processString);
			process.waitForFinished();

		QString output = process.readAllStandardOutput().simplified();

		if( output.endsWith("1/1 files completed.") ) {

			int index = 0;
			for( ; index < ui->listWidget_textures->count(); ++index ) {

				QListWidgetItem* item = ui->listWidget_textures->item(index);

				if( item->toolTip().left( item->toolTip().size() - 4 ) == fileName ) {

					break;
				}
			}

			QString toolTip = "Converted: " + ui->listWidget_textures->item(index)->toolTip();

			ui->listWidget_textures->item(index)->setToolTip(toolTip);

			QMap<QString, bool>::iterator it = listEntriesWithDirectories.begin();
			while( it != listEntriesWithDirectories.constEnd() ) {

				if( it.key() == fileName ) {

					it.value() = true;

					break;
				}

				++it;
			}

			ui->listWidget_textures->item(index)->setIcon(QIcon(":/icons/success"));

			qApp->processEvents();

		} else if( output.contains("Width must be a power of two (nearest powers are") ||
				   output.contains("Height must be a power of two (nearest powers are")) {

			MsgBox msgBox(this);
				msgBox.setWindowTitle("Error while converting!");
				msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
				msgBox.setDefaultButton( QMessageBox::Yes );
				msgBox.setIcon( QMessageBox::Warning );

			msgBox.setText( "An error occurred while trying to convert: \n\n\"" +
							filePath +
							"\"\n\nThe width or height of the image both need to be a power of two.\n" +
							"Would you like to try again with the \"Resize to a power of 2\" argument checked?" );

			QApplication::restoreOverrideCursor();

			switch( msgBox.exec() ) {

				case QMessageBox::Abort:

					goto done;

				case QMessageBox::No:

					QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

					ui->listWidget_textures->item(i)->setIcon(QIcon(":/icons/failed"));

					break;

				case QMessageBox::Yes:

					QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

					argumentString.append(" -resize");

					goto anotherTry;
			}
		}
	}

	done:

	QApplication::restoreOverrideCursor();
}

void ConversionDialog::addRequested() {

	QStringList fileNames = QFileDialog::getOpenFileNames( this,
														   "Select one or more images files to convert",
														   settings->value( "lastConvertAddDirectory", "" ).toString(),
														   "Images (*.bmp *.dds *.gif *.jpg *.png *.tga)");
	QStringList filesToAddInListWidget;

	if( fileNames.count() > 0 ) {

		settings->setValue( "lastConvertAddDirectory", fileNames.last().left( fileNames.last().lastIndexOf("\\") ));

		bool addFiles = false;

		for( int i = 0; i < fileNames.count(); ++i ) {

			QString tmp = fileNames.at(i);

			if( listEntriesWithDirectories.contains(tmp) )
				continue;

			listEntriesWithDirectories.insert( tmp, false );

			// for instance: nature\rock_cbm.vtf
			tmp = tmp.right( tmp.size() - tmp.lastIndexOf("\\", tmp.lastIndexOf("\\") - 1) - 1 );

			filesToAddInListWidget.append(tmp);

			addFiles = true;
		}

		if(addFiles) {

			ui->listWidget_textures->insertItems(0, filesToAddInListWidget);

			ui->listWidget_textures->sortItems();

			int counter = 0;
			QMap<QString, bool>::const_iterator it = listEntriesWithDirectories.constBegin();
			while( it != listEntriesWithDirectories.constEnd() ) {

				if( it.value() )
					ui->listWidget_textures->item(counter)->setToolTip( "Converted: " + it.key() );
				else
					ui->listWidget_textures->item(counter)->setToolTip( it.key() );

				++it;
				++counter;
			}
		}
	}
}

void ConversionDialog::removeRequested() {

	qDeleteAll( ui->listWidget_textures->selectedItems() );

	listEntriesWithDirectories.clear();

	for( int i = 0; i < ui->listWidget_textures->count(); ++i ) {

		QString itemToolTip = ui->listWidget_textures->item(i)->toolTip();

		if( itemToolTip.startsWith("Converted: ") )
			listEntriesWithDirectories.insert( itemToolTip.mid(6), true );
		else
			listEntriesWithDirectories.insert(itemToolTip, false);
	}

	ui->listWidget_textures->sortItems();
}

void ConversionDialog::clearRequested() {

	ui->listWidget_textures->clear();

	listEntriesWithDirectories.clear();
}

void ConversionDialog::convertAskModeChanged() {

	if(ui->radioButton_ask->isChecked()) {

		settings->setValue( "convertAskMode", 0);

	} else if(ui->radioButton_overwrite->isChecked()) {

		settings->setValue( "convertAskMode", 1 );

	} else {

		settings->setValue( "convertAskMode", 2 );
	}
}

void ConversionDialog::resetWidgets()
{
	ui->comboBox_noAlphaTextures->setCurrentIndex(0);
	ui->comboBox_alphaTextures->setCurrentIndex(0);
	ui->comboBox_vtfVersion->setCurrentIndex(1);

	ui->checkBox_disableMipmaps->setChecked(false);
	ui->comboBox_mipmapFilter->setCurrentIndex(0);
	ui->comboBox_mipmapSharpenFilter->setCurrentIndex(0);

	ui->checkBox_resizeToPowerOfTwo->setChecked(true);
	ui->comboBox_resizeMethod->setCurrentIndex(0);
	ui->comboBox_resizeFilter->setCurrentIndex(0);
	ui->comboBox_resizeSharpenFilter->setCurrentIndex(0);
	ui->spinBox_specificHeight->setValue(0);
	ui->spinBox_specificWidth->setValue(0);

	ui->spinBox_maximumHeight->setValue(4096);
	ui->spinBox_maximumWidth->setValue(4096);

	ui->checkBox_convertToNormalMap->setChecked(false);
	ui->comboBox_generationKernel->setCurrentIndex(0);
	ui->comboBox_heightCalculation->setCurrentIndex(0);
	ui->comboBox_alphaResult->setCurrentIndex(0);
	ui->doubleSpinBox_normalMapScale->setValue(2.0);
	ui->checkBox_wrapForTiledTextures->setChecked(false);

	ui->doubleSpinBox_bumpMappingScale->setValue(1.0);
	ui->checkBox_thumbnailImage->setChecked(true);
	ui->checkBox_reflectivity->setChecked(true);
	ui->checkBox_gammaCorrection->setChecked(false);
	ui->doubleSpinBox_gammaCorrection->setValue(0.0);

	ui->checkBox_pointsample->setChecked(false);
	ui->checkBox_trilinear->setChecked(false);
	ui->checkBox_clamps->setChecked(false);
	ui->checkBox_clampt->setChecked(false);
	ui->checkBox_anisotropic->setChecked(false);
	ui->checkBox_hintDXT5->setChecked(false);
	ui->checkBox_normal->setChecked(false);
	ui->checkBox_noMIP->setChecked(false);
	ui->checkBox_noLOD->setChecked(false);
	ui->checkBox_minMIP->setChecked(false);
	ui->checkBox_depthRendertarget->setChecked(false);
	ui->checkBox_noDebugOverride->setChecked(false);
	ui->checkBox_singleCopy->setChecked(false);
	ui->checkBox_noDepthbuffer->setChecked(false);
	ui->checkBox_clampU->setChecked(false);
	ui->checkBox_vertexTexture->setChecked(false);
	ui->checkBox_ssbump->setChecked(false);
	ui->checkBox_border->setChecked(false);
	ui->checkBox_rendertarget->setChecked(false);
	ui->checkBox_procedural->setChecked(false);
}

void ConversionDialog::setTemplate()
{
	resetWidgets();

	QWidget* caller = qobject_cast<QWidget *>( sender() );

	if( caller->objectName() == "pushButton_t01" ){
		ui->comboBox_noAlphaTextures->setCurrentIndex(0);
		ui->comboBox_alphaTextures->setCurrentIndex(1);
	}
	else if( caller->objectName() == "pushButton_t02" ){
		ui->comboBox_noAlphaTextures->setCurrentIndex(5);
		ui->comboBox_alphaTextures->setCurrentIndex(6);
	}
	else if( caller->objectName() == "pushButton_t03" ){
		ui->comboBox_noAlphaTextures->setCurrentIndex(5);
		ui->comboBox_alphaTextures->setCurrentIndex(5);
	}
	else if( caller->objectName() == "pushButton_t04" ){
		ui->comboBox_noAlphaTextures->setCurrentIndex(5);
		ui->comboBox_alphaTextures->setCurrentIndex(5);
		ui->checkBox_disableMipmaps->setChecked(true);
		ui->checkBox_clamps->setChecked(true);
		ui->checkBox_clampt->setChecked(true);
		ui->checkBox_noMIP->setChecked(true);
	}
	else if( caller->objectName() == "pushButton_t05" ){
		ui->comboBox_noAlphaTextures->setCurrentIndex(0);
		ui->comboBox_alphaTextures->setCurrentIndex(1);
		ui->checkBox_disableMipmaps->setChecked(true);
		ui->checkBox_clamps->setChecked(true);
		ui->checkBox_clampt->setChecked(true);
		ui->checkBox_noMIP->setChecked(true);
	}


}

