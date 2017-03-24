#include "shading-reflection.h"

using namespace utils;

namespace shadingreflection {

void insertParametersFromViews(VmtFile *vmt, Ui::MainWindow *ui)
{
	if (ui->groupBox_shadingReflection->isHidden())
		return;

	if (isChecked(ui->checkBox_cubemap)) {
		vmt->parameters.insert("$envmap", "env_cubemap");
	} else {
		const QString text = getText(ui->lineEdit_envmap);
		if (!text.isEmpty())
			vmt->parameters.insert("$envmap", text);
	}

	const QString contrast = 
		getNonDef(ui->doubleSpinBox_contrast, "0");
	if (!contrast.isEmpty())
		vmt->parameters.insert("$envmapcontrast", contrast);

	if (ui->doubleSpinBox_fresnelReflection->isEnabled()) {
		const double fresnel = 
			ui->doubleSpinBox_fresnelReflection->value();
		if (fresnel != 1.0) {
			if (vmt->shaderName == "VertexLitGeneric") {
				vmt->parameters.insert(
					"$envmapfresnel", STR(1.0 - fresnel));
			} else {
				vmt->parameters.insert(
					"$fresnelreflection", STR(fresnel));
			}
		}
	}

	const QString saturation = 
		getNonDef(ui->doubleSpinBox_saturation, "1");
	if (!saturation.isEmpty())
		vmt->parameters.insert("$envmapsaturation", saturation);

	if (ui->toolButton_envmapTint->isEnabled()) {
		const QString tint = 
			toParameterBig(utils::getBG(ui->color_envmapTint),
						   ui->doubleSpinBox_envmapTint->value());
		if (tint != "[1 1 1]")
			vmt->parameters.insert("$envmaptint", tint);
	}

	const QString mask = getText(ui->lineEdit_specmap);
	if (!mask.isEmpty())
		vmt->parameters.insert("$envmapmask", mask);

	const QString mask2 = getText(ui->lineEdit_specmap2);
	if (!mask2.isEmpty())
		vmt->parameters.insert("$envmapmask2", mask2);

	if (isChecked(ui->checkBox_basealpha))
		vmt->parameters.insert("$basealphaenvmapmask", "1");

	if( ui->checkBox_tintSpecMask->isChecked() && ui->checkBox_tintSpecMask->isVisible())
		vmt->parameters.insert( "$envmapmaskintintmasktexture", "1" );

	if (isChecked(ui->checkBox_normalalpha))
		vmt->parameters.insert("$normalmapalphaenvmapmask", "1");

	const QString light = getNonDef(ui->doubleSpinBox_envmapLight, "0");
	if (!light.isEmpty())
		vmt->parameters.insert("$envmaplightscale", light);

	if (ui->doubleSpinBox_envmapLightMin->isEnabled()) {
		const QString lMin = 
			stripZeroes(ui->doubleSpinBox_envmapLightMin);
		const QString lMax = 
			stripZeroes(ui->doubleSpinBox_envmapLightMax);
		if (lMin != "0" || lMax != "1") {
			vmt->parameters.insert("$envmaplightscaleminmax", 
				QString("[%1 %2]").arg(lMin, lMax));
		}
	}

	const QString aniso = getNonDef(ui->doubleSpinBox_envmapAniso, "0");
	if (!aniso.isEmpty()) {
		vmt->parameters.insert("$envmapanisotropy", "1");
		vmt->parameters.insert("$envmapanisotropyscale", aniso);
	}
}

} // namespace shadingreflection
