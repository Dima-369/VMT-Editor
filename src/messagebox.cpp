#include "messagebox.h"

#include "utilities.h"

MsgBox::MsgBox(QWidget* parent) :
	QMessageBox(parent),
	msgBox(new QMessageBox(parent))
{
	msgBox->setStyleSheet("QPushButton{ font-family: Segoe Ui; font-size: 9pt; height: 11px;} QLabel{ font-size: 11pt; qproperty-alignment: 'AlignVCenter | AlignLeft'; } QMessageBox { background-color: #404040; }");
}

void MsgBox::setWindowTitle(const QString &title)
{
	msgBox->setWindowTitle(title);
}

void MsgBox::setStandardButtons(QMessageBox::StandardButtons buttons)
{
	msgBox->setStandardButtons(buttons);
}

QPushButton* MsgBox::addButton ( const QString & text, ButtonRole role )
{
	return msgBox->addButton(text, role);
}

QPushButton* MsgBox::addButton ( StandardButton button )
{
	return msgBox->addButton(button);
}

void MsgBox::addButton(QAbstractButton* button, ButtonRole role)
{
	msgBox->addButton(button, role);
}

void MsgBox::setDefaultButton(StandardButton button)
{
	msgBox->setDefaultButton(button);
}

void MsgBox::setDefaultButton(QPushButton* button)
{
	msgBox->setDefaultButton(button);
}

void MsgBox::setIconPixmap(const QPixmap& pixmap)
{
	msgBox->setIconPixmap(pixmap);
}

void MsgBox::setStyleSheet(const QString& styleSheet)
{
	msgBox->setStyleSheet(styleSheet);
}

void MsgBox::setIcon(QMessageBox::Icon icon)
{
	msgBox->setIcon(icon);
}

void MsgBox::setText(const QString &text)
{
	msgBox->setText(text);
}

int MsgBox::exec()
{
	return msgBox->exec();
}

QMessageBox::StandardButton MsgBox::information(QWidget *parent, const QString &title,
												const QString &text,
												QMessageBox::StandardButtons buttons,
												QMessageBox::StandardButton defaultButton)
{
	QMessageBox* staticMsg = new QMessageBox(parent);
		staticMsg->setWindowTitle(title);
		staticMsg->setText(text);
		staticMsg->setStandardButtons(buttons);
		staticMsg->setDefaultButton(defaultButton);
		staticMsg->setIcon(Information);
		staticMsg->setStyleSheet("QPushButton{    color: silver;    background-color: QLinearGradient( x1: 0, y1: 1, x2: 0, y2: 0,    stop: 0 #333, stop: 1 #404040);    border-width: 1px;    border-color: #555;    border-style: solid;    padding-top: 5px;min-width: 65px;    padding-bottom: 5px;    padding-left: 5px;    padding-right: 5px;    font-family: Segoe Ui; font-size: 9pt; height: 11px;}QPushButton:disabled{    background-color:#505050;    border-width: 1px;    border-color: #555;    border-style: solid;    padding-top: 5px;    padding-bottom: 5px;min-width: 65px;    padding-left: 5px;    padding-right: 5px;    color: #3A3939;}QPushButton:focus{border: 1px solid #78879b;}QPushButton:hover{    background-color: QLinearGradient( x1: 0, y1: 1, x2: 0, y2: 0,    stop: 0 #444, stop: 1 #505050);} QLabel{ color: silver; font-size: 12pt; } QMessageBox { background-color: #404040; }");
		staticMsg->setIconPixmap(QPixmap(":/icons/info_warning"));
		staticMsg->show();

	return (QMessageBox::StandardButton)staticMsg->exec();
}

QMessageBox::StandardButton MsgBox::warning(QWidget *parent, const QString &title,
											const QString &text,
											QMessageBox::StandardButtons buttons,
											QMessageBox::StandardButton defaultButton)
{
	QMessageBox* staticMsg = new QMessageBox(parent);
		staticMsg->setWindowTitle(title);
		staticMsg->setText(text);
		staticMsg->setStandardButtons(buttons);
		staticMsg->setDefaultButton(defaultButton);
		staticMsg->setIcon(Warning);
		staticMsg->setStyleSheet("QPushButton{    color: silver;    background-color: QLinearGradient( x1: 0, y1: 1, x2: 0, y2: 0,    stop: 0 #333, stop: 1 #404040);    border-width: 1px;    border-color: #555;    border-style: solid;    padding-top: 5px;min-width: 65px;    padding-bottom: 5px;    padding-left: 5px;    padding-right: 5px;    font-family: Segoe Ui; font-size: 9pt; height: 11px;}QPushButton:disabled{    background-color:#505050;    border-width: 1px;    border-color: #555;    border-style: solid;    padding-top: 5px;    padding-bottom: 5px;min-width: 65px;    padding-left: 5px;    padding-right: 5px;    color: #3A3939;}QPushButton:focus{border: 1px solid #78879b;}QPushButton:hover{    background-color: QLinearGradient( x1: 0, y1: 1, x2: 0, y2: 0,    stop: 0 #444, stop: 1 #505050);} QLabel{ color: silver; font-size: 12pt; } QMessageBox { background-color: #404040; }");
		staticMsg->setIconPixmap(QPixmap(":/icons/info_warning"));
		staticMsg->show();

	return (QMessageBox::StandardButton)staticMsg->exec();
}
