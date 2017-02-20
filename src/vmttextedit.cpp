#include "vmttextedit.h"

VmtTextEdit::VmtTextEdit(QWidget* parent) :
	QTextEdit(parent),
	c(NULL)
{
	new VmtTextEditHighlighter(document());
}

void VmtTextEdit::setWordList(const QStringList& wordList)
{
	c = new QCompleter(wordList);
	c->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	c->setCaseSensitivity(Qt::CaseInsensitive);
	c->setFilterMode(Qt::MatchContains);
	c->setCompletionMode(QCompleter::PopupCompletion);
	c->setCaseSensitivity(Qt::CaseInsensitive);
	c->setWidget(this);
	QObject::connect(c, SIGNAL(activated(QString)),
		this, SLOT(insertCompletion(QString)));
}

void VmtTextEdit::keyPressEvent(QKeyEvent *e)
{
	if (c->popup()->isVisible()) {
		// The following keys are forwarded by the completer to the widget
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return; // let the completer do default behavior
		default:
			break;
		}
	}

	if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
		QTextCursor tc = textCursor();
		const auto oldPos = tc.position();
		tc.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
		const auto toStart = tc.selectedText();
		int currentIndent = 0;
		// assuming that the entered VMT is correct
		for (int i = toStart.size() - 1; i >= 0; --i) {
			const auto ch = toStart.at(i);
			if (ch == '{') {
				currentIndent++;
			} else if (ch == '}') {
				currentIndent--;
			}
		}
		tc.setPosition(oldPos);
		if (currentIndent <= 0) {
			tc.insertText("\n");
		} else {
			tc.insertText("\n" + addTabs(currentIndent));
		}
	} else if (e->key() == Qt::Key_Backspace) {
		QTextCursor tc = textCursor();
		const auto oldPos = tc.position();
		tc.movePosition(QTextCursor::StartOfLine);
		tc.movePosition(QTextCursor::EndOfLine,
			QTextCursor::KeepAnchor);
		if (isWhitespace(tc.selectedText())) {
			tc.removeSelectedText();
			// need to remove previous \n as well
			tc.movePosition(QTextCursor::PreviousCharacter,
				QTextCursor::KeepAnchor);
			tc.removeSelectedText();
		} else {
			tc.setPosition(oldPos);
			QTextEdit::keyPressEvent(e);
		}
	} else {
		QTextEdit::keyPressEvent(e);
	}

	const bool ctrlOrShift = e->modifiers() &
		(Qt::ControlModifier | Qt::ShiftModifier);
	if (ctrlOrShift && e->text().isEmpty())
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
	bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3
		|| eow.contains(e->text().right(1))) {
		c->popup()->hide();
		return;
	}

	if (completionPrefix != c->completionPrefix()) {
		c->setCompletionPrefix(completionPrefix);
		c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
	}
	auto cr = cursorRect();
	cr.setWidth(c->popup()->sizeHintForColumn(0)
		+ c->popup()->verticalScrollBar()->sizeHint().width());
	c->complete(cr);
}

QString VmtTextEdit::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void VmtTextEdit::focusInEvent(QFocusEvent *e)
{
	c->setWidget(this);
	QTextEdit::focusInEvent(e);
}

void VmtTextEdit::insertCompletion(const QString& completion)
{
	if (c->widget() != this)
		return;

	QTextCursor tc = textCursor();
	// this will still leave the dollar sign because it seems to not be
	// part of the 'Word' regex by Qt
	tc.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
	tc.removeSelectedText();

	// clearing away all prepending dollar signs
	int oldPos = tc.position();
	tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	const auto toStartOfLine = tc.selectedText();
	int dollarCount = 0;
	for (int i = toStartOfLine.size() - 1; i >= 0; --i) {
		if (toStartOfLine.at(i) == '$') {
			dollarCount++;
		} else {
			break;
		}
	}
	tc.setPosition(oldPos);
	if (dollarCount >= 1) {
		tc.movePosition(QTextCursor::PreviousCharacter,
			QTextCursor::KeepAnchor, dollarCount);
		tc.removeSelectedText();
	}
	tc.insertText(completion);
	setTextCursor(tc);
}

VmtTextEditHighlighter::VmtTextEditHighlighter(QTextDocument* parent) :
	QSyntaxHighlighter(parent)
{
	parameterFormat.setForeground(QColor(200, 220, 0));
	HighlightingRule rule;
	rule.pattern = QRegExp(R"(\$\S+)");
	rule.format = parameterFormat;
	rules.append(rule);

	quoteFormat.setForeground(QColor(114, 220, 114));
	rule.pattern = QRegExp("\".*\"");
	rule.format = quoteFormat;
	rules.append(rule);
}

void VmtTextEditHighlighter::highlightBlock(const QString& text)
{
	foreach(const HighlightingRule& rule, rules) {
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);

		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}
}
