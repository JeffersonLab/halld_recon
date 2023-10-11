// $Id$
//
//    File: JEventProcessor_FCAL_online.h
// Created: Fri Nov  9 11:58:09 EST 2012
// Creator: wolin (on Linux stan.jlab.org 2.6.32-279.11.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCAL_online_
#define _JEventProcessor_FCAL_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class TH1D;
class TH1I;
class TH2I;
class TH2F;
class TH1F;
class TProfile;
class TProfile2D;

class JEventProcessor_FCAL_online:public JEventProcessor{
 public:
  JEventProcessor_FCAL_online();
  ~JEventProcessor_FCAL_online();

 private:

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  double m_targetZ;

  TH1D *fcal_num_events;

  TH1I* m_digInt;
  TH1I* m_digCoarseT;
  TProfile* m_digCoarseTChan;
  TProfile* m_multihit;
  TH1I* m_digPreciseT;
  TProfile* m_digPreciseTChan;
  TH1I* m_digT;
  TH1I* m_digT0;
  TH1I* m_digTmT0;
  TH2F* m_digTmT02D;
  TH1I* m_digPed;
  TProfile* m_digPedChan;
  TH2F* m_digPed2D;
  TH2F* m_digPedSq2D;
  TH1I* m_digQual;
  TH1I* m_digNUnder;
  TH1I* m_digNOver;
  TH1I* m_digN;
  TH1I* m_digPeakV;
  TProfile2D* m_digPeakV2D;
  TH2F* m_digOcc2D;
  TH2I* m_digIntVsPeak;
  TH1I* m_digIntToPeak;

  TH1I* m_hitN;
  TH1I* m_hitE;
  TH1I* m_hitETot;
  TH1I* m_hitT;
  TH1I* m_hitT0;
  TH1I* m_hitTmT0;
  TH2F* m_hitE2D;
  TH2F* m_hitTmT02D;
  TH2F* m_hitTmT0Sq2D;
  TH2F* m_hitOcc2D;
  
  TH1I* m_clusN;
  TH1I* m_clusE;
  TH1I* m_clusETot;
  TH1I* m_clusT;
  TH1I* m_clusT0;
  TH1I* m_clusTmT0;
  TH2I* m_clusXYHigh;
  TH2I* m_clusXYLow;
  TH1I* m_clusPhi;
  TH1I* m_clus2GMass;

  TH1I* m_show2GMass;
  TH2I* m_showZvsE;
  TH2I* m_showECorVsE;
  TH2I* m_showTsMTcVsZ;

};

#endif // _JEventProcessor_FCAL_online_

