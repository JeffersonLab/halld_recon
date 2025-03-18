// $Id$
//
//    File: JEventProcessor_Pi0Finder.h
// Created: Fri Jul 11 14:18:49 EDT 2014
// Creator: mashephe (on Darwin 149-160-160-51.dhcp-bl.indiana.edu 13.3.0 i386)
//

#ifndef _JEventProcessor_Pi0Finder_
#define _JEventProcessor_Pi0Finder_

#include <JANA/JEventProcessor.h>
#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include <FCAL/DFCALGeometry.h>
#include "ANALYSIS/DTreeInterface.h"

using namespace std;

class DTrackWireBased;
class TTree;

class JEventProcessor_Pi0Finder : public JEventProcessor
{
 public:
  JEventProcessor_Pi0Finder(){
      SetTypeName("JEventProcessor_Pi0Finder");
  }
  ~JEventProcessor_Pi0Finder(){}

 private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;

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

