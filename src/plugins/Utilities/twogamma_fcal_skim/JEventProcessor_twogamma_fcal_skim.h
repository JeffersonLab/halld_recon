// $Id$
//
//    File: JEventProcessor_twogamma_fcal_skim.h
// Created: Sat Nov 30 16:32:06 EST 2019
// Creator: cakondi (on Linux ifarm1801 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_twogamma_fcal_skim_
#define _JEventProcessor_twogamma_fcal_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"

class TTree;
class DFCALCluster;

#include <vector>

class JEventProcessor_twogamma_fcal_skim:public JEventProcessor{
	public:                
                 enum { kMaxHits = 500, kMaxClus = 20 };
		JEventProcessor_twogamma_fcal_skim();
		~JEventProcessor_twogamma_fcal_skim();

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

  int WRITE_ROOT;
  int WRITE_EVIO;

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

#endif // _JEventProcessor_twogamma_fcal_skim_

