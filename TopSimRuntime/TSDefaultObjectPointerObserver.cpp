#include "stdafx.h"
#include "HRUserConfig.h"

#include <QDir>
#include <QSettings>

#include "HRAppConfig.h"

const char *kRecnetFileListKey = "recentFileList";

QStringList HRUserConfig::GetRecentFileList( const QString &group )
{
	QSettings settings(QDir::homePath()+"/" + TSString2QString(GetAppConfig<TSString>("Root.UserConfigFileName")), QSettings::IniFormat);
	settings.beginGroup(group);
	return settings.value(kRecnetFileListKey).toStringList();
}

QStringList UpdateRecentFile( const QString &group, const QString &filePath, bool remove)
{
	QString path = QDir::toNativeSeparators(filePath);
	QSettings settings(QDir::homePath()+"/" + TSString2QString(GetAppConfig<TSString>("Root.UserConfigFileName")), QSettings::IniFormat);
	settings.beginGroup(group);
	QStringList files = settings.value(kRecnetFileListKey).toStringList();
	files.removeAll(path);
	files.removeAll("");

	if (!remove)
	{
		files.prepend(path);
	}

	while (files.size() > 9)
	{
		files.removeLast();
	}
	settings.setValue(kRecnetFileListKey, files);
	settings.endGroup();

	return files;
}

void HRUserConfig::ClearRecentFileList( const QString &group )
{
	QSettings settings(QDir::homePath()+"/" + TSString2QString(GetAppConfig<TSString>("Root.UserConfigFileName")), QSettings::IniFormat);
	settings.beginGroup(group);
	settings.setValue(kRecnetFileListKey, QStringList());
	settings.endGroup();
}

QStringList HRUserConfig::AddRecentFile( const QString &group, const QString &filePath )
{
	return UpdateRecentFile(group, filePath, false);
}

QStringList HRUserConfig::RemoveRecentFile( const QString &group, const QString &filePath )
{
	return UpdateRecentFile(group, filePath, true);
}
