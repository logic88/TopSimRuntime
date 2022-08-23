#ifndef TSITransferLog_h__
#define TSITransferLog_h__

#include <TopSimDataInterface/TSTopicTypes.h>

#include "TopSimDataInterface.h"

#define INTERFACE_LEVEL0 0
#define INTERFACE_LEVEL1 1
#define INTERFACE_LEVEL2 2
#define INTERFACE_LEVEL3 3
#define INTERFACE_LEVEL4 4
#define INTERFACE_LEVEL5 5
#define INTERFACE_LEVEL6 6

struct TSITransferLogPrivate;
class TOPSIMDATAINTERFACE_DECL TSITransferLog
{
	TS_MetaType(TSITransferLog);
public:
	~TSITransferLog(void);

public:
	typedef TSITransferLogPrivate PrivateType;
	PrivateType * _Private_Ptr;
	TSITransferLog(PrivateType * p);

public:
    virtual void SetLoggingLevel(UINT64 Level) = 0;
    virtual UINT64 GetLoggingLevel() = 0;

    virtual void WriteLog(TSString Text, UINT64 Level = INTERFACE_LEVEL0) = 0;
};

CLASS_PTR_DECLARE(TSITransferLog);

#endif // TSITransferLog_h__



