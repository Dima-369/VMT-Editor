#include "batchdialog.h"
#include "ui_batchdialog.h"

#include "vmtparser.h"
#include "messagebox.h"

BatchDialog::BatchDialog(const QMap<QString, QString>& availableGames,
						  const VmtFile &VMTFile, QSettings *settings, QWidget *parent ) :
	DialogWithoutHelpButton(parent),
	VMTFile(VMTFile),
	ui(new Ui::BatchDialog),
	settings(settings),
	basetextureOnly(false),
	availableGames(availableGames)
{
	ui->setupUi(this);

	QString basetexture = VMTFile.parameters.value("$basetexture");
	if( basetexture.isEmpty() ) {

		basetextureOnly = true;

	} else {

		if( VMTFile.parameters.contains("$bumpmap") ) {

			defaultParameters.insert(Bumpmap, VMTFile.parameters.value("$bumpmap"));

			bumpmap = VMTFile.parameters.value("$bumpmap");
			if( bumpmap.startsWith(basetexture) ) {

				bumpmap = bumpmap.remove( 0, basetexture.length() );

				suffixes.insert(Bumpmap, bumpmap);

				ui->label_bumpmap->setText("$bumpmap: " + bumpmap);
			}

		} else {

			ui->label_bumpmap->setVisible(false);
		}

		if( VMTFile.parameters.contains("$selfillummask") ) {

			defaultParameters.insert(SelfillumMask, VMTFile.parameters.value("$selfillummask"));

			selfillummask = VMTFile.parameters.value("$selfillummask");
			if( selfillummask.startsWith(basetexture) ) {

				selfillummask = selfillummask.remove( 0, basetexture.length() );

				suffixes.insert(SelfillumMask, selfillummask);

				ui->label_selfillummask->setText("$selfillummask: " + selfillummask);
			}

		} else {

			ui->label_selfillummask->setVisible(false);
		}

		if( VMTFile.parameters.contains("$phongexponenttexture") ) {

			defaultParameters.insert(PhongExponentTexture, VMTFile.parameters.value("$phongexponenttexture"));

			phongexponenttexture = VMTFile.parameters.value("$phongexponenttexture");
			if( phongexponenttexture.startsWith(basetexture) ) {

				phongexponenttexture = phongexponenttexture.remove( 0, basetexture.length() );

				suffixes.insert(PhongExponentTexture, phongexponenttexture);

				ui->label_phongexponenttexture->setText("$phongexponenttexture: " + phongexponenttexture);
			}

		} else {

			ui->label_phongexponenttexture->setVisible(false);
		}


		if( VMTFile.parameters.contains("$tooltexture") ) {

			defaultParameters.insert(ToolTexture, VMTFile.parameters.value("$tooltexture"));

			tooltexture = VMTFile.parameters.value("$tooltexture");
			if( tooltexture.startsWith(basetexture) ) {

				tooltexture = tooltexture.remove( 0, basetexture.length() );

				suffixes.insert(ToolTexture, tooltexture);

				ui->label_tooltexture->setText("$tooltexture: " + tooltexture);
			}

		} else {

			ui->label_tooltexture->setVisible(false);
		}

		if( VMTFile.parameters.contains("$envmapmask") ) {

			defaultParameters.insert(EnvmapMask, VMTFile.parameters.value("$envmapmask"));

			envmapmask = VMTFile.parameters.value("$envmapmask");
			if( envmapmask.startsWith(basetexture) ) {

				envmapmask = envmapmask.remove( 0, basetexture.length() );

				suffixes.insert(EnvmapMask, envmapmask);

				ui->label_envmapmask->setText("$envmapmask: " + envmapmask);
			}

		} else {

			ui->label_envmapmask->setVisible(false);
		}
	}

	switch( settings->value("convertAskMode", 0).toInt() ) {

	case 0:

		ui->radioButton_ask->setChecked(true);

		break;

	case 1:

		ui->radioButton_overwrite->setChecked(true);

		break;

	case 2:

		ui->radioButton_skip->setChecked(true);
	}

	connect( ui->pushButton_batch, SIGNAL(pressed()), this, SLOT(batchRequested()) );

	connect( ui->pushButton_browseImage, SIGNAL(pressed()), this, SLOT(addRequested()) );
	connect( ui->pushButton_removeImage, SIGNAL(pressed()), this, SLOT(removeRequested()) );
	connect( ui->pushButton_clearList, SIGNAL(pressed()), this, SLOT(clearRequested()) );

	connect( ui->radioButton_ask, SIGNAL(toggled(bool)), this, SLOT(convertAskModeChanged()));
	connect( ui->radioButton_overwrite, SIGNAL(toggled(bool)), this, SLOT(convertAskModeChanged()));
	connect( ui->radioButton_skip, SIGNAL(toggled(bool)), this, SLOT(convertAskModeChanged()));
}

BatchDialog::~BatchDialog() {

	delete ui;
}

int BatchDialog::countFilesToConvert() {

	int count = 0;

	for( int i = 0; i < ui->listWidget_batchFiles->count(); ++i ) {

		if( !ui->listWidget_batchFiles->item(i)->toolTip().startsWith("Converted: ") )
			++count;
	}

	return count;
}

void BatchDialog::resetVMTFile() {

	if( defaultParameters.contains(Bumpmap) )
		VMTFile.parameters.insert( "$bumpmap", defaultParameters.value(Bumpmap) );

	if( defaultParameters.contains(SelfillumMask) )
		VMTFile.parameters.insert( "$selfillummask", defaultParameters.value(SelfillumMask) );

	if( defaultParameters.contains(PhongExponentTexture) )
		VMTFile.parameters.insert( "$phongexponenttexture", defaultParameters.value(PhongExponentTexture) );

	if( defaultParameters.contains(ToolTexture) )
		VMTFile.parameters.insert( "$tooltexture", defaultParameters.value(ToolTexture) );

	if( defaultParameters.contains(EnvmapMask) )
		VMTFile.parameters.insert( "$envmapmask", defaultParameters.value(EnvmapMask) );
}

QString BatchDialog::isMaterialDirectory( const QString& fileName ) const {

	foreach( QString dir, availableGames.values() ) {

		dir.replace("\\", "/");

		if( fileName.startsWith( dir, Qt::CaseInsensitive )) {

			return (dir + "/materials");
		}
	}

	return "";
}

void BatchDialog::_processParameter( const QString& parameter, QString fileName ) {

	QString dir = isMaterialDirectory(fileName);

	if( !dir.isEmpty() ) {

		fileName.remove( 0, dir.length() + 1 );

		VMTFile.parameters.insert( parameter, fileName.replace("\\", "/") );

	} else {

		VMTFile.parameters.insert( parameter,
								   fileName.right( fileName.size() - fileName.lastIndexOf("\\") - 1 ).replace("\\", "/") );
	}
}

void BatchDialog::rek( QList<QString>* input, QList<QString>* output ) {

	if( input->isEmpty() )
		return;

	QString firstElement( input->takeFirst() );
		output->append(firstElement);
		firstElement.chop(4);

	bool isBaseTexture = true;

	QMap<Parameters, QString>::const_iterator it = suffixes.constBegin();
	while( it != suffixes.constEnd() ) {

		if( firstElement.endsWith( it.value(), Qt::CaseInsensitive )) {

			isBaseTexture = false;

			break;
		}

		++it;
	}

	if(isBaseTexture) {

		for( int i = 0; i < input->count(); ++i ) {

			QString elementLoop = input->at(i);
			elementLoop.chop(4);

			QMap<Parameters, QString>::const_iterator it = suffixes.constBegin();
			while( it != suffixes.constEnd() ) {

				if( elementLoop.compare( firstElement + it.value(), Qt::CaseInsensitive ) == 0 ) {

					QString tmp = output->takeLast();

					output->append( tmp + "|" + input->takeAt(i) );

					break;
				}

				++it;
			}
		}
	}

	rek( input, output );
}

void BatchDialog::batchRequested() {

	if( countFilesToConvert() == 0 ) {

		MsgBox::information(this, "Information",
							"There is nothing to convert anymore.\n\nUse the Add... button to add images!");
		return;
	}

	//QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QList<QString> listEntries( listEntriesWithDirectories.keys() );
		qSort(listEntries);

	QStringList output;

	rek( &listEntries, &output );

	for( int i = 0; i < output.count(); ++i ) {

		QStringList singleEntries( output.at(i).split( '|', QString::SkipEmptyParts ));
		QString firstEntry( singleEntries.first() );
		bool isBaseTexture = false;

		if( singleEntries.count() > 2) {

			isBaseTexture = true;

		} else {

			bool found = false;

			QMap<Parameters, QString>::const_iterator it = suffixes.constBegin();
			while( it != suffixes.constEnd() ) {

				if( firstEntry.endsWith( it.value(), Qt::CaseInsensitive )) {

					found = true;

					break;
				}

				++it;
			}

			if(!found)
				isBaseTexture = true;
		}

		for( int j = 0; j < singleEntries.count(); ++j ) {

			bool skipEntry = false;

			QMap<QString, bool>::iterator it = listEntriesWithDirectories.begin();
			while( it != listEntriesWithDirectories.constEnd() ) {

				if( it.key() == singleEntries.at(j) ) {

					if( it.value() )
						skipEntry = true;

					break;
				}

				++it;
			}

			if(skipEntry)
				continue;

			QString fileName = singleEntries.at(j);
				fileName.chop(4);

			if(isBaseTexture) {

				isBaseTexture = false;

				_processParameter( "$basetexture", fileName );

			} else {

				QMap<Parameters, QString>::const_iterator it = suffixes.constBegin();
				while( it != suffixes.constEnd() ) {

					if( singleEntries.at(j).endsWith( it.value() )) {

						switch( it.key() ) {

						case Bumpmap:

							_processParameter( "$bumpmap", fileName );
							break;

						case SelfillumMask:

							_processParameter( "$selfillummask", fileName );
							break;

						case PhongExponentTexture:

							_processParameter( "$phongexponenttexture", fileName );
							break;

						case ToolTexture:

							_processParameter( "$tooltexture", fileName );
							break;

						case EnvmapMask:

							_processParameter( "$envmap", fileName );
						}
					}

					++it;
				}
			}

			QString convertedVMT = VmtParser::convertVmt( VMTFile,
														  settings->value("parameterSortStyle").toInt() == Settings::Grouped,
														  settings->value("useQuotesForTexture").toBool(),
														  settings->value("useIndentation").toBool() );

			QString VMTFileName( singleEntries.at(j).left( singleEntries.at(j).lastIndexOf(".") + 1 ) + "vmt" );

			bool overwrite = true;

			if( QFile::exists(VMTFileName) ) {

				switch( settings->value("convertAskModeBatch", 0).toInt() ) {

					case 0: {

						MsgBox msgBox(this);
							msgBox.setWindowTitle("File already exists!");
							msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort );
							msgBox.setDefaultButton( QMessageBox::No );
							msgBox.setIcon( QMessageBox::Warning );

						msgBox.setText( "A file with the name of: \n\n\"" + VMTFileName + "\"\n\n already exists."
										" Would you like to overwrite it?"  );

						switch( msgBox.exec() ) {

							case QMessageBox::Abort:

								resetVMTFile();

								return;

							case QMessageBox::No:

								int index = 0;
								for( ; index < ui->listWidget_batchFiles->count(); ++index ) {

									QListWidgetItem* item = ui->listWidget_batchFiles->item(index);

									if( item->toolTip().left( item->toolTip().size() - 4 ) == fileName ) {

										ui->listWidget_batchFiles->item(index)->setIcon(QIcon(":/icons/failed"));
										break;
									}
								}

								resetVMTFile();

								continue;
						}

						break;
					}

					case 2:

						continue;
				}
			}

			bool error = false;

			if(overwrite) {

				QFile VMTFile(VMTFileName);

				if( !VMTFile.open( QIODevice::WriteOnly | QIODevice::Text )) {

					MsgBox::warning( this, "VMT Editor", "\"" + VMTFileName + "\" failed to be opened in WriteOnly mode!" );

					int index = 0;
					for( ; index < ui->listWidget_batchFiles->count(); ++index ) {

						QListWidgetItem* item = ui->listWidget_batchFiles->item(index);

						if( item->toolTip().left( item->toolTip().size() - 4 ) == fileName ) {

							break;
						}
					}

					ui->listWidget_batchFiles->item(index)->setIcon(QIcon(":/icons/failed"));

					error = true;

				} else {

					QTextStream out(&VMTFile);

					out << convertedVMT;

					VMTFile.close();
				}
			}

			if(!error) {

				int index = 0;
				for( ; index < ui->listWidget_batchFiles->count(); ++index ) {

					QListWidgetItem* item = ui->listWidget_batchFiles->item(index);

					if( item->toolTip().left( item->toolTip().size() - 4 ) == fileName ) {

						break;
					}
				}

				QString toolTip = "Done: " + ui->listWidget_batchFiles->item(index)->toolTip();

				ui->listWidget_batchFiles->item(index)->setToolTip(toolTip);

				QMap<QString, bool>::iterator it = listEntriesWithDirectories.begin();
				while( it != listEntriesWithDirectories.constEnd() ) {

					if( it.key() == singleEntries.at(j) ) {

						it.value() = true;

						break;
					}

					++it;
				}

				ui->listWidget_batchFiles->item(index)->setIcon(QIcon(":/icons/success"));
			}

			resetVMTFile();
		}
	}
}

void BatchDialog::addRequested() {

	QStringList fileNames = QFileDialog::getOpenFileNames( this,
														   "Select one or more images files to use in the batch process",
														   QDir::toNativeSeparators(settings->value("lastSaveAsDir", "").toString()),
														   "Images (*.vtf *.bmp *.dds *.gif *.jpg *.png *.tga)" );

	QStringList filesToAddInListWidget;

	if( fileNames.count() > 0 ) {

		settings->setValue("lastSaveAsDir", QDir::toNativeSeparators(fileNames.last()).left( QDir::toNativeSeparators(fileNames.last()).lastIndexOf("\\")));
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

			// qSort(listEntriesWithDirectories);

			ui->listWidget_batchFiles->insertItems(0, filesToAddInListWidget);

			ui->listWidget_batchFiles->sortItems();

			int counter = 0;
			QMap<QString, bool>::const_iterator it = listEntriesWithDirectories.constBegin();
			while( it != listEntriesWithDirectories.constEnd() ) {

				if( it.value() )
					ui->listWidget_batchFiles->item(counter)->setToolTip( "Done: " + it.key() );
				else
					ui->listWidget_batchFiles->item(counter)->setToolTip( it.key() );

				++it;
				++counter;
			}
		}
	}
}

void BatchDialog::removeRequested() {

	qDeleteAll( ui->listWidget_batchFiles->selectedItems() );

	listEntriesWithDirectories.clear();

	for( int i = 0; i < ui->listWidget_batchFiles->count(); ++i ) {

		QString itemToolTip = ui->listWidget_batchFiles->item(i)->toolTip();

		if( itemToolTip.startsWith("Done: ") )
			listEntriesWithDirectories.insert( itemToolTip.mid(6), true );
		else
			listEntriesWithDirectories.insert(itemToolTip, false);
	}

	// qSort(listEntriesWithDirectories);

	ui->listWidget_batchFiles->sortItems();
}

void BatchDialog::clearRequested() {

	ui->listWidget_batchFiles->clear();

	listEntriesWithDirectories.clear();
}

void BatchDialog::convertAskModeChanged() {

	if(ui->radioButton_ask->isChecked()) {

		settings->setValue( "convertAskModeBatch", 0);

	} else if(ui->radioButton_overwrite->isChecked()) {

		settings->setValue( "convertAskModeBatch", 1 );

	} else {

		settings->setValue( "convertAskModeBatch", 2 );
	}
}
