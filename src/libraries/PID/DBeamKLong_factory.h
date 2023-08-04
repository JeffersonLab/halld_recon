// $Id$
//
//

#ifndef _DBeamKLong_factory_
#define _DBeamKLong_factory_

#include <JANA/JFactory.h>
#include <PID/DBeamKLong.h>
#include <DANA/DApplication.h>
#include "DResourcePool.h"

class DBeamKLong_factory:public jana::JFactory<DBeamKLong>
{
	public:
		DBeamKLong_factory(void)
		{
// 			dResourcePool_BeamPhotons = new DResourcePool<DBeamPhoton>();
// 			dResourcePool_BeamPhotons->Set_ControlParams(100, 20, 200, 500, 0);
// 
// 			dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>();
// 			dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 50);
		}

// 		void Recycle_Resources(vector<const DBeamPhoton*>& locBeams){dResourcePool_BeamPhotons->Recycle(locBeams);}
// 		void Recycle_Resources(vector<DBeamPhoton*>& locBeams){dResourcePool_BeamPhotons->Recycle(locBeams);}
// 		void Recycle_Resource(const DBeamPhoton* locBeam){dResourcePool_BeamPhotons->Recycle(locBeam);}
// 
// 		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_BeamPhotons->Get_NumObjectsAllThreads();}
// 		DBeamPhoton* Get_Resource(void)
// 		{
// 			auto locBeam = dResourcePool_BeamPhotons->Get_Resource();
// 			locBeam->Reset();
// 			return locBeam;
// 		}

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.

	private:
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t locEventNumber);	///< Called every event.
		jerror_t fini(void)
		{
// 			for(auto locBeam : _data)
// 				Recycle_Resource(locBeam);
// 			_data.clear();
// 			delete dResourcePool_BeamPhotons;
// 			return NOERROR;
		}

		double dTargetCenterZ;


		//RESOURCE POOL
// 		vector<DBeamPhoton*> dCreated;
// 		DResourcePool<DBeamPhoton>* dResourcePool_BeamPhotons = nullptr;
// 		shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

};

#endif // _DBeamKLong_factory_

