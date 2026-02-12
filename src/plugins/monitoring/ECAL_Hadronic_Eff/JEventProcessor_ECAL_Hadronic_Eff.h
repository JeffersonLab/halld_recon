// $Id$
//
//    File: JEventProcessor_ECAL_Hadronic_Eff.h
// Created: Mon Jul  7 11:40:05 AM EDT 2025
// Creator: aaustreg (on Linux ifarm2402.jlab.org 5.14.0-503.23.2.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_ECAL_Hadronic_Eff_
#define _JEventProcessor_ECAL_Hadronic_Eff_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h> // Required for accessing services

#include "TH1I.h"
#include "TH2I.h"

#include "TRIGGER/DTrigger.h"
#include "TRACKING/DTrackTimeBased.h"

#include "ECAL/DECALShower.h"
#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"
#include "PID/DParticleID.h"
#include "PID/DDetectorMatches.h"
#include "ANALYSIS/DCutActions.h"
#include "ANALYSIS/DTreeInterface.h"


class JEventProcessor_ECAL_Hadronic_Eff:public JEventProcessor{
    public:
        JEventProcessor_ECAL_Hadronic_Eff();
        ~JEventProcessor_ECAL_Hadronic_Eff();
        const char* className(void){return "JEventProcessor_ECAL_Hadronic_Eff";}

    private:
        void Init() override;                       ///< Called once at program start.
        void BeginRun(const std::shared_ptr<const JEvent>& event) override; ///< Called everytime a new run number is detected.
        void Process(const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
        void EndRun() override;                     ///< Called everytime run number changes, provided BeginRun has been called.
        void Finish() override;                     ///< Called after last event of last event source has been processed.

    	std::shared_ptr<JLockService> lockService; //Used to access all the services, its value should be set inside Init()

        //double Calc_ECALTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis, const DParticleID* locParticleID, const DEventRFBunch* locEventRFBunch, double& locDeltaT);
		bool Cut_TOFTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis);

        //TRACK REQUIREMENTS
		double dMaxTOFDeltaT;
		double dMinTrackingFOM;
		unsigned int dMinNumTrackHits;
		int dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage;
		double dMaxFCALThetaCut;
		double dMaxVertexR;
		DCutAction_TrackHitPattern* dCutAction_TrackHitPattern;

		//HISTOGRAMS
		TH2I* dHist_TrackECALYVsX_HasHit;
		TH2I* dHist_TrackECALYVsX_TotalHit;
		TH2I* dHist_TrackECALRowVsColumn_HasHit;
		TH2I* dHist_TrackECALRowVsColumn_TotalHit;

        TH1I* dHist_TrackECAL_ResX;
		TH1I* dHist_TrackECAL_ResY;
		TH1I* dHist_TrackECAL_ResX_all;
		TH1I* dHist_TrackECAL_ResY_all;
		TH2I* dHist_TrackECALRowVsColumn_ResX;
		TH2I* dHist_TrackECALRowVsColumn_ResY;
};

#endif // _JEventProcessor_ECAL_Hadronic_Eff_

