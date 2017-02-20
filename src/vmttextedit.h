#pragma once

#include <QTextEdit>
#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QSyntaxHighlighter>

#include "utilities.h"

class VmtTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	VmtTextEdit(QWidget *parent = 0);
	void setWordList(const QStringList& wordList);

protected:
	void keyPressEvent(QKeyEvent *e) override;
	void focusInEvent(QFocusEvent *e) override;

private slots:
	void insertCompletion(const QString &completion);

private:
	QString textUnderCursor() const;

private:
	QCompleter *c;
};


class VmtTextEditHighlighter : public QSyntaxHighlighter
{
public:
	VmtTextEditHighlighter(QTextDocument* parent = NULL);

protected:
	void highlightBlock(const QString& text);

private:
	struct HighlightingRule {
		QRegExp pattern;
		QTextCharFormat format;
	};

	QVector<HighlightingRule> rules;
	// every word which starts with a dollar sign
	QTextCharFormat parameterFormat;
	QTextCharFormat quoteFormat;
	QTextCharFormat numberFormat;
	QTextCharFormat vectorFormat;
};
