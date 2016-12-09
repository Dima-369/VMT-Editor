#include "colordialog.h"
#include "ui_colordialog.h"

ColorDialog::ColorDialog (QWidget* parent) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::ColorDialog),
	mIgnoreChange(false)
{
	ui->setupUi(this);

	updateColorPreview();
}

ColorDialog::~ColorDialog()
{
	delete ui;
}

void ColorDialog::updateColorPreview()
{
	ui->plainTextEdit_colorPreview->setStyleSheet( QString("background-color: rgb(%1, %2, %3)")
        .arg(ui->spinBox_red->value()   < 0 ? 0 : ui->spinBox_red->value()   > 255 ? 255 : ui->spinBox_red->value())
		.arg(ui->spinBox_green->value() < 0 ? 0 : ui->spinBox_green->value() > 255 ? 255 : ui->spinBox_green->value())
        .arg(ui->spinBox_blue->value()  < 0 ? 0 : ui->spinBox_blue->value()  > 255 ? 255 : ui->spinBox_blue->value()) );
}

void ColorDialog::editedSpinBox()
{
	if (!mIgnoreChange) {

		mIgnoreChange = true;

        ui->doubleSpinBox_red->setValue(static_cast<double>(ui->spinBox_red->value())     / 255.0);
		ui->doubleSpinBox_green->setValue(static_cast<double>(ui->spinBox_green->value()) / 255.0);
        ui->doubleSpinBox_blue->setValue(static_cast<double>(ui->spinBox_blue->value())   / 255.0);
	}


	mIgnoreChange = false;
}

void ColorDialog::editedDoubleSpinBox()
{
	if (!mIgnoreChange) {

		mIgnoreChange = true;

        ui->spinBox_red->setValue(static_cast<int>(ui->doubleSpinBox_red->value()     * 255.0));
		ui->spinBox_green->setValue(static_cast<int>(ui->doubleSpinBox_green->value() * 255.0));
        ui->spinBox_blue->setValue(static_cast<int>(ui->doubleSpinBox_blue->value()   * 255.0));

	}

	mIgnoreChange = false;
}
