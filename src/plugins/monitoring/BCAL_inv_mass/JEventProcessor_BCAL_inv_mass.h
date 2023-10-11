// $Id$
//
//    File: DEventProcessor_BCAL_Shower.h
// Created: Fri Oct 10 16:41:18 EDT 2014
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_BCAL_Shower_
#define _DEventProcessor_BCAL_Shower_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
#include "TRACKING/DTrackFitter.h"

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class JEventProcessor_BCAL_inv_mass : public JEventProcessor
{
	public:
		JEventProcessor_BCAL_inv_mass(){
			SetTypeName("JEventProcessor_inv_mass");
		};
		~JEventProcessor_BCAL_inv_mass(){};
		DVector3 Calc_CrudeVertex(const deque<const DKinematicData*>& locParticles) const;
		
	       	
	private:
		const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
};

#endif // _DEventProcessor_BCAL_Shower_

