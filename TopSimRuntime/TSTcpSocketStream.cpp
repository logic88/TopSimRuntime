#include "stdafx.h"
#include "TopSimDataInterface_LANG.h"

#if defined(XSIM3_3) || defined(XSIM3_2)

#include <boost/atomic.hpp>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_queue.h>
#include <TopSimTypes/TSRWSpinLock.h>
#include <TopSimTypes/TSGenericHandleTable.h>
#include <TopSimUtil/TSConversion.h>
#include <TopSimDataInterface/TSTopicTypeManager.h>

#include "TSSpaceGridSpec.h"
#include "TSSpaceGridSpec_p.h"

typedef TSConArray<UINT64> UUIDArray_T;

struct TSSpaceCell 
{
	UUIDArray_T Objects;
	tbb::concurrent_queue<std::size_t> FreeSlots;
	std::size_t AddObject(UINT64 hObj);
	void RemoveObject(std::size_t ObjectSlot);
};


TSSpaceCellContainer::~TSSpaceCellContainer()
{
	Cleanup();
}

TSSpaceCellPtr TSSpaceCellContainer::GetCell( std::size_t index,bool bCreate )
{
	++index;

	TSSpaceCellPtr result = GetObjectFromHandle(index);

	if (!result && bCreate)
	{
		result = boost::make_shared<TSSpaceCell>();

		TSHANDLE h = VerifyHandle(result,index);
		result = GetObjectFromHandle(h);
	}

	return result;
}

void TSSpaceCellContainer::Cleanup()
{

}

TSSpaceGridSpecPrivate::TSSpaceGridSpecPrivate()
	:_NumberOfRows(180),_NumberOfColumns(360),_NorthLat(PI_2),_SouthLat(-PI_2),_EastLon(PI),_WestLon(-PI),_LonScale(PI),_LatScale(PI),_LatDistance(0)
{
	
}

BEGIN_METADATA(TSISpatialObject)
	END_METADATA()

	std::size_t TSSpaceCell::AddObject( UINT64 hObj )
{
	std::size_t FreeSlot;

	if(FreeSlots.try_pop(FreeSlot))
	{
		Objects[FreeSlot] = hObj;
	}
	else
	{
		UUIDArray_T::iterator it = Objects.push_back(hObj);

		FreeSlot = it - Objects.begin();
	}

	return FreeSlot;
}

void TSSpaceCell::RemoveObject( std::size_t ObjectSlot )
{
	if (ObjectSlot < Objects.size())
	{
		FreeSlots.push(ObjectSlot);

		Objects[ObjectSlot] = 0;
	}
}

class TSDefaultObjectSpaceCellSpec : public TSIObjectSpaceCellSpec
{
public:
	TSDefaultObjectSpaceCellSpec()
		:_CellId(-1),_Slot(-1)
	{

	}
	virtual void SetCell(int CellId);
	virtual int GetCell();
	virtual void SetSlot(std::size_t Slot);
	virtual std::size_t GetSlot();
private:
	int _CellId;
	std::size_t _Slot;
};

void TSDefaultObjectSpaceCellSpec::SetCell( int CellId )
{
	_CellId = CellId;
}

int TSDefaultObjectSpaceCellSpec::GetCell()
{
	return _CellId;
}

void TSDefaultObjectSpaceCellSpec::SetSlot( std::size_t Slot )
{
	_Slot = Slot;
}

std::size_t TSDefaultObjectSpaceCellSpec::GetSlot()
{
	return _Slot;
}

BEGIN_METADATA(TSSpaceGridSpec)
	REG_BASE(TSObject);
REG_PROP(NumberOfRows,CN_TOPSIMDATAINTERFACE_TSSPACEGRIDSPEC_CPP_130);
REG_PROP(NumberOfColumns,CN_TOPSIMDATAINTERFACE_TSSPACEGRIDSPEC_CPP_131);
END_METADATA()

TSSpaceGridSpec::TSSpaceGridSpec( void )
{
	_Private_Ptr = new TSSpaceGridSpecPrivate;
}

TSSpaceGridSpec::~TSSpaceGridSpec( void )
{
	delete _Private_Ptr;
}

DOUBLE TSSpaceGridSpec::GetNorthLat() const
{
	return GetPP()->_NorthLat;
}

void TSSpaceGridSpec::SetNorthLat( DOUBLE NorthLat )
{
	GetPP()->_NorthLat = NorthLat;
}

DOUBLE TSSpaceGridSpec::GetSouthLat() const
{
	return GetPP()->_SouthLat;
}

void TSSpaceGridSpec::SetSouthLat( DOUBLE SouthLat )
{
	GetPP()->_SouthLat = SouthLat;
}

DOUBLE TSSpaceGridSpec::GetEastLon() const
{
	return GetPP()->_EastLon;
}

void TSSpaceGridSpec::SetEastLon( DOUBLE EastLon )
{
	GetPP()->_EastLon = EastLon;
}

DOUBLE TSSpaceGridSpec::GetWestLon() const
{
	return GetPP()->_WestLon;
}

void TSSpaceGridSpec::SetWestLon( DOUBLE WestLon )
{
	GetPP()->_WestLon = WestLon;
}

UINT16 TSSpaceGridSpec::GetNumberOfRows() const
{
	return GetPP()->_NumberOfRows;
}

void TSSpaceGridSpec::SetNumberOfRows( UINT16 NumberOfRows )
{
	GetPP()->_NumberOfRows = NumberOfRows;
}

UINT16 TSSpaceGridSpec::GetNumberOfColumns() const
{
	return GetPP()->_NumberOfColumns;
}

void TSSpaceGridSpec::SetNumberOfColumns( UINT16 NumberOfColumns )
{
	GetPP()->_NumberOfColumns = NumberOfColumns;
}

DOUBLE TSSpaceGridSpec::GetLonScale() const
{
	return GetPP()->_LonScale;
}

DOUBLE TSSpaceGridSpec::GetLatScale() const
{
	return GetPP()->_LatScale;
}

DOUBLE TSSpaceGridSpec::GetLatDistance() const
{
	return GetPP()->_LatDistance;
}

DOUBLE TSSpaceGridSpec::GetLonDistance( int LatHash ) const
{
	if (LatHash == 0)
	{
		double LonDistance = EARTHRADIUS * cos(GetPP()->_SouthLat + LatHash * GetPP()->_LatScale) * GetPP()->_LonScale;

		return LonDistance;
	}
	else if (LatHash > 0)
	{
		double dblSouthSideDis = EARTHRADIUS * cos(GetPP()->_SouthLat + (LatHash - 1) * GetPP()->_LatScale) * GetPP()->_LonScale;
		double dblNorthSideDis = EARTHRADIUS * cos(GetPP()->_SouthLat + LatHash * GetPP()->_LatScale) * GetPP()->_LonScale;

		return MIN(dblSouthSideDis,dblNorthSideDis);
	}

	return EARTHRADIUS;
}

bool TSSpaceGridSpec::Initialize()
{
	DOUBLE	NorthLat = GetPP()->_NorthLat;
	DOUBLE	SouthLat = GetPP()->_SouthLat;
	DOUBLE	EastLon = GetPP()->_EastLon; 
	DOUBLE	WestLon = GetPP()->_WestLon; 

	GetPP()->_NorthLat = MAX(SouthLat,NorthLat);
	GetPP()->_SouthLat = MIN(SouthLat,NorthLat);

	GetPP()->_EastLon = MAX(WestLon,EastLon);
	GetPP()->_WestLon = MIN(WestLon,EastLon);


	// 区间经度跨度（(东经度-西经度)/划分数量）
	GetPP()->_LonScale = (GetPP()->_EastLon - GetPP()->_WestLon) / (DOUBLE)GetPP()->_NumberOfColumns;

	// 区间纬度跨度（(北纬度-南纬度)/ 划分数量）。
	GetPP()->_LatScale = (GetPP()->_NorthLat - GetPP()->_SouthLat) / (DOUBLE)GetPP()->_NumberOfRows;

	GetPP()->_LatDistance = EARTHRADIUS * GetPP()->_LatScale;		// 区间纬度区间跨度距离

	return true;
}


void TSSpaceGridSpec::UpdateObjectCell( TSISpatialObjectPtr SpatialObject )
{
	if (SpatialObject)
	{
		TSIObjectSpaceCellSpecPtr ObjectSpaceCellSpec = SpatialObject->GetObjectSpaceCellSpec();

		if (!ObjectSpaceCellSpec)
		{
			ObjectSpaceCellSpec = boost::make_shared<TSDefaultObjectSpaceCellSpec>();

			SpatialObject->SetObjectSpaceCellSpec(ObjectSpaceCellSpec);
		}

		DOUBLE Lat,Lon;

		TSConversion::ECEFtoLL(SpatialObject->Position(),Lat,Lon);

		int CellIndex = GetCellIndex( Lat,Lon );
		int OriCellIndex = ObjectSpaceCellSpec->GetCell();

		if (CellIndex!=OriCellIndex)
		{
			if (OriCellIndex!=-1)
			{
				if (TSSpaceCellPtr Cell = GetPP()->_Cells.GetCell(OriCellIndex,false))
				{
					Cell->RemoveObject(ObjectSpaceCellSpec->GetSlot());
				}
			}

			TSSpaceCellPtr Cell = GetPP()->_Cells.GetCell(CellIndex,true);

			ObjectSpaceCellSpec->SetSlot(Cell->AddObject(SpatialObject->Handle()));
			ObjectSpaceCellSpec->SetCell(CellIndex);
		}

	}

}

int TSSpaceGridSpec::HashLatitude( DOUBLE dblLatitude )
{
	int latHash =int((dblLatitude - GetPP()->_SouthLat) / GetPP()->_LatScale);

	if( latHash < 0 || latHash >= GetPP()->_NumberOfRows )
	{
		return 0;
	}

	return latHash + 1;
}

int TSSpaceGridSpec::HashLongitude( DOUBLE dblLongitude )
{
	int lonHash = int((dblLongitude - GetPP()->_WestLon) / GetPP()->_LonScale);

	if( lonHash < 0 || lonHash >= GetPP()->_NumberOfColumns )
	{
		return 0;
	}

	return lonHash + 1;
}

int TSSpaceGridSpec::GetCellIndex( DOUBLE dblLatitude,DOUBLE dblLongitude )
{
	int latHash = HashLatitude(dblLatitude);
	int lonHash = HashLongitude(dblLongitude);

	if (latHash==0 || lonHash==0)
	{
		return 0;
	}

	return latHash * GetPP()->_NumberOfColumns + lonHash;
}

UINT64 TSSpaceGridSpec::GetCellNextObject( int CellId,std::size_t & NextSlot )
{
	if (TSSpaceCellPtr Cell = GetPP()->_Cells.GetCell(CellId,false))
	{
		while (NextSlot < Cell->Objects.size())
		{
			UINT64 hObject = Cell->Objects[NextSlot];

			++NextSlot;

			if (hObject)
			{
				return hObject;
			}
		}
	}

	return 0;
}

void TSSpaceGridSpec::Cleanup()
{
	GetPP()->_Cells.Cleanup();
}

void TSSpaceGridSpec::SetObjectEntryCell( TSISpatialObjectPtr SpatialObject,TSIObjectSpaceCellSpecPtr ObjectSpaceCellSpec )
{
	ASSERT(SpatialObject);
	ASSERT(ObjectSpaceCellSpec);

	SpatialObject->SetObjectSpaceCellSpec(ObjectSpaceCellSpec);
}

TSIObjectSpaceCellSpecPtr TSSpaceGridSpec::GetObjectEntryCell( TSISpatialObjectPtr SpatialObject )
{
	ASSERT(SpatialObject);

	return SpatialObject->GetObjectSpaceCellSpec();
}

#endif


