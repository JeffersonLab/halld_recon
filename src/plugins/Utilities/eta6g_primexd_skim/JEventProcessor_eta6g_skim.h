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

#ifndef _JEventProcessor_eta6g_skim_
#define _JEventProcessor_eta6g_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"
#include "Combination.h"
#include <DLorentzVector.h>

class TTree;
class DFCALCluster;

#include <vector>

class JEventProcessor_eta6g_skim:public jana::JEventProcessor{
 public:

  enum { kMaxHits = 500, kMaxClus = 20 };

  JEventProcessor_eta6g_skim();
  ~JEventProcessor_eta6g_skim();
  const char* className(void){return "JEventProcessor_eta6g_skim";}

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  void writeClustersToRoot( const vector< const DFCALCluster* > clusVec );
  void Combined6g(vector<DLorentzVector>&EMList,
		  Double_t &bestChi2Eta,
		  Double_t &bestChi2EtaPrim,
		  vector<DLorentzVector>&PhotonEta6gList,
		  vector<DLorentzVector>&PhotonEtaprim6gList);
  void Combined7g(vector<DLorentzVector>&EMList,
		  Double_t &bestChi2Eta,
		  Double_t &bestChi2EtaPrim,
		  vector<DLorentzVector>&PhotonEta6gList,
		  vector<DLorentzVector>&PhotonEtaprim6gList);

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
  
  Combination  *combi6;       
  Combination  *combi7;       
  
  const double me = 0.510998928e-3;
  const double pi0Mass = 0.13497666;
  const double etaMass = 0.547962;
  const double etaprimeMass = 0.95778;  
};

#endif // _JEventProcessor_eta6g_skim_

