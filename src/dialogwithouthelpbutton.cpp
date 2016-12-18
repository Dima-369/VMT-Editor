#include "dialogwithouthelpbutton.h"

#include "utilities.h"

DialogWithoutHelpButton::DialogWithoutHelpButton(QWidget* parent) :
	QDialog(parent)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}
