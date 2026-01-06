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
#include "TH2F.h"
#include <vector>

using namespace std;

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
      
      vector<TH2F *> hWireT0s;
      vector<TProfile2D *> hWirePositions;
      
      vector<TH2F *> hCathodeUProjections;
      vector<TH2F *> hCathodeVProjections;
      vector<TH2F *> hCathodeUProjections_Pos;
      vector<TH2F *> hCathodeVProjections_Pos;
      vector<TH2F *> hCathodeUProjections_Neg;
      vector<TH2F *> hCathodeVProjections_Neg;
};

#endif // _JEventProcessor_FDC_InternalAlignment_

