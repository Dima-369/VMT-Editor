#pragma once

#include <QString>

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
inline QString stripZeroes(const QString &s)
{
	return QString(s).remove(QRegExp("\\.0*(?!0)$"));
}

/*!
 * Calls stripZeroes() on the cleanText() value of the spin box.
 */
inline QString stripZeroes(QDoubleSpinBox *sp)
{
	return stripZeroes(sp->cleanText());
}

} // namespace utils
