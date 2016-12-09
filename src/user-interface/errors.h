#ifndef ERRORS_H
#define ERRORS_H

#include <QString>

/*!
 * Returns a string for the user interface stating how the passed double tuple
 * has an incorrect format.
 *
 * Make sure that the parameter has the dollar prefix.
 */
inline QString doubleTupleBadFormat(const QString &parameter,
		const QString &tupleValue)
{
	return parameter + " value: \"" + tupleValue + "\" is not in the " +
		"valid format: \"[<x float> <y float> <z float>]\"!";
}

/*!
 * Returns a string for the user interface stating how any value of double tuple
 * is outside its valid range.
 *
 * Make sure that the parameter has the dollar prefix.
 */
inline QString doubleTupleBadBetween(const QString &parameter,
		const QString &tupleValue, double min, double max)
{
	return "Not all values of " + parameter + ": " + tupleValue + " are in"
		"range [" + min + ", " + max + ")!";
}
#endif // ERRORS_H
