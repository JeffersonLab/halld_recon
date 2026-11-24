// $Id$
//
//    File: JEventProcessor_dc_basics.h
// Created: Thu Oct  8 03:30:26 PM EDT 2026
// Creator: njarvis (on Linux gluon00 5.14.0-687.53.1.el9_8.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_dc_basics_
#define _JEventProcessor_dc_basics_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services


class JEventProcessor_dc_basics:public JEventProcessor{
    public:
        JEventProcessor_dc_basics();
        ~JEventProcessor_dc_basics();
        const char* className(void){return "JEventProcessor_dc_basics";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

    	std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_dc_basics_

