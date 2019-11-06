#include "emissive-blend.h"

#include "user-interface/constants.h"
#include "errors.h"
#include "logging/logging.h"
#include "utilities/strings.h"

bool emissiveblend::hasChanged(Ui::MainWindow *ui)
{
	// simplifying default checks
	#define START return (
	#define VAL(w, s) stripZeroes(w->cleanText()) != s ||
	#define CHE(w) w->isChecked() ||
	#define COL(w) utils::getBG(w) != white ||
	#define TEX(w) w->text() != "" ||
	#define END false);

	START
	TEX(ui->lineEdit_emissiveBlendTexture)
	TEX(ui->lineEdit_emissiveBlendBaseTexture)
	TEX(ui->lineEdit_emissiveBlendFlowTexture)
	VAL(ui->doubleSpinBox_emissiveBlendScrollX, "0.0")
	VAL(ui->doubleSpinBox_emissiveBlendScrollY, "0.0")
	VAL(ui->doubleSpinBox_emissiveBlendStrength, "1.0")
	COL(ui->toolButton_emissiveBlendTint)
	END
}

void emissiveblend::resetAction(Ui::MainWindow *ui)
{
	ui->groupBox_emissiveBlend->setVisible(false);
	ui->action_emissiveBlend->setChecked(false);
}

void emissiveblend::resetWidgets(Ui::MainWindow *ui)
{
	ui->lineEdit_emissiveBlendTexture->clear();
	ui->lineEdit_emissiveBlendBaseTexture->clear();
	ui->lineEdit_emissiveBlendFlowTexture->clear();
	ui->toolButton_emissiveBlendTint->setStyleSheet(whiteBG);
	ui->doubleSpinBox_emissiveBlendTint->setValue(1.0);
	ui->doubleSpinBox_emissiveBlendScrollX->setValue(0.0);
	ui->doubleSpinBox_emissiveBlendScrollY->setValue(0.0);
}

void initializeEmissiveBlend(Ui::MainWindow *ui, VmtFile *vmt)
{
	utils::BooleanResult raw = utils::takeBoolean("$emissiveblendenabled", vmt, ui);
	if (!raw.present)
		return;

	vmt->state.showEmissiveBlend = true;

	switch (vmt->shader)
	{
	case Shader::S_Deferred_Model:
		ui->groupBox_emissiveBlend->setEnabled(true);
		ui->groupBox_emissiveBlend->setChecked(true);
		vmt->state.emissiveBlendEnabled = true;
		break;
	default:
		ERROR("Emissive blend only works with the Deferred_Model shader");
	}
}

void processScrollVector(const QString &parameter, const QString &value,
		Ui::MainWindow *ui)
{
	const utils::DoubleTuple tuple = utils::toDoubleTuple(value, 2);

	if (tuple.valid) {
		double xd = tuple.values.at(0);
		double yd = tuple.values.at(1);
		ui->doubleSpinBox_emissiveBlendScrollX->setValue(xd);
		ui->doubleSpinBox_emissiveBlendScrollY->setValue(yd);
	} else {
		logging::error(parameter + " has invalid value: " + value,
			ui);
	}
}

void emissiveblend::parseParameters(Ui::MainWindow *ui, VmtFile *vmt, MainWindow *mainwindow)
{
	// golfing away the checks to quickly parse parameters
	// note that the DO... macros all require an additional end bracket as
	// PREP opens an if
	#define PREP(p) \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.emissiveBlendEnabled) \
				ERROR(p " only works with $emissiveblendenabled 1") \
			vmt->state.showEmissiveBlend = true;
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
			utils::applyColor(p, v, color, slider, ui); \
		} \
	}
	#define TEXTURE(p, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::parseTexture(p, v, ui, widget, *vmt); \
			mainwindow->createReconvertAction(widget, v); \
		} \
	}

	initializeEmissiveBlend(ui, vmt);

	DO("$emissiveblendscrollvector", processScrollVector)
	TEXTURE("$emissiveblendtexture", ui->lineEdit_emissiveBlendTexture)
	TEXTURE("$emissiveblendbasetexture", ui->lineEdit_emissiveBlendBaseTexture)
	TEXTURE("$emissiveblendflowtexture", ui->lineEdit_emissiveBlendFlowTexture)
	COLOR("$emissiveblendtint", ui->toolButton_emissiveBlendTint, ui->doubleSpinBox_emissiveBlendTint)
	DOUBLE("$emissiveblendstrength", "1", ui->doubleSpinBox_emissiveBlendStrength)
	DOUBLE("$bumpdetailscale2", "1", ui->doubleSpinBox_bumpdetailscale2)


}
