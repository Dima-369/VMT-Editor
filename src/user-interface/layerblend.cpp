#include "layerblend.h"

#include "user-interface/constants.h"
#include "errors.h"
#include "logging/logging.h"
#include "utilities/strings.h"

bool layerblend::hasChanged(Ui::MainWindow *ui)
{
	// simplifying default checks
	#define START return (
	#define VAL(w, s) stripZeroes(w->cleanText()) != s ||
	#define CHE(w) w->isChecked() ||
	#define COL(w) utils::getBG(w) != white ||
	#define TEX(w) w->text() != "" ||
	#define END false);

	START
	VAL(ui->doubleSpinBox_layer1tint, "1.0")
	COL(ui->color_layer1tint)
	VAL(ui->doubleSpinBox_layer2tint, "1.0")
	COL(ui->color_layer2tint)
	CHE(ui->checkBox_newLayerBlend)
	VAL(ui->doubleSpinBox_layerBlendSoftness, "0.5")
	VAL(ui->doubleSpinBox_layerBorderOffset, "0.0")
	VAL(ui->doubleSpinBox_layerBorderSoftness, "0.5")
	VAL(ui->doubleSpinBox_layerBorderStrength, "0.5")
	VAL(ui->doubleSpinBox_layerBorderTint, "1.0")
	COL(ui->color_layerBorderTint)
	CHE(ui->checkBox_layerEdgeNormal)
	CHE(ui->checkBox_layerEdgePunchin)
	VAL(ui->doubleSpinBox_layerEdgeSoftness, "0.5")
	VAL(ui->doubleSpinBox_layerEdgeStrength, "0.5")
	VAL(ui->doubleSpinBox_layerEdgeOffset, "0.0")
	END
}

void layerblend::resetAction(Ui::MainWindow *ui)
{
	ui->groupBox_layerblend->setVisible(false);
	ui->action_layerBlend->setChecked(false);
}

void layerblend::resetWidgets(Ui::MainWindow *ui)
{
	ui->doubleSpinBox_layer1tint->setValue(1.0);
	ui->doubleSpinBox_layer2tint->setValue(1.0);
	ui->color_layer1tint->setStyleSheet(whiteBG);
	ui->color_layer2tint->setStyleSheet(whiteBG);

	ui->checkBox_newLayerBlend->setChecked(false);
	ui->doubleSpinBox_layerBlendSoftness->setValue(0.5);

	ui->doubleSpinBox_layerBorderOffset->setValue(0);
	ui->doubleSpinBox_layerBorderSoftness->setValue(0.5);
	ui->doubleSpinBox_layerBorderStrength->setValue(0.5);
	ui->doubleSpinBox_layerBorderTint->setValue(1.0);
	ui->color_layerBorderTint->setStyleSheet(whiteBG);

	ui->checkBox_layerEdgeNormal->setChecked(false);
	ui->checkBox_layerEdgePunchin->setChecked(false);

	ui->doubleSpinBox_layerEdgeOffset->setValue(0.0);
	ui->doubleSpinBox_layerEdgeSoftness->setValue(0.5);
	ui->doubleSpinBox_layerEdgeStrength->setValue(0.5);

}

void initializeLayerblend(Ui::MainWindow *ui, VmtFile *vmt)
{
	switch (vmt->shader)
	{
	case Shader::S_WorldVertexTransition:
		ui->groupBox_layerblend->setEnabled(true);
		ui->groupBox_layerblend->setChecked(true);
		vmt->state.layerBlendEnabled = true;
		break;
	default:
		break;
	}
}

void layerblend::parseParameters(Ui::MainWindow *ui, VmtFile *vmt)
{
	// golfing away the checks to quickly parse parameters
	// note that the DO... macros all require an additional end bracket as
	// PREP opens an if
	#define PREP(p) \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.layerBlendEnabled) \
				ERROR(p " only works with WorldVertexTransition shader") \
			vmt->state.showLayerBlend = true;

	#define DOUBLE(p, def, widget) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::DoubleResult r = \
				utils::parseDouble(p, v, def, ui); \
			if (r.notDefault) widget->setValue(r.value); \
		} \
	}
	#define COLOR(p, color, spinbox) { \
		PREP(p) \
			const QString &v = vmt->parameters.take(p); \
			utils::applyColor(p, v, color, spinbox, ui); \
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
	initializeLayerblend(ui, vmt);

	COLOR("$layertint1", ui->color_layer1tint, ui->doubleSpinBox_layer1tint)
	COLOR("$layertint2", ui->color_layer2tint, ui->doubleSpinBox_layer2tint)
	COLOR("$layerbordertint", ui->color_layerBorderTint, ui->doubleSpinBox_layerBorderTint)
	BOOL("$newlayerblending", ui->checkBox_newLayerBlend)
	//BOOL("$layeredgenormal", ui->checkBox_layerEdgeNormal)
	//BOOL("$layeredgepunchin", ui->checkBox_layerEdgePunchin)
	DOUBLE("$blendsoftness", "0.5", ui->doubleSpinBox_layerBlendSoftness)
	DOUBLE("$layerborderoffset", "0.0", ui->doubleSpinBox_layerBorderOffset)
	DOUBLE("$layerbordersoftness", "0.5", ui->doubleSpinBox_layerBorderSoftness)
	DOUBLE("$layerborderstrength", "0.5", ui->doubleSpinBox_layerBorderStrength)
	//DOUBLE("$layeredgeoffset", "0.0", ui->doubleSpinBox_layerEdgeOffset)
	//DOUBLE("$layeredgesotfness", "0.5", ui->doubleSpinBox_layerEdgeSoftness)
	//DOUBLE("$layeredgestrength", "0.5", ui->doubleSpinBox_layerEdgeStrength)


}
