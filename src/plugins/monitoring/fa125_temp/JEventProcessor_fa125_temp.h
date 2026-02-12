// $Id$
//
//    File: JEventProcessor_fa125_temp.h
// Created: Tue Apr 29 02:00:21 PM EDT 2025
// Creator: njarvis (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_fa125_temp_
#define _JEventProcessor_fa125_temp_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services


class JEventProcessor_fa125_temp:public JEventProcessor{
    public:
        JEventProcessor_fa125_temp();
        ~JEventProcessor_fa125_temp();
        const char* className(void){return "JEventProcessor_fa125_temp";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

        int rocmap[70];   // which bin to use for each roc in the histogram, to make the histogram compact, not gappy        
        bool histo_filled; // only need to fill it once
        std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_fa125_temp_

