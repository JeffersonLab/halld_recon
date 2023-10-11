// $Id$
//
//    File: JEventProcessor_PS_timing.h
// Created: Sat Nov 21 17:21:28 EST 2015
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_PS_timing_
#define _JEventProcessor_PS_timing_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_PS_timing:public JEventProcessor{
public:
    JEventProcessor_PS_timing();
    ~JEventProcessor_PS_timing();

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_PS_timing_

