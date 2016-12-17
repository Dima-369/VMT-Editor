#pragma once

#include <QSyntaxHighlighter>

#include "utilities/strings.h"

class Highlighter : public QSyntaxHighlighter
{
public:

	Highlighter( QTextDocument* parent = NULL );

protected:

	void highlightBlock( const QString& text );

private:

	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};

	QVector<HighlightingRule> highlightingRules;

	QTextCharFormat groupFormat;
	QTextCharFormat parameterFormat;
	QTextCharFormat quotationFormat;
};
