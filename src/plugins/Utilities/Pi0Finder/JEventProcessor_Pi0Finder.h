// $Id$
//
//    File: JEventProcessor_Pi0Finder.h
// Created: Fri Jul 11 14:18:49 EDT 2014
// Creator: mashephe (on Darwin 149-160-160-51.dhcp-bl.indiana.edu 13.3.0 i386)
//

#ifndef _JEventProcessor_Pi0Finder_
#define _JEventProcessor_Pi0Finder_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>
#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include <FCAL/DFCALGeometry.h>

using namespace jana;
using namespace std;

class DTrackWireBased;
class TTree;

class JEventProcessor_Pi0Finder : public jana::JEventProcessor
{
 public:
  JEventProcessor_Pi0Finder(){}
  ~JEventProcessor_Pi0Finder(){}
  const char* className(void){return "JEventProcessor_Pi0Finder";}

 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop* locEventLoop, int32_t locRunNumber);	///< Called every time a new run number is detected.
  jerror_t evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber);	///< Called every event.
  jerror_t erun(void);						///< Called every time run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  vector< const DTrackWireBased* > filterWireBasedTracks( vector< const DTrackWireBased* >& wbTracks ) const;
  
  //For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

  DFCALGeometry m_fcalGeom;
  
  double m_zTarget;
  double m_FCALUpstream;

  TTree* m_tree;
  
  float m_xL;
  float m_yL;
  float m_zL;
  float m_eL;
  float m_xH;
  float m_yH;
  float m_zH;
  float m_eH;
 
//-------------
  int m_nHitsH;
  float m_sumUShH;
  float m_sumVShH;
  float m_e1e9ShH;
  float m_e9e25ShH;
  float m_asymUVShH;
  float m_speedShH;
  float m_dtTrShH;

  float m_t0RF;
  float m_disShH;
  float m_dtShH;
  float m_tTrH;
  float m_tShH;


  float m_beamE;
  int m_typeShL;
  int m_typeShH;

  int m_nHitsL;
  float m_sumUShL;
  float m_sumVShL;
  float m_e1e9ShL;
  float m_e9e25ShL;
  float m_asymUVShL;
  float m_speedShL;
  float m_dtTrShL;

  float m_disShL;
  float m_dtShL;
  float m_tTrL;
  float m_tShL;
  float m_eHitL;
  float m_eHitH;
//-------------


  float m_qualL;
  float m_qualH;
  
  float m_invM;
  int m_nTrk;
};
#endif // _JEventProcessor_Pi0Finder_

