// $Id$
//
//    File: JEventProcessor_BCAL_LEDonline.h
//

#ifndef _JEventProcessor_BCAL_LEDonline_
#define _JEventProcessor_BCAL_LEDonline_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <BCAL/DBCALGeometry.h>

class JEventProcessor_BCAL_LEDonline:public JEventProcessor{
 public:
  JEventProcessor_BCAL_LEDonline();
  ~JEventProcessor_BCAL_LEDonline();

  int NOtrig, GTPtrig, FPtrig, FPGTPtrig, trigUS, trigDS, trigCosmic;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  
};

#endif // _JEventProcessor_BCAL_LEDonline_

