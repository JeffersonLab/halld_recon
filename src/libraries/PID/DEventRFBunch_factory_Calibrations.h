// $Id$
//
//    File: DEventRFBunch_factory_Calibrations.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DEventRFBunch_factory_Calibrations_
#define _DEventRFBunch_factory_Calibrations_

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

#include <TTAB/DTTabUtilities.h>
#include <PID/DDetectorMatches.h>
#include <PID/DParticleID.h>
#include <PID/DEventRFBunch.h>
#include <RF/DRFTDCDigiTime.h>
#include <TRACKING/DTrackWireBased.h>
#include <START_COUNTER/DSCHit.h>

#include <HDGEOMETRY/DGeometry.h>

#include <ANALYSIS/DCutActions.h>

using namespace std;


class DEventRFBunch_factory_Calibrations : public JFactoryT<DEventRFBunch>
{
	public:
		DEventRFBunch_factory_Calibrations(){
			SetTag("Calibrations");
		};
		~DEventRFBunch_factory_Calibrations(){};

	private:

		void Select_GoodTracks(const std::shared_ptr<const JEvent>& event, vector<const DTrackWireBased*>& locSelectedWireBasedTracks) const;
		jerror_t Select_RFBunch(const std::shared_ptr<const JEvent>& event, vector<const DTrackWireBased*>& locTrackWireBasedVector, double locRFTime);

		bool Find_TrackTimes_SC(const DDetectorMatches* locDetectorMatches, const vector<const DTrackWireBased*>& locTrackWireBasedVector, vector<pair<double, const JObject*> >& locTimes) const;
		int Conduct_Vote(const std::shared_ptr<const JEvent>& event, double locRFTime, vector<pair<double, const JObject*> >& locTimes, int& locHighestNumVotes);

		int Find_BestRFBunchShifts(double locRFHitTime, const vector<pair<double, const JObject*> >& locTimes, map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);
		int Break_TieVote_Tracks(map<int, vector<const JObject*> >& locNumBeamBucketsShiftedMap, set<int>& locBestRFBunchShifts);

		jerror_t Create_NaNRFBunch(void);

		const DParticleID* dParticleID;

		double dBeamBunchPeriod;
		DVector3 dTargetCenter;

		DetectorSystem_t dRFTDCSourceSystem;
		double dMinTrackingFOM;
		unsigned int dMinHitRingsPerCDCSuperlayer;
		unsigned int dMinHitPlanesPerFDCPackage;

		DCutAction_TrackHitPattern *dCutAction_TrackHitPattern;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DEventRFBunch_factory_Calibrations_

