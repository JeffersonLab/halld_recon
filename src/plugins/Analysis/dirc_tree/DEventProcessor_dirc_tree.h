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


using namespace std;

class DEventProcessor_dirc_tree : public JEventProcessor
{
public:
   DEventProcessor_dirc_tree() {
      SetTypeName("DEventProcessor_dirc_tree");
   }
   ~DEventProcessor_dirc_tree() override {}

private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
  void Process(const std::shared_ptr<const JEvent>& locEvent) override;
  void EndRun() override;
  void Finish() override;

  //For non-custom reaction-independent histograms, it is recommended that you simultaneously run the monitoring_hists plugin instead of defining them here
		
  const DParticleID* dParticleID;		  
  const DDIRCGeometry* dDIRCGeometry;		  
  TClonesArray *fcEvent;
  DrcEvent *fEvent;
  TTree *fTree;
  const DAnalysisUtilities* fAnalysisUtilities;

};

#endif // _DEventProcessor_dirc_tree_

