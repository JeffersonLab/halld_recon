// $Id$
//
//    File: JEventProcessor_FCALLEDTree.h
// Created: Fri May 19 12:39:24 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#ifndef _JEventProcessor_FCALLEDTree_
#define _JEventProcessor_FCALLEDTree_

class TTree;

#include <JANA/JEventProcessor.h>
#include "HistogramTools.h"

class JEventProcessor_FCALLEDTree:public JEventProcessor{
 public:
  JEventProcessor_FCALLEDTree();
  ~JEventProcessor_FCALLEDTree();

  enum { kMaxHits = 10000 };
  
 private:
  
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;

  TTree* m_tree;

  int m_nHits;
  int btree;
  int m_chan[kMaxHits];
  float m_x[kMaxHits];
  float m_y[kMaxHits];
  float m_E[kMaxHits];
  float m_t[kMaxHits];
  float m_integ[kMaxHits];
  float m_ped[kMaxHits];
  float m_peak[kMaxHits];

  int m_run;
  uint64_t m_event;
  float m_eTot;
  
};

#endif // _JEventProcessor_FCALLEDTree_

