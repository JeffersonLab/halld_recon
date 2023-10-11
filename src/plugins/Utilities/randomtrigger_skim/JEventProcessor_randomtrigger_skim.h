//
//	 File: JEventProcessor_randomtrigger_skim.h
// Created: Wed Nov 9 15:08:37 EDT 2014
// Creator: Paul Mattione
//

#ifndef _JEventProcessor_randomtrigger_skim_
#define _JEventProcessor_randomtrigger_skim_

#include <string>
#include <vector>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>
#include <JANA/JApplication.h>
#include <JANA/JEventSource.h>
#include <JANA/JEvent.h>

#include "evio_writer/DEventWriterEVIO.h"

#include "PID/DChargedTrack.h"
#include "DAQ/DEPICSvalue.h"

#include "DAQ/DBeamCurrent.h"
#include "DAQ/DBeamCurrent_factory.h"

using namespace std;

class JEventProcessor_randomtrigger_skim : public JEventProcessor
{
  public:
    
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

  private:
    DBeamCurrent_factory *dBeamCurrentFactory;
  
};

#endif // _JEventProcessor_randomtrigger_skim_

