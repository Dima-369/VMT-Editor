#include "utilities.h"

void fatalError(const QString& msg)
{
	qDebug() << "Fatal error: " << QString(msg);
	MsgBox::warning(NULL, "Fatal error occurred!", msg);
	exit(1);
}

bool isWhitespace( const QString& input )
{
	static const QRegExp reg("\\s+");
	return reg.exactMatch(input);
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

QString addTabs(int amount)
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

double smoothstep(double edge0, double edge1, double x)
{
	// Scale, bias and saturate x to 0..1 range
	x = qBound(0.0, (x - edge0)/(edge1 - edge0), 1.0);
	// Evaluate polynomial
	return x*x*(3 - 2*x);
}
