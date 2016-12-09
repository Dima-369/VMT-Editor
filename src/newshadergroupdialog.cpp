#include "newshadergroupdialog.h"
#include "ui_newshadergroupdialog.h"
#include "utilities.h"

NewShaderGroupDialog::NewShaderGroupDialog(QVector<Shader> customGroups, QWidget* parent) :
	DialogWithoutHelpButton(parent),
	ui(new Ui::NewShaderGroupDialog),
	mCustomGroups(customGroups)
{
	ui->setupUi(this);

	ui->label_error->hide();

	QStringList groups;

	for (int i = 0; i < mCustomGroups.count(); ++i) {

		if (mCustomGroups.at(i).groups.count() > 0)
			groups.append(mCustomGroups.at(i).name);
	}

	groups.sort();

	ui->comboBox_copyFrom->addItem("<None>");
	ui->comboBox_copyFrom->addItems(groups);

	mResults.copyFrom = "<None>";

	connect(ui->lineEdit_shaderGroupName, SIGNAL(textChanged(QString)), this, SLOT(lineEditTextEdited(QString)));

	connect(ui->comboBox_copyFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxItemChanged()));

	for (int i = 0; i < customGroups.count(); ++i) {

		// ?
	}

	if (!containsShaderName("Shader", customGroups)) {

		ui->lineEdit_shaderGroupName->setText("Shader");

	} else {

		int index = 2;

		while (true) {

			if (!containsShaderName("Shader #" + Str(index), customGroups)) {

				ui->lineEdit_shaderGroupName->setText("Shader #" + Str(index));

				break;
			}

			++index;
		}
	}
}

NewShaderGroupDialog::~NewShaderGroupDialog()
{
	delete ui;
}

bool NewShaderGroupDialog::containsShaderName(const QString& shaderName, const QVector<Shader>& customGroups)
{
	for (int i = 0; i < customGroups.count(); ++i) {

		if (customGroups.at(i).name == shaderName)
			return true;
	}

	return false;
}

void NewShaderGroupDialog::lineEditTextEdited(QString text)
{
	text = text.trimmed();

	bool error = false;

	if (text.isEmpty()) {

		ui->label_error->setText("Empty shader group name is not permitted!");

		error = true;

	} else if (text == "<None>") {

		ui->label_error->setText("Shader group name: \"<None>\" is not permitted!");

		error = true;
	}

	for (int i = 0; i < mCustomGroups.count(); ++i ) {

		if (text.toLower() == mCustomGroups.at(i).name.toLower()) {

			if (ui->label_error->text().isEmpty())
				ui->label_error->setText("Name already exists in shader groups!");

			error = true;

			break;
		}
	}

	if (!error)
		ui->label_error->setText("");

	ui->label_error->setVisible(!ui->label_error->text().isEmpty());
	ui->pushButton_ok->setDisabled(ui->label_error->isVisible());

	mResults.shaderName = text;
}

void NewShaderGroupDialog::comboBoxItemChanged()
{
	mResults.copyFrom = ui->comboBox_copyFrom->currentText();
}
