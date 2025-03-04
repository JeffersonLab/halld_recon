// $Id$
//
//    File: DBeamPhoton_factory.h
// Created: Mon Aug  5 14:29:24 EST 2014
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DBeamPhoton_factory_
#define _DBeamPhoton_factory_

#include <JANA/JFactoryT.h>
#include <PID/DBeamPhoton.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>
#include "DResourcePool.h"

class DBeamPhoton_factory:public JFactoryT<DBeamPhoton>
{
	public:
		DBeamPhoton_factory(void)
		{
			dResourcePool_BeamPhotons = new DResourcePool<DBeamPhoton>();
			dResourcePool_BeamPhotons->Set_ControlParams(100, 20, 200, 500, 0);

			dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>();
			dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 50);
		}

		void Recycle_Resources(vector<const DBeamPhoton*>& locBeams){dResourcePool_BeamPhotons->Recycle(locBeams);}
		void Recycle_Resources(vector<DBeamPhoton*>& locBeams){dResourcePool_BeamPhotons->Recycle(locBeams);}
		void Recycle_Resource(const DBeamPhoton* locBeam){dResourcePool_BeamPhotons->Recycle(locBeam);}

		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_BeamPhotons->Get_NumObjectsAllThreads();}
		DBeamPhoton* Get_Resource(void)
		{
			auto locBeam = dResourcePool_BeamPhotons->Get_Resource();
			locBeam->Reset();
			return locBeam;
		}

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;

	private:
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override
		{
			for(auto locBeam : mData)
				Recycle_Resource(locBeam);
			mData.clear();
			delete dResourcePool_BeamPhotons;
		}

		double dTargetCenterZ;

		//RESOURCE POOL
		vector<DBeamPhoton*> dCreated;
		DResourcePool<DBeamPhoton>* dResourcePool_BeamPhotons = nullptr;
		shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

		// config. parameters
		double DELTA_T_DOUBLES_MAX;
		double DELTA_E_DOUBLES_MAX;

		void Set_BeamPhoton(DBeamPhoton* gamma, const DTAGHHit* hit, uint64_t locEventNumber);
		void Set_BeamPhoton(DBeamPhoton* gamma, const DTAGMHit* hit, uint64_t locEventNumber);
};

#endif // _DBeamPhoton_factory_

