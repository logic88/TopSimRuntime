#include <iostream>
#include "TSDiscoveryProxy.h"
#include "TopSimRuntime/TSTcpSocketStream.h"
#include "TopSimDataInterface/TSQosStruct.h"
#include "TopSimDataInterface/TSTopicTypeManager.h"
#include "TopSimRuntime/TSLogsUtil.h"
#include "boost/algorithm/string/replace.hpp"
#include "boost/thread.hpp"
#include "boost/chrono.hpp"

TSDiscoveryProxy::TSDiscoveryProxy()
{
	TSDiscoveryProxy::Initialize();
}

TSDiscoveryProxy::~TSDiscoveryProxy()
{

}

void TSDiscoveryProxy::Initialize()
{
	_Connected = false;
	_IsRunning= true;
	_Socket = boost::make_shared<TSTcpSocketStream>();
	_ThreadWork = boost::thread(boost::bind(&TSDiscoveryProxy::WorkThread, this));//处理服务器消息
}

void TSDiscoveryProxy::Clean()
{
	_IsRunning = false;
	if (_Socket->IsOpened())
	{
		_Socket->Close();
	}
	_Connected = false;
	_ThreadWork.join();
}

void TSDiscoveryProxy::OnCreateDomain(TSDomainId domainID)
{
	//告知服务器加入域
	SendCmd(APPEND_DOMAIN, domainID);
}

void TSDiscoveryProxy::OnDestroyDomain(TSDomainId domainID)
{
	//告知服务器离开域
	SendCmd(REMOVE_DOMAIN,domainID);
}

void TSDiscoveryProxy::WorkThread()
{
	TSString discoveryPackageNum;
	while (_IsRunning)
	{
		if (_Socket->IsOpened())
		{
			if (_Socket->IsOpened()&&_Connected)//已连接,只接收消息
			{
				//Todo:需要一种更好的修改发现地址的方式
				TSString domainID="";
				TSString ipAddress = "";
				bool result = false;
				UINT8 ret = _Socket->ReadUInt8(&result);
				if (result)
				{
					boost::unique_lock<boost::mutex>  lock(_Mutex);
					switch (ret)
					{
					case APPEND_HOST://添加主机
					{
						domainID = _Socket->ReadString();
						ipAddress = _Socket->ReadString();

						ParticipantQosPolicy participantQosPolicy;
						if (DataManager->GetParticipantQosPolicy(domainID, participantQosPolicy))
						{
							DiscoveryQosPolicy& discoveryQosPolicy = participantQosPolicy._discoveryQosPolicy;
							TSString& initial_peers = discoveryQosPolicy._initialPeers;
							TSStringArray peersList = TSStringUtil::Split(initial_peers, ";");
							TSString foundPeer = "";
							for (TSStringArray::iterator it = peersList.begin(); it != peersList.end(); ++it)
							{
								TSStringArray strArray = TSStringUtil::Split(*it, "://");
								if (strArray.size() == 2)
								{
									if (strArray[1] == ipAddress)
									{//已添加主机
										foundPeer = *it;
										break;
									}
								}
							}
							if (foundPeer.empty())//未找到对应地址,可以添加
							{
								switch (participantQosPolicy._transportBuiltinQosPolicy._transportBuiltinMask)
								{
								case 0:
									break;
								case 1:
									ipAddress = discoveryPackageNum + "@builtin.udpv4://" + ipAddress;
									break;
								case 2:
									break;
								default:
									break;
								}
								initial_peers = ipAddress + ";" + initial_peers;
								if (DataManager->SetParticipantQosPolicy(domainID, participantQosPolicy))
								{
									
								}
							}
						}
						break;
					}
					case REMOVE_HOST://移除主机
					{
						domainID = _Socket->ReadString();
						ipAddress = _Socket->ReadString();

						ParticipantQosPolicy participantQosPolicy;

						if (DataManager->GetParticipantQosPolicy(domainID, participantQosPolicy))
						{
							DiscoveryQosPolicy& discoveryQosPolicy = participantQosPolicy._discoveryQosPolicy;
							TSString& initial_peers = discoveryQosPolicy._initialPeers;
							TSStringArray peersList = TSStringUtil::Split(initial_peers, ";");
							TSString foundPeer="";
							for (TSStringArray::iterator it = peersList.begin(); it != peersList.end(); ++it)
							{
								TSStringArray strArray = TSStringUtil::Split(*it, "://");
								if (strArray.size() == 2)
								{
									if (strArray[1] == ipAddress)
									{//已添加主机
										foundPeer = *it;
										break;
									}
								}
							}
							if (!foundPeer.empty())//找到对应地址,可以移除
							{
								boost::replace_all(initial_peers, foundPeer + ";", "");
								boost::replace_all(initial_peers, foundPeer, "");
								boost::replace_all(initial_peers, ";;", ";");
								if (DataManager->SetParticipantQosPolicy(domainID, participantQosPolicy))
								{
								}
							}
						}
						break;
					}
					case INVALID:
						break;
					default:
						break;
					}
				}
				else
				{//服务器断开
					boost::unique_lock<boost::mutex>  lock(_Mutex);
					if (_Socket->IsOpened())
					{
						_Socket->Close();
					}
					_Connected = false;
				}
			}
			else if (_Socket->IsOpened()&&_Socket->Connect())//尝试连接,且成功
			{
				{
					_Connected = true;
					//发现包个数;
					discoveryPackageNum = TSTopicTypeManager::Instance()->GetExtendCfg("DiscoveryPackageNum");
					discoveryPackageNum = discoveryPackageNum.empty() ? "4" : discoveryPackageNum;
				}
				SendCmd(CONNECT);
				TSStringArray domainIDList;
				DataManager->GetJoinedDomains(domainIDList);//每次连接将加入的域告诉服务器
				for (TSStringArray::iterator it = domainIDList.begin(); it != domainIDList.end(); ++it)
				{
					SendCmd(APPEND_DOMAIN, *it);
				}
			}
			else if (_Socket->IsOpened())
			{
				//DEF_LOG_ERROR(GB18030ToTSString("无法连接到中心服务器"));
				_Socket->Close();
			}
	
		}
		else//Socket未成功打开
		{
			TSString address = TSTopicTypeManager::Instance()->GetExtendCfg("CenterServer");
			if (!address.empty())
			{
				if (!_Connected)
				{
					TSStringArray strArrary = TSStringUtil::Split(address, ":");
					if (strArrary.size() == 2)
					{
						TSString server_ip = strArrary[0];
						int  server_port = atoi(strArrary[1].c_str());
						if (_Socket->Open(server_ip.c_str(), server_port))
						{	
						}
						else
						{
							//DEF_LOG_ERROR(GB18030ToTSString("TCP套接字打开失败"));
						}
					}
					else
					{
						//DEF_LOG_ERROR(GB18030ToTSString("中心发现服务地址解析错误,请确认设置格式(ip:port)"));
					}
				}
			}
			else
			{
				//DEF_LOG_ERROR(GB18030ToTSString("ExtendConfigs中未找到中心服务器设置(CenterServer)"));
			}
		}
	}
}

void TSDiscoveryProxy::SendCmd(CMD cmd, TSString domainID)
{
	boost::unique_lock<boost::mutex>  lock(_Mutex);
	if (_Socket->IsOpened()&&_Connected)
	{
		_Socket->WriteUInt8(cmd);
		switch (cmd)
		{
		case CONNECT:
			_Socket->WriteUInt32(DataManager->GetFrontAppUniquelyId());
			break;
		case APPEND_DOMAIN:
			_Socket->WriteString(domainID);
			break;
		case REMOVE_DOMAIN:
			_Socket->WriteString(domainID);
			break;
		default:
			break;
		}
	}
}
