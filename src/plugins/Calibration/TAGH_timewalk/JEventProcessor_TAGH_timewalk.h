// $Id$
//
//    File: JEventProcessor_TAGH_timewalk.h
// Created: Fri Nov 13 10:13:02 EST 2015
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-229.20.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGH_timewalk_
#define _JEventProcessor_TAGH_timewalk_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <RF/DRFTime_factory.h>

class JEventProcessor_TAGH_timewalk:public JEventProcessor{
public:
    JEventProcessor_TAGH_timewalk();
    ~JEventProcessor_TAGH_timewalk();

private:
    DRFTime_factory *dRFTimeFactory;
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TAGH_timewalk_

