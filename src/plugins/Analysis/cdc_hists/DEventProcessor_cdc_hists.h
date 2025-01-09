// $Id: $
//
//    File: DEventProcessor_cdc_hists.h
//

#ifndef _DEventProcessor_cdc_hists_
#define _DEventProcessor_cdc_hists_

#include <pthread.h>
#include <map>
using std::map;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <PID/DKinematicData.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRACKING/DMCTrackHit.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCHit.h>

#include "CDC_branch.h"
#include "CDChit_branch.h"

class DEventProcessor_cdc_hists:public JEventProcessor{

	public:
		DEventProcessor_cdc_hists();
		~DEventProcessor_cdc_hists();
		
		TTree *cdctree;
		CDC_branch cdc;
		CDC_branch *cdc_ptr;
		TTree *cdchittree;
		CDChit_branch cdchit;
		CDChit_branch *cdchit_ptr;
		TBranch *cdcbranch, *cdchitbranch;
		
		TH1D *idEdx;
		TH2D *idEdx_vs_p;

	private:
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		pthread_mutex_t mutex;
		
		const DMagneticFieldMap *bfield;
};

#endif // _DEventProcessor_cdc_hists_

