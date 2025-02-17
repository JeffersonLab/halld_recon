// $Id$
//
//    File: JEventProcessor_ps_skim.h
// Created: Mon May 18 09:52:08 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_ps_skim_
#define _JEventProcessor_ps_skim_

#include <JANA/JEventProcessor.h>

class JEventProcessor_ps_skim:public JEventProcessor{
public:
    JEventProcessor_ps_skim();
    ~JEventProcessor_ps_skim();

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;
};

#endif // _JEventProcessor_ps_skim_

