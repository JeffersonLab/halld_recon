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
#include "ANALYSIS/DTreeInterface.h"

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

  //For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here

//  DFCALGeometry m_fcalGeom;
  
  double m_zTarget;
  double m_FCALUpstream;

//  TTree* m_tree;
  
  float xL;
  float yL;
  float zL;
  float eL;
  float xH;
  float yH;
  float zH;
  float eH;
 
//-------------
  int nHitsH;
  float sumUShH;
  float sumVShH;
  float e1e9ShH;
  float e9e25ShH;
  float asymUVShH;
  float speedShH;
  float dtTrShH;

  float t0RF;
  float disShH;
  float dtShH;
  float tTrH;
  float tShH;


  float beamE;
  int typeShL;
  int typeShH;

  int nHitsL;
  float sumUShL;
  float sumVShL;
  float e1e9ShL;
  float e9e25ShL;
  float asymUVShL;
  float speedShL;
  float dtTrShL;

  float disShL;
  float dtShL;
  float tTrL;
  float tShL;
  float eHitL;
  float eHitH;
//-------------


  float qualL;
  float qualH;
  
  float invM;
 // int nTrk;

DTreeInterface* dTreeInterface;
static thread_local DTreeFillData dTreeFillData;

};
#endif // _JEventProcessor_Pi0Finder_

