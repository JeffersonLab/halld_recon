#ifndef _DNeutralParticle_factory_Combo_
#define _DNeutralParticle_factory_Combo_

#include <string>
#include <vector>
#include <set>

#include <JANA/JFactoryT.h>
#include "PID/DEventRFBunch.h"
#include <PID/DNeutralParticle.h>
#include "PID/DVertex.h"
#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DNeutralShower.h>
#include "PID/DNeutralParticleHypothesis_factory.h"
#include "ANALYSIS/DReaction.h"

using namespace std;

class DNeutralParticle_factory_Combo : public JFactoryT<DNeutralParticle>
{
	public:
		DNeutralParticle_factory_Combo(){
			SetTag("Combo");
		}
		~DNeutralParticle_factory_Combo(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;

		string dShowerSelectionTag;
		set<Particle_t> dNeutralPIDs;
		vector<DNeutralParticleHypothesis*> dCreatedHypotheses;
		DNeutralParticleHypothesis_factory* dNeutralParticleHypothesisFactory;
};

#endif // _DNeutralParticle_factory_Combo_
