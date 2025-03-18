// $Id$
//
//    File: DEventRFBunch_factory_CalorimeterOnly.h
//

#ifndef _DEventRFBunch_factory_CalorimeterOnly_
#define _DEventRFBunch_factory_CalorimeterOnly_

#include <iostream>
#include <iomanip>
#include <map>
#include <utility>
#include <deque>
#include <vector>

#include <TMath.h>

#include <JANA/JFactoryT.h>

#include <DVector3.h>
#include <DMatrix.h>

#include <PID/DDetectorMatches.h>
#include <PID/DParticleID.h>
#include <PID/DEventRFBunch.h>
#include <RF/DRFTime.h>
#include <TRACKING/DTrackTimeBased.h>
#include <START_COUNTER/DSCHit.h>
#include <BCAL/DBCALShower.h>
#include <TOF/DTOFPoint.h>
#include <PID/DBeamPhoton.h>

#include <HDGEOMETRY/DGeometry.h>

using namespace std;


class DEventRFBunch_factory_CalorimeterOnly : public JFactoryT<DEventRFBunch>
{
	public:
		DEventRFBunch_factory_CalorimeterOnly(){
			SetTag("CalorimeterOnly");
		}
		~DEventRFBunch_factory_CalorimeterOnly(){};

	private:

		jerror_t Select_RFBunch(const std::shared_ptr<const JEvent>& event, const DRFTime* locRFTime);
		int Conduct_Vote(const std::shared_ptr<const JEvent>& event, double locRFTime, vector<pair<double, const JObject*> >& locTimes, bool locUsedTracksFlag, int& locHighestNumVotes);

		bool Find_NeutralTimes(const std::shared_ptr<const JEvent>& event, vector<pair<double, const JObject*> >& locTimes);

		int Find_BestRFBunchShifts(double locRFHitTime, const vector<pair<double, const JObject*> >& locTimes, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);

		bool Break_TieVote_BeamPhotons(vector<const DBeamPhoton*>& locBeamPhotons, double locRFTime, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts, int locHighestNumVotes);
		//int Break_TieVote_Tracks(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);
		int Break_TieVote_Neutrals(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);

		//jerror_t Select_RFBunch_NoRFTime(const std::shared_ptr<const JEvent>& event, vector<const DTrackTimeBased*>& locTrackTimeBasedVector);

		//void Get_RFTimeGuess(vector<pair<double, const JObject*> >& locTimes, double& locRFTimeGuess, double& locRFVariance) const;

		jerror_t Create_NaNRFBunch(void);

		const DParticleID* dParticleID;

		double dBeamBunchPeriod;
		DVector3 dTargetCenter;

		double dMinTrackingFOM;
		
		bool USE_FCAL;
		bool USE_BCAL;
		bool USE_CCAL;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DEventRFBunch_factory_

