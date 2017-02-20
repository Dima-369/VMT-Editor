#include "highlighter.h"

#include "utilities.h"

Highlighter::Highlighter( QTextDocument* parent ) :
	QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	//////////////////////////////////////////////

	QStringList groupPatterns = extractLines(":/files/subGroups");

	for( int i = 0; i < groupPatterns.size(); ++i )
		groupPatterns[i] = "\\b" + groupPatterns.at(i) + "\\b";

	groupFormat.setForeground( QColor(255, 90, 42) );
	groupFormat.setFontWeight(QFont::Bold);
	groupFormat.setFont( QFont("Consolas", 9) );

	foreach( const QString& pattern, groupPatterns )
	{
		 rule.pattern = QRegExp(pattern, Qt::CaseInsensitive);
		 rule.format = groupFormat;

		 highlightingRules.append(rule);
	}

	//////////////////////////////////////////////

	QStringList parameterPatterns = extractLines(":/files/parameters");

	for( int i = 0; i < parameterPatterns.size(); ++i )
	{
		parameterPatterns[i] = "\\b" + parameterPatterns.at(i) + "\\b";
	}

	groupFormat.setForeground( QColor(117, 148, 187) );
	groupFormat.setFontWeight(QFont::Normal);

	foreach( const QString& pattern, parameterPatterns )
	{
		 rule.pattern = QRegExp(pattern, Qt::CaseInsensitive);
		 rule.format = groupFormat;

		 highlightingRules.append(rule);
	}

	//////////////////////////////////////////////

	quotationFormat.setForeground( QColor(220, 220, 220) );
	rule.pattern = QRegExp("\".*\"");
	rule.format = quotationFormat;

	highlightingRules.append(rule);
}

void Highlighter::highlightBlock( const QString& text )
 {
	foreach( const HighlightingRule& rule, highlightingRules )
	{
		 QRegExp expression(rule.pattern);
		 int index = expression.indexIn(text);

		 while( index >= 0 )
		 {
			 int length = expression.matchedLength();

			 setFormat(index, length, rule.format);
			 index = expression.indexIn(text, index + length);
		 }
	}
}
