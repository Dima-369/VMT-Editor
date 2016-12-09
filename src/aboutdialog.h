#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "dialogwithouthelpbutton.h"


namespace Ui {

	class AboutDialog;
}

class AboutDialog : public DialogWithoutHelpButton
{
	Q_OBJECT
	
public:

	explicit AboutDialog( QWidget* parent = NULL );

	~AboutDialog();
	
private:

	Ui::AboutDialog* ui;
};

#endif // ABOUTDIALOG_H
