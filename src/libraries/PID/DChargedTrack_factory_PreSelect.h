// $Id$
//
//    File: DChargedTrack_factory_PreSelect.h
// Created: Mon Dec  7 14:29:24 EST 2009
// Creator: staylor (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DChargedTrack_factory_PreSelect_
#define _DChargedTrack_factory_PreSelect_

#include <JANA/JFactoryT.h>
#include <PID/DChargedTrack.h>
#include <PID/DChargedTrackHypothesis.h>
#include <TRACKING/DTrackTimeBased.h>
#include "DResourcePool.h"

using namespace std;


class DChargedTrack_factory_PreSelect : public JFactoryT<DChargedTrack>
{
	public:
		DChargedTrack_factory_PreSelect(){
			SetTag("PreSelect");
		};
		~DChargedTrack_factory_PreSelect(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		bool Cut_HasDetectorMatch(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DDetectorMatches* locDetectorMatches) const;
		bool Cut_TrackingFOM(const DChargedTrackHypothesis* locChargedTrackHypothesis) const;

		//Command-line values will override these
		double dMinTrackingFOM; //PRESELECT:MIN_TRACKING_FOM 
		bool dHasDetectorMatchFlag; //PRESELECT:HAS_DETECTOR_MATCH_FLAG: if true/false, do/don't require tracks to have a detector match
};

#endif // _DChargedTrack_factory_PreSelect_

