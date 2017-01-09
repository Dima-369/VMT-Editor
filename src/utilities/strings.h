#pragma once

#include <QStringListModel>
#include <QDoubleSpinBox>
#include <QFile>
#include <QCompleter>

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

// TODO: Place in view-helper
/*!
 * Calls stripZeroes() on the cleanText() value of the spin box.
 */
QString stripZeroes(QDoubleSpinBox *sp);

/*!
 * Extracts non empty lines from the passed file.
 */
QStringList extractLines(const QString& fileName);
