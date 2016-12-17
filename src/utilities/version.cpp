#include "version.h"

QString versionToString(Version v)
{
	return QString("%1.%2.%3")
		.arg(v.major).arg(v.minor).arg(v.patch);
}

QString getCurrentVersion()
{
	QFile f(":/files/version");
	f.open(QFile::ReadOnly | QFile::Text);
	const auto version = f.readLine().trimmed();
	f.close();

	const auto parts = version.split('.');
	return versionToString(Version(
		parts[0].toInt(), parts[1].toInt(), parts[2].toInt()));
}

Version checkForNewVersion()
{
	return Version(1, 1, 0);
}
