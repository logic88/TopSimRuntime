#ifndef __TSNTPTIMEHOLDER_H__
#define __TSNTPTIMEHOLDER_H__

#ifndef  _WRS_KERNEL

#include "TopSimDataInterface.h"

#include <TopSimDataInterface/TSDomain.h>

struct TSNTPTimeHolderPrivate;
class TOPSIMDATAINTERFACE_DECL TSNTPTimeHolder : public TSAbstractTimeHolder
{
	TS_MetaType(TSNTPTimeHolder,TSAbstractTimeHolder);
public:
	TSNTPTimeHolder(void);
	~TSNTPTimeHolder(void);

protected:
	typedef TSNTPTimeHolderPrivate PrivateType;
	PrivateType * _Private_Ptr;
	TSNTPTimeHolder(PrivateType * p);

public:
	virtual TSTime GetTime(const TSDomainId& DomainId,TSTOPICHANDLE THandle,const void * d);

public:
	virtual bool Init(const TSString& IpAddress = "127.0.0.1",UINT16 Port = 28050,
		UINT32 CalibrationInterval = 3000,UINT16 Samples = 1);
	virtual bool IsReady();
	virtual TSTimeDuration GetInterval();
	virtual void Clean();

private:
	void CalibrationTime_p();
	void Start_p();
	void OnTimeout_p(const boost::system::error_code& ec);
	void HandleRead_p(const boost::system::error_code & e, std::size_t bytes_transferred);
	void HandleWrite_p(const boost::system::error_code & e, std::size_t bytes_transferred);
};

typedef boost::shared_ptr<TSNTPTimeHolder> TSNTPTimeHolderPtr;

#endif //_WRS_KERNEL



#endif

