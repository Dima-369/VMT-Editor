#pragma once

#include "ui_aboutdialog.h"
#include "dialogwithouthelpbutton.h"
#include "utilities/version.h"

namespace Ui {
	class AboutDialog;
}

class AboutDialog : public DialogWithoutHelpButton
{
	Q_OBJECT
public:
	explicit AboutDialog(QWidget* parent = nullptr);
	~AboutDialog();
	
private:
	Ui::AboutDialog* ui;
};
