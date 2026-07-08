// $Id$
//
//    File: JEventProcessor_FdcTimeToDistance.h
// Created: Tue Mar 10 11:22:46 AM EDT 2026
// Creator: staylor (on Linux ifarm2402.jlab.org 5.14.0-611.30.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_FdcTimeToDistance_
#define _JEventProcessor_FdcTimeToDistance_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services
#include <TH2F.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>

class JEventProcessor_FdcTimeToDistance:public JEventProcessor{
public:
  JEventProcessor_FdcTimeToDistance();
  ~JEventProcessor_FdcTimeToDistance();
  const char* className(void){return "JEventProcessor_FdcTimeToDistance";}
  
private:
  void Init() override;                       ///< Called once at program start.
  void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
  void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
  void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
  void Finish() override;                     ///< Called after last event of last event source has been processed.
 
  std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

  TH2F *HDVsT[24];
  double FDC_DRIFT_BSCALE_PAR1,FDC_DRIFT_BSCALE_PAR2;
  
  const DMagneticFieldMap *bfield;

};

#endif // _JEventProcessor_FdcTimeToDistance_

