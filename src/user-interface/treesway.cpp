#include "treesway.h"

#include "user-interface/constants.h"
#include "errors.h"
#include "logging/logging.h"
#include "utilities/strings.h"

bool treesway::hasChanged(Ui::MainWindow *ui)
{
	// simplifying default checks
	#define START return (
	#define VAL(w, s) stripZeroes(w->cleanText()) != s ||
	#define CHE(w) w->isChecked() ||
	#define COL(w) utils::getBG(w) != white ||
	#define TEX(w) w->text() != "" ||
	#define END false);

	START
	VAL(ui->spinBox_treeswayHeight, "0")
	VAL(ui->spinBox_treeswayRadius, "0")
	VAL(ui->doubleSpinBox_treeswayStartHeight, "0.0")
	VAL(ui->doubleSpinBox_treeswayStartRadius, "0.0")
	VAL(ui->spinBox_treeswayFalloff, "0")
	VAL(ui->spinBox_treeswaySpeedLerpEnd, "0")
	VAL(ui->spinBox_treeswaySpeedLerpStart, "0")
	VAL(ui->doubleSpinBox_treeswayStrength, "0.0")
	VAL(ui->doubleSpinBox_treeswaySpeed, "0.0")
	VAL(ui->doubleSpinBox_treeswayspeedHighWind, "0.0")
	VAL(ui->doubleSpinBox_treeswayScrumbleStrength, "0.0")
	VAL(ui->doubleSpinBox_treeswayScrumbleSpeed, "0.0")
	VAL(ui->spinBox_treeswayScrumbleFrequency, "0")
	VAL(ui->spinBox_treeswayScrumbleFalloff, "0")
	END
}

void treesway::resetAction(Ui::MainWindow *ui)
{
	ui->groupBox_treeSway->setVisible(false);
	ui->action_treeSway->setChecked(false);
}

void treesway::resetWidgets(Ui::MainWindow *ui)
{
	ui->spinBox_treeswayHeight->setValue(0);
	ui->spinBox_treeswayRadius->setValue(0);

	ui->doubleSpinBox_treeswayStartHeight->setValue(0.0);
	ui->doubleSpinBox_treeswayStartRadius->setValue(0.0);
	ui->spinBox_treeswayFalloff->setValue(0);

	ui->spinBox_treeswaySpeedLerpEnd->setValue(0);
	ui->spinBox_treeswaySpeedLerpStart->setValue(0);

	ui->doubleSpinBox_treeswayStrength->setValue(0.0);
	ui->doubleSpinBox_treeswaySpeed->setValue(0.0);
	ui->doubleSpinBox_treeswayspeedHighWind->setValue(0.0);

	ui->doubleSpinBox_treeswayScrumbleStrength->setValue(0.0);
	ui->doubleSpinBox_treeswayScrumbleSpeed->setValue(0.0);

	ui->spinBox_treeswayScrumbleFrequency->setValue(0);
	ui->spinBox_treeswayScrumbleFalloff->setValue(0);
}

void initializeTreesway(Ui::MainWindow *ui, VmtFile *vmt)
{
	utils::BooleanResult raw = utils::takeBoolean("$treesway", vmt, ui);
	if (!raw.present)
		return;

	vmt->state.showTreeSway = true;

	switch (vmt->shader)
	{
	case Shader::S_Deferred_Model:
		ui->groupBox_treeSway->setEnabled(true);
		ui->groupBox_treeSway->setChecked(true);
		vmt->state.treeSwayEnabled = true;
		break;
	default:
		ERROR("$treesway only works with the Deferred_Model shader");
	}
}

void treesway::parseParameters(Ui::MainWindow *ui, VmtFile *vmt)
{
	// golfing away the checks to quickly parse parameters
	// note that the DO... macros all require an additional end bracket as
	// PREP opens an if
	#define PREP(p) \
		if (vmt->parameters.contains(p)) { \
			if (!vmt->state.treeSwayEnabled) \
				ERROR(p " only works with $treesway") \
			vmt->state.showTreeSway = true;

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
	initializeTreesway(ui, vmt);

	INT("$treeswayheight", 0, ui->spinBox_treeswayHeight)
	INT("$treeswayradius", 0, ui->spinBox_treeswayRadius)
	INT("$treeswayspeedlerpend", 0, ui->spinBox_treeswaySpeedLerpEnd)
	INT("$treeswayspeedlerpstart", 0, ui->spinBox_treeswaySpeedLerpStart)
	INT("$treeswayscrumblefrequency", 0, ui->spinBox_treeswayScrumbleFrequency)
	INT("$treeswayfalloffexp", 0, ui->spinBox_treeswayFalloff)
	INT("$treeswayscrumblefalloffexp", 0, ui->spinBox_treeswayScrumbleFalloff)

	DOUBLE("$treeswaystartheight", "0.0", ui->doubleSpinBox_treeswayStartHeight)
	DOUBLE("$treeswaystartradius", "0.0", ui->doubleSpinBox_treeswayStartRadius)
	DOUBLE("$treeswaystrength", "1000.0", ui->doubleSpinBox_treeswayStrength)
	DOUBLE("$treeswayspeedhighwindmultiplier", "1000.0", ui->doubleSpinBox_treeswayspeedHighWind)
	DOUBLE("$treeswayscrumblestrength", "1000.0", ui->doubleSpinBox_treeswayScrumbleStrength)
	DOUBLE("$treeswayspeed", "1000.0", ui->doubleSpinBox_treeswaySpeed)
	DOUBLE("$treeswayscrumblespeed", "1000.0", ui->doubleSpinBox_treeswayScrumbleSpeed)


}
