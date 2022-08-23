#ifndef __TSDBCONNECTOR_H__
#define __TSDBCONNECTOR_H__

#include "TopSimDataInterface.h"

#if !defined(XSIM3_3) && !defined(XSIM3_2)
#include <TopSimRuntime/TSAbstractOODB.h>
#include <TopSimRuntime/TSDbAccess.h>


CLASS_PTR_DECLARE(TSDBConnect);
CLASS_PTR_DECLARE(TSAbstractOODB);

struct TSDBConnectPrivate;
class TOPSIMDATAINTERFACE_DECL TSDBConnect
{
public:
	enum DBType
	{
		DBT_Sqlite,
		DBT_Oracle,
		DBT_MySQL,
		DBT_SqlServer
	};
	static TSDBConnectPtr MakeDBConnection(const TSDBConnectInfo& Info);

public:
	TSDBConnect(const TSDBConnectInfo& ConnectInfo);
	~TSDBConnect();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	连接. </summary>
	///
	/// <remarks>	孟龙龙, 2017/8/31. </remarks>
	///
	/// <returns>	true if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Connect();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	断开连接. </summary>
	///
	/// <remarks>	孟龙龙, 2017/8/31. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	void Disconnect();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	是否连接. </summary>
	///
	/// <remarks>	孟龙龙, 2017/8/31. </remarks>
	///
	/// <returns>	true if connect, false if not. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsConnect();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	获取抽象OODB对象. </summary>
	///
	/// <remarks>	孟龙龙, 2017/8/31. </remarks>
	///
	/// <returns>	The oodb. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	TSAbstractOODBPtr GetOODB();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets the 连接信息. </summary>
	///
	/// <remarks>	孟龙龙, 2017/8/31. </remarks>
	///
	/// <returns>	The connect information. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	const TSDBConnectInfo& GetConnectInfo();

private:
	TSDBConnectPrivate * _p;
};

struct TSIDbOperator
{
	virtual void GetRecords(TSDBConnectPtr Connect,std::vector<TSDataUniquelyId>& Records) = 0;
	virtual bool Save(TSDBConnectPtr Connect,const TSVariant& Data,const TSVariantHashMap& AdditionData = TSVariantHashMap()) = 0;
	virtual std::pair<TSVariant,TSVariantHashMap> Load(TSDBConnectPtr Connect,const TSDataUniquelyId& Record) = 0;
	virtual void Load(TSDBConnectPtr Connect,std::vector<std::pair<TSDataUniquelyId,std::pair<TSVariant,TSVariantHashMap> > >& Objs,const TSVariantHashMap& Conditions = TSVariantHashMap()) = 0;
	virtual bool Delete(TSDBConnectPtr Connect,const TSDataUniquelyId& Record) = 0;
	virtual bool Update(TSDBConnectPtr Connect,const TSDataUniquelyId& Record,const TSVariant& Data,const TSVariantHashMap& AdditionData = TSVariantHashMap()) = 0;
};

#endif //defined(XSIM3_3) || defined(XSIM3_2)



#endif