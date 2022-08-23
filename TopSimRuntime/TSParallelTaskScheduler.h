#ifndef __TSDISCOVERYPROXY_H__
#define __TSDISCOVERYPROXY_H__

#include "TopSimRuntime/TSTcpSocketStream.h"
#include "TopSimDataInterface/TSDataInteractManager.h"
#include "TopSimDataInterface/TSDomain.h"

#include "boost/thread/mutex.hpp"
#include <vector>

enum CMD
{
	INVALID,
	CONNECT,
	APPEND_DOMAIN,
	REMOVE_DOMAIN,
	APPEND_HOST,
	REMOVE_HOST
};

CLASS_PTR_DECLARE(TSDiscoveryProxy);
class TSDiscoveryProxy
{
public:
	void Initialize();
	void Clean();
	void OnCreateDomain(TSDomainId);
	void OnDestroyDomain(TSDomainId);

	TSDiscoveryProxy();
	~TSDiscoveryProxy();
private:
/*
	void GetHostNameAndIpP(unsigned int & hostIp, std::string & hostName);
	void ConvertIP2StringP(unsigned int ip, std::string & strIp);//获取本机ip点分串
*/

	void WorkThread();//服务器通信

	void SendCmd(CMD,TSString domainID="");//发消息给服务器

private:
	TSTcpSocketStreamPtr _Socket;//用于与服务器通信
	//TSString _IPAddress;//本节点IP

	boost::thread _ThreadWork;//子线程,用于服务器通信

	boost::mutex    _Mutex;//保证连接状态

	boost::atomic_bool _IsRunning;//子线程控制
	boost::atomic_bool _Connected;//TCP连接状态
};

#endif !__TSDISCOVERYPROXY_H__
