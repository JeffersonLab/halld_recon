// $Id$
//
//

#ifndef _DBeamParticle_factory_
#define _DBeamParticle_factory_

#include <JANA/JFactory.h>
#include <PID/DBeamParticle.h>
#include <DANA/DApplication.h>
#include "DResourcePool.h"

class DBeamParticle_factory:public jana::JFactory<DBeamParticle>
{
	public:
		DBeamParticle_factory(void)
		{
			dResourcePool_BeamParticles = new DResourcePool<DBeamParticle>();
			dResourcePool_BeamParticles->Set_ControlParams(100, 20, 200, 500, 0);

			dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>();
			dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 50);
		}

		void Recycle_Resources(vector<const DBeamParticle*>& locBeams){dResourcePool_BeamParticles->Recycle(locBeams);}
		void Recycle_Resources(vector<DBeamParticle*>& locBeams){dResourcePool_BeamParticles->Recycle(locBeams);}
		void Recycle_Resource(const DBeamParticle* locBeam){dResourcePool_BeamParticles->Recycle(locBeam);}

		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_BeamParticles->Get_NumObjectsAllThreads();}
		DBeamParticle* Get_Resource(void)
		{
			auto locBeam = dResourcePool_BeamParticles->Get_Resource();
			locBeam->Reset();
			return locBeam;
		}

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.

	private:
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t locEventNumber);	///< Called every event.
		jerror_t fini(void)
		{
			for(auto locBeam : _data)
				Recycle_Resource(locBeam);
			_data.clear();
			delete dResourcePool_BeamParticles;
 			return NOERROR;
		}

		double dSkipTaggedPhotons = true;    // default for KLF, fix this before merging
		double dSkipKLongBeam = false;


		//RESOURCE POOL
		vector<DBeamParticle*> dCreated;
		DResourcePool<DBeamParticle>* dResourcePool_BeamParticles = nullptr;
		shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

};

#endif // _DBeamParticle_factory_

