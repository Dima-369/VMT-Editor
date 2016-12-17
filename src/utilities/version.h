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

QString getCurrentVersion();

// if no new version is available, major is set to 0
// otherwise the new version is returned
Version checkForNewVersion();
