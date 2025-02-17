// $Id$
//
//    File: JEventProcessor_imaging.h
// Created: Thu Nov  9 10:49:12 EST 2017
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_imaging_
#define _JEventProcessor_imaging_

#include <JANA/JEventProcessor.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>
#include <HDGEOMETRY/DGeometry.h>
#include <ANALYSIS/DAnalysisUtilities.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TH3I.h>

class JEventProcessor_imaging:public JEventProcessor{
	public:
		JEventProcessor_imaging();
		~JEventProcessor_imaging();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
	
		TH3I *TwoTrackXYZ;
		TH1F *TwoTrackZ;
		TH1F *TwoTrackZFit;
		TH2F *TwoTrackPocaCut;
		TH2F *TwoTrackXY_at_65cm;
		TH1F *TwoTrackDz,*TwoTrackDoca;
		TH3I *MCVertexDiff;
		TH2F *MCVertexDxVsZ,*MCVertexDyVsZ,*MCVertexDzVsZ;
		TH2F *MCVertexDxVsR,*MCVertexDyVsR,*MCVertexDzVsR;


		const DMagneticFieldMap *bfield;
		const DAnalysisUtilities* dAnalysisUtilities = nullptr;

		bool MC_RECON_CHECK,dIsNoFieldFlag;
		int DEBUG_LEVEL;
		double TRACK_CL_CUT,FIT_CL_CUT,DOCA_CUT;
};

#endif // _JEventProcessor_imaging_

