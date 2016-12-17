#include "aboutdialog.h"

AboutDialog::AboutDialog( QWidget* parent ) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

	ui->label_version->setTextFormat(Qt::PlainText);

	ui->label_version->setText(QString("%1 - Compiled on %2")
		.arg(to_string(fetch_version())).arg(__DATE__));
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
