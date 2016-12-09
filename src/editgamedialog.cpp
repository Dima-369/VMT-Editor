#include "editgamedialog.h"
#include "ui_editgamedialog.h"

#include "messagebox.h"

#include <QRegExpValidator>
#include <QDir>
#include <QFileDialog>

EditGameDialog::EditGameDialog(QMap<QString, QString>* games, QWidget* parent) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::EditGameDialog),
	games(games)
{
	ui->setupUi(this);

	connect(ui->toolButton_browse, SIGNAL(clicked()), this, SLOT(browsePath()));

	setWindowTitle("Add a new game");
}

EditGameDialog::EditGameDialog(QString currentGame, QMap<QString, QString>* games, QWidget* parent) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::EditGameDialog),
	games(games),
	currentGame(currentGame)
{
	ui->setupUi(this);

	ui->lineEdit_name->setText(currentGame);
	ui->lineEdit_path->setText(games->value(currentGame));

	connect(ui->toolButton_browse, SIGNAL(clicked()), this, SLOT(browsePath()));

	setWindowTitle("Edit \"" + currentGame + "\"");
}

EditGameDialog::~EditGameDialog()
{
	delete ui;
}

void EditGameDialog::checkName()
{
	if(currentGame.isEmpty()) {

		if( ui->lineEdit_name->text().trimmed().isEmpty() ) {

			MsgBox::warning(this, "Invalid name!", "Game Name can not be empty!");
			return;
		}

		QDir path(ui->lineEdit_path->text().trimmed());

		if( !QDir(path.absoluteFilePath(ui->lineEdit_path->text().trimmed())).exists() ) {

			MsgBox::warning(this, "Invalid path!", "Path does not exist!");
			return;
		}

		if( ui->lineEdit_path->text().trimmed().isEmpty() ) {

			MsgBox::warning(this, "Invalid path!", "Path can not be empty!");
			return;
		}

		if( games->value(ui->lineEdit_name->text().trimmed()).isEmpty() ) {

			games->insert(ui->lineEdit_name->text().trimmed(), ui->lineEdit_path->text().trimmed());

			emit selectItem(ui->lineEdit_name->text().trimmed());

			accept();

		} else {

			MsgBox::warning(this, "Invalid name!", "Game name has to be unique!");
			return;
		}

	} else {

		if( ui->lineEdit_name->text().trimmed().isEmpty() ) {

			MsgBox::warning(this, "Invalid name!", "Game Name can not be empty!");
			return;
		}

		QDir path(ui->lineEdit_path->text().trimmed());

		if( !QDir(path.absoluteFilePath(ui->lineEdit_path->text().trimmed())).exists() ) {

			MsgBox::warning(this, "Invalid path!", "Path does not exist!");
			return;
		}

		if( ui->lineEdit_path->text().trimmed().isEmpty() ) {

			MsgBox::warning(this, "Invalid path!", "Path can not be empty!");
			return;
		}

		if( ui->lineEdit_name->text().trimmed() != currentGame ) {

			if( games->value(ui->lineEdit_name->text().trimmed()).isEmpty() ) {

				games->remove(currentGame);

				games->insert(ui->lineEdit_name->text().trimmed(), ui->lineEdit_path->text().trimmed());

				emit selectItem(ui->lineEdit_name->text().trimmed());

				accept();

			} else {

				MsgBox::warning(this, "Invalid name!", "Game name has to be unique!");
				return;
			}

		} else {

			games->remove(currentGame);

			games->insert(ui->lineEdit_name->text().trimmed(), ui->lineEdit_path->text().trimmed());

			emit selectItem(ui->lineEdit_name->text().trimmed());

			accept();
		}
	}
}

void EditGameDialog::browsePath()
{
	QFileDialog dialog(this);
		dialog.setDirectory( ui->lineEdit_path->text().trimmed() );
		dialog.setFileMode(QFileDialog::Directory);
		dialog.setOption(QFileDialog::ShowDirsOnly);

	if( dialog.exec() ) {

		ui->lineEdit_path->setText( dialog.selectedFiles().at(0) );
	}
}
