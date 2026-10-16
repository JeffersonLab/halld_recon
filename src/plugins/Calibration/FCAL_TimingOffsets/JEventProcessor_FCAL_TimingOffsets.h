
#ifndef _JEventProcessor_FCAL_TimingOffsets_
#define _JEventProcessor_FCAL_TimingOffsets_

#include <JANA/JEventProcessor.h>
#include <FCAL/DFCALGeometry.h>
#include "ANALYSIS/DTreeInterface.h"
#include <thread>
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"
#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALShower.h"
#include "DAQ/Df250PulseIntegral.h"
#include "DAQ/Df250PulsePedestal.h"
#include "DAQ/DEPICSvalue.h"
#include "TRIGGER/DL1Trigger.h"
#include "DANA/DEvent.h"
#include <DANA/DStatusBits.h>
#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "HDGEOMETRY/DGeometry.h"
#include <TTree.h>


class DFCALHit;

class JEventProcessor_FCAL_TimingOffsets:public JEventProcessor{
 public:

  JEventProcessor_FCAL_TimingOffsets();
  ~JEventProcessor_FCAL_TimingOffsets();

 private:

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  double m_targetZ;

  int m_column;
  int m_row;
  int m_chan;


  const DFCALGeometry* m_fcalGeom;
  double m_FCALfront;
 DVector3 dTargetCenter;
 DVector3 dFCALblockcenter;


  
  
 
};

#endif // _JEventProcessor_FCAL_TimingOffsets_

