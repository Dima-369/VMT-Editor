#pragma once

#include <QFile>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QThread>
#include <QUrl>
#include <QEventLoop>

#include "mainwindow.h"

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

class CheckVersionThread : public QThread
{
	Q_OBJECT

public:

	virtual void run();

signals:
	// this should only use Info() to inform the user and not display any
	// message boxes because this thread is used for quick checking on app
	// lanuch
	void notifyOnNewVersion(QString version);
};

QString versionToString(const Version& v);

QString getCurrentVersion();

// Removes the trailing .0, so "1.1.0" becomes "1.1"
QString removeTrailingVersionZero(const QString& vs);

// If no new version is available, major is 0
// If the network request failed, major is -1
// Otherwise the new version is returned
Version checkForNewVersion();
