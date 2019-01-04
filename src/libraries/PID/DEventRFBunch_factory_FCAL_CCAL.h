// $Id$
//
//    File: DEventRFBunch_factory_FCAL_CCAL.h
//

#ifndef _DEventRFBunch_factory_FCAL_CCAL_
#define _DEventRFBunch_factory_FCAL_CCAL_

#include <iostream>
#include <iomanip>
#include <map>
#include <utility>
#include <deque>
#include <vector>

#include <TMath.h>

#include <JANA/JFactory.h>

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
#include <DANA/DApplication.h>

using namespace std;
using namespace jana;

class DEventRFBunch_factory_FCAL_CCAL : public jana::JFactory<DEventRFBunch>
{
	public:
		DEventRFBunch_factory_FCAL_CCAL(){};
		~DEventRFBunch_factory_FCAL_CCAL(){};
		const char* Tag(void){return "FCAL_CCAL";}

	private:

		jerror_t Select_RFBunch(JEventLoop* locEventLoop, const DRFTime* locRFTime);
		int Conduct_Vote(JEventLoop* locEventLoop, double locRFTime, vector<pair<double, const JObject*> >& locTimes, bool locUsedTracksFlag, int& locHighestNumVotes);

		bool Find_NeutralTimes(JEventLoop* locEventLoop, vector<pair<double, const JObject*> >& locTimes);

		int Find_BestRFBunchShifts(double locRFHitTime, const vector<pair<double, const JObject*> >& locTimes, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);

		bool Break_TieVote_BeamPhotons(vector<const DBeamPhoton*>& locBeamPhotons, double locRFTime, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts, int locHighestNumVotes);
		//int Break_TieVote_Tracks(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);
		int Break_TieVote_Neutrals(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);

		//jerror_t Select_RFBunch_NoRFTime(JEventLoop* locEventLoop, vector<const DTrackTimeBased*>& locTrackTimeBasedVector);

		//void Get_RFTimeGuess(vector<pair<double, const JObject*> >& locTimes, double& locRFTimeGuess, double& locRFVariance) const;

		jerror_t Create_NaNRFBunch(void);

		const DParticleID* dParticleID;

		double dBeamBunchPeriod;
		DVector3 dTargetCenter;

		double dMinTrackingFOM;
		
		bool USE_FCAL;
		bool USE_BCAL;
		bool USE_CCAL;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *locEventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *locEventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _DEventRFBunch_factory_

