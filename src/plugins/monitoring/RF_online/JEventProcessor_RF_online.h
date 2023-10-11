// $Id$
//
//    File: JEventProcessor_RF_online.h
// Created: Wed Apr  8 11:58:09 EST 2015
// Creator: pmatt (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_RF_online_
#define _JEventProcessor_RF_online_

#include <map>
#include <set>
#include <vector>
#include <limits>
#include <string>

#include "TH1I.h"
#include "TDirectoryFile.h"

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "GlueX.h"
#include "DAQ/DCODAROCInfo.h"
#include <TTAB/DTTabUtilities.h>

#include <TAGGER/DTAGHHit.h>
#include <RF/DRFTDCDigiTime.h>
#include <RF/DRFDigiTime.h>
#include <RF/DRFTime_factory.h>

using namespace std;

class JEventProcessor_RF_online : public JEventProcessor
{
	public:
		JEventProcessor_RF_online() {
			SetTypeName("JEventProcessor_RF_online");
		}
		~JEventProcessor_RF_online() override = default;

	private:
		TDirectoryFile* dROCTIDirectory;
		
		TH1I *rf_itself_num_events;

		double dRFSignalPeriod; //not the same as the period of the beam //before multiplexing
		vector<DetectorSystem_t> dRFSignalSystems;

		map<uint32_t, TH1I*> dHistMap_ROCInfoDeltaT; //key is rocid
		map<DetectorSystem_t, TH1I*> dHistMap_NumSignals;
		map<DetectorSystem_t, TH1I*> dHistMap_RFSignalPeriod;
		map<DetectorSystem_t, TH1I*> dHistMap_RFFirstTimeDeltaT;

		map<DetectorSystem_t, TH1I*> dHistMap_RFHitsFound;
		map<DetectorSystem_t, TH1I*> dHistMap_NumRFHitsMissing;
		map<DetectorSystem_t, size_t> dMaxDeltaTHits;
		map<DetectorSystem_t, double> dRFSamplingFactor;
		map<DetectorSystem_t, map<pair<size_t, size_t>, TH1I*> > dHistMap_AdjacentRFDeltaTs;

		TH1I* dHist_RFBeamBunchPeriod;
		map<DetectorSystem_t, TH1I*> dHistMap_SelfResolution;

		map<DetectorSystem_t, TH1I*> dHistMap_RFTaggerDeltaT;
		map<pair<DetectorSystem_t, DetectorSystem_t>, TH1I*> dHistMap_RFRFDeltaTs;
		map<pair<DetectorSystem_t, DetectorSystem_t>, TH1I*> dHistMap_AverageRFRFDeltaTs;
		map<pair<DetectorSystem_t, DetectorSystem_t>, TH1I*> dHistMap_AbsoluteRFRFDeltaTs;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent, int32_t runnumber);	///< Called everytime a new run number is detected.
		void Process(const std::shared_ptr<const JEvent>& locEvent, uint64_t eventnumber);	///< Called every event.
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_RF_online_
