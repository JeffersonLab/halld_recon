// $Id$
//
//    File: JEventProcessor_scaler_primex.h
//    A.S.
// 
//

#ifndef _JEventProcessor_scaler_primex_
#define _JEventProcessor_scaler_primex_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include "TProfile.h"
#include <TTree.h>


#include "ANALYSIS/DTreeInterface.h"

class JEventProcessor_scaler_primex:public JEventProcessor{
	public:
		JEventProcessor_scaler_primex(){
			SetTypeName("JEventProcessor_scaler_primex");
		};
		~JEventProcessor_scaler_primex(){};

		//		enum { khit = 32 };

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;


		//TREE
                DTreeInterface* dTreeInterface;
                //thread_local: Each thread has its own object: no lock needed
		//important: manages it's own data internally: don't want to call new/delete every event!

                static thread_local DTreeFillData dTreeFillData;



};

#endif // _JEventProcessor_scaler_primex_

