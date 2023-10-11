// $Id$
//
//    File: JEventProcessor_PSPair_online.h
// Created: Fri Mar 20 16:32:04 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_PSPair_online_
#define _JEventProcessor_PSPair_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_PSPair_online:public JEventProcessor{
public:
    JEventProcessor_PSPair_online();
    ~JEventProcessor_PSPair_online();

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_PSPair_online_

