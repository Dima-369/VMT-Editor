#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QIcon>

/*!
 * Entirely white background through background-color.
 *
 * Use this for QWidget::setStyleSheet()
 */
static const QString whiteBG = "background-color: rgb(255, 255, 255)";

/*!
 * White color value used for checking the default value of the return value
 * of MainWindow::getBackgroundColor().
 */
static const QColor white = QColor(255, 255, 225);

#endif // CONSTANTS_H
