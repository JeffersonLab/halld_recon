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

class JEventProcessor_twogamma_fcal_skim:public jana::JEventProcessor{
	public:                
                 enum { kMaxHits = 500, kMaxClus = 20 };
		JEventProcessor_twogamma_fcal_skim();
		~JEventProcessor_twogamma_fcal_skim();
		const char* className(void){return "JEventProcessor_twogamma_fcal_skim";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

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

