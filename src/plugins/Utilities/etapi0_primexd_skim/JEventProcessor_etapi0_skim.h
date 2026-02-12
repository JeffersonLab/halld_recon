/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2020       GlueX and PrimEX-D Collaborations               * 
*                                                                         *                                                                
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#ifndef _JEventProcessor_etapi0_skim_
#define _JEventProcessor_etapi0_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"
#include "Combination.h"

class TTree;
class DFCALCluster;

#include <vector>

class JEventProcessor_etapi0_skim:public JEventProcessor{
 public:

  enum { kMaxHits = 500, kMaxClus = 20 };

  JEventProcessor_etapi0_skim();
  ~JEventProcessor_etapi0_skim();

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  void writeClustersToRoot( const vector< const DFCALCluster* > clusVec );
  void Combined4g(vector<TLorentzVector>&EMList,
		  Double_t &bestChi22Pi0,
		  Double_t &bestChi2EtaPi0,
		  Double_t &bestChi22Eta,
		  vector<TLorentzVector>&Photon2Pi0List,
		  vector<TLorentzVector>&PhotonEtaPi0List,
		  vector<TLorentzVector>&Photon2EtaList);
  void Combined5g(vector<TLorentzVector>&EMList,
		  Double_t &bestChi22Pi0,
		  Double_t &bestChi2EtaPi0,
		  Double_t &bestChi22Eta,
		  vector<TLorentzVector>&Photon2Pi0List,
		  vector<TLorentzVector>&PhotonEtaPi0List,
		  vector<TLorentzVector>&Photon2EtaList);
  double MIN_MASS;
  double MAX_MASS;
  double MIN_E;
  double MIN_R;
  double MAX_DT;
  double MAX_ETOT;
  int    MIN_BLOCKS;

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

  double m_beamSpotX;
  double m_beamSpotY;
  double m_targetZ;

  Combination  *combi4;       
  Combination  *combi5;       

  const double me = 0.510998928e-3;
  const double pi0Mass = 0.13497666;
  const double etaMass = 0.547962;
  const double etaprimeMass = 0.95778;  
};

#endif // _JEventProcessor_etapi0_skim_

