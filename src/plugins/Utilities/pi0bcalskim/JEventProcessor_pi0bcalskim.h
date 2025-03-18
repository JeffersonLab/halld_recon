// $Id$
//
//    File: JEventProcessor_pi0bcalskim.h
// Created: Mon feb 6 15:46:00 EST 2015
// Creator: wmcginle(on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_pi0bcalskim_
#define _JEventProcessor_pi0bcalskim_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>
#include "evio_writer/DEventWriterEVIO.h"
#include <TRACKING/DTrackFitter.h>

#include <vector>

using namespace std;

class JEventProcessor_pi0bcalskim:public JEventProcessor{
 public:

  JEventProcessor_pi0bcalskim();
  ~JEventProcessor_pi0bcalskim();


 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;


  double MIN_SH1_E;
  double MIN_SH2_E;
 
  int WRITE_EVIO;
  int WRITE_HDDM;
  int num_epics_events;

  double m_beamSpotX;
  double m_beamSpotY;
  double m_targetZ;
};
#endif // _JEventProcessor_pi0bcalskim_

