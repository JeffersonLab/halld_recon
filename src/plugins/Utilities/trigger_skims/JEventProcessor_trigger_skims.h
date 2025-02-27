//
//	 File: JEventProcessor_trigger_skims.h
// Created: Wed Nov 9 15:08:37 EDT 2014
// Creator: Paul Mattione
//

#ifndef _JEventProcessor_trigger_skims_
#define _JEventProcessor_trigger_skims_

#include <string>
#include <vector>

#include <JANA/JEventProcessor.h>

#include "evio_writer/DEventWriterEVIO.h"

#include "PID/DChargedTrack.h"
#include "DAQ/DEPICSvalue.h"

using namespace std;

class JEventProcessor_trigger_skims : public JEventProcessor
{
  public:
    
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;
};

#endif // _JEventProcessor_trigger_skims_

