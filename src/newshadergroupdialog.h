#ifndef NEWSHADERGROUPDIALOG_H
#define NEWSHADERGROUPDIALOG_H

#include "dialogwithouthelpbutton.h"
#include "optionsdialog.h"

namespace Ui {

	class NewShaderGroupDialog;
}

class NewShaderGroupDialog : public DialogWithoutHelpButton
{
	Q_OBJECT
	
public:

	struct Results {

		QString shaderName;
		QString copyFrom;
	};

	NewShaderGroupDialog(QVector<Shader> customGroups, QWidget* parent = NULL);

	~NewShaderGroupDialog();

	Results getResults() const { return mResults; }
	
private:

	Ui::NewShaderGroupDialog* ui;

	QVector<Shader> mCustomGroups;

	Results mResults;

	bool containsShaderName(const QString& shaderName, const QVector<Shader>& customGroups);

public slots:

	void lineEditTextEdited(QString text);

	void comboBoxItemChanged();
};

#endif // NEWSHADERGROUPDIALOG_H
