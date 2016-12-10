#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog( QWidget* parent ) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

	ui->label_version->setTextFormat(Qt::PlainText);
    ui->label_version->setText(QString("1.0.2 - Compiled on ") + __DATE__);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
