// $Id$
//
//    File: DEventProcessor_track_skimmer.h
// Created: Tue Jan 13 11:08:16 EST 2015
// Creator: Paul (on Darwin Pauls-MacBook-Pro.local 14.0.0 i386)
//

#ifndef _DEventProcessor_track_skimmer_
#define _DEventProcessor_track_skimmer_

#include <map>
#include <fstream>

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_track_skimmer.h"

#include <TRIGGER/DTrigger.h>

using namespace std;

class DEventProcessor_track_skimmer : public JEventProcessor
{
	public:
		DEventProcessor_track_skimmer() {
			SetTypeName("DEventProcessor_track_skimmer");
		}
		~DEventProcessor_track_skimmer() override = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		std::shared_ptr<JLockService> lockService;

		int Get_FileNumber(const std::shared_ptr<const JEvent>& locEvent) const;

		map<string, ofstream*> dIDXAStreamMap;
};

#endif // _DEventProcessor_track_skimmer_

