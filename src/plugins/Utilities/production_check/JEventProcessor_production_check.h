// $Id$
//
//    File: JEventProcessor_production_check.h
// Created: Thu May 22 12:12:27 PM EDT 2025
// Creator: ilarin (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//
//  plugin to check production OUTPUT for corruption
//  creates output file pcheck.dat with two CRC values
//
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_production_check_
#define _JEventProcessor_production_check_

#include <JANA/JEventProcessor.h>
// #include <JANA/Services/JLockService.h> // Required for accessing services

#include <TRIGGER/DL1Trigger.h>
#include <FCAL/DFCALHit.h>
#include <BCAL/DBCALHit.h>

#include <PID/DEventRFBunch.h>
#include <PID/DBeamPhoton.h>
#include <TOF/DTOFHit.h>
#include <PID/DChargedTrack.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>
#include <PAIR_SPECTROMETER/DPSPair.h>

#include <stdio.h>
using namespace std;

class JEventProcessor_production_check:public JEventProcessor{
    public:
        JEventProcessor_production_check();
        ~JEventProcessor_production_check();
        const char* className(void){return "JEventProcessor_production_check";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

    	// std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

    long int control_word = 0l, control_word2 = 0l;
    int event_count = 0;

};

#endif // _JEventProcessor_production_check_

