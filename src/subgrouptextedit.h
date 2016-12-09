#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>

#include "utilities.h"


class QCompleter;

class SubGroupTextEdit : public QTextEdit
{
	Q_OBJECT

public:

	SubGroupTextEdit( QWidget* parent = NULL );

	~SubGroupTextEdit();

protected:

	void keyPressEvent( QKeyEvent* event );

	void focusInEvent( QFocusEvent*event );

	void insertFromMimeData( const QMimeData* source );

private slots:

	void insertCompletion( const QString& completion );

private:

	QString textUnderCursor() const;

private:

	QCompleter* mCompleter;
};

#endif // TEXTEDIT_H
