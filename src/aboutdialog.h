#pragma once

#include <QWidget>
#include <QDesktopServices>
#include <QLabel>


// this needs to be placed above the ui_aboutdialog.h include
// or (maybe better) in a different file
class GitHubLinkLabel : public QLabel
{
	Q_OBJECT
public:
	explicit GitHubLinkLabel(QWidget* parent = nullptr);

protected:
	void mousePressEvent(QMouseEvent* event);
};


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
	explicit AboutDialog(QWidget* parent = NULL);
	~AboutDialog();
	
private:
	Ui::AboutDialog* ui;
};
