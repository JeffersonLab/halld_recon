// $Id$
//
//    File: DEventProcessor_dirc_tree.h
//

#ifndef _DEventProcessor_dirc_tree_
#define _DEventProcessor_dirc_tree_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include <ANALYSIS/DAnalysisUtilities.h>

#include "DFactoryGenerator_dirc_tree.h"

#include "DIRC/DDIRCGeometry.h"
#include "DrcHit.h"
#include "DrcEvent.h"

#include <TVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>


using namespace jana;
using namespace std;

class DEventProcessor_dirc_tree : public jana::JEventProcessor
{
public:
  const char* className(void){return "DEventProcessor_dirc_tree";}

private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop* locEventLoop, int locRunNumber);	///< Called every time a new run number is detected.
  jerror_t evnt(jana::JEventLoop* locEventLoop, uint64_t locEventNumber);	///< Called every event.
  jerror_t erun(void);						///< Called every time run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

  //For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here
		
  const DParticleID* dParticleID;		  
  const DDIRCGeometry* dDIRCGeometry;		  
  TClonesArray *fcEvent;
  DrcEvent *fEvent;
  TTree *fTree;
  const DAnalysisUtilities* fAnalysisUtilities;

};

#endif // _DEventProcessor_dirc_tree_

