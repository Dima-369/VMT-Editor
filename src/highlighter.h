#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>


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

#endif // HIGHLIGHTER_H
