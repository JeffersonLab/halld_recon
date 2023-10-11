// $Id$
//
//    File: JEventProcessor_FDC_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FDC_online_
#define _JEventProcessor_FDC_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


class JEventProcessor_FDC_online:public JEventProcessor{
 public:
  JEventProcessor_FDC_online();
  ~JEventProcessor_FDC_online();

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  
  unsigned int thresh;
  float wire_pitch;
  float strip_pitch_u;
  float strip_pitch_d;
  float strip_angle;
  float cell_rot_step;
  
  float ADCmax[4][6][2][192][20];
  float ADCtime[4][6][2][192][20];
  int ADCnh[4][6][2][192];
  float TDCval[4][6][96][20];
  int TDCnh[4][6][96];
  
};

#endif // _JEventProcessor_FDC_online_

