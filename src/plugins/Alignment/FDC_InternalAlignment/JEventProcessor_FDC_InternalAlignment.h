// $Id$
//
//    File: JEventProcessor_FDC_InternalAlignment.h
// Created: Sun Nov 27 16:10:26 EST 2016
// Creator: mstaib (on Linux ifarm1102 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FDC_InternalAlignment_
#define _JEventProcessor_FDC_InternalAlignment_

#include <JANA/JEventProcessor.h>
#include "TH3I.h"
#include "TProfile.h"

class JEventProcessor_FDC_InternalAlignment:public JEventProcessor{
	public:
		JEventProcessor_FDC_InternalAlignment();
		~JEventProcessor_FDC_InternalAlignment();
		const char* className(void){return "JEventProcessor_FDC_InternalAlignment";}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
      TH3I *Hist3D[24];
      TProfile *HistCurrentConstants;
};

#endif // _JEventProcessor_FDC_InternalAlignment_

