#pragma once

#include <QTextEdit>
#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>

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
