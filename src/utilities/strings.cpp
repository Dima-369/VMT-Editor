#include "strings.h"

QString stripZeroes(const QString &s)
{
	return QString(s).remove(QRegExp("\\.0*(?!0)$"));
}

QStringList extractLines(const QString& fileName)
{
	QFile file(fileName);
	file.open(QFile::ReadOnly);

	QStringList words;
	while (!file.atEnd()) {
		const auto line = file.readLine().trimmed();
		if (!line.isEmpty())
			words << line;
	}
	return words;
}
