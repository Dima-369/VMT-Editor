#include "vmtparser.h"

#include "utilities.h"

#include <QMimeData>


QStringList VmtParser::mQuoteParameters;
QStringList VmtParser::mGroups;

VmtParser::VmtParser( QListWidget* logger ) :
	mLogger(logger)
{
	mQuoteParameters = extractLines(":/files/quoteParameters");

	// Used for nice VMT formatting, the order here is important

	mGroups.append( "$basetexture;$texture2;$bumpmap;$surfaceprop;$ssbump;$texture1_lumstart;$texture1_lumend" );
	mGroups.append( "$basetexture2;$bumpmap2;$surfaceprop2;$texture2_uvscale;$texture2_lumstart;$texture2_lumend;$texture2_blendstart;$texture2_blendend;$lumblendfactor2" );

	mGroups.append( "$basetexture3;$texture3_uvscale;$texture3_lumstart;$texture3_lumend;$texture3_blendstart;$texture3_blendend;$lumblendfactor3" );
	mGroups.append( "$basetexture4;$texture4_uvscale;$texture4_lumstart;$texture4_lumend;$texture4_blendstart;$texture4_blendend;$lumblendfactor4" );
	mGroups.append( "$seamless_scale;$lightwarptexture;$reflectivity;$reflectivity2");
	mGroups.append( "$detail;$detailblendmode;$detailscale;$detailblendfactor;$detail2;$detailblendmode2;$detailscale2;$detailblendfactor2;$detailblendfactor3;$detailblendfactor4" );
	mGroups.append( "$decaltexture;$decalblendmode" );
	mGroups.append( "$blendmodulatetexture;$alpha;$translucent;$alphatest;$alphatestreference;$additive;$decal;$nocull" );

	mGroups.append( "$envmap;$envmapmask;$envmapmask2;$normalmapalphaenvmapmask;$basealphaenvmapmask;$envmaptint;$envmapcontrast;$envmapsaturation;$envmaptint;$envmapfresnel;$envmapfresnelminmaxexp;$fresnelreflection;$envmaplightscale;$envmaplightscaleminmax;$envmapanisotropy;$envmapanisotropyscale" );
	mGroups.append( "$phong;$phongexponenttexture;$basemapluminancephongmask;$phongfresnelranges;$phongwarptexture;$phongtint;$halflambert;$phongdisablehalflambert;$phongalbedotint;$phongexponent;$phongboost;$phongalbedoboost;$basemapalphaphongmask;$phongamount;$phongmaskcontrastbrightness;$phongexponent2;$phongamount2;$phongmaskcontrastbrightness2" );
	mGroups.append( "$rimlight;$rimlightexponent;$rimlightboost;$rimlightmask");
	mGroups.append( "$selfillum;$selfillum_envmapmask_alpha;$selfillumfresnelminmaxexp;$selfillummask;$selfillumtint" );

	// Needed to allow a proper grouping of $normalmap in the Water and Refract Shaders
	mGroups.append( "");

	mGroups.append( "$abovewater;$forceexpensive;$forcecheap;$reflect2dskybox" );
	mGroups.append( "$scroll1;$scroll2");
	mGroups.append( "$flow_debug;$flowmap;$flow_noise_texture;$flow_normaluvscale;$flow_worlduvscale;$flow_uvscrolldistance;$flow_bumpstrength;$flow_noise_scale;$flow_timescale;$flow_timeintervalinseconds" );
	mGroups.append( "$reflecttexture;$reflecttint;$reflectamount;$reflectentities;$reflectskyboxonly;$reflectonlymarkedentities" );
	mGroups.append( "$refracttexture;$refract;$refracttinttexture;$refracttint;$refractamount;$bluramount;$refractblur" );
	mGroups.append( "$fogenable;$fogcolor;$fogstart;$fogend;$flashlighttint;$lightmapwaterfog" );

	mGroups.append( "$addbumpmaps;$bumpdetailscale1;$bumpdetailscale2;$bumptransform2" );
	mGroups.append( "$basetexturetransform;$bumptransform" );
	mGroups.append( "$vertexcolor;$vertexalpha;$nodecal;$ignorez;$nofog;$nolod;$no_fullbright;$disablecsmlookup" );
	mGroups.append( "$color;$color2;$blendtintbybasealpha;$tintmasktexture;$envmapmaskintintmasktexture" );
	mGroups.append( "$spriteorientation;$spriteorigin" );
	mGroups.append( "$treesway;$treeswayheight;$treeswaystartheight;$treeswayradius;$treeswaystartradius;$treeswaystrength;$treeswayspeed;$treeswayfalloffexp;$treeswayscrumblestrength;$treeswayscrumblespeed;$treeswayscrumblefrequency;$treeswayscrumblefalloffexp;$treeswayspeedhighwindmultiplier;$treeswayspeedlerpstart;$treeswayspeedlerpend" );

	mGroups.append( "%compilewater;%tooltexture;%keywords" );
}

// The game executable directory from which the vmt files are accessed
void VmtParser::setDirectory( const QString& directory )
{
	if( directory.isEmpty() )
	{
		Info("Parameter: \"directory\" is empty!\n" );

		mWorkingDirectory = "";
	}
	else
	{
		if (!QDir(directory).exists())
			fatalError("Directory specified does not exist!");


		if( directory.endsWith('/') )
			mWorkingDirectory = directory;
		else
			mWorkingDirectory = directory + '/';

	}
}

// Returns true if the Vmt is valid
bool VmtParser::verifyVmt( const VmtFile& vmtFile ) const
{
	if( !gShaders.contains( vmtFile.shaderName, Qt::CaseInsensitive ))
		return false;

	if( vmtFile.shaderName.compare("Patch") == 0 )
	{
		if( !vmtFile.parameters.contains("include") )
			return false;
	}

	return true;
}

QString VmtParser::convertVmt( VmtFile vmtEntry, bool groupedParameters, bool quotesForTexture, bool useIndentation ) {

	// Quotes are required, but can be turned off, even if there is no whitespace in the value, for VTFEdit to correctly browse to the VTF

	QString out( vmtEntry.shaderName + "\n{\n" );
	QString includeValue( vmtEntry.parameters.take("include") );

	if( vmtEntry.shaderName == "Patch" ) {

		if( !includeValue.isEmpty() ) {

			if(useIndentation)
				out.append( addTabs(1) + "include \"" + includeValue + "\"\n\n" + addTabs(1) + "insert\n\t{\n" );
			else
				out.append( "include \"" + includeValue + "\"\n\ninsert\n{\n");

		} else {

			return out.append("}");
		}
	}

	VmtFile tmp(vmtEntry);
	QString parameters;

	if(groupedParameters) {

		parameters = groupParameters( &tmp.parameters,
									  !gShaders.contains(vmtEntry.shaderName, Qt::CaseInsensitive),
									  vmtEntry.shaderName.compare("Water") == 0,
									  vmtEntry.shaderName.compare("Patch") == 0,
									  quotesForTexture,
									  useIndentation );


		out.append(parameters);
		bool moreParameters = false;

		QMap<QString, QString>::const_iterator it = tmp.parameters.constBegin();
		while( it != tmp.parameters.constEnd() ) {

			moreParameters = true;

			QString indentation = (useIndentation ? addTabs( vmtEntry.shaderName.compare("Patch") == 0 ? 2 : 1 ) : "");

			if(quotesForTexture) {

				if( it.value().contains(QRegExp(R"([\s\\\/]+)")) )
					out.append( indentation + it.key() + " \"" + it.value() + "\"\n" );
				else
					out.append( indentation + it.key() + " " + it.value() + "\n" );

			} else {

					out.append( indentation + "\"" + it.key() + "\" \"" + it.value() + "\"\n" );
			}

			/*if(useIndentation) {

				if( vmtEntry.shaderName.compare("Patch") != 0 )
					out.append( addTabs(1) + it.key() + " " + it.value() + "\n" );
				else
					out.append( addTabs(2) + it.key() + " " + it.value() + "\n" );

			} else {

				out.append( it.key() + " " + it.value() + "\n" );
			}*/

			++it;
		}

		if( !moreParameters && vmtEntry.parameters.size() > 0 )
			out.chop(1);

	} else {

		parameters = alphabeticallySortedParameters( &tmp.parameters,
													 vmtEntry.shaderName.compare("Patch") == 0,
													 quotesForTexture,
													 useIndentation );
		out.append(parameters);
	}

	if( !vmtEntry.subGroups.isEmpty() ) {

		if( vmtEntry.shaderName != "Patch" )
			out.append( "\n" + formatSubGroups( vmtEntry.subGroups, 1 ) + "\n" );
		else
			out.append( "\n" + formatSubGroups( vmtEntry.subGroups, 2 ) + "\n" );
	}

	if( vmtEntry.shaderName == "Patch" )
		out.append( (useIndentation ? "\t}\n}" : "}\n}") );
	else
		out.append("}");

	return out;
}

QString VmtParser::alphabeticallySortedParameters( QMap< QString, QString >* parameters, bool isPatchShader, bool quotesForTextures, bool useIndentation ) {

	QString output;

	QMap< QString, QString >::const_iterator it = parameters->constBegin();
	while( it != parameters->constEnd() ) {

		if( !it.value().isEmpty() ) {

			if(quotesForTextures) {

				if(useIndentation) {

					if( it.value().contains(QRegExp(R"([\s\\\/]+)")) || mQuoteParameters.contains( it.key(), Qt::CaseInsensitive ))
						output.append( addTabs( isPatchShader ? 2 : 1 ) + it.key() + " \"" + it.value() + "\"\n" );
					else
						output.append( addTabs( isPatchShader ? 2 : 1 ) + it.key() + " " + it.value() + "\n" );

				} else {

					if( it.value().contains(QRegExp(R"([\s\\\/]+)")) || mQuoteParameters.contains( it.key(), Qt::CaseInsensitive ))
						output.append( it.key() + " \"" + it.value() + "\"\n" );
					else
						output.append( it.key() + " " + it.value() + "\n" );
				}

			} else {

				if(useIndentation) {

					/*if( it.value().contains( " " ) )
						output.append( addTabs( isPatchShader ? 2 : 1 ) + it.key() + " \"" + it.value() + "\"\n" );
					else*/
						output.append( addTabs( isPatchShader ? 2 : 1 ) + "\"" + it.key() + "\" \"" + it.value() + "\"\n" );

				} else {

					/*if( it.value().contains( " " ) )
						output.append( it.key() + " \"" + it.value() + "\"\n" );
					else*/
						output.append( "\"" + it.key() + "\" \"" + it.value() + "\"\n" );
				}
			}
		}

		++it;
	}

	return output;
}

QString VmtParser::groupParameters( QMap< QString, QString >* parameters, bool isCustomShader, bool isWaterShader, bool isPatchShader, bool quotesForTextures, bool useIndentation )
{
	// Needed for $normalmap because it is found in the Water and Refract shader
	mGroups[12] = (isWaterShader || isCustomShader) ?
				"$normalmap;$bottommaterial;$bumpframe" :
				"$refract;$normalmap;$normalmap2;$refracttinttexture;$refracttint;$refractamount;$bluramount";

	QString output;

	for( int i = 0; i < mGroups.size(); ++i ) {

		QStringList groupParameters( mGroups.at(i).split(";") );
		bool groupEdited = false;

		for( int j = 0; j < groupParameters.size(); ++j ) {

			QString value = parameters->take( groupParameters.at(j) );

			if( !value.isEmpty() ) {

				QString indentation = (useIndentation ? addTabs( isPatchShader ? 2 : 1 ) : "");

				if(quotesForTextures) {

					if( value.contains(QRegExp(R"([\s\\\/]+)")) || mQuoteParameters.contains( groupParameters.at(j), Qt::CaseInsensitive ))
						output.append( indentation + groupParameters.at(j) + " \"" + value + "\"\n" );
					else
						output.append( indentation + groupParameters.at(j) + " " + value + "\n" );

				} else {

					/*if( value.contains( " " ) )
						output.append( indentation + groupParameters.at(j) + " \"" + value + "\"\n" );
					else*/
						output.append( indentation + "\"" + groupParameters.at(j) + "\" \"" + value + "\"\n" );
				}

				groupEdited = true;
			}
		}

		if(groupEdited)
			output.append("\n");
	}

	return output;
}

void VmtParser::saveVmtFile( const QString& vmt, const QString& relativeFileName, bool isTemp )
{
	if( relativeFileName.isEmpty() )
	{
		Info("\"relativeFileName\" is empty!")
	}
	else
	{
		QFile vmtFile( mWorkingDirectory + relativeFileName );

		if( !vmtFile.open( QIODevice::WriteOnly | QIODevice::Text ))
		{
			Info("\"" + relativeFileName + "\" failed to be opened in WriteOnly mode!" );
		}
		else
		{
			QTextStream out(&vmtFile);

			out << vmt;

			vmtFile.close();

			if(!isTemp) {
				mLastVmtFile.fileName = QFileInfo(vmtFile).fileName();
				mLastVmtFile.directory = QFileInfo(vmtFile).absolutePath();
			}
		}
	}
}

VmtFile VmtParser::loadVmtFile( const QString& relativeFileName, bool isTemplate )
{
	if( relativeFileName.isEmpty() )
	{
		Info("relativeFileName\" is empty!")
	}
	else
	{
		QFile vmtFile( mWorkingDirectory + relativeFileName );

		// QIODevice::Text:  Converting "\r\n" to "\n" (Not really needed)
		if( !vmtFile.open(QIODevice::ReadOnly | QIODevice::Text) )
		{
			Info(relativeFileName + "\" failed to be opened in ReadOnly mode!" );
		}
		else
		{
			QString subGroups;

			bool inShader = false;
			bool inSubBlock = false;

			bool expectingShaderName = true;
			bool expectingOpenBracket = false;
			bool expectingEOF = false;

			// Hack for allowing shader name lines like (but only those): VertexLitGeneric {
			bool hack = false;

			uint shaderBracketCounter = 0;
			uint subShaderBracketCounter = 0;

			VmtFile vmtEntry;

			QTextStream in( &vmtFile );
			for( uint lineCount = 1; !in.atEnd(); ++lineCount)
			{
				QString line = in.readLine();
				line = line.simplified(); // Removing whitespace characters

				if(line.isEmpty())
					continue;

				// Removing comments which start with //
				removeSingleLineComment(line);

				line.remove("\""); // Removing double quote characters for easier parsing

				line = line.simplified(); // Removing whitespace characters... again

				if(line.isEmpty())
					continue;

				//////////////////////////////

				if( line.endsWith(" {") )
				{
					hack = true;

					line.chop(2);
				}

				if(expectingEOF)
				{
					Info("Line " + Str(lineCount) + " (" + line + ": Expected end of file!")
					continue;
				}

				//////////////////////////////

				if( line == "{" )
				{
					hackGoto:
					hack = false;

					if(expectingShaderName)
					{
						Info("Line " + Str(lineCount) + ": Expected shader name!")
					}
					else if(expectingOpenBracket)
					{
						if(inShader)
						{
							inSubBlock = true;

							++subShaderBracketCounter;
							expectingOpenBracket = false;
						}
						else
						{
							if( vmtEntry.shaderName == "Patch" )
							{
								vmtEntry = loadPatchVmt( &vmtFile, in.pos(), lineCount );
								vmtEntry.shaderName = "Patch";

								vmtFile.close();

								QFileInfo fileInfo(vmtFile);

								vmtEntry.fileName = fileInfo.fileName();
								vmtEntry.directory = fileInfo.absolutePath();

								return vmtEntry;
							}

							inShader = true;

							++shaderBracketCounter;
							expectingOpenBracket = false;
						}
					}
					else
					{
						Info("Line " + Str(lineCount) + ": Open bracket without a group name found!")
					}
				}
				else if( line == "}" )
				{
					if( !expectingShaderName && inShader )
					{
						if(inSubBlock)
						{
							--subShaderBracketCounter;

							subGroups.append("}\n");

							if( subShaderBracketCounter == 0 )
							{
								inSubBlock = false;
							}
						}
						else
						{
							inShader = false;

							--shaderBracketCounter;
							expectingEOF = true;
						}
					}
					else if(expectingOpenBracket)
					{
						Info("Line " + Str(lineCount) + ": Expected open bracket!")
					}
				}
				else
				{
					int index = line.indexOf( ' ' );
					if( index == -1 )
					{
						if(expectingShaderName)
						{
							if( line.contains('{') || line.contains('}') )
							{
								Info("Line " + Str(lineCount) + ": \"" + line + "\" is not a valid shader name!")
							}

							vmtEntry.shaderName = line;

							expectingShaderName = false;
							expectingOpenBracket = true;
						}
						else if(expectingOpenBracket)
						{
							Info("Line " + Str(lineCount) + ": Expected open bracket!")
						}
						else
						{
							// Probably a subgroup, parsing malformed group names for the sake of simplicity

							if( line.contains('{') || line.contains('}') )
							{
								Info("Line " + Str(lineCount) + ": \"" + line + "\" is not a valid sub group name!")
							}

							expectingOpenBracket = true;

							subGroups.append( line + "\n{\n" );
						}
					}
					else
					{
						if(expectingOpenBracket)
						{
							Info("Line " + Str(lineCount) + ": Expected open bracket!")
						}
						else
						{
							if(inSubBlock)
							{
								subGroups.append( line + "\n" );
							}
							else
							{
								vmtEntry.parameters.insert( line.left(index).toLower(), line.right( line.size() - index - 1 ).toLower() );
							}
						}
					}
				}

				if(hack)
				{
					goto hackGoto;
				}
			}

			vmtFile.close();

			if( subShaderBracketCounter != 0 )
			{
				Info("ERROR: Unexpected end of shader group!")
			}
			if( shaderBracketCounter != 0 )
			{
				Info("ERROR: Unexpected end of file!")
			}

			/////////////////////////////////////////////////////////////////////

			QFileInfo fileInfo(vmtFile);

			if (!isTemplate) {
				vmtEntry.fileName = fileInfo.fileName();
				vmtEntry.directory = fileInfo.absolutePath();

				mLastVmtFile = vmtEntry;
			}
				vmtEntry.subGroups = formatSubGroups(subGroups, 0);
				vmtEntry.shader = Shader::convert(vmtEntry.shaderName);
			return vmtEntry;
		}
	}

	return VmtFile();
}

VmtFile VmtParser::loadVmtFileSilently( const QString& relativeFileName )
{
	if( !relativeFileName.isEmpty() )
	{
		QFile vmtFile( mWorkingDirectory + relativeFileName );

		if( vmtFile.open(QIODevice::ReadOnly | QIODevice::Text) )
		{
			QString subGroups;

			bool inShader = false;
			bool inSubBlock = false;

			bool expectingShaderName = true;
			bool expectingOpenBracket = false;
			bool expectingEOF = false;

			bool hack = false;

			uint shaderBracketCounter = 0;
			uint subShaderBracketCounter = 0;

			QRegExp reg("[a-zA-Z0-9_]+");

			VmtFile vmtEntry;

			QTextStream in( &vmtFile );
			for( uint lineCount = 1; !in.atEnd(); ++lineCount)
			{
				QString line = in.readLine();
				line = line.simplified(); // Removing whitespace characters

				if(line.isEmpty())
					continue;

				if( line.startsWith("//") )
					continue; // A comment was found, thus we skip that line

				removeSingleLineComment(line);

				line.remove("\""); // Removing double quote characters for easier parsing

				line = line.simplified(); // Removing whitespace characters... again

				if(line.isEmpty())
					continue;

				//////////////////////////////

				if( line.endsWith(" {") )
				{
					hack = true;

					line.chop(2);
				}

				if(expectingEOF)
				{
					continue;
				}

				//////////////////////////////

				if( line == "{" )
				{
					hackGoto:
					hack = false;

					if(expectingOpenBracket)
					{
						if(inShader)
						{
							inSubBlock = true;

							++subShaderBracketCounter;
							expectingOpenBracket = false;
						}
						else
						{
							inShader = true;

							++shaderBracketCounter;
							expectingOpenBracket = false;
						}
					}
				}
				else if( line == "}" )
				{
					if( !expectingShaderName && inShader )
					{
						if(inSubBlock)
						{
							--subShaderBracketCounter;

							subGroups.append("}\n");

							if( subShaderBracketCounter == 0 )
							{
								inSubBlock = false;
							}
						}
						else
						{
							inShader = false;

							--shaderBracketCounter;
							expectingEOF = true;
						}
					}
				}
				else
				{
					int index = line.indexOf( ' ' );
					if( index == -1 )
					{
						if(expectingShaderName)
						{
							if( reg.exactMatch(line) )
							{
								vmtEntry.shaderName = line;
							}

							expectingShaderName = false;
							expectingOpenBracket = true;
						}
						else if( !expectingOpenBracket )
						{
							// Probably a subgroup

							if( reg.exactMatch(line) )
							{
								expectingOpenBracket = true;

								subGroups.append( line + "\n{\n" );
							}
						}
					}
					else
					{
						if(!expectingOpenBracket)
						{
							if(inSubBlock)
							{
								subGroups.append( line + "\n" );
							}
							else
							{
								vmtEntry.parameters.insert( line.left(index), line.right( line.size() - index - 1 ) );
							}
						}
					}
				}

				if(hack)
				{
					goto hackGoto;
				}
			}

			vmtFile.close();

			/////////////////////////////////////////////////////////////////////

			vmtEntry.shader = Shader::convert(vmtEntry.shaderName);
			return vmtEntry;
		}
	}

	return VmtFile();
}

QString VmtParser::formatSubGroups( const QString& subGroups, int intendStart )
{
	if( !subGroups.isEmpty() )
	{
		QString tmp(subGroups);
		QTextStream ts(&tmp);

		QString output;

		int intendCount = intendStart;
		bool lastIntended = false;

		QString line;
		do
		{
			line = ts.readLine();
			line = line.simplified();

			if(line.isEmpty())
				continue;

			if(intendCount > 0)
				output.append( addTabs(intendCount) );

			if( line == "{" )
			{
				output.append(line);
				++intendCount;

				lastIntended = false;
			}
			else if( line == "}" )
			{
				if( intendCount != 0 )
					output.chop(1);

				--intendCount;

				if(lastIntended)
				{
					output = output.left( output.lastIndexOf("\n") );
					output.append( addTabs(intendCount) );
				}

				output.append(line);
				output.append("\n");



				lastIntended = true;
			}
			else
			{
				output.append(line);

				lastIntended = false;
			}

			output.append("\n");

		} while( !line.isNull() );

		output = output.trimmed();

		if(intendStart != 0)
		{
			return ( output.prepend( addTabs(intendStart) ));
		}

		return output;
	}

	return "";
}

QString VmtParser::parseSubGroups( const QString& subGroups, QString* output )
{
	// Basically just a stripped down version of the regular vmt parser with an immediate break on any error

	output->clear();

	bool inSubGroups = false;

	bool expectingInitialSubGroupName = true;
	bool expectingOpenBracket = false;

	int subShaderBracketCounter = 0;

	QRegExp reg("[a-zA-Z0-9_><]+");

	QString tmp(subGroups);
	QTextStream in( &tmp );

	for( uint lineCount = 1; !in.atEnd(); ++lineCount )
	{
		QString line = in.readLine();
		line = line.simplified(); // Removing whitespace characters

		if(line.isEmpty())
			continue;

		line.remove("\""); // Removing double quote characters for easier parsing

		line = line.simplified(); // Removing whitespace characters... again

		if(line.isEmpty())
			continue;


		if( line == "{" )
		{
			if(expectingInitialSubGroupName)
			{
				return "Line " + Str(lineCount) + ": Expected subgroup name!";
			}
			else if(expectingOpenBracket)
			{
				if(!inSubGroups)
				{
					inSubGroups = true;

					output->append("{\n");

					++subShaderBracketCounter;
					expectingOpenBracket = false;
				}
				else
				{
					output->append("{\n");

					expectingOpenBracket = false;

					++subShaderBracketCounter;
					//return "Line " + Str(lineCount) + ": Open bracket without a subgroup name found!";
				}
			}
			else
			{
				return "Line " + Str(lineCount) + ": Open bracket without a subgroup name found!";
			}
		}
		else if( line == "}" )
		{
			if(expectingOpenBracket)
			{
				return "Line " + Str(lineCount) + ": New subgroup requires open bracket! (Maybe you just forgot to specify a parameter's value?)";
			}
			else if( !expectingInitialSubGroupName && inSubGroups )
			{
				--subShaderBracketCounter;

				output->append("}\n");

				if( subShaderBracketCounter == 0 )
				{
					inSubGroups = false;
				}
			}
		}
		else
		{
			int index = line.indexOf( ' ' );
			if( index == -1 )
			{
				if(expectingInitialSubGroupName)
				{
					if( reg.exactMatch(line) )
					{
						output->append(line + "\n");
					}
					else
					{
						return "Line " + Str(lineCount) + ": \"" + line + "\" is not a valid subgroup name!";
					}

					expectingInitialSubGroupName = false;
					expectingOpenBracket = true;
				}
				else if(expectingOpenBracket)
				{
					return "Line " + Str(lineCount) + ": Expected open bracket!";
				}
				else
				{
					// Probably a subgroup

					if( reg.exactMatch(line) )
					{
						expectingOpenBracket = true;

						output->append( line + "\n" );
					}
					else
					{
						return "Line " + Str(lineCount) + ": \"" + line + "\" is not a valid sub group name!";
					}
				}
			}
			else
			{
				if(expectingOpenBracket)
				{
					return "Line " + Str(lineCount) + ": Expected open bracket!";
				}
				else
				{
					output->append( line + "\n" );
				}
			}
		}
	}

	if (subShaderBracketCounter == 1)
		return QString("Unmatched open bracket found!")
			.arg(subShaderBracketCounter);
	if (subShaderBracketCounter >= 2)
		return QString("%1 unmatched open brackets found!")
			.arg(subShaderBracketCounter);

	// TODO: Can this even happen? It looks like errors caused by
	// unmatched closed brackets are already handled by the
	// "Expected open bracked found" check
	if (subShaderBracketCounter == -1)
		return QString("Unmatched closed bracket found!")
			.arg(subShaderBracketCounter);
	if (subShaderBracketCounter <= -2)
		return QString("%1 unmatched closed brackets found!")
			.arg(subShaderBracketCounter);

	return "";
}

VmtFile VmtParser::loadPatchVmt( QFile* file, qint64 pos, int startLine )
{
	VmtFile output;


	QString subGroups;

	bool inShader = true;
	bool inInsertBlock = false;
	bool inSubBlock = false;

	bool expectingOpenBracket = false;
	bool expectingEOF = false;

	uint subShaderBracketCounter = 0;

	QTextStream stream(file);
	stream.seek(pos);

	for( uint lineCount = startLine; !stream.atEnd(); ++lineCount)
	{
		QString line = stream.readLine();
		line = line.simplified(); // Removing whitespace characters

		if(line.isEmpty())
			continue;

		if( line.startsWith("//") )
			continue; // A comment was found, thus we skip that line

		removeSingleLineComment(line);

		line.remove("\""); // Removing double quote characters for easier parsing

		line = line.simplified(); // Removing whitespace characters... again

		if(line.isEmpty())
			continue;

		//////////////////////////////

		if(expectingEOF)
		{
			Info("Line " + Str(lineCount) + " (" + line + ": Expected end of file!")
			continue;
		}

		//////////////////////////////

		if( line == "{" )
		{
			if(expectingOpenBracket)
			{
				if(inSubBlock)
				{
					++subShaderBracketCounter;
				}
				else if(inInsertBlock)
				{
					inSubBlock = true;

					++subShaderBracketCounter;
				}
				else if(inShader)
				{
					inInsertBlock = true;
				}

				expectingOpenBracket = false;
			}
			else
			{
				Info("Line " + Str(lineCount) + ": Open bracket without a group name found!")
			}
		}
		else if( line == "}" )
		{
			if(expectingOpenBracket)
			{
				Info("Line " + Str(lineCount) + ": Expected open bracket!")
			}
			else
			{
				if(inSubBlock)
				{
					--subShaderBracketCounter;

					subGroups.append("}\n");

					if( subShaderBracketCounter == 0 )
					{
						inSubBlock = false;
					}
				}
				else if(inInsertBlock)
				{
					inInsertBlock = false;
				}
				else
				{
					inShader = false;

					expectingEOF = true;
				}
			}
		}
		else
		{
			int index = line.indexOf( ' ' );
			if( index == -1 )
			{
				if(expectingOpenBracket)
				{
					Info("Line " + Str(lineCount) + ": Expected open bracket!")
				}
				else
				{
					// Probably a subgroup, parsing malformed group names for the sake of simplicity

					if( line.contains('{') || line.contains('}') )
					{
						Info("Line " + Str(lineCount) + ": \"" + line + "\" is not a valid sub group name!")
					}

					expectingOpenBracket = true;

					if( line.toLower() == "insert" )
					{
						if(inInsertBlock)
						{
							Info("ERROR: Only one insert block is allowed per shader!")
						}
						else if(inSubBlock)
						{
							Info("ERROR: Only one insert block is allowed per shader!")
						}
						else
						{
							// inInsertBlock = true;
						}
					}
					else
					{
						if(inInsertBlock || inSubBlock)
						{
							subGroups.append( line + "\n{\n" );
						}
						else if(inShader)
						{
							Info("ERROR: Patch shader block only allows one block named insert!")
						}
					}
				}
			}
			else
			{
				if(expectingOpenBracket)
				{
					Info("Line " + Str(lineCount) + ": Expected open bracket!")
				}
				else
				{
					if(inSubBlock)
					{
						subGroups.append( line + "\n" );
					}
					else if(inInsertBlock)
					{
						output.parameters.insert( line.left(index), line.right( line.size() - index - 1 ) );
					}
					else
					{
						if( line.left(index).toLower() == "include" )
						{
							if( output.parameters.contains("include") )
							{
								Info("ERROR: Only one include parameter is allowed within the Patch block!")
							}
							else
							{
								output.parameters.insert( line.left(index), line.right( line.size() - index - 1 ) );
							}
						}
						else
						{
							Info("ERROR: Patch shader is only allowed to have one include parameter inside the Patch block!")
						}
					}
				}
			}
		}
	}

	if( subShaderBracketCounter != 0 )
		Info("ERROR: Unexpected end of file!")

	/////////////////////////////////////////////////////////////////////

	output.subGroups = formatSubGroups(subGroups, 0);

	mLastVmtFile = output;
	return output;
}
