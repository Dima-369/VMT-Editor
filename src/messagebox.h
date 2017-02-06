#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>
#include <QtGui>
#include <QSpacerItem>
#include <QGridLayout>

class MsgBox : public QMessageBox
{
	Q_OBJECT

public:

	explicit MsgBox( QWidget* parent = NULL );

	~MsgBox();

	void setWindowTitle(const QString &title);

	void setStandardButtons(StandardButtons buttons);

	QPushButton* addButton ( const QString & text, ButtonRole role );

	QPushButton* addButton ( StandardButton button );

	void addButton(QAbstractButton* button, ButtonRole role);

	void setDefaultButton(StandardButton button);

	void setDefaultButton(QPushButton* button);

	void setIconPixmap(const QPixmap& pixmap);

	void setStyleSheet(const QString& styleSheet);

	void setIcon(Icon icon);

	void setText(const QString &text);

	int exec();

	static StandardButton information(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);

	static StandardButton warning(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);

private:

	QMessageBox* msgBox;

signals:
	
public slots:
	
};

#endif // MESSAGEBOX_H
