#include "editshaderdialog.h"
#include "ui_editshaderdialog.h"

#include <QComboBox>
#include <QStandardItemModel>

#include "optionsdialog.h"
#include "utilities.h"
#include "newshadergroupdialog.h"


EditShaderDialog::EditShaderDialog( QWidget* parent ) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::EditShaderDialog),
	deletingLastShaderEntry(false),
	mIgnoreUpdate(true)
{
	ui->setupUi(this);


	connect( ui->lineEdit_search, SIGNAL( textChanged(QString) ), this, SLOT( updateFilters() ));

	connect( ui->listWidget_shader, SIGNAL( itemSelectionChanged() ), this, SLOT( shaderSelectionChanged() ));
	connect(ui->listWidget_shader, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(shaderSelectionChanged()));

	connect( ui->pushButton_disableShader, SIGNAL( released() ), this, SLOT( toggleShader() ));

	connect( ui->pushButton_deleteShader, SIGNAL( released() ), this, SLOT( deleteShader() ));

	connect( ui->pushButton_newShader, SIGNAL( released() ), this, SLOT( newShader() ));

	connect( ui->pushButton_defaultShader, SIGNAL( released() ), this, SLOT( defaultShader() ));

	connect(ui->listWidget_shader, SIGNAL(updateShader()), this, SLOT(updateShader()));

	connect(ui->listWidget_shader, SIGNAL(adjustShader(QString,QString)), this, SLOT(adjustShader(QString,QString)));

	//----------------------------------------------------------------------------------------//

	loadAllowedShaderGroups();

	resetCheckboxes();
}

EditShaderDialog::~EditShaderDialog()
{
	delete ui;
}

void EditShaderDialog::parseSettings( const QVector< Shader >& settingShaders, const QVector< Shader >& defaultShaders )
{
	mSettingShaders = settingShaders;
	mDefaultShaders = defaultShaders;

	//----------------------------------------------------------------------------------------//

	for( int i = 0; i < mSettingShaders.count(); ++i )
	{
		QListWidgetItem* item = new QListWidgetItem;
		item->setText( mSettingShaders.at(i).name );


		if( !mSettingShaders.at(i).enabled )
			item->setTextColor( Qt::darkGray );


		/*if( gShaders.contains( mSettingShaders.at(i).name ))
		{
			ui->listWidget_shader->addItem( item );
		}
		else
		{
			item->setFlags( item->flags() | Qt::ItemIsEditable );
*/
			ui->listWidget_shader->addItem( item );
		//}
	}

	mChangedShaders = mSettingShaders;
}

void EditShaderDialog::loadAllowedShaderGroups()
{
	QFile file(":/files/allowedShaderGroups");
	file.open(QFile::ReadOnly | QFile::Text);

	int count = 0;
	while( !file.atEnd() )
	{
		QString line = file.readLine();
		line = line.simplified();

		if( !line.isEmpty() )
		{
			if (line.startsWith("//"))
				continue;

			QStringList list( line.split('?'));

			if (list.count() > 0) {

				QVector<Shader::Groups> allowedGroups;
				for (int i = 0; i < list.count(); ++i) {

					allowedGroups.push_back( static_cast<Shader::Groups>(list.at(i).toInt()));
				}

				mAllowedShaderGroups.insert( static_cast<Shader::Shaders>(count), allowedGroups);

				++count;
			}
		}
	}

	file.close();
}

void EditShaderDialog::resetCheckboxes()
{
	ui->checkBox_baseTexture->setChecked(false);
	ui->checkBox_baseTexture2->setChecked(false);
	ui->checkBox_transparency->setChecked(false);
	ui->checkBox_detail->setChecked(false);
	ui->checkBox_color->setChecked(false);
	ui->checkBox_other->setChecked(false);
	ui->checkBox_phong->setChecked(false);
	ui->checkBox_reflection->setChecked(false);
	ui->checkBox_selfIllumination->setChecked(false);
	ui->checkBox_rimLight->setChecked(false);
	ui->checkBox_water->setChecked(false);
	ui->checkBox_flowmap->setChecked(false);
	ui->checkBox_waterReflection->setChecked(false);
	ui->checkBox_refraction->setChecked(false);
	ui->checkBox_fog->setChecked(false);
	ui->checkBox_scroll->setChecked(false);
	ui->checkBox_baseTextureTextureTransforms->setChecked(false);
	ui->checkBox_bumpmapTextureTransforms->setChecked(false);
	ui->checkBox_misc->setChecked(false);
	ui->checkBox_refract->setChecked(false);
	ui->checkBox_sprite->setChecked(false);
	ui->checkBox_unlitTwoTexture->setChecked(false);

	ui->checkBox_baseTexture->setDisabled(true);
	ui->checkBox_baseTexture2->setDisabled(true);
	ui->checkBox_transparency->setDisabled(true);
	ui->checkBox_detail->setDisabled(true);
	ui->checkBox_color->setDisabled(true);
	ui->checkBox_other->setDisabled(true);
	ui->checkBox_phong->setDisabled(true);
	ui->checkBox_reflection->setDisabled(true);
	ui->checkBox_selfIllumination->setDisabled(true);
	ui->checkBox_rimLight->setDisabled(true);
	ui->checkBox_water->setDisabled(true);
	ui->checkBox_flowmap->setDisabled(true);
	ui->checkBox_waterReflection->setDisabled(true);
	ui->checkBox_refraction->setDisabled(true);
	ui->checkBox_fog->setDisabled(true);
	ui->checkBox_scroll->setDisabled(true);
	ui->checkBox_baseTextureTextureTransforms->setDisabled(true);
	ui->checkBox_bumpmapTextureTransforms->setDisabled(true);
	ui->checkBox_misc->setDisabled(true);
	ui->checkBox_refract->setDisabled(true);
	ui->checkBox_sprite->setDisabled(true);
	ui->checkBox_unlitTwoTexture->setDisabled(true);

	ui->checkBox_baseTexture->setChecked(false);
	ui->checkBox_baseTexture2->setChecked(false);
	ui->checkBox_transparency->setChecked(false);
	ui->checkBox_detail->setChecked(false);
	ui->checkBox_color->setChecked(false);
	ui->checkBox_other->setChecked(false);
	ui->checkBox_phong->setChecked(false);
	ui->checkBox_reflection->setChecked(false);
	ui->checkBox_selfIllumination->setChecked(false);
	ui->checkBox_rimLight->setChecked(false);
	ui->checkBox_water->setChecked(false);
	ui->checkBox_flowmap->setChecked(false);
	ui->checkBox_waterReflection->setChecked(false);
	ui->checkBox_refraction->setChecked(false);
	ui->checkBox_fog->setChecked(false);
	ui->checkBox_scroll->setChecked(false);
	ui->checkBox_baseTextureTextureTransforms->setChecked(false);
	ui->checkBox_bumpmapTextureTransforms->setChecked(false);
	ui->checkBox_misc->setChecked(false);
	ui->checkBox_refract->setChecked(false);
	ui->checkBox_sprite->setChecked(false);
	ui->checkBox_unlitTwoTexture->setChecked(false);

	/*ui->groupBox_texture->setDisabled(true);
	ui->groupBox_shading->setDisabled(true);
	ui->groupBox_water->setDisabled(true);
	ui->groupBox_other->setDisabled(true);*/
}

void EditShaderDialog::shaderSelectionChanged()
{
	mIgnoreUpdate = true;

	resetCheckboxes();

	if( ui->listWidget_shader->selectedItems().count() == 0 )
		return;

	const Shader::Shaders shader = Shader::convert(ui->listWidget_shader->currentItem()->text());

	if( !deletingLastShaderEntry )
	{
		ui->pushButton_deleteShader->setEnabled( ui->listWidget_shader->count() != 0 && !gShaders.contains( ui->listWidget_shader->currentItem()->text() ) );

		//emit shaderSelectionModified(ui->listWidget_shader->currentItem()->text());

		for (int i = 0; i < mAllowedShaderGroups.value(shader).count(); ++i) {

			switch(mAllowedShaderGroups.value(shader).at(i)) {

				case Shader::G_Base_Texture: ui->checkBox_baseTexture->setEnabled(true); break;
				case Shader::G_Base_Texture2: ui->checkBox_baseTexture2->setEnabled(true); break;
				case Shader::G_Transparency: ui->checkBox_transparency->setEnabled(true); break;
				case Shader::G_Detail: ui->checkBox_detail->setEnabled(true); break;
				case Shader::G_Color: ui->checkBox_color->setEnabled(true); break;
				case Shader::G_Other: ui->checkBox_other->setEnabled(true); break;
				case Shader::G_Phong: ui->checkBox_phong->setEnabled(true); break;
				case Shader::G_Reflection: ui->checkBox_reflection->setEnabled(true); break;
				case Shader::G_Self_Illumination: ui->checkBox_selfIllumination->setEnabled(true); break;
				case Shader::G_Rim_Light: ui->checkBox_rimLight->setEnabled(true); break;
				case Shader::G_Water: ui->checkBox_water->setEnabled(true); break;
				case Shader::G_Flowmap: ui->checkBox_flowmap->setEnabled(true); break;
				case Shader::G_WaterReflection: ui->checkBox_waterReflection->setEnabled(true); break;
				case Shader::G_Refraction: ui->checkBox_refraction->setEnabled(true); break;
				case Shader::G_Fog: ui->checkBox_fog->setEnabled(true); break;
				case Shader::G_Scroll: ui->checkBox_scroll->setEnabled(true); break;
				case Shader::G_Base_Texture_Texture_Transforms: ui->checkBox_baseTextureTextureTransforms->setEnabled(true); break;
				case Shader::G_Bumpmap_Texture_Transforms: ui->checkBox_bumpmapTextureTransforms->setEnabled(true); break;
				case Shader::G_Miscellaneous: ui->checkBox_misc->setEnabled(true); break;
				case Shader::G_Refract: ui->checkBox_refract->setEnabled(true); break;
				case Shader::G_Patch: MsgBox::warning(NULL, "Wrong parameter passed!", "void EditShaderDialog::shaderSelectionChanged()\n\nValue: G_Patch is not valid!"); break;
				case Shader::G_Sprite: ui->checkBox_sprite->setEnabled(true); break;
				case Shader::G_UnlitTwoTexture: ui->checkBox_unlitTwoTexture->setEnabled(true); break;
				case Shader::G_Base_Texture3: break;
				case Shader::G_Base_Texture4: break;
				default: MsgBox::warning(NULL, "Wrong parameter value passed!",
										 "void EditShaderDialog::shaderSelectionChanged()\n\nParameter: mAllowedShaderGroups.value(" + Str(shader) + ").at(" + Str(i) + "): \n\n"
										 + Str(mAllowedShaderGroups.value(shader).at(i)) + " is not valid!");
			}
		}

		if(ui->checkBox_baseTexture->isEnabled() ||
			ui->checkBox_baseTexture2->isEnabled() ||
			ui->checkBox_transparency->isEnabled() ||
			ui->checkBox_detail->isEnabled() ||
			ui->checkBox_color->isEnabled() ||
			ui->checkBox_other->isEnabled()) ui->groupBox_texture->setEnabled(true);

		if(ui->checkBox_phong->isEnabled() ||
			ui->checkBox_reflection->isEnabled() ||
			ui->checkBox_selfIllumination->isEnabled() ||
			ui->checkBox_rimLight->isEnabled()) ui->groupBox_shading->setEnabled(true);

		if(ui->checkBox_water->isEnabled() ||
			ui->checkBox_flowmap->isEnabled() ||
			ui->checkBox_waterReflection->isEnabled() ||
			ui->checkBox_refraction->isEnabled() ||
			ui->checkBox_fog->isEnabled() ||
			ui->checkBox_scroll->isEnabled()) ui->groupBox_water->setEnabled(true);

		if(ui->checkBox_baseTextureTextureTransforms->isEnabled() ||
			ui->checkBox_bumpmapTextureTransforms->isEnabled() ||
			ui->checkBox_misc->isEnabled() ||
			ui->checkBox_refract->isEnabled() ||
			ui->checkBox_unlitTwoTexture->isEnabled() ||
			ui->checkBox_sprite->isEnabled()) ui->groupBox_other->setEnabled(true);

		// Hack:

		if(shader != Shader::S_Patch && shader != Shader::S_Refract && shader != Shader::S_UnlitTwoTexture && shader != Shader::S_Water) {

			ui->checkBox_baseTexture->setChecked(true);

		} if( shader == Shader::S_Refract) {

			ui->checkBox_refract->setChecked(true);

		} if( shader == Shader::S_UnlitTwoTexture) {

			ui->checkBox_unlitTwoTexture->setChecked(true);

		} if( shader == Shader::S_Sprite) {

			ui->checkBox_sprite->setChecked(true);

		} if( shader == Shader::S_Water) {

			ui->checkBox_water->setChecked(true);
		}

		//


		for (int i = 0; i < mChangedShaders.count(); ++i) {

			if( mChangedShaders.at(i).name == ui->listWidget_shader->currentItem()->text() ) {

				for (int j = 0; j < mChangedShaders.at(i).groups.count(); ++j) {

					switch(mChangedShaders.at(i).groups.at(j)) {

						case Shader::G_Base_Texture: ui->checkBox_baseTexture->setChecked(true); break;
						case Shader::G_Base_Texture2: ui->checkBox_baseTexture2->setChecked(true); break;
						case Shader::G_Transparency: ui->checkBox_transparency->setChecked(true); break;
						case Shader::G_Detail: ui->checkBox_detail->setChecked(true); break;
						case Shader::G_Color: ui->checkBox_color->setChecked(true); break;
						case Shader::G_Other: ui->checkBox_other->setChecked(true); break;
						case Shader::G_Phong: ui->checkBox_phong->setChecked(true); break;
						case Shader::G_Reflection: ui->checkBox_reflection->setChecked(true); break;
						case Shader::G_Self_Illumination: ui->checkBox_selfIllumination->setChecked(true); break;
						case Shader::G_Rim_Light: ui->checkBox_rimLight->setChecked(true); break;
						case Shader::G_Water: ui->checkBox_water->setChecked(true); break;
						case Shader::G_Flowmap: ui->checkBox_flowmap->setChecked(true); break;
						case Shader::G_WaterReflection: ui->checkBox_waterReflection->setChecked(true); break;
						case Shader::G_Refraction: ui->checkBox_refraction->setChecked(true); break;
						case Shader::G_Fog: ui->checkBox_fog->setChecked(true); break;
						case Shader::G_Scroll: ui->checkBox_scroll->setChecked(true); break;
						case Shader::G_Base_Texture_Texture_Transforms: ui->checkBox_baseTextureTextureTransforms->setChecked(true); break;
						case Shader::G_Bumpmap_Texture_Transforms: ui->checkBox_bumpmapTextureTransforms->setChecked(true); break;
						case Shader::G_Miscellaneous: ui->checkBox_misc->setChecked(true); break;
						case Shader::G_Refract: ui->checkBox_refract->setChecked(true); break;
						case Shader::G_Patch: MsgBox::warning(NULL, "Wrong parameter passed!", "void EditShaderDialog::shaderSelectionChanged()\n\nValue: G_Patch is not valid!"); break;
						case Shader::G_Sprite: ui->checkBox_sprite->setChecked(true); break;
						case Shader::G_UnlitTwoTexture: ui->checkBox_unlitTwoTexture->setChecked(true); break;
						case Shader::G_Base_Texture3: break;
						case Shader::G_Base_Texture4: break;
						default: MsgBox::warning(NULL, "Wrong parameter value passed!", "void EditShaderDialog::shaderSelectionChanged()\n\nParameter: mChangedShaders.at(" + Str(i) + ").groups.at(" + Str(j) + "): \n\n"
													  + Str(mChangedShaders.at(i).groups.at(j)) + " is not valid!");
					}
				}

				break;
			}
		}

		/*for( int i = 0; i < mChangedShaders.count(); ++i )
		{
			if( mChangedShaders.at(i).name == ui->listWidget_shader->currentItem()->text() )
			{
				//ui->tableView_shaderGroups->displayShaderGroups( mChangedShaders.at(i).groups );

				QVector< Shader::Groups > groups;
				for( int j = 0; j < ui->tableView_shaderGroups->model()->rowCount(); ++j )
				{
					groups.push_back( static_cast< Shader::Groups >( ui->tableView_shaderGroups->model()->data( ui->tableView_shaderGroups->model()->index( j, 0 )).toInt()));
				}

				mChangedShaders[i].groups = groups;

				break;
			}
		}*/
	}
	else
	{
		ui->pushButton_deleteShader->setEnabled(false);
	}

	mIgnoreUpdate = false;
}

void EditShaderDialog::toggleShader()
{
	if( ui->listWidget_shader->selectedItems().count() == 0 )
		return;

	if( ui->listWidget_shader->currentItem()->textColor() == Qt::darkGray )
		ui->listWidget_shader->currentItem()->setTextColor( Qt::white );
	else
		ui->listWidget_shader->currentItem()->setTextColor( Qt::darkGray );


	for( int i = 0; i < mChangedShaders.count(); ++i )
	{
		if( mChangedShaders.at(i).name == ui->listWidget_shader->currentItem()->text() )
		{
			mChangedShaders[i].enabled = ui->listWidget_shader->currentItem()->textColor() == Qt::black;
		}
	}
}

void EditShaderDialog::deleteShader()
{
	if( ui->listWidget_shader->selectedItems().count() == 0 )
		return;

	deletingLastShaderEntry = ui->listWidget_shader->count() == 1;

	for( int i = 0; i < mChangedShaders.count(); ++i )
	{
		if( mChangedShaders.at(i).name == ui->listWidget_shader->currentItem()->text() )
		{
			mChangedShaders.remove(i);
		}
	}

	for( int i = 0; i < ui->listWidget_shader->count(); ++i )
	{
		if( ui->listWidget_shader->item(i)->text() == ui->listWidget_shader->currentItem()->text() )
		{
			delete ui->listWidget_shader->takeItem(i);
			break;
		}
	}

	if( ui->listWidget_shader->count() > 2 )
	{
		ui->listWidget_shader->setCurrentItem( ui->listWidget_shader->item( ui->listWidget_shader->row( ui->listWidget_shader->currentItem() ) - 1));
	}

	deletingLastShaderEntry = false;

	if (ui->listWidget_shader->currentItem() != NULL )
		ui->pushButton_deleteShader->setEnabled( ui->listWidget_shader->count() != 0 && !gShaders.contains( ui->listWidget_shader->currentItem()->text() ) );
}

void EditShaderDialog::newShader()
{
	NewShaderGroupDialog dialog(mChangedShaders);
	dialog.show();

	switch (dialog.exec()) {

	case QDialog::Accepted:

		if (dialog.getResults().copyFrom != "<None>") {

			for (int i = 0; i < mChangedShaders.count(); ++i) {

				if (dialog.getResults().copyFrom == mChangedShaders.at(i).name) {

					mChangedShaders.append(Shader(dialog.getResults().shaderName, true, mChangedShaders.at(i).groups));

					break;
				}
			}

		} else {

			mChangedShaders.append(Shader(dialog.getResults().shaderName, true));
		}

		QListWidgetItem* item = new QListWidgetItem();
		item->setText(dialog.getResults().shaderName);

		ui->listWidget_shader->addItem(item);
		ui->listWidget_shader->setCurrentItem(item);
		ui->listWidget_shader->scrollToItem(item);

		//delete ui->tableView_shaderGroups->model();

		//QStandardItemModel* model = new QStandardItemModel( 0, 1 );
		//ui->tableView_shaderGroups->setModel(model);
	}
}

void EditShaderDialog::defaultShader()
{
	if( ui->listWidget_shader->selectedItems().count() == 0 )
		return;

	for( int i = 0; i < mChangedShaders.count(); ++i )
	{
		if( mChangedShaders.at(i).name == ui->listWidget_shader->currentItem()->text() )
		{
			bool found = false;

			for( int j = 0; j < mDefaultShaders.count(); ++j )
			{
				if( mChangedShaders.at(i).name == mDefaultShaders.at(j).name )
				{
					found = true;
					break;
				}
			}

			mChangedShaders[i].groups = (found) ? mDefaultShaders.at(i).groups : QVector< Shader::Groups >();

			break;
		}
	}
}

void EditShaderDialog::updateShader()
{
	if(mIgnoreUpdate)
		return;

	mLastShader = ui->listWidget_shader->currentItem()->text();

	for( int i = 0; i < mChangedShaders.count(); ++i ) {

		if( mChangedShaders.at(i).name == ui->listWidget_shader->currentItem()->text() ) {

			QVector<Shader::Groups> enabledGroups;

			if (ui->checkBox_baseTexture->isChecked() &&
				ui->checkBox_baseTexture->isEnabled()) enabledGroups.push_back(Shader::G_Base_Texture);

			if (ui->checkBox_baseTexture2->isChecked() &&
				ui->checkBox_baseTexture2->isEnabled()) enabledGroups.push_back(Shader::G_Base_Texture2);

			if (ui->checkBox_transparency->isChecked() &&
				ui->checkBox_transparency->isEnabled()) enabledGroups.push_back(Shader::G_Transparency);

			if (ui->checkBox_detail->isChecked() &&
				ui->checkBox_detail->isEnabled()) enabledGroups.push_back(Shader::G_Detail);

			if (ui->checkBox_color->isChecked() &&
				ui->checkBox_color->isEnabled()) enabledGroups.push_back(Shader::G_Color);

			if (ui->checkBox_other->isChecked() &&
				ui->checkBox_other->isEnabled()) enabledGroups.push_back(Shader::G_Other);

			if (ui->checkBox_phong->isChecked() &&
				ui->checkBox_phong->isEnabled()) enabledGroups.push_back(Shader::G_Phong);

			if (ui->checkBox_reflection->isChecked() &&
				ui->checkBox_reflection->isEnabled()) enabledGroups.push_back(Shader::G_Reflection);

			if (ui->checkBox_selfIllumination->isChecked() &&
				ui->checkBox_selfIllumination->isEnabled()) enabledGroups.push_back(Shader::G_Self_Illumination);

			if (ui->checkBox_rimLight->isChecked() &&
				ui->checkBox_rimLight->isEnabled()) enabledGroups.push_back(Shader::G_Rim_Light);

			if (ui->checkBox_water->isChecked() &&
				ui->checkBox_water->isEnabled()) enabledGroups.push_back(Shader::G_Water);

			if (ui->checkBox_flowmap->isChecked() &&
				ui->checkBox_flowmap->isEnabled()) enabledGroups.push_back(Shader::G_Flowmap);

			if (ui->checkBox_waterReflection->isChecked() &&
				ui->checkBox_waterReflection->isEnabled()) enabledGroups.push_back(Shader::G_WaterReflection);

			if (ui->checkBox_refraction->isChecked() &&
				ui->checkBox_refraction->isEnabled()) enabledGroups.push_back(Shader::G_Refraction);

			if (ui->checkBox_fog->isChecked() &&
				ui->checkBox_fog->isEnabled()) enabledGroups.push_back(Shader::G_Fog);

			if (ui->checkBox_scroll->isChecked() &&
				ui->checkBox_scroll->isEnabled()) enabledGroups.push_back(Shader::G_Scroll);

			if (ui->checkBox_baseTextureTextureTransforms->isChecked() &&
				ui->checkBox_baseTextureTextureTransforms->isEnabled()) enabledGroups.push_back(Shader::G_Base_Texture_Texture_Transforms);

			if (ui->checkBox_bumpmapTextureTransforms->isChecked() &&
				ui->checkBox_bumpmapTextureTransforms->isEnabled()) enabledGroups.push_back(Shader::G_Bumpmap_Texture_Transforms);

			if (ui->checkBox_misc->isChecked() &&
				ui->checkBox_misc->isEnabled()) enabledGroups.push_back(Shader::G_Miscellaneous);

			if (ui->checkBox_refract->isChecked() &&
				ui->checkBox_refract->isEnabled()) enabledGroups.push_back(Shader::G_Refract);

			if (ui->checkBox_sprite->isChecked() &&
				ui->checkBox_sprite->isEnabled()) enabledGroups.push_back(Shader::G_Sprite);

			if (ui->checkBox_unlitTwoTexture->isChecked() &&
				ui->checkBox_unlitTwoTexture->isEnabled()) enabledGroups.push_back(Shader::G_UnlitTwoTexture);

			mChangedShaders[i].groups = enabledGroups;

			break;
		}
	}
}

void EditShaderDialog::updateFilters()
{
	mSelectedShader = (ui->listWidget_shader->currentItem() != NULL) ? ui->listWidget_shader->currentItem()->text() : "";

	ui->listWidget_shader->clear();

	QString tmp( ui->lineEdit_search->text() );

	if( tmp.contains("?") )
		ui->lineEdit_search->setText( tmp.remove("?") );


	for( int i = 0; i < mChangedShaders.count(); ++i )
	{
		if( mChangedShaders.at(i).name.contains( tmp, Qt::CaseInsensitive ))
		{
			QListWidgetItem* item = new QListWidgetItem;
			item->setText( mChangedShaders.at(i).name );

			if( !mChangedShaders.at(i).enabled )
				item->setTextColor( Qt::darkGray );

			ui->listWidget_shader->addItem( item );
		}
	}

	QList<QListWidgetItem*> items(ui->listWidget_shader->findItems(mSelectedShader, Qt::MatchExactly));
	if( items.count() > 0 ) {

		ui->listWidget_shader->setCurrentItem(items.at(0));
	}
}
