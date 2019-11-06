#include "normal-blend.h"

#include "user-interface/constants.h"
#include "errors.h"
#include "logging/logging.h"
#include "utilities/strings.h"

utils::Preview normalblend::toggle(Ui::MainWindow *ui)
{
	utils::Preview result;
	result.texture = ui->lineEdit_bump2->text();
	return result;
}

bool normalblend::hasChanged(Ui::MainWindow *ui)
{
	// simplifying default checks
	#define START return (
	#define VAL(w, s) stripZeroes(w->cleanText()) != s ||
	#define CHE(w) w->isChecked() ||
	#define COL(w) utils::getBG(w) != white ||
	#define TEX(w) w->text() != "" ||
	#define END false);

	START
	TEX(ui->lineEdit_bump2)
	VAL(ui->doubleSpinBox_bm_scaleX_2, "1.0")
	VAL(ui->doubleSpinBox_bm_scaleY_2, "1.0")
	VAL(ui->doubleSpinBox_bm_angle_2, "0")
	VAL(ui->doubleSpinBox_bm_centerX_2, "0.5")
	VAL(ui->doubleSpinBox_bm_centerY_2, "0.5")
	VAL(ui->doubleSpinBox_bm_translateX_2, "0")
	VAL(ui->doubleSpinBox_bm_translateY_2, "0")
	END
}

void normalblend::resetAction(Ui::MainWindow *ui)
{
	ui->groupBox_normalBlend->setVisible(false);
	ui->action_normalBlend->setChecked(false);
}

void normalblend::resetWidgets(Ui::MainWindow *ui)
{
	ui->lineEdit_bump2->clear();
	ui->doubleSpinBox_bm_scaleX_2->setValue(1.0);
	ui->doubleSpinBox_bm_scaleY_2->setValue(1.0);
	ui->doubleSpinBox_bm_angle_2->setValue(0.0);

	ui->doubleSpinBox_bm_centerX_2->setValue(0.5);
	ui->doubleSpinBox_bm_centerY_2->setValue(0.5);
	ui->doubleSpinBox_bm_translateX_2->setValue(0.0);
	ui->doubleSpinBox_bm_translateY_2->setValue(0.0);

	ui->doubleSpinBox_bumpdetailscale->setValue(1.0);
	ui->doubleSpinBox_bumpdetailscale2->setValue(1.0);
}

void initializeNormalBlend(Ui::MainWindow *ui, VmtFile *vmt)
{
	utils::BooleanResult raw = utils::takeBoolean("$addbumpmaps", vmt, ui);
	if (!raw.present)
		return;

	vmt->state.showNormalBlend = true;

	switch (vmt->shader)
	{
	case Shader::S_Deferred_Brush:
		ui->groupBox_normalBlend->setEnabled(true);
		ui->groupBox_normalBlend->setChecked(true);
		vmt->state.normalBlendEnabled = true;
		break;
	default:
		ERROR("addbumpmaps only works with the Deferred_Brush shader");
	}
}

void processBumpTransform2(const QString &parameter, const QString &value,
		Ui::MainWindow *ui)
{
	// format is "center 0 0 scale 1 1 rotate 0 translate 0 0"
	QStringList parts = value.split(' ');
	if (parts.length() != 11 || parts[0] != "center" || parts[3] != "scale"
			|| parts[6] != "rotate" || parts[8] != "translate") {
		ERROR(parameter + " has incorrect value: " + value);
		return;
	}

	// removing the center, scale, rotate and translate strings
	parts.removeAt(0);
	parts.removeAt(2);
	parts.removeAt(4);
	parts.removeAt(5);
	const bool valid = utils::isBetween(parts, 0.0, 100.0);
	if (!valid) {
		ERROR(parameter + " has values outside [0.0, 100)");
		return;
	}

	const QString wrapped = "[" + parts.join(' ') + "]";
	const utils::DoubleTuple r = utils::toDoubleTuple(wrapped, 7);
	if (!r.valid) {
		ERROR(parameter + " has incorrect value: " + value);
		return;
	}

	ui->doubleSpinBox_bm_centerX_2->setValue(r.values.at(0));
	ui->doubleSpinBox_bm_centerY_2->setValue(r.values.at(1));
	ui->doubleSpinBox_bm_scaleX_2->setValue(r.values.at(2));
	ui->doubleSpinBox_bm_scaleY_2->setValue(r.values.at(3));
	ui->doubleSpinBox_bm_angle_2->setValue(r.values.at(4));
	ui->doubleSpinBox_bm_translateX_2->setValue(r.values.at(5));
	ui->doubleSpinBox_bm_translateY_2->setValue(r.values.at(6));
}

void normalblend::parseParameters(Ui::MainWindow *ui, VmtFile *vmt)
{
	// golfing away the checks to quickly parse parameters
	// note that the DO... macros all require an additional end bracket as
	// PREP opens an if
	#define PREP(p) \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.normalBlendEnabled) \
				ERROR(p " only works with $addbumpmaps") \
			vmt->state.showNormalBlend = true;
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

	initializeNormalBlend(ui, vmt);

	DO("$bumptransform2", processBumpTransform2)
	TEXTURE("$bumpmap2", ui->lineEdit_bump2)
	DOUBLE("$bumpdetailscale1", "1", ui->doubleSpinBox_bumpdetailscale)
	DOUBLE("$bumpdetailscale2", "1", ui->doubleSpinBox_bumpdetailscale2)
}
