#include "subgrouptextedit.h"

class StringListModel : public QAbstractListModel
{
public:

	StringListModel( QStringList list, QObject * parent = 0 ) :
		QAbstractListModel(parent),
		mList(list),
		mCompletions( extractLines(":/files/subGroups") )
	{

	}

	int rowCount( const QModelIndex& parent ) const
	{
		Q_UNUSED(parent)

		return mList.count();
	}

	QVariant data(const QModelIndex& index, int role) const
	{
		if( index.row() < 0 || index.row() >= mList.size() )
			return QVariant();

		if( role == Qt::DecorationRole )
		{
			if( mCompletions.contains( mList.at( index.row() ), Qt::CaseInsensitive ))
			{
				return QIcon(":/icons/subGroup");
			}
			else
			{
				return QIcon(":/icons/parameter");
			}
		}

		if( role == Qt::DisplayRole || role == Qt::EditRole )
			return mList.at( index.row() );

		return QVariant();
	}

private:

	QStringList mList;
	QStringList mCompletions;
};

SubGroupTextEdit::SubGroupTextEdit(QWidget* parent) :
	QPlainTextEdit(parent),
	mCompleter(new QCompleter(this)),
	lineNumberArea(new LineNumberArea(this))
{
	QStringList tmp = extractLines(":/files/subGroups");
	tmp << extractLines(":/files/parameters");

	mCompleter->setModel( new StringListModel( tmp, mCompleter ) );

	mCompleter->setWidget(this);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	connect(this, SIGNAL(blockCountChanged(int)),
		SLOT(updateLineNumberAreaWidth()));

	connect(this, SIGNAL(updateRequest(QRect,int)),
		SLOT(updateLineNumberArea(QRect,int)));

	updateLineNumberAreaWidth();

	connect(mCompleter, SIGNAL(activated(QString)), SLOT(insertCompletion(QString)));

	// to color groups like PlayerLogo
	// we do not clear the memory because this TextEdit is only created once
	new Highlighter(document());

	// otherwise 1 tab is very large!
	QFont font;
	font.setFamily("Consolas");
	font.setStyleHint(QFont::Monospace);
	font.setPointSize(9);
	QFontMetrics metrics(font);
	setTabStopWidth(4 * metrics.width(' '));
}

int SubGroupTextEdit::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}
	int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
	return space + 8;
}

void SubGroupTextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
{
	QPainter painter(lineNumberArea);
	// same color as the Parameters/Proxies tabs when inactive
	painter.fillRect(event->rect(), QColor(28, 28, 28));

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();
	const auto current = textCursor().block();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			const QString number = QString::number(blockNumber + 1);
			if (block == current) {
				painter.setPen(QColor(210, 210, 210));
			} else {
				painter.setPen(QColor(110, 110, 110));
			}
			const int paddingRight = 5;
			painter.drawText(0, top,
				lineNumberArea->width() - paddingRight,
				fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

void SubGroupTextEdit::updateLineNumberArea(const QRect& rect, int dy)
{
	if (dy) {
		lineNumberArea->scroll(0, dy);
	} else {
		lineNumberArea->update(0, rect.y(),
			lineNumberArea->width(), rect.height());
	}

	if (rect.contains(viewport()->rect())) {
		updateLineNumberAreaWidth();
	}
}

void SubGroupTextEdit::updateLineNumberAreaWidth()
{
	// the margins at which the actual text starts
	const int marginLeft = 5;
	setViewportMargins(lineNumberAreaWidth() + marginLeft, 0, 0, 0);
}

void SubGroupTextEdit::insertCompletion( const QString& completion )
{
	QTextCursor tc = textCursor();
	QString tmp;

	QTextCharFormat format;
	format.setFontFamily("Consolas");
	format.setFontPointSize(9);

	// Removing the word(-chunk) which was typed before a completion was needed to match upper- and lowercase
	tc.movePosition( QTextCursor::WordLeft, QTextCursor::KeepAnchor );
	tc.removeSelectedText();

	tc.movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
	tc.removeSelectedText();

	if( completion == mCompleter->completionPrefix() )
	{
		// We basically just need to fix upper- and lowercase by inserting the entire word here
		tc.insertText( completion );
		setTextCursor(tc);

		return;
	}

	tc.select( QTextCursor::LineUnderCursor );
	tmp = tc.selectedText();

	tc.movePosition(QTextCursor::EndOfLine);

	tc.setCharFormat(format);
	if( extractLines(":/files/subGroups").contains(completion) )
	{
		// Check if there is text before the actual insertion
		if( isWhitespace(tmp) || tmp.isEmpty() )
		{
			tc.insertText( completion + "\n" + tmp + "{\n" + tmp + "\t\n" + tmp + "}");

			tc.movePosition( QTextCursor::Up );
			tc.movePosition( QTextCursor::EndOfLine );
		}
		else
		{
			tc.insertText( completion );
		}
	}
	else
	{
		tc.insertText( completion );
	}

	setTextCursor(tc);
}

QString SubGroupTextEdit::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);

	return tc.selectedText();
}

void SubGroupTextEdit::focusInEvent(QFocusEvent* event)
{
	mCompleter->setWidget(this);
	QPlainTextEdit::focusInEvent(event);
}

void SubGroupTextEdit::resizeEvent(QResizeEvent* e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(
		QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void SubGroupTextEdit::keyPressEvent( QKeyEvent* event )
{
	if( mCompleter->popup()->isVisible() )
	{
		switch( event->key() )
		{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Backtab:

			event->ignore();
			return; // let the completer insert the completion while ignoring the current key
	   }
	}

	const bool isShortcut = ( (event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_E ); // CTRL+E
	if( !isShortcut ) // do not process the shortcut when we have a completer
	{
		if( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter )
		{
			QTextCursor tc = textCursor();
			tc.movePosition( QTextCursor::EndOfLine, QTextCursor::KeepAnchor );

			if( !tc.selectedText().isEmpty() )
			{
				QPlainTextEdit::keyPressEvent(event);
			}
			else
			{
				tc.select( QTextCursor::LineUnderCursor );

				int index = tc.selectedText().indexOf( QRegExp("[^\\s]+") );
				QString tmp = tc.selectedText();

				// Used to not accidentally overwrite the line
				tc.movePosition( QTextCursor::EndOfLine );
				tc.insertText( "\n" + tmp.left(index) );
			}
		}
		else
		{
			QPlainTextEdit::keyPressEvent(event);
		}
	}

	const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if( ctrlOrShift && event->text().isEmpty() )
		return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
	bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
	QString completionPrefix = textUnderCursor();

	if( !isShortcut && (hasModifier || event->text().isEmpty() || completionPrefix.length() < 1 || eow.contains(event->text().right(1))))
	{
		mCompleter->popup()->hide();
		return;
	}

	if( completionPrefix != mCompleter->completionPrefix() )
	{
		mCompleter->setCompletionPrefix(completionPrefix);
		mCompleter->popup()->setCurrentIndex( mCompleter->completionModel()->index(0, 0) );
	}

	QRect rect = cursorRect();
	rect.setWidth(mCompleter->popup()->sizeHintForColumn(0) + mCompleter->popup()->verticalScrollBar()->sizeHint().width() );

	mCompleter->complete(rect);
}

void SubGroupTextEdit::insertFromMimeData ( const QMimeData * source )
{
	// Replacing 4 spaces with 1 tab

	QMimeData* data = new QMimeData;

	QString tmp( source->text() );
	tmp.replace( "    ", "\t" );

	data->setText(tmp);

	QPlainTextEdit::insertFromMimeData(data);
}

LineNumberArea::LineNumberArea(SubGroupTextEdit* editor) : 
	QWidget(editor),
	editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
	return QSize(editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
	editor->lineNumberAreaPaintEvent(event);
}
