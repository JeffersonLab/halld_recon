// $Id$
//
//    File: DDetectorMatches_factory_WireBased.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DDetectorMatches_factory_WireBased_
#define _DDetectorMatches_factory_WireBased_

#include <iostream>
#include <iomanip>
#include <memory>

#include <JANA/JFactoryT.h>
#include <PID/DDetectorMatches.h>
#include <TRACKING/DTrackWireBased.h>
#include <PID/DParticleID.h>
#include <TOF/DTOFPoint.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include <TMath.h>

using namespace std;


class DDetectorMatches_factory_WireBased : public JFactoryT<DDetectorMatches>
{
	public:
		DDetectorMatches_factory_WireBased(){
			SetTag("WireBased");
		};
		virtual ~DDetectorMatches_factory_WireBased(){};

		//called by DDetectorMatches tag=Combo factory
		DDetectorMatches* Create_DDetectorMatches(const std::shared_ptr<const JEvent>& event, vector<const DTrackWireBased*>& locTrackWireBasedVector);

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;

		//matching tracks to hits/showers routines
		void MatchToTOF(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DTOFPoint*>& locTOFPoints, DDetectorMatches* locDetectorMatches) const;
		void MatchToBCAL(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DBCALShower*>& locBCALShowers, DDetectorMatches* locDetectorMatches) const;
		void MatchToFCAL(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DFCALShower*>& locFCALShowers, DDetectorMatches* locDetectorMatches) const;
		void MatchToSC(const DParticleID* locParticleID, const DTrackWireBased* locTrackWireBased, const vector<const DSCHit*>& locSCHits, DDetectorMatches* locDetectorMatches) const;
		void MatchToFCAL(const DParticleID* locParticleID,
				 const DTrackWireBased *locTrackWireBased,
				 vector<const DFCALHit *>&locSingleHits,
				 DDetectorMatches* locDetectorMatches) const;

		//matching showers to tracks routines
		void MatchToTrack(const DParticleID* locParticleID, const DBCALShower* locBCALShower, const vector<const DTrackWireBased*>& locTrackWireBasedVector, DDetectorMatches* locDetectorMatches) const;
		void MatchToTrack(const DParticleID* locParticleID, const DFCALShower* locFCALShower, const vector<const DTrackWireBased*>& locTrackWireBasedVector, DDetectorMatches* locDetectorMatches) const;

		bool ENABLE_FCAL_SINGLE_HITS;
};

#endif // _DDetectorMatches_factory_WireBased_
