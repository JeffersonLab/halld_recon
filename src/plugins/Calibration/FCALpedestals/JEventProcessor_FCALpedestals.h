// $Id$
//
//    File: JEventProcessor_FCALpedestals.h
// Created: Fri Jan 30 08:18:41 EST 2015
// Creator: shepherd (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCALpedestals_
#define _JEventProcessor_FCALpedestals_

#include <JANA/JEventProcessor.h>
#include <FCAL/DFCALGeometry.h>

class DFCALHit;
class TTree;

class JEventProcessor_FCALpedestals:public JEventProcessor{
 public:

 
  JEventProcessor_FCALpedestals();
  ~JEventProcessor_FCALpedestals();

 private:

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;



  double m_FCALfront;
  
 
  int m_r;
  int m_c;
  int m_chan;
  float m_pedestal;


 
};

#endif // _JEventProcessor_FCALpedestals_

