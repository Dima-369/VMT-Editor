#include "aboutdialog.h"

GitHubLinkLabel::GitHubLinkLabel(QWidget* parent) :
	QLabel(parent) {}


void GitHubLinkLabel::mousePressEvent(QMouseEvent* event)
{
	QDesktopServices::openUrl
		(QString("https://github.com/Gira-X/VMT-Editor"));
}

AboutDialog::AboutDialog( QWidget* parent ) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

	ui->label_version->setTextFormat(Qt::PlainText);
	ui->label_version->setText(QString("%1 - Compiled on %2")
		.arg(removeTrailingVersionZero(getCurrentVersion()))
		.arg(__DATE__));
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
