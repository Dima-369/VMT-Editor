#include "user-interface/detail-texture.h"

#include "view-helper.h"
#include "vmt/vmt-helper.h"
#include "logging/logging.h"

void detailtexture::reset(Ui::MainWindow *ui)
{
	ui->doubleSpinBox_detailScale->setValue(4.0);
	ui->doubleSpinBox_detailScaleY->setValue(4.0);
	ui->checkBox_detailScaleUniform->setChecked(true);

	ui->doubleSpinBox_detailScale2->setValue(4.0);
	ui->doubleSpinBox_detailScaleY2->setValue(4.0);
	ui->checkBox_detailScaleUniform2->setChecked(true);

	// we fake a change to quickly disable all widgets
	processDetailTextureChange("", ui);
}

bool detailtexture::hasChanged(Ui::MainWindow *ui)
{
	return !(ui->lineEdit_detail->text().isEmpty() &&
			 ui->comboBox_detailBlendMode->currentIndex() == 0 &&
			 utils::equal(ui->doubleSpinBox_detailScale, "4.") &&
			 utils::equal(ui->doubleSpinBox_detailScaleY, "4.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount, "1.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount2, "1.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount3, "1.") &&
			 utils::equal(ui->doubleSpinBox_detailAmount4, "1.") &&
			 ui->lineEdit_detail2->text().isEmpty() &&
			 ui->comboBox_detailBlendMode2->currentIndex() == 0 &&
			 utils::equal(ui->doubleSpinBox_detailScale2, "4.") &&
			 utils::equal(ui->doubleSpinBox_detailScaleY2, "4."));
}

void detailtexture::processDetailTextureChange(const QString &text,
		Ui::MainWindow *ui, bool second)
{
	// TODO: Are those all widgets in the groupbox or why exactly those?
	const bool enable = !text.isEmpty();

	if (!second) {
		//ui->label_5->setEnabled(enable);
		ui->comboBox_detailBlendMode->setEnabled(enable);
		ui->label_detailScale->setEnabled(enable);
		ui->label_detailAmount->setEnabled(enable);
		ui->doubleSpinBox_detailAmount->setEnabled(enable);
		ui->horizontalSlider_detailAmount->setEnabled(enable);
		ui->doubleSpinBox_detailScale->setEnabled(enable);
		ui->checkBox_detailScaleUniform->setEnabled(enable);
		if(ui->label_detailAmount3->isVisible()) {
			ui->label_detailAmount2->setEnabled(enable);
			ui->horizontalSlider_detailAmount2->setEnabled(enable);
			ui->doubleSpinBox_detailAmount2->setEnabled(enable);
		}
		ui->label_detailAmount3->setEnabled(enable);
		ui->label_detailAmount4->setEnabled(enable);

		ui->horizontalSlider_detailAmount3->setEnabled(enable);
		ui->horizontalSlider_detailAmount4->setEnabled(enable);
		ui->doubleSpinBox_detailAmount2->setEnabled(enable);
		ui->doubleSpinBox_detailAmount3->setEnabled(enable);
		ui->doubleSpinBox_detailAmount4->setEnabled(enable);

		if (!ui->checkBox_detailScaleUniform->isChecked()) {
			ui->doubleSpinBox_detailScaleY->setEnabled(enable);
		}
		ui->label_detailAmount2->setEnabled(enable);
		ui->horizontalSlider_detailAmount2->setEnabled(enable);
		ui->doubleSpinBox_detailAmount2->setEnabled(enable);

		//ui->label_detailBlendMode2->setEnabled(enable);
		ui->comboBox_detailBlendMode2->setEnabled(enable);
		ui->label_detailScale2->setEnabled(enable);
		ui->doubleSpinBox_detailScale2->setEnabled(enable);
		ui->checkBox_detailScaleUniform2->setEnabled(enable);
	} else {
		ui->label_detailAmount2->setEnabled(enable);
		ui->horizontalSlider_detailAmount2->setEnabled(enable);
		ui->doubleSpinBox_detailAmount2->setEnabled(enable);

		//ui->label_detailBlendMode2->setEnabled(enable);
		ui->comboBox_detailBlendMode2->setEnabled(enable);
		ui->label_detailScale2->setEnabled(enable);
		ui->doubleSpinBox_detailScale2->setEnabled(enable);
		ui->checkBox_detailScaleUniform2->setEnabled(enable);
	}

}

void detailtexture::toggledUniformScale(bool checked, Ui::MainWindow *ui, bool second)
{
	if (!second) {
		if (!ui->lineEdit_detail->text().isEmpty()) {
			ui->doubleSpinBox_detailScaleY->setEnabled(!checked);
		}
	} else {
		if (!ui->lineEdit_detail2->text().isEmpty()) {
			ui->doubleSpinBox_detailScaleY2->setEnabled(!checked);
		}
	}
}

QString detailtexture::param::initialize(Ui::MainWindow *ui, VmtFile *vmt, bool second)
{
	QString raw = "";
	if (second)
		raw = vmt->parameters.take("$detail2");
	else
		raw = vmt->parameters.take("$detail");

	if (!raw.isEmpty()) {
		if (vmt->parameters.contains("$seamless_scale")) {
			logging::error("$detail and $seamless_scale are not "
				"supported at the same time!", ui);
		}

		vmt->state.detailEnabled = true;
		vmt->state.showDetail = true;
	}

	return raw;
}

void processDetailScaleAsTuple(const QString &parameter, const QString &raw,
		Ui::MainWindow *ui, bool second)
{
	const utils::DoubleTuple tuple = utils::toDoubleTuple(raw, 2);

	if (tuple.valid) {
		const QString x = tuple.strings.at(0);
		const QString y = tuple.strings.at(1);
		if (x == "4." && y == "4.") {
			logging::error(parameter + " has default value: [4 4]",
				ui);
		} else if (second) {
			double xd = tuple.values.at(0);
			double yd = tuple.values.at(1);
			ui->doubleSpinBox_detailScale2->setValue(xd);
			ui->doubleSpinBox_detailScaleY2->setValue(yd);
			ui->checkBox_detailScaleUniform2->setChecked(false);
		} else {
			double xd = tuple.values.at(0);
			double yd = tuple.values.at(1);
			ui->doubleSpinBox_detailScale->setValue(xd);
			ui->doubleSpinBox_detailScaleY->setValue(yd);
			ui->checkBox_detailScaleUniform->setChecked(false);
		}
	} else {
		logging::error(parameter + " has invalid value: " + raw,
			ui);
	}
}

/*!
 * The parameter can be $detailscale 4.0 or $detailscale [4 3] and we need to
 * handle both cases correctly!
 */
void processDetailScale(const QString &parameter, const QString &value,
		Ui::MainWindow *ui, bool second)
{
	bool logErrors = false;
	const utils::DoubleResult result = utils::parseDouble(parameter, value,
			"4", ui, logErrors);

	if (result.invalid) {
		processDetailScaleAsTuple(parameter, result.string, ui, second);

	} else if (second) {
		if (result.notDefault) {
			ui->doubleSpinBox_detailScale2->setValue(result.value);
			ui->checkBox_detailScaleUniform2->setChecked(true);
			if (!ui->lineEdit_detail2->text().isEmpty()) {
				ui->doubleSpinBox_detailScaleY2->setEnabled(
					false);
			}
		} else {
			logging::error(parameter + " has default value: 4.0",
				ui);
		}
	} else {
		if (result.notDefault) {
			ui->doubleSpinBox_detailScale->setValue(result.value);
			ui->checkBox_detailScaleUniform->setChecked(true);
			if (!ui->lineEdit_detail->text().isEmpty()) {
				ui->doubleSpinBox_detailScaleY->setEnabled(
					false);
			}
		} else {
			logging::error(parameter + " has default value: 4.0",
				ui);
		}
	}
}

void detailtexture::param::parse(const detailtexture::Parameter &parameter,
	Ui::MainWindow *ui, VmtFile *vmt)
{
	#define CHECK(p, m, b) { \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.detailEnabled) { \
				ERROR(p " only works with $detail") \
			} \
			vmt->state.showDetail = true; \
			m(p, vmt->parameters.take(p), ui, b); \
		} \
		break; \
	}

	switch (parameter) {
	case detailtexture::detailscale:
		CHECK("$detailscale", processDetailScale, false);
	case detailtexture::detailscale2:
		CHECK("$detailscale2", processDetailScale, true)
	}
}
