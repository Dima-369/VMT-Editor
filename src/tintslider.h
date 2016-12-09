#ifndef TINTSLIDER_H
#define TINTSLIDER_H

#include <QSlider>
#include <QPlainTextEdit>


class TintSlider : public QSlider {

	Q_OBJECT

public:

	explicit TintSlider( QWidget* parent = NULL );

	void wheelEvent(QWheelEvent* event);

	void initialize( QPlainTextEdit* colorWidget, const QColor& color = QColor(255, 255, 255) );

private:

	QPlainTextEdit* colorWidget;

	QColor color;

private slots:

	void valueChangedSlot(int value);
};

#endif // TINTSLIDER_H
