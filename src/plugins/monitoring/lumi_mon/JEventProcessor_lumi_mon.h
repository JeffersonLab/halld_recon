// $Id$
//
//    File: JEventProcessor_lumi_mon.h
//    A.S.
// 
//

#ifndef _JEventProcessor_lumi_mon_
#define _JEventProcessor_lumi_mon_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include "TProfile.h"
#include <TTree.h>


class JEventProcessor_lumi_mon:public JEventProcessor{
	public:
		JEventProcessor_lumi_mon(){
			SetTypeName("JeventProcessor_lumi_mon");
		};
		~JEventProcessor_lumi_mon(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

};

#endif // _JEventProcessor_lumi_mon_

