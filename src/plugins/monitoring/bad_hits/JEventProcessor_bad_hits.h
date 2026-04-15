// $Id$
//
//    File: JEventProcessor_bad_hits.h
// Created: Sat Jan 31 01:57:43 PM EST 2026
// Creator: njarvis (on Linux gluon00 5.14.0-611.20.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_bad_hits_
#define _JEventProcessor_bad_hits_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services


class JEventProcessor_bad_hits:public JEventProcessor{
    public:
        JEventProcessor_bad_hits();
        ~JEventProcessor_bad_hits();
        const char* className(void){return "JEventProcessor_bad_hits";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

    	std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()
};

#endif // _JEventProcessor_bad_hits_

