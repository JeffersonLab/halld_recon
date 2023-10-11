// $Id$
//
//    File: DNeutralParticle_factory_PreSelect.h
// Created: Mon Dec  7 14:29:24 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DNeutralParticle_factory_PreSelect_
#define _DNeutralParticle_factory_PreSelect_

#include <JANA/JFactoryT.h>
#include <PID/DNeutralParticle.h>
#include <PID/DNeutralShower.h>

using namespace std;


class DNeutralParticle_factory_PreSelect : public JFactoryT<DNeutralParticle>
{
	public:
		DNeutralParticle_factory_PreSelect(){
			SetTag("PreSelect");
		}
		~DNeutralParticle_factory_PreSelect(){};

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

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		//Command-line values will override these
		double dMaxNeutronBeta; //PRESELECT:MAX_NEUTRON_BETA 
};

#endif // _DNeutralParticle_factory_PreSelect_

