#pragma once

#include <QDialog>

class DialogWithoutHelpButton : public QDialog
{
public:
	explicit DialogWithoutHelpButton(QWidget* parent = nullptr);
};
