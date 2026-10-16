// $Id$
//
//    File: JEventProcessor_FCAL_cpp.h
// Created: Mon Feb 19 16:33:33 EST 2024
// Creator: ilarin (on Linux ifarm1801.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCAL_cpp_
#define _JEventProcessor_FCAL_cpp_

#include <JANA/JEventProcessor.h>
#include "FCAL/DFCALGeometry.h"

#include <TH1.h>
#include <TH2.h>
#include "TProfile.h"
#include <TProfile2D.h>
#include <TDirectory.h>

const int N_FCAL_BLOCKS = 2800, NROW = 59, NCOL = 59;

class JEventProcessor_FCAL_cpp:public JEventProcessor{
	public:
		JEventProcessor_FCAL_cpp();
		~JEventProcessor_FCAL_cpp();
		const char* className(void){return "JEventProcessor_FCAL_cpp";}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;


  double fcal_nhit[NROW][NCOL], fcal_nclust[NROW][NCOL], fcal_nhitonly[NROW][NCOL][2], nfcaltriggers, ntoftriggers;

  const DFCALGeometry *fcalGeom;

  TH2F *h_fcal_nclust, *h_fcal_nhitonly, *h_fcal_hcr;
  TProfile *h_fcalshower_tall, *h_fcalshower_tw;
  TProfile2D *h_fcalshower_tall_map, *h_fcalehit_tall_map;
};

#endif // _JEventProcessor_FCAL_cpp_

