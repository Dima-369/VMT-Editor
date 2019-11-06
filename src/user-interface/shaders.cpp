#include "shaders.h"

void handleGroupBoxAndActions(Ui::MainWindow *ui, const QString &shader)
{
	if (shader == "infected" || shader == "Deferred_Model") {
		ui->action_phong->setVisible(true);
		ui->action_phong->setEnabled(true);

		ui->action_phongBrush->setEnabled(false);
		ui->action_phongBrush->setChecked(false);
		ui->groupBox_phongBrush->setVisible(false);

		ui->action_rimLight->setEnabled(true);
		ui->action_rimLight->setEnabled(true);

	}  else if (shader == "Deferred_Brush" ||
			shader == "worldvertextransition") {
		ui->action_phong->setEnabled(false);
		ui->action_phong->setChecked(false);
		ui->groupBox_phong->setVisible(false);

		ui->action_phongBrush->setVisible(true);
		ui->action_phongBrush->setEnabled(true);

		ui->action_rimLight->setVisible(true);
		ui->action_rimLight->setEnabled(false);
		ui->action_rimLight->setChecked(false);
		ui->groupBox_rimLight->setVisible(false);

	} else {
		ui->action_phong->setEnabled(false);
		ui->action_phong->setChecked(false);
		ui->groupBox_phong->setVisible(false);

		ui->action_phongBrush->setEnabled(false);
		ui->action_phongBrush->setChecked(false);
		ui->groupBox_phongBrush->setVisible(false);

		ui->action_rimLight->setVisible(true);
		ui->action_rimLight->setEnabled(false);
		ui->action_rimLight->setChecked(false);
		ui->groupBox_rimLight->setVisible(false);
	}
}

void shaders::handlePhongBrushRim(Ui::MainWindow *ui, const QString &shader)
{
	handleGroupBoxAndActions(ui, shader);

	// we hide all amount 2 / glossiness 2 / ... UI widgets on
	// the Deferred_Brush shader and unhide otherwise
	bool hide = shader != "Deferred_Brush";
	ui->label_spec_amount2->setVisible(hide);
	ui->horizontalSlider_spec_amount2->setVisible(hide);
	ui->doubleSpinBox_spec_amount2->setVisible(hide);
	ui->toolButton_spec_amount2->setVisible(hide);
	ui->toolButton_spec_amount2->setVisible(hide);
	ui->doubleSpinBox_spec_amountAlpha2->setVisible(hide);

	ui->label_spec_exponent2->setVisible(hide);
	ui->horizontalSlider_spec_exponent2->setVisible(hide);
	ui->spinBox_spec_exponent2->setVisible(hide);

	ui->label_spec_maskContrast2->setVisible(hide);
	ui->horizontalSlider_spec_maskContrast2->setVisible(hide);
	ui->doubleSpinBox_spec_maskContrast2->setVisible(hide);

	ui->label_spec_maskBrightness2->setVisible(hide);
	//ui->horizontalSlider_spec_maskBrightness2->setVisible(hide);
	ui->doubleSpinBox_spec_maskBrightness2->setVisible(hide);
}
