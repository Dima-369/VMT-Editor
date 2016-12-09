#ifndef EDITSHADERDIALOG_H
#define EDITSHADERDIALOG_H

#include "dialogwithouthelpbutton.h"
#include "shader.h"

#include <QSettings>
#include <QMultiMap>
#include <QListWidget>
#include <QVector>
#include <QItemDelegate>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTableView>


struct Settings;

namespace Ui
{
	class EditShaderDialog;
}

class EditShaderDialog : public DialogWithoutHelpButton
{
	Q_OBJECT
	
public:

	explicit EditShaderDialog( QWidget* parent = NULL );

	~EditShaderDialog();


	void parseSettings( const QVector<Shader>& settingShaders, const QVector<Shader>& defaultShaders );

	QVector<Shader> getShaders() const
	{
		return mChangedShaders;
	}

private:

	void loadAllowedShaderGroups();


	Ui::EditShaderDialog* ui;

	QVector<Shader> mSettingShaders;

	QVector<Shader> mChangedShaders;

	QVector<Shader> mDefaultShaders;

	bool deletingLastShaderEntry;

	QString mLastShader;

	QMap<Shader::Shaders, QVector<Shader::Groups> > mAllowedShaderGroups;

	bool mIgnoreUpdate;

	void resetCheckboxes();

	QString mSelectedShader;

public slots:

	void updateShader();

	//void adjustShader( QString oldShaderName, QString newShaderName );

private slots:

	void shaderSelectionChanged();

	void toggleShader();

	void deleteShader();

	void newShader();

	void defaultShader();

	// Processes filters like the checkbox to display custom shaders only and the search LineEdit
	void updateFilters();

signals:

	void shaderSelectionModified( QString shader );
};

#endif // EDITSHADERDIALOG_H
