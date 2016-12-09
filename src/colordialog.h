#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include "dialogwithouthelpbutton.h"

namespace Ui {

	class ColorDialog;
}

class ColorDialog : public DialogWithoutHelpButton {

	Q_OBJECT
	
public:

	explicit ColorDialog (QWidget* parent = NULL);

	~ColorDialog();
	
private:

	Ui::ColorDialog* ui;

	bool mIgnoreChange;

private slots:

	void updateColorPreview();

	void editedSpinBox();
	void editedDoubleSpinBox();
};

#endif // COLORDIALOG_H
