// $Id$
//
//

#ifndef _DBeamKLong_factory_
#define _DBeamKLong_factory_

#include <JANA/JFactoryT.h>
#include <PID/DBeamKLong.h>
#include <DANA/DApplication.h>
#include "DResourcePool.h"

class DBeamKLong_factory : public JFactoryT<DBeamKLong>
{
	public:
		DBeamKLong_factory(void)
		{
			dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>();
			dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 50);
		}

// 		void Recycle_Resources(vector<const DBeamKLong*>& locBeams){dResourcePool_BeamKLongs->Recycle(locBeams);}
// 		void Recycle_Resources(vector<DBeamKLong*>& locBeams){dResourcePool_BeamKLongs->Recycle(locBeams);}
// 		void Recycle_Resource(const DBeamKLong* locBeam){dResourcePool_BeamKLongs->Recycle(locBeam);}
// 
// 		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_BeamKLongs->Get_NumObjectsAllThreads();}
// 		DBeamKLong* Get_Resource(void)
// 		{
// 			auto locBeam = dResourcePool_BeamKLongs->Get_Resource();
// 			locBeam->Reset();
// 			return locBeam;
// 		}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override; 
		void Process(const std::shared_ptr<const JEvent>& event) override; 
// 		void EndRun() override; 
// 		void Finish() override;

		bool dEnableKLongBeamRecon;
		double dTargetCenterZ;


		//RESOURCE POOL
		vector<DBeamKLong*> dCreated;
		//DResourcePool<DBeamKLong>* dResourcePool_BeamKLongs = nullptr;
		shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

};

#endif // _DBeamKLong_factory_

