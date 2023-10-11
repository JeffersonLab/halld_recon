// $Id$
//
//    File: JEventProcessor_CDC_amp.h
// Created: Tue Sep  6 10:13:02 EDT 2016
// Creator: njarvis (on Linux egbert 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_amp_
#define _JEventProcessor_CDC_amp_


#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TRACKING/DTrackFitter.h"
#include "TRACKING/DTrackTimeBased.h"

#include "TRACKING/DMCThrown.h"

#include "CDC/DCDCTrackHit.h"
#include "CDC/DCDCHit.h"
#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125CDCPulse.h"

#include "DAQ/Df125Config.h"
#include "TRIGGER/DTrigger.h"
#include "PID/DVertex.h"

#include <stdint.h>
#include <vector>
#include <TMath.h>

#include <TDirectory.h>

#include <TH2.h>
#include <TH1.h>


using namespace std;





class JEventProcessor_CDC_amp:public JEventProcessor{
	public:
		JEventProcessor_CDC_amp();
		~JEventProcessor_CDC_amp();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

                int32_t run_number;


                // default scaling factors will be overridden by Df125Config if present

                uint16_t ASCALE = 1;   //amplitude  this was 8 for runs before 40,000
                uint16_t PSCALE = 1;   //ped

                // histograms

                TH1I *time = NULL;
                TH1I *a = NULL; 
                TH2I *an = NULL; 
                TH2D *atime = NULL;
                TH2D *atheta = NULL;

                TH1I *a30 = NULL; 
                TH1I *a30_100ns = NULL; 
                TH2I *an30_100ns = NULL; 
                TH2D *atime30 = NULL;
                TH2D *adoca30 = NULL;

                TH1I *a45 = NULL; 
                TH1I *a45_100ns = NULL; 
                TH2I *an45_100ns = NULL; 
                TH2D *atime45 = NULL;
                TH2D *adoca45 = NULL;

                TH1I *a90 = NULL; 
                TH1I *a90_100ns = NULL; 
                TH2I *an90_100ns = NULL; 
                TH2D *atime90 = NULL;
                TH2D *adoca90 = NULL;

                TH2I *an90 = NULL; 
                TH1I *time90 = NULL;
                TH2D *xt90 = NULL;

};

#endif // _JEventProcessor_CDC_amp_

