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

class JEventProcessor_imaging:public jana::JEventProcessor{
	public:
		JEventProcessor_imaging();
		~JEventProcessor_imaging();
		const char* className(void){return "JEventProcessor_imaging";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed. 
	
		TH3I *TwoTrackXYZ;
		TH1F *TwoTrackZ;
		TH1F *TwoTrackZFit;
		TH2F *TwoTrackXYFit_at_65cm;
		TH3I *TwoTrackXYZFit;
		TH2F *TwoTrackPocaCut,*TwoTrackPocaCutFit;
		TH2F *TwoTrackXY_at_65cm;
		TH1F *TwoTrackProb;
		TH1F *TwoTrackDz,*TwoTrackDoca;
		TH3I *MCVertexDiff;
		TH2F *MCVertexDxVsZ,*MCVertexDyVsZ,*MCVertexDzVsZ;
		TH2F *MCVertexDxVsR,*MCVertexDyVsR,*MCVertexDzVsR;


		const DMagneticFieldMap *bfield;
		const DAnalysisUtilities* dAnalysisUtilities = nullptr;

		bool MC_RECON_CHECK;
		int DEBUG_LEVEL;
		double TRACK_CL_CUT,FIT_CL_CUT,DOCA_CUT;
};

#endif // _JEventProcessor_imaging_

