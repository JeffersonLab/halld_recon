// $Id$
//
//    File: JEventProcessor_TS_scaler.h
// Created: Thu May 26 12:16:56 EDT 2016
// Creator: jrsteven (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_TS_scaler_
#define _JEventProcessor_TS_scaler_

#include <map>

#include "TFile.h"
#include "TTree.h"
#include "TH1I.h"
#include "TH2I.h"

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "ANALYSIS/DTreeInterface.h"

class JEventProcessor_TS_scaler:public JEventProcessor{
	public:
		JEventProcessor_TS_scaler();
		~JEventProcessor_TS_scaler();
		enum { kScalers = 32 };
		enum { kFPScalers = 16 };

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		float dCurrent;
		uint64_t dEventNumber;
		uint32_t dScalerTriggerBitPrevious[kScalers];
		uint32_t dRecordedTriggerBitPrevious[kScalers];
		uint32_t dTrigCount[kScalers];
		uint32_t dFPRecordedTriggerBitPrevious[kFPScalers];
		uint32_t dFPScalerTriggerBitPrevious[kFPScalers];
		uint32_t dFPTrigCount[kFPScalers];

		//TREE
                DTreeInterface* dTreeInterface;
		//thread_local: Each thread has its own object: no lock needed
                        //important: manages it's own data internally: don't want to call new/delete every event!
                static thread_local DTreeFillData dTreeFillData;

		bool dIsFirstInterval; // first SYNC event in file
		bool dIsLastInterval;  // interval after last SYNC event in file (need to combine with first SYNC event in next file)
		ULong64_t dTotalEventNumber;                     // Total event number at sync event
		uint32_t dSyncEventNumber;                       // SYNC event number
		uint32_t dSyncEventLiveTime;                     // Live time: in clock counts (integrated)
		uint32_t dSyncEventBusyTime;                     // Busy time: in clock counts (integrated)
		uint32_t dSyncEventInstLiveTime;                 // Live time: in percent x10 (instantaneous)
		uint32_t dSyncEventUnixTime;                     // SYNC event UNIX timestamp
		uint32_t dScalerTriggerBit[kScalers];            // GTP scaler value by bit over current interval between SYNC events
		uint32_t dFPScalerTriggerBit[kFPScalers];        // FP scaler value by bit over current interval between SYNC events
		uint32_t dScalerRateTriggerBit[kScalers];        // GTP scaler value by bit over current interval between SYNC events
		uint32_t dFPScalerRateTriggerBit[kFPScalers];    // FP scaler value by bit over current interval between SYNC events

		uint32_t dRecordedTriggerBit[kScalers];      // Recoreded GTP triggers by bit over current interval between SYNC events
		uint32_t dFPRecordedTriggerBit[kFPScalers];  // Recoreded FP triggers by bit over current interval between SYNC events

		vector<uint32_t> dTrigBits;
		vector<uint32_t> dFPTrigBits;
		TH1I *dHistTS_trgbits, *dHistTS_livetime_tot, *dHistTS_liveinst_tot;
		TH1I *dHistTS_SyncEvents, *dHistTS_livetimeEvents, *dHistTS_Current;
		map<uint32_t, TH1I*> dHistTS_trigrate, dHistTS_FPtrigrate;
		map<uint32_t, TH1I*> dHistTS_livetime, dHistTS_FPlivetime;
		map<uint32_t, TH1I*> dHistTS_Recorded, dHistTS_FPRecorded;
		map<uint32_t, TH1I*> dHistTS_Scaler, dHistTS_FPScaler;
		
};

#endif // _JEventProcessor_TS_scaler_

