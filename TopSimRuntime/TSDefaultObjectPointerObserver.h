#ifndef HRUserConfig_h__
#define HRUserConfig_h__

#include "HRUICommon.h"

class HRUICOMMON_EXPORT HRUserConfig
{
public:
	static QStringList GetRecentFileList(const QString &group);

	static QStringList AddRecentFile(const QString &group, const QString &filePath);

	static QStringList  RemoveRecentFile(const QString &group, const QString &filePath);

	static void ClearRecentFileList(const QString &group);
};

#endif // HRUserConfig_h__
