#pragma once

#include <QTextEdit>

#include "utilities.h"
#include "highlighter.h"

class SubGroupTextEdit : public QTextEdit
{
	Q_OBJECT

public:

	SubGroupTextEdit( QWidget* parent = NULL );

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
