#ifndef _DChargedTrack_factory_Combo_
#define _DChargedTrack_factory_Combo_

#include <string>
#include <vector>
#include <unordered_map>

#include <JANA/JFactoryT.h>
#include <TRACKING/DTrackTimeBased.h>
#include "PID/DEventRFBunch.h"
#include "PID/DDetectorMatches.h"
#include <PID/DChargedTrack.h>
#include <PID/DChargedTrackHypothesis.h>
#include "PID/DChargedTrackHypothesis_factory.h"

using namespace std;

class DChargedTrack_factory_Combo : public JFactoryT<DChargedTrack>
{
	public:
		DChargedTrack_factory_Combo(){
			SetTag("Combo");
		}
		~DChargedTrack_factory_Combo() = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;

		string dTrackSelectionTag;
		DChargedTrackHypothesis_factory* dChargedTrackHypothesisFactory;
		vector<DChargedTrackHypothesis*> dCreatedHypotheses;
};

#endif // _DChargedTrack_factory_Combo_

