#include "utilities.h"

#include <QRegExp>


bool isWhitespace( const QString& input )
{
	static const QRegExp reg("\\s+");

	return reg.exactMatch(input);
}


QStringList listFromFile( const QString& fileName )
{
	QFile file(fileName);
	if( !file.open(QFile::ReadOnly | QFile::Text) )
		return QStringList();

	QStringList words;

	while( !file.atEnd() )
	{
		QString line = file.readLine();
		line = line.simplified();

		if( !line.isEmpty() )
		{
			words << line;
		}
	}

	file.close();

	return words;
}

QString setKey(const QString& name, const QString& def, QSettings* settings)
{
	if( settings->contains(name) )
		return settings->value(name).toString();
	settings->setValue(name, def);
	return def;
}

bool setKey(const QString &name, bool def, QSettings *settings)
{
	if (settings->contains(name))
		return settings->value(name).toBool();

	settings->setValue(name, def);
	return def;
}

QString addTabs( int amount )
{
	QString tmp;

	while(amount > 0)
	{
		tmp.append("\t");
		--amount;
	}

	return tmp;
}

void removeSingleLineComment( QString& string )
{
	int startIndex = string.indexOf( "//" );

	if(startIndex == -1)
		return;

	string.remove( startIndex, string.size() - startIndex );

	string = string.simplified();
}
