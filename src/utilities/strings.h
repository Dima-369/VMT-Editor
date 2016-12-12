#pragma once

#include <QStringListModel>
#include <QDoubleSpinBox>
#include <QFile>
#include <QCompleter>

namespace utils {

/*!
 * Macro to convert the passed number to a QString.
 */
#define STR(x) QString::number(x)

/*!
 * Strips all trailing zeroes and the dot character from the passed string.
 *
 * If you pass 4.00, it returns "4".
 *
 * This is used to avoid comparing against double values where possible.
 */
QString stripZeroes(const QString& s);

/*!
 * Calls stripZeroes() on the cleanText() value of the spin box.
 */
QString stripZeroes(QDoubleSpinBox *sp);

/*!
 * Parses the lines from the passed file to a QStringListModel and sets it on
 * the passed QCompleter.
 */
QCompleter* setModelFromFile(const QString &fileName, QObject* o);

} // namespace utils
