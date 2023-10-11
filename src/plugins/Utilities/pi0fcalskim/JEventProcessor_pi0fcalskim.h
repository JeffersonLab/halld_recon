// $Id$
//
//    File: JEventProcessor_pi0fcalskim.h
// Created: Mon Dec  1 14:57:11 EST 2014
// Creator: shepherd (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_pi0fcalskim_
#define _JEventProcessor_pi0fcalskim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"

class TTree;
class DFCALCluster;

#include <vector>

class JEventProcessor_pi0fcalskim:public JEventProcessor{
 public:

  enum { kMaxHits = 500, kMaxClus = 20 };

  JEventProcessor_pi0fcalskim();
  ~JEventProcessor_pi0fcalskim();

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  void writeClustersToRoot( const vector< const DFCALCluster* > clusVec );

  double MIN_MASS;
  double MAX_MASS;
  double MIN_E;
  double MIN_R;
  double MAX_DT;
  double MAX_ETOT;
  int    MIN_BLOCKS;
  
  int SAVE_TOF_HITS;

  int WRITE_ROOT;
  int WRITE_EVIO;
  int WRITE_HDDM;

  TTree* m_tree;
  int m_nClus;
  int m_hit0[kMaxClus];
  float m_px[kMaxClus];
  float m_py[kMaxClus];
  float m_pz[kMaxClus];

  int m_nHit;
  int m_chan[kMaxHits];
  float m_e[kMaxHits];

  int num_epics_events;
};

#endif // _JEventProcessor_pi0fcalskim_

