#include "vmttextedit.h"
#include <qdebug.h>

VmtTextEdit::VmtTextEdit(QWidget* parent) :
	QTextEdit(parent),
	c(NULL)
{
}

void VmtTextEdit::setCompleter(QCompleter *completer)
{
	if (c) {
		QObject::disconnect(c, 0, this, 0);
	}
	c = completer;

	c->setWidget(this);
	c->setCompletionMode(QCompleter::PopupCompletion);
	c->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(c, SIGNAL(activated(QString)),
		this, SLOT(insertCompletion(QString)));
}

QCompleter *VmtTextEdit::completer() const
{
	return c;
}

void VmtTextEdit::keyPressEvent(QKeyEvent *e)
{
	if (c && c->popup()->isVisible()) {
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

	QTextEdit::keyPressEvent(e);

	const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if ((hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
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
	if (c) {
		c->setWidget(this);
	}
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

	//tc.select(QTextCursor::WordUnderCursor);
	//tc.movePosition(QTextCursor::Left);
	//tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion);
	setTextCursor(tc);
}
