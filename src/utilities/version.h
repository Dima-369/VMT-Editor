#pragma once

#include <QFile>
#include <QString>

// those are methods specified in POSIX.1, we don't want those!
#undef major
#undef minor

struct Version {
	int major, minor, patch;

	Version(int major, int minor, int patch) :
		major(major),
		minor(minor),
		patch(patch) {}
};

Version fetch_version();

QString to_string(Version v);
