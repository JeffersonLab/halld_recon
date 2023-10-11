// $Id$
//
//    File: JEventProcessor_PSC_TW.h
// Created: Fri Aug 21 10:42:28 EDT 2015
// Creator: aebarnes (on Linux ifarm1102 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_PSC_TW_
#define _JEventProcessor_PSC_TW_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <PAIR_SPECTROMETER/DPSCHit.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>

class JEventProcessor_PSC_TW:public JEventProcessor{
   public:
      JEventProcessor_PSC_TW();
      ~JEventProcessor_PSC_TW();

   private:
      void Init() override;
      void BeginRun(const std::shared_ptr<const JEvent>& event) override;
      void Process(const std::shared_ptr<const JEvent>& event) override;
      void EndRun() override;
      void Finish() override;

      std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_PSC_TW_

