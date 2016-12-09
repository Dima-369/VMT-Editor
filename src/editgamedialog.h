#ifndef EDITGAMEDIALOG_H
#define EDITGAMEDIALOG_H

#include "dialogwithouthelpbutton.h"

namespace Ui {

	class EditGameDialog;
}

class EditGameDialog : public DialogWithoutHelpButton
{
	Q_OBJECT

public:

	EditGameDialog(QMap<QString, QString>* games, QWidget* parent = NULL);

	EditGameDialog(QString currentGame, QMap<QString, QString>* games, QWidget* parent = NULL);

	~EditGameDialog();

private:

	Ui::EditGameDialog* ui;

	QMap<QString, QString>* games;

	QString currentGame;

signals:

	void selectItem(QString item);

private slots:

	void checkName();

	void browsePath();
};

#endif // EDITGAMEDIALOG_H
