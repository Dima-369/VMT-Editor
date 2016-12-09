#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <QLabel>
#include <QToolButton>


class IconLabel : public QLabel
{
public:

	explicit IconLabel( QWidget* parent = NULL );

	void showIcon( bool show );
	
protected:

	void resizeEvent( QResizeEvent* );

	QToolButton* clickableIcon;
};

#endif // ICONLABEL_H
