// $Id$
//
//    File: JEventProcessor_TAGH_doubles.h
// Created: Fri Apr 29 15:19:27 EDT 2016
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.13.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGH_doubles_
#define _JEventProcessor_TAGH_doubles_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_TAGH_doubles:public JEventProcessor{
    public:
        JEventProcessor_TAGH_doubles();
        ~JEventProcessor_TAGH_doubles();

    private:
        void Init() override;
        void BeginRun(const std::shared_ptr<const JEvent>& event) override;
        void Process(const std::shared_ptr<const JEvent>& event) override;
        void EndRun() override;
        void Finish() override;

        std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TAGH_doubles_
