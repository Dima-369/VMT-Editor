#ifndef VMTPARSER_H
#define VMTPARSER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QFile>
#include <QDir>

#include "utilities.h"

/*!
 * Represents the state of the parsed VMT like if $phong is enabled to help
 * in validating other parameters quickly.
 *
 * The state is set in MainWindow::parseVMT() and should not be used outside of
 * the parsing process!
 */
struct VmtState
{
	bool phongEnabled;

	/*!
	 * True if the phong or phongbrush group box should be shown.
	 *
	 * You need to check the shader to determine which group box needs to be
	 * shown. Show the phong group box on the VertexLitGeneric shader and
	 * the phongbrush otherwise.
	 */
	bool showPhong;

	bool normalBlendEnabled;

	bool showNormalBlend;

	bool detailEnabled;

	bool showDetail;

	/*!
	 * Helper field to persist the directory of the game so textures can be
	 * validated and previewed.
	 *
	 * Should be set in the start of parseVmt(). Set it to 0 if no game is
	 * selected.
	 */
	QDir *gameDirectory;

	VmtState() :
		phongEnabled(false),
		showPhong(false),
		normalBlendEnabled(false),
		showNormalBlend(false),
		detailEnabled(false),
		showDetail(false),
		gameDirectory(0) {}
};

struct VmtFile
{
	QMap< QString, QString > parameters;

	QString shaderName;

	/*!
	 * The shader name to a converted enum for quick comparison.
	 */
	Shader::Shaders shader;

	/*!
	 * Contains the state after the VMT is processed.
	 *
	 * Aids in parameter validation.
	 */
	VmtState state;

	QString subGroups;

	// Misc.
	QString directory;
	QString fileName;
};

//----------------------------------------------------------------------------------------//

class VmtParser
{
public:

	VmtParser( QListWidget* logger );

	// The game executable directory from which the vmt files are accessed
	void setDirectory( const QString& directory );

	// Returns true if the Vmt is valid
	bool verifyVmt( const VmtFile& vmtFile ) const;

	static QString convertVmt( VmtFile vmtEntry, bool groupedParameters, bool quotesForTexture, bool useIndentation );

	void saveVmtFile( const QString& vmt, const QString& relativeFileName );

	VmtFile loadVmtFile( const QString& relativeFileName );

	VmtFile loadVmtFileSilently( const QString& relativeFileName );

	VmtFile loadPatchVmt( QFile* file, qint64 pos, int startLine );

	//----------------------------------------------------------------------------------------//

	// No intendation used
	// static QString formatSubGroups( QString subGroups );

	static QString formatSubGroups( const QString& subGroups, int intendStart );

	static QString parseSubGroups( const QString& subGroups, QString* output );

	//----------------------------------------------------------------------------------------//

	VmtFile lastVMTFile() const {

		return mLastVmtFile;
	}

	QString directory() const {

		return mWorkingDirectory;
	}

private:

	QListWidget* mLogger;

	QString mWorkingDirectory;

	static QStringList mQuoteParameters;

	VmtFile mLastVmtFile;

	static QStringList mGroups;

	//----------------------------------------------------------------------------------------//

	static QString groupParameters( QMap< QString, QString >* parameters, bool isCustomShader, bool isWaterShader, bool isPatchShader, bool quotesForTextures, bool useIndentation );

	static QString alphabeticallySortedParameters( QMap< QString, QString >* parameters, bool isPatchShader, bool quotesForTextures, bool useIndentation );
};

#endif // VMTPARSER_H
