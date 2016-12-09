#ifndef EDITGAMESDIALOG_H
#define EDITGAMESDIALOG_H

#include <QSettings>

#include "dialogwithouthelpbutton.h"

namespace Ui {

	class EditGamesDialog;
}

class EditGamesDialog : public DialogWithoutHelpButton
{
	Q_OBJECT

public:

	EditGamesDialog( QMap<QString, QString>* games, QSettings* settings, QWidget* parent = NULL );

	~EditGamesDialog();

private:

	Ui::EditGamesDialog* ui;

	QSettings* settings;

	QMap<QString, QString>* games;

	QString toSelect;

signals:

    void refreshGames();

public slots:

	void selectRow(QString game);

private slots:

	void edit();

	void add();

	void remove();

	void refresh();

	void fillWidget();

	void editOK();
};

#endif // EDITGAMESDIALOG_H
