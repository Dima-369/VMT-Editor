#pragma once

#include <QFile>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QEventLoop>

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

QString versionToString(const Version& v);

QString getCurrentVersion();

// Removes the trailing .0, so "1.1.0" becomes "1.1"
QString removeTrailingVersionZero(const QString& vs);

// If no new version is available, major is 0
// If the network request failed, major is -1
// Otherwise the new version is returned
Version checkForNewVersion();
