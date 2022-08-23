#include "stdafx.h"

#include "TSDBconnector.h"

#if !defined(XSIM3_3) && ! defined(XSIM3_2)
#include <TopSimRuntime/TSDbAccess.h>
#include <TopSimRuntime/TSSqliteDbAccess.h>
#include <TopSimRuntime/TSMySqlDbAccess.h>
#include <TopSimRuntime/TSOCIDbAccess.h>
#include <TopSimRuntime/TSSqlserverAccess.h>

CLASS_PTR_DECLARE(TSDbAccess);
CLASS_PTR_DECLARE(TSSqliteDbAccess);
CLASS_PTR_DECLARE(TSMySqlDbAccess);
CLASS_PTR_DECLARE(TSSqlserverAccess);

CLASS_PTR_DECLARE(TSOCIDbAccess);


struct TSDBConnectPrivate
{
	TSDBConnectPrivate()
		:_IsConnect(false)
	{

	}

	bool              _IsConnect;
	TSDBConnectInfo   _ConnectInfo;
	TSDbAccessPtr     _DBAccess;
};

TSDBConnectPtr TSDBConnect::MakeDBConnection( const TSDBConnectInfo& Info )
{
	TSDBConnectPtr Connect = boost::make_shared<TSDBConnect>(Info);

	TSDataArchiveContextPtr Ctx = boost::make_shared<TSDataArchiveContext>();

	switch(Info.db_type)
	{
	case DBT_Sqlite:
		{
			Connect->_p->_DBAccess = boost::make_shared<TSSqliteDbAccess>(Ctx,Info.db_name);
		}
		break;
	case DBT_MySQL:
		{
			Connect->_p->_DBAccess = boost::make_shared<TSMySqlDbAccess>(Ctx,Info.db_host,
				Info.db_name,Info.db_usr,Info.db_pwd);
		}
		break;
	case DBT_SqlServer:
		{
			Connect->_p->_DBAccess = boost::make_shared<TSSqlserverAccess>(Ctx,Info.db_host,
				Info.db_name,Info.db_usr,Info.db_pwd);
		}
		break;
	case DBT_Oracle:
		{
			Connect->_p->_DBAccess = boost::make_shared<TSOCIDbAccess>(Ctx,Info.db_host,
				Info.db_name,Info.db_usr,Info.db_pwd);
		}
		break;
	default:
		std::cerr << "invalid db type !";
	}

	return Connect;
}

TSDBConnect::TSDBConnect(const TSDBConnectInfo& ConnectInfo )
	:_p(new TSDBConnectPrivate)
{

}

bool TSDBConnect::Connect()
{
	ASSERT(_p->_DBAccess);

	if(!_p->_IsConnect)
	{
		if(_p->_DBAccess)
		{
			if(_p->_DBAccess->ConnectDB())
			{
				_p->_IsConnect = true;
			}
		}
	}

	return _p->_IsConnect;
}

void TSDBConnect::Disconnect()
{
	ASSERT(_p->_DBAccess);

	if(_p->_IsConnect)
	{
		if(_p->_DBAccess)
		{
			_p->_DBAccess->DisconnectDB();
		}
	}
}

bool TSDBConnect::IsConnect()
{
	return _p->_IsConnect;
}

TSAbstractOODBPtr TSDBConnect::GetOODB()
{
	return _p->_DBAccess;
}

const TSDBConnectInfo& TSDBConnect::GetConnectInfo()
{
	return _p->_ConnectInfo;
}

TSDBConnect::~TSDBConnect()
{
	delete _p;
}

#endif // XSIM3_3


