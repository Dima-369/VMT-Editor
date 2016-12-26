#include "editgamesdialog.h"
#include "ui_editgamesdialog.h"

#include "editgamedialog.h"
#include "utilities.h"

#include <QTableWidgetItem>
#include <QList>

EditGamesDialog::EditGamesDialog( QMap<QString, QString>* games, QSettings* settings, QWidget* parent) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::EditGamesDialog),
	settings(settings),
	games(games)
{
	ui->setupUi(this);

	ui->tableWidget->setRowCount(games->count());
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui->tableWidget->setFocusPolicy(Qt::NoFocus);

	fillWidget();

	connect(ui->pushButton_add, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->pushButton_scan, SIGNAL(clicked()), this, SLOT(refresh()));
	connect(ui->pushButton_edit, SIGNAL(clicked()), this, SLOT(edit()));
	connect(ui->pushButton_remove, SIGNAL(clicked()), this, SLOT(remove()));
}

EditGamesDialog::~EditGamesDialog()
{
	delete ui;
}

void EditGamesDialog::selectRow(QString game)
{
	toSelect = game;
}

void EditGamesDialog::fillWidget()
{
	ui->tableWidget->clear();

	ui->tableWidget->setRowCount( games->count() );

	int counter = 0;
	QMapIterator<QString, QString> it(*games);
	while( it.hasNext() ) {

		it.next();

		if( it.key().isEmpty() )
			continue;

		QTableWidgetItem* item = new QTableWidgetItem(it.key());
			item->setFlags(item->flags() & (~Qt::ItemIsEditable));

		ui->tableWidget->setItem(counter, 0, item);

		item = new QTableWidgetItem(it.value());
					item->setFlags(item->flags() & (~Qt::ItemIsEditable));

		ui->tableWidget->setItem(counter, 1, item);

		++counter;
	}
}

void EditGamesDialog::editOK()
{
	ui->tableWidget->selectRow( ui->tableWidget->findItems(toSelect, Qt::MatchExactly).at(0)->row() );
}

void EditGamesDialog::edit()
{
	QList<QTableWidgetItem*> selected = ui->tableWidget->selectedItems();

	if(selected.count() > 0) {

		EditGameDialog dialog(selected.at(0)->text(), games, this);

		connect(&dialog, SIGNAL(accepted()), this, SLOT(fillWidget()));
		connect(&dialog, SIGNAL(accepted()), this, SLOT(editOK()));
		connect(&dialog, SIGNAL(selectItem(QString)), this, SLOT(selectRow(QString)));

		dialog.show();
		dialog.exec();
	}
}

void EditGamesDialog::add()
{
	EditGameDialog dialog(games, this);

	connect(&dialog, SIGNAL(accepted()), this, SLOT(fillWidget()));
	connect(&dialog, SIGNAL(accepted()), this, SLOT(editOK()));
	connect(&dialog, SIGNAL(selectItem(QString)), this, SLOT(selectRow(QString)));

	dialog.show();
	dialog.exec();
}

void EditGamesDialog::remove()
{
    QList<QTableWidgetItem*> selected = ui->tableWidget->selectedItems();

	if(selected.count() > 0) {

		int currentRow = selected.at(0)->row();

		games->remove(selected.at(0)->text());

		fillWidget();

		if( currentRow >= ui->tableWidget->rowCount() ) {
			currentRow--;
		}

		ui->tableWidget->selectRow(currentRow);
	}
}

void EditGamesDialog::refresh()
{
	emit refreshGames();

	fillWidget();
}
