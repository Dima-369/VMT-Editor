#include "phong.h"

#include "user-interface/constants.h"
#include "errors.h"
#include "logging/logging.h"
#include "utilities/strings.h"

void phong::initialize(Ui::MainWindow *ui)
{
	ui->toolButton_phongAmount->setStyleSheet(whiteBG);
	ui->toolButton_spec_amount2->setStyleSheet(whiteBG);
}

utils::Preview phong::toggle(Ui::MainWindow *ui)
{
	utils::Preview result;
	if (ui->groupBox_phongBrush->isVisible()) {

		if (!ui->groupBox_shadingReflection->isVisible()) {
			result.mode = GLWidget_Spec::None;
			result.texture = "";
		}

	} else {
		QCheckBox* alpha = ui->checkBox_basealpha;
		QCheckBox* nalpha = ui->checkBox_normalalpha;

		if (alpha->isEnabled() && alpha->isChecked()) {
			result.mode = GLWidget_Spec::Diffuse;
			result.texture = ui->lineEdit_diffuse->text();

		} else if (nalpha->isEnabled() && nalpha->isChecked()) {
			result.mode = GLWidget_Spec::Bumpmap;
			result.texture = ui->lineEdit_bumpmap->text();
		}
	}
	return result;
}

bool phong::hasChanged(MainWindow::GroupBoxes groupBox, Ui::MainWindow *ui)
{
	// simplifying default checks
	#define START return (
	#define VAL(w, s) stripZeroes(w->cleanText()) != s ||
	#define CHE(w) w->isChecked() ||
	#define COL(w) utils::getBG(w) != white ||
	#define TEX(w) w->text() != "" ||
	#define END false);

	if (groupBox == MainWindow::Phong) {
		START
		VAL(ui->doubleSpinBox_fresnelRangesX, "0.5")
		VAL(ui->doubleSpinBox_fresnelRangesY, "0.75")
		VAL(ui->doubleSpinBox_fresnelRangesZ, "1")
		COL(ui->toolButton_phongTint)
		CHE(ui->checkBox_albedoTint)
		VAL(ui->spinBox_exponent, "5")
		TEX(ui->lineEdit_exponentTexture)
		TEX(ui->lineEdit_phongWarp)
		VAL(ui->doubleSpinBox_boost, "1")
		VAL(ui->doubleSpinBox_albedoBoost, "1")
		CHE(ui->checkBox_exponentBaseAlpha)
		CHE(ui->checkBox_baseLuminanceMask)
		CHE(ui->checkBox_halfLambert)
		CHE(ui->checkBox_disableHalfLambert)
		END
	} else {
		// MainWindow::PhongBrush

		START
		// LightmappedGeneric shader
		VAL(ui->doubleSpinBox_phongAmount, "1")
		COL(ui->toolButton_phongAmount)
		VAL(ui->doubleSpinBox_phongAmountAlpha, "1")
		VAL(ui->spinBox_spec_exponent2, "5")
		VAL(ui->doubleSpinBox_maskContrast, "1")
		VAL(ui->doubleSpinBox_maskBrightness, "1")
		// WorldVertexTransition shader
		VAL(ui->doubleSpinBox_spec_amount2, "1")
		COL(ui->toolButton_spec_amount2)
		VAL(ui->doubleSpinBox_spec_amountAlpha2, "1")
		VAL(ui->spinBox_spec_exponent2, "5")
		VAL(ui->doubleSpinBox_spec_maskContrast2, "1")
		VAL(ui->doubleSpinBox_spec_maskBrightness2, "1")
		END
	}
}

void phong::resetAction(Ui::MainWindow *ui)
{
	ui->groupBox_phongBrush->setVisible(false);
	ui->action_phongBrush->setChecked(false);
}

void phong::resetWidgets(Ui::MainWindow *ui)
{
	// VertexLitGeneric shader
	ui->doubleSpinBox_fresnelRangesX->setValue(0.5);
	ui->doubleSpinBox_fresnelRangesY->setValue(0.75);
	ui->doubleSpinBox_fresnelRangesZ->setValue(1.0);

	ui->toolButton_phongTint->setStyleSheet(whiteBG);

	ui->checkBox_albedoTint->setChecked(false);

	ui->spinBox_exponent->setValue(5);
	ui->lineEdit_exponentTexture->clear();

	ui->lineEdit_phongWarp->clear();

	ui->doubleSpinBox_boost->setValue(1.0);
	ui->doubleSpinBox_albedoBoost->setValue(1.0);

	ui->checkBox_exponentBaseAlpha->setChecked(false);
	ui->checkBox_baseLuminanceMask->setChecked(false);
	ui->checkBox_halfLambert->setChecked(false);
	ui->checkBox_disableHalfLambert->setChecked(false);

	// LightmappedGeneric shader (WorldVertexTransition also has those!)
	ui->doubleSpinBox_phongAmount->setValue(1.0);
	ui->toolButton_phongAmount->setStyleSheet(whiteBG);
	ui->doubleSpinBox_phongAmountAlpha->setValue(1.0);

	ui->spinBox_exponent2->setValue(5);

	ui->doubleSpinBox_maskContrast->setValue(1.0);

	ui->doubleSpinBox_maskBrightness->setValue(1.0);

	ui->checkBox_phongBaseAlpha->setChecked(false);
	ui->checkBox_phongNormalAlpha->setChecked(false);

	// WorldVertexTransition shader
	ui->doubleSpinBox_spec_amount2->setValue(1.0);
	ui->toolButton_spec_amount2->setStyleSheet(whiteBG);
	ui->doubleSpinBox_spec_amountAlpha2->setValue(1.0);

	ui->spinBox_spec_exponent2->setValue(5);

	ui->doubleSpinBox_spec_maskContrast2->setValue(1.0);

	ui->doubleSpinBox_spec_maskBrightness2->setValue(1.0);
}

void initializePhong(Ui::MainWindow *ui, VmtFile *vmt)
{
	const QString parameter = "$phong";
	utils::BooleanResult raw = utils::takeBoolean(parameter, vmt, ui);

	if (!raw.present)
		return;

	vmt->state.showPhong = true;

	switch (vmt->shader)
	{
	case Shader::S_VertexLitGeneric:
	case Shader::S_Custom:
		ui->groupBox_phong->setEnabled(true);
		ui->groupBox_phong->setChecked(true);
		vmt->state.phongEnabled = true;
		break;
	case Shader::S_LightmappedGeneric:
	case Shader::S_WorldVertexTransition:
		ui->groupBox_phongBrush->setEnabled(true);
		ui->groupBox_phongBrush->setChecked(true);
		vmt->state.phongEnabled = true;
		break;
	default:
		logging::error(parameter + " only works with the " +
			"VertexLitGeneric, LightmappedGeneric and " +
			"WorldVertexTransition shaders", ui);
	}
}

void setupPhongFresnelRangesUI(Ui::MainWindow *ui,
		const utils::DoubleTuple tuple)
{
	ui->doubleSpinBox_fresnelRangesX->setValue(tuple.values.at(0));
	ui->doubleSpinBox_fresnelRangesY->setValue(tuple.values.at(1));
	ui->doubleSpinBox_fresnelRangesZ->setValue(tuple.values.at(2));
	// TODO: Adjust slider
}

void processFresnelRanges(const QString &parameter, const QString &value,
		Ui::MainWindow *ui, VmtFile vmt)
{
	if (vmt.shader != Shader::S_VertexLitGeneric &&
		vmt.shader != Shader::S_Custom) {
		logging::error(parameter + " only works with the " +
			"VertexLitGeneric shader", ui);
	}

	utils::DoubleTuple parsed = utils::toDoubleTuple(value, 3);

	if (parsed.valid) {
		bool valid = utils::isTupleBetween(parsed, 0.0, 100.0);
		if (valid) {
			setupPhongFresnelRangesUI(ui, parsed);
		} else {
			const QString error = doubleTupleBadBetween(parameter,
				value, 0.0, 100.0);
			logging::error(error, ui);
		}
	} else {
		logging::error(doubleTupleBadFormat(parameter, value), ui);
	}
}

void processExponent1(const QString &parameter, const QString &value,
		Ui::MainWindow *ui, VmtFile vmt)
{
	Q_UNUSED(parameter);

	if(vmt.parameters.contains("$phongexponenttexture"))
		ERROR("$phongexponent overrides $phongexponenttexture!")

	utils::IntResult r = utils::parseIntNoDefault(value);
	ui->spinBox_exponent->setValue(r.value);
	ui->spinBox_spec_exponent2->setValue(r.value);
	ui->spinBox_exponent2->setValue(r.value);
}

/*!
 * Processes $phongmaskcontrastbrightness or $phongmaskcontrastbrightness2 to
 * initialize the double spin boxes.
 */
void processMaskContrastBrightness(const QString &parameter,
		const QString &value, Ui::MainWindow *ui, bool first)
{
	utils::DoubleTuple r = utils::toDoubleTuple(value, 2);

	if (!r.valid) {
		ERROR(doubleTupleBadFormat(parameter, value))
		return;
	}

	bool valid = utils::isTupleBetween(r, 0.0, 100.0);
	if (valid) {
		double mc = r.values.at(0);
		double mb = r.values.at(1);
		if (first) {
			ui->doubleSpinBox_maskContrast->setValue(mc);
			ui->doubleSpinBox_maskBrightness->setValue(mb);
			ui->doubleSpinBox_spec_maskContrast2->setValue(mc);
			ui->doubleSpinBox_spec_maskBrightness2->setValue(mb);
		} else {
			ui->doubleSpinBox_spec_maskContrast2->setValue(mc);
			ui->doubleSpinBox_spec_maskBrightness2->setValue(mb);
		}
	} else {
		const QString error = doubleTupleBadBetween(parameter, value,
			0.0, 100.0);
		ERROR(error)
	}
}

/*!
 * Processes $phongamount and $phongamount2 to load the color and initialize
 * the spinboxes.
 *
 * If first is true, $phongamount is processed, otherwise $phongamount2.
 */
void processPhongAmount(const QString &parameter, const QString &value,
		Ui::MainWindow *ui, bool first)
{
	utils::DoubleTuple r = utils::toDoubleTuple(value, 4);

	if (!r.valid) {
		ERROR(doubleTupleBadFormat(parameter, value))
		return;
	}

	bool valid = utils::isTupleBetween(r, 0.0, 100.0);

	if (!valid) {
		ERROR(doubleTupleBadBetween(parameter, value, 0.0, 100.0))
		return;
	}

	foreach (double x, r.values) {
		if (x > 1.0) {
			break;
		}
	}

	float red = r.values.at(0);
	float green = r.values.at(1);
	float blue = r.values.at(2);
	float alpha = r.values.at(3);

	double max = static_cast<double>(qMax(red, qMax(green, blue)));

	red = red / max;
	green = green / max;
	blue = blue / max;

	if (first) {
		utils::applyBackgroundColor(red * 255, green * 255, blue * 255,
			ui->toolButton_phongAmount);
		ui->doubleSpinBox_phongAmount->setValue(max);
		ui->doubleSpinBox_phongAmountAlpha->setValue(alpha);
		utils::applyBackgroundColor(red * 255, green * 255, blue * 255,
			ui->toolButton_spec_amount2);
		ui->doubleSpinBox_spec_amount2->setValue(max);
		ui->doubleSpinBox_spec_amountAlpha2->setValue(alpha);
	} else {
		utils::applyBackgroundColor(red * 255, green * 255, blue * 255,
			ui->toolButton_spec_amount2);
		ui->doubleSpinBox_spec_amount2->setValue(max);
		ui->doubleSpinBox_spec_amountAlpha2->setValue(alpha);
	}
}

void phong::parseParameters(Ui::MainWindow *ui, VmtFile *vmt)
{
	// golfing away the checks to quickly parse parameters
	// note that the DO... macros all require an additional end bracket as
	// PREP opens an if
	#define PREP(p) \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.phongEnabled) \
				ERROR(p " only works with $phong") \
			vmt->state.showPhong = true;
	#define DO(p, m) { \
		PREP(p) \
			m(p, vmt->parameters.take(p), ui); \
		} \
	}
	#define DO_CHOICE(p, m, b) { \
		PREP(p) \
			m(p, vmt->parameters.take(p), ui, b); \
		} \
	}
	#define DO_WITH_VMT(p, m) { \
		PREP(p) \
			m(p, vmt->parameters.take(p), ui, *vmt); \
		} \
	}
	#define DOUBLE(p, def, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::DoubleResult r = \
				utils::parseDouble(p, v, def, ui); \
			if (r.notDefault) widget->setValue(r.value); \
		} \
	}
	#define INT(p, def, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::IntResult r = \
				utils::parseInt(p, v, def, ui); \
			if (r.notDefault) widget->setValue(r.value); \
		} \
	}
	#define INTNODEF(p, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::IntResult r = utils::parseIntNoDefault(v); \
			widget->setValue(r.value); \
		} \
	}
	#define BOOL(p, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::BooleanResult r = \
				utils::parseBoolean(p, v, ui); \
			if (r.value) widget->setChecked(true); \
		} \
	}
	#define COLOR(p, color, slider) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::applyBackgroundColor(p, v, color, slider, ui); \
		} \
	}
	#define TEXTURE(p, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::parseTexture(p, v, ui, widget, *vmt); \
		} \
	}

	initializePhong(ui, vmt);

	// VertexLitGeneric shader
	DO_WITH_VMT("$phongfresnelranges", processFresnelRanges)
	COLOR("$phongtint", ui->toolButton_phongTint,
		ui->horizontalSlider_phongTint);
	BOOL("$phongalbedotint", ui->checkBox_albedoTint)
	DO_WITH_VMT("$phongexponent", processExponent1)
	//TEXTURE("$phongexponenttexture", ui->lineEdit_exponentTexture)
	//TEXTURE("$phongwarptexture", ui->lineEdit_phongWarp)
	DOUBLE("$phongboost", "1", ui->doubleSpinBox_boost)
	DOUBLE("$phongalbedoboost", "1", ui->doubleSpinBox_albedoBoost)
	BOOL("$basemapalphaphongmask", ui->checkBox_exponentBaseAlpha)
	BOOL("$basemapluminancephongmask", ui->checkBox_baseLuminanceMask)
	BOOL("$halflambert", ui->checkBox_halfLambert)
	BOOL("$phongdisablehalflambert", ui->checkBox_disableHalfLambert)

	// LightmappedGeneric shader (WorldVertexTransition also has those!)
	DO_CHOICE("$phongamount", processPhongAmount, true)
	DO_CHOICE("$phongmaskcontrastbrightness",
		processMaskContrastBrightness, true)

	// WorldVertexTransition shader
	DO_CHOICE("$phongamount2", processPhongAmount, false)
	DO_CHOICE("$phongmaskcontrastbrightness2",
		processMaskContrastBrightness, false)
	INTNODEF("$phongexponent2", ui->spinBox_spec_exponent2)
}
