#include "subgrouptextedit.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QFile>
#include <QStringListModel>
#include <QMimeData>
#include <QTextDocumentFragment>
#include <QFileSystemModel>
#include <QDirModel>
#include <QTableView>
#include <QHeaderView>

#include "vmtparser.h"
#include "highlighter.h"


class StringListModel : public QAbstractListModel
{
public:

	StringListModel( QStringList list, QObject * parent = 0 ) :
		QAbstractListModel(parent),
		mList(list),
		mCompletions( listFromFile(":/files/subGroups") )
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

SubGroupTextEdit::SubGroupTextEdit( QWidget* parent )
	: QTextEdit(parent)
{
	mCompleter = new QCompleter(this);

	QStringList tmp = listFromFile(":/files/subGroups");
	tmp << listFromFile(":/files/parameters");

	mCompleter->setModel( new StringListModel( tmp, mCompleter ) );


	mCompleter->setWidget(this);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	QObject::connect( mCompleter, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

	// to color groups like PlayerLogo
	new Highlighter(document());

	// Calculating the tab stop width by taking 4 random characters as Consolas is a monospaced font
	QFont font( fontFamily(), 9 );
	QFontMetrics fm(font);

	setTabStopWidth( fm.boundingRect("aaaa").width() );
}

SubGroupTextEdit::~SubGroupTextEdit()
{

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
	if( listFromFile(":/files/subGroups").contains(completion) )
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

void SubGroupTextEdit::focusInEvent( QFocusEvent* event )
{
	mCompleter->setWidget(this);

	QTextEdit::focusInEvent(event);
}

void SubGroupTextEdit::keyPressEvent( QKeyEvent* event )
{
	// For some reason we have to set the font family again because when the textedit contains no characters and the backspace key
	// is hit, the font family will reset

	setFontFamily("Consolas");
	setFontPointSize(9);

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
				QTextEdit::keyPressEvent(event);
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
			QTextEdit::keyPressEvent(event);
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

	QTextEdit::insertFromMimeData(data);
}
