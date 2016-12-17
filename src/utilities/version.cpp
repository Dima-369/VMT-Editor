#include "version.h"

Version fetch_version()
{
	QFile f(":/files/version");
	f.open(QFile::ReadOnly | QFile::Text);
	const auto version = f.readLine().trimmed();
	f.close();

	const auto parts = version.split('.');
	return Version(parts[0].toInt(), parts[1].toInt(), parts[2].toInt());
}

QString to_string(Version v)
{
	return QString("%1.%2.%3")
		.arg(v.major).arg(v.minor).arg(v.patch);
}
