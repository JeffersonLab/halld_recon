// $Id$
//
//    File: DNeutralParticle_factory_PreSelect.h
// Created: Mon Dec  7 14:29:24 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DNeutralParticle_factory_PreSelect_
#define _DNeutralParticle_factory_PreSelect_

#include <JANA/JFactory.h>
#include <PID/DNeutralParticle.h>
#include <PID/DNeutralShower.h>

using namespace std;
using namespace jana;

class DNeutralParticle_factory_PreSelect : public jana::JFactory<DNeutralParticle>
{
	public:
		DNeutralParticle_factory_PreSelect(){};
		~DNeutralParticle_factory_PreSelect(){};
		const char* Tag(void){return "PreSelect";}

		void Recycle_Hypotheses(vector<DNeutralParticle*>& locHypos){dResourcePool_NeutralParticle->Recycle(locHypos);}
		void Recycle_Hypotheses(vector<const DNeutralParticle*>& locHypos){dResourcePool_NeutralParticle->Recycle(locHypos);}
		void Recycle_Hypothesis(const DNeutralParticle* locHypo){dResourcePool_NeutralParticle->Recycle(locHypo);}

		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_NeutralParticle->Get_NumObjectsAllThreads();}
		DNeutralParticle* Get_Resource(void)
		{
			auto locHypo = dResourcePool_NeutralParticle->Get_Resource();
			return locHypo;
		}

	private:
		//RESOURCE POOL
		vector<DNeutralParticle*> dCreated;
		DResourcePool<DNeutralParticle>* dResourcePool_NeutralParticle = nullptr;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

		//Command-line values will override these
		double dMaxNeutronBeta; //PRESELECT:MAX_NEUTRON_BETA 
};

#endif // _DNeutralParticle_factory_PreSelect_

