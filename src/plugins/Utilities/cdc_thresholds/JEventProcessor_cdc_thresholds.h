// $Id$
//
//    File: JEventProcessor_cdc_thresholds.h
// Created: Wed Jul  1 08:44:10 PM EDT 2026
// Creator: njarvis (on Linux ifarm2402.jlab.org 5.14.0-611.55.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_cdc_thresholds_
#define _JEventProcessor_cdc_thresholds_

#include <JANA/JEventProcessor.h>
// #include <JANA/Services/JLockService.h> // Required for accessing services

#include "DAQ/Df125BORConfig.h"
#include "DAQ/bor_roc.h"
#include <TTAB/DTranslationTable.h>

class JEventProcessor_cdc_thresholds:public JEventProcessor{
    public:
        JEventProcessor_cdc_thresholds();
        ~JEventProcessor_cdc_thresholds();
        const char* className(void){return "JEventProcessor_cdc_thresholds";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

        const DTranslationTable* ttab;
        uint thresholds[3522];
        bool thresholds_found;
        uint64_t run_number;
    	// std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_cdc_thresholds_

