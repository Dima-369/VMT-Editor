#pragma once

#include <QPlainTextEdit>
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
#include "utilities.h"
#include "highlighter.h"

class LineNumberArea;

class SubGroupTextEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	SubGroupTextEdit(QWidget* parent = NULL);

	void lineNumberAreaPaintEvent(QPaintEvent* event);
	int lineNumberAreaWidth();

protected:
	void keyPressEvent(QKeyEvent* event);
	void focusInEvent(QFocusEvent* event);
	void resizeEvent(QResizeEvent* event) override;

	void insertFromMimeData(const QMimeData* source);

private slots:
	void insertCompletion(const QString& completion);
	void updateLineNumberAreaWidth();
	void updateLineNumberArea(const QRect& rect, int dy);

private:
	QCompleter* mCompleter;
	LineNumberArea* lineNumberArea;

	QString textUnderCursor() const;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(SubGroupTextEdit* editor);
	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	SubGroupTextEdit* editor;
};
