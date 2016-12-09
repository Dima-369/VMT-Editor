#include "iconlabel.h"

#include <QStyle>

IconLabel::IconLabel(QWidget *parent) :
	QLabel(parent)
{
	QPixmap pixmap( ":/icons/openVmt" );

	clickableIcon = new QToolButton(this);

	clickableIcon->setIcon(		  QIcon(pixmap) );
	clickableIcon->setIconSize(   pixmap.size() );
	clickableIcon->setCursor(	  Qt::ArrowCursor );
	clickableIcon->setStyleSheet( "QToolButton { border: none; padding: 0px; }" );
	clickableIcon->hide();

	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	setStyleSheet( QString("QLineEdit { padding-right: %1px; } ").arg( clickableIcon->sizeHint().width() + frameWidth + 1 ));
}

void IconLabel::resizeEvent( QResizeEvent* )
{
	QSize sz = clickableIcon->sizeHint();
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

	clickableIcon->move( rect().right() - frameWidth - sz.width() + 1, (rect().bottom() + 1 - sz.height())/2 );
}

void IconLabel::showIcon( bool show )
{
	clickableIcon->setVisible(show);
}
