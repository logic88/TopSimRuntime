#include "stdafx.h"

#ifndef  _WRS_KERNEL

#undef BOOST_ASIO_HAS_STD_CHRONO

#include <boost/asio/high_resolution_timer.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/chrono/duration.hpp>

#if defined(BOOST_ASIO_HAS_STD_CHRONO)
using namespace std::chrono;
#elif defined(BOOST_ASIO_HAS_BOOST_CHRONO)
using namespace boost::chrono;
#endif

#include "TSNTPTimeHolder.h"
#include "TSNTPTimeHolder_p.h"

BEGIN_METADATA(TSNTPTimeHolder)
	REG_BASE(TSAbstractTimeHolder);
END_METADATA()

	TSNTPTimeHolderPrivate::TSNTPTimeHolderPrivate()
{
	_SendSampleCount = 0;
	_ReceivedSampleCount = 0;
	_CalibrationInterval = 3000;
	_CurrentErrorTime = TSSeconds(0);

	_TimerIOS = boost::make_shared<TSIOSStruct>();
	_TimerIOS->RunAlways();

	_SocketIOS = boost::make_shared<boost::asio::io_service>();
#ifndef __VXWORKS__
	_Work = boost::make_shared<boost::asio::io_service::work>(*_SocketIOS);
#else
	boost::asio::io_service & ios = *_SocketIOS;
	_Work.reset(new boost::asio::io_service::work(ios));
#endif
	_Thread = boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run,_SocketIOS.get()));
}

TSNTPTimeHolder::TSNTPTimeHolder(void)
	:_Private_Ptr(new PrivateType)
{

}

TSNTPTimeHolder::TSNTPTimeHolder( PrivateType * p )
	:_Private_Ptr(p)
{

}


TSNTPTimeHolder::~TSNTPTimeHolder(void)
{
	_Private_Ptr->_TimerIOS->Stop();

	if (_Private_Ptr->_Thread)
	{
		if (_Private_Ptr->_SocketIOS)
		{
			_Private_Ptr->_Work.reset();
			_Private_Ptr->_SocketIOS->stop();
		}

		_Private_Ptr->_Thread->join();
	}

	delete _Private_Ptr;
}

TSTime TSNTPTimeHolder::GetTime( const TSDomainId& DomainId,TSTOPICHANDLE THandle,const void * d )
{
	UNUSED(DomainId);
	UNUSED(THandle);
	UNUSED(d);

	_Private_Ptr->_CurrentErrorTimeLock.BeginRead();
	TSTime CurrentTime = ::high_resolution_clock().now() + _Private_Ptr->_CurrentErrorTime;
	_Private_Ptr->_CurrentErrorTimeLock.EndRead();

	return CurrentTime;
}

bool TSNTPTimeHolder::Init( const TSString& IpAddress,UINT16 Port,UINT32 CalibrationInterval,UINT16 Samples /*= 1*/ )
{
	if(IpAddress.empty() || !Port)
	{
		return false;
	}

	_Private_Ptr->_CalibrationInterval = CalibrationInterval;

	_Private_Ptr->_NTP = boost::make_shared<TSNTPCommon>(Samples);

	boost::asio::ip::udp::resolver resolver(*_Private_Ptr->_SocketIOS);
	boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(),IpAddress,TSValue_Cast<TSString>(Port));

	boost::system::error_code ec;
	boost::asio::ip::udp::resolver::iterator it = resolver.resolve(query,ec);

	if(ec)
	{
		return false;
	}

	_Private_Ptr->_ServiceEp = *it;

	_Private_Ptr->_Socket.reset(new boost::asio::ip::udp::socket(*_Private_Ptr->_SocketIOS,boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),0)));

	_Private_Ptr->_Socket->async_receive_from(
		boost::asio::buffer(&_Private_Ptr->_NTP->_ReceData,LEN_PKT_NOMAC),
		_Private_Ptr->_ReceivedEp,
		boost::bind(&TSNTPTimeHolder::HandleRead_p,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));


	Start_p();

	return true;
}

void TSNTPTimeHolder::Clean()
{
	if (_Private_Ptr->_Socket)
	{
		boost::system::error_code ec;
		_Private_Ptr->_Socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both,ec);
	}
}

void TSNTPTimeHolder::HandleRead_p( const boost::system::error_code & ec, std::size_t bytes_transferred )
{
	if(!ec)
	{
		if(bytes_transferred > 0)
		{
			_Private_Ptr->_NTP->DoReceive(&_Private_Ptr->_NTP->_ReceData,_Private_Ptr->_ReceivedSampleCount++);

			if(_Private_Ptr->_ReceivedSampleCount >= _Private_Ptr->_NTP->_Samples)
			{
				_Private_Ptr->_ReceivedSampleCount = 0;

				l_fp offset;
				s_fp delay;

				_Private_Ptr->_NTP->Clock_Filter(&offset,&delay);
				double Offset = 0.0;
				LFPTOD(&offset, Offset);
				double DelayD = delay * 1.0/NTP_MAXSKW * 0.01;
				double ErrorTime = Offset + DelayD;  //Îó²îÊ±¼ä

				_Private_Ptr->_CurrentErrorTimeLock.BeginWrite();
				_Private_Ptr->_CurrentErrorTime = TSSeconds(ErrorTime);
				_Private_Ptr->_CurrentErrorTimeLock.EndWrite();
			}
		}

		_Private_Ptr->_Socket->async_receive_from(
			boost::asio::buffer(&_Private_Ptr->_NTP->_ReceData,LEN_PKT_NOMAC),
			_Private_Ptr->_ReceivedEp,
			boost::bind(&TSNTPTimeHolder::HandleRead_p,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		_Private_Ptr->_ReceivedSampleCount = 0;
	}
}

void TSNTPTimeHolder::Start_p()
{
	boost::system::error_code ec;
	_Private_Ptr->_Timer.reset(new boost::asio::high_resolution_timer(*_Private_Ptr->_SocketIOS));
	_Private_Ptr->_SocketIOS->post(boost::bind(&TSNTPTimeHolder::OnTimeout_p,this,ec));
}

void TSNTPTimeHolder::OnTimeout_p(const boost::system::error_code& ec)
{
	if(!ec)
	{
		if(!_Private_Ptr->_ReceivedSampleCount 
			&& !_Private_Ptr->_SendSampleCount)
		{
			CalibrationTime_p();
		}

		_Private_Ptr->_Timer->expires_from_now(milliseconds(_Private_Ptr->_CalibrationInterval));
		_Private_Ptr->_Timer->async_wait(boost::bind(&TSNTPTimeHolder::OnTimeout_p,this,boost::asio::placeholders::error));
	}
}

void TSNTPTimeHolder::CalibrationTime_p()
{
	pkt xpkt;

	_Private_Ptr->_NTP->Transmit(xpkt);

	_Private_Ptr->_Socket->async_send_to(boost::asio::buffer(&xpkt, LEN_PKT_NOMAC), 
		_Private_Ptr->_ServiceEp,
		boost::bind(&TSNTPTimeHolder::HandleWrite_p, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void TSNTPTimeHolder::HandleWrite_p( const boost::system::error_code & ec, std::size_t bytes_transferred )
{
	if(!ec && bytes_transferred)
	{
		if(_Private_Ptr->_NTP->_Samples > ++_Private_Ptr->_SendSampleCount)
		{
			CalibrationTime_p();
		}
		else
		{
			_Private_Ptr->_SendSampleCount = 0;
		}
	}

	_Private_Ptr->_SendSampleCount = 0;
}

bool TSNTPTimeHolder::IsReady()
{
	TSRWSpinLock::ReadLock lock(_Private_Ptr->_CurrentErrorTimeLock);
	bool rnt = _Private_Ptr->_CurrentErrorTime == TSSeconds(0) ? false : true;
	return rnt;
}

TSTimeDuration TSNTPTimeHolder::GetInterval()
{
	return _Private_Ptr->_CurrentErrorTime;
}


#endif //_WRS_KERNEL

