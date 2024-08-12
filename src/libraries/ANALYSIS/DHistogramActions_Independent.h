#ifndef _DHistogramActions_Independent_
#define _DHistogramActions_Independent_

#include <map>
#include <set>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <limits>

#include "TROOT.h"
#include "TDirectoryFile.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

#include <JANA/JEvent.h>
#include "particleType.h"
#include "DResourcePool.h"

#include <DANA/DStatusBits.h>
#include "RF/DRFTime.h"
#include "RF/DRFTime_factory.h"
#include "RF/DRFDigiTime.h"
#include "RF/DRFTDCDigiTime.h"
#include "START_COUNTER/DSCHit.h"
#include "TAGGER/DTAGHHit.h"
#include "TAGGER/DTAGMHit.h"
#include "CDC/DCDCHit.h"
#include "FDC/DFDCHit.h"
#include "FDC/DFDCPseudo.h"
#include "FMWPC/DFMWPCHit.h"
#include "TOF/DTOFPoint.h"
#include "TOF/DTOFHit.h"
#include "TOF/DTOFPaddleHit.h"
#include "BCAL/DBCALShower.h"
#include "BCAL/DBCALHit.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALHit.h"
#include "CCAL/DCCALShower.h"
#include "CCAL/DCCALHit.h"

#include "TRACKING/DTrackTimeBased.h"
#include "TRACKING/DTrackWireBased.h"
#include "TRACKING/DTrackCandidate.h"
#include "TRACKING/DMCThrown.h"

#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"
#include "PID/DNeutralParticle.h"
#include "PID/DNeutralShower.h"
#include "PID/DVertex.h"
#include "PID/DDetectorMatches.h"
#include "PID/DNeutralParticleHypothesis.h"
#include "PID/DEventRFBunch.h"

#include "ANALYSIS/DReaction.h"
#include "ANALYSIS/DMCThrownMatching.h"
#include "ANALYSIS/DMCThrownMatching_factory.h"
#include "ANALYSIS/DParticleCombo.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include "ANALYSIS/DAnalysisAction.h"
#include "ANALYSIS/DCutActions.h"
#include "ANALYSIS/DAnalysisResults.h"

#include "ANALYSIS/DTrackTimeBased_factory_Combo.h"

#include "ANALYSIS/DCutActions.h"
#include "ANALYSIS/DSourceCombo.h"

#include "TRIGGER/DTrigger.h"

using namespace std;

class DParticleCombo_factory_PreKinFit;
class DAnalysisResults;

/*
REACTION-INDEPENDENT ACTIONS:
	DHistogramAction_ObjectMemory
	DHistogramAction_TrackMultiplicity
	DHistogramAction_EventVertex
	DHistogramAction_DetectedParticleKinematics
	DHistogramAction_DetectorMatchParams
	DHistogramAction_Neutrals
	DHistogramAction_DetectorPID
	DHistogramAction_DetectorMatching
	DHistogramAction_Reconstruction
	DHistogramAction_NumReconstructedObjects
	DHistogramAction_TrackShowerErrors
	DHistogramAction_TriggerStudies
*/

class DHistogramAction_ObjectMemory : public DAnalysisAction
{
	// This action SHOULD NOT be added to a DReaction. If it is ... it's not the end of the world, but it won't work. 

	public:
		DHistogramAction_ObjectMemory(string locActionUniqueString) : 
		DAnalysisAction(NULL, "Hist_ObjectMemory", false, locActionUniqueString), 
		dMaxNumEvents(50000), dEventCounter(0) {}

		DHistogramAction_ObjectMemory(void) : 
		DAnalysisAction(NULL, "Hist_ObjectMemory", false, ""), 
		dMaxNumEvents(50000), dEventCounter(0) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

		void Read_MemoryUsage(double& vm_usage, double& resident_set);

		unsigned int dMaxNumEvents;

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		unsigned int dEventCounter; //not the same as event #: running with multiple threads over many files, possibly starting at event # != 1

		map<string, int> dBinMap;

		DResourcePool<TMatrixFSym> dResourcePool_TMatrixFSym;
		DResourcePool<DKinematicData::DKinematicInfo> dResourcePool_KinematicInfo;
		DResourcePool<DChargedTrackHypothesis::DTimingInfo> dResourcePool_ChargedHypoTimingInfo;
		DResourcePool<DChargedTrackHypothesis::DTrackingInfo> dResourcePool_ChargedHypoTrackingInfo;
		DResourcePool<DNeutralParticleHypothesis::DTimingInfo> dResourcePool_NeutralHypoTimingInfo;

		DResourcePool<DKinematicData> dResourcePool_KinematicData;
		DResourcePool<DBeamPhoton> dResourcePool_BeamPhotons;
		DResourcePool<DNeutralParticleHypothesis> dResourcePool_NeutralParticleHypothesis;
		DResourcePool<DChargedTrackHypothesis> dResourcePool_ChargedTrackHypothesis;
		DResourcePool<DEventRFBunch> dResourcePool_EventRFBunch;

		DResourcePool<DSourceCombo> dResourcePool_SourceCombo;
		DResourcePool<vector<const DSourceCombo*>> dResourcePool_SourceComboVector;
		DResourcePool<DParticleCombo> dResourcePool_ParticleCombo;
		DResourcePool<DParticleComboStep> dResourcePool_ParticleComboStep;

		DResourcePool<DKinFitParticle> dResourcePool_KinFitParticle;
		DResourcePool<DKinFitChainStep> dResourcePool_KinFitChainStep;
		DResourcePool<DKinFitChain> dResourcePool_KinFitChain;
		DResourcePool<DKinFitResults> dResourcePool_KinFitResults;

		DResourcePool<DKinFitConstraint_Mass> dResourcePool_MassConstraint;
		DResourcePool<DKinFitConstraint_P4> dResourcePool_P4Constraint;
		DResourcePool<DKinFitConstraint_Vertex> dResourcePool_VertexConstraint;
		DResourcePool<DKinFitConstraint_Spacetime> dResourcePool_SpacetimeConstraint;

		TH2I* dHist_NumObjects = nullptr;
		TH2F* dHist_Memory = nullptr;

		TH1F* dVirtualMemoryVsEventNumber = nullptr;
		TH1F* dResidentMemoryVsEventNumber = nullptr;
		TH1F* dHist_TotalMemory = nullptr;
};

class DHistogramAction_Reconstruction : public DAnalysisAction
{
	public:

		//user can call any of these three constructors
		DHistogramAction_Reconstruction(const DReaction* locReaction, string locActionUniqueString = "") :
		DAnalysisAction(locReaction, "Hist_Reconstruction", false, locActionUniqueString),
		dNumFCALTOFXYBins(260), dNumShowerEnergyBins(800), dNumPhiBins(720), dNum2DBCALZBins(450), dNum2DPhiBins(360), dNumHitEnergyBins(500),
		dNum2DHitEnergyBins(250), dNum2DThetaBins(280), dNumFOMBins(500), dNum2DFOMBins(200), dNum2DPBins(250), dNum2DDeltaTBins(800),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinPhi(-180.0), dMaxPhi(180.0), dMinHitEnergy(0.0),
		dMaxHitEnergy(50.0), dMinTheta(0.0), dMaxTheta(140.0), dMinP(0.0), dMaxP(10.0), dMinDeltaT(-20.0), dMaxDeltaT(20.0),
		dGoodTrackFOM(5.73303E-7), dHighTrackFOM(0.98)
		{
			dTargetCenter.SetXYZ(0.0, 0.0, -9.9E9);
		}

		DHistogramAction_Reconstruction(string locActionUniqueString) :
		DAnalysisAction(NULL, "Hist_Reconstruction", false, locActionUniqueString),
		dNumFCALTOFXYBins(260), dNumShowerEnergyBins(800), dNumPhiBins(720), dNum2DBCALZBins(450), dNum2DPhiBins(360), dNumHitEnergyBins(500),
		dNum2DHitEnergyBins(250), dNum2DThetaBins(280), dNumFOMBins(500), dNum2DFOMBins(200), dNum2DPBins(250), dNum2DDeltaTBins(800),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinPhi(-180.0), dMaxPhi(180.0), dMinHitEnergy(0.0),
		dMaxHitEnergy(50.0), dMinTheta(0.0), dMaxTheta(140.0), dMinP(0.0), dMaxP(10.0), dMinDeltaT(-20.0), dMaxDeltaT(20.0),
		dGoodTrackFOM(5.73303E-7), dHighTrackFOM(0.98)
		{
			dTargetCenter.SetXYZ(0.0, 0.0, -9.9E9);
		}

		DHistogramAction_Reconstruction(void) :
		DAnalysisAction(NULL, "Hist_Reconstruction", false, ""),
		dNumFCALTOFXYBins(260), dNumShowerEnergyBins(800), dNumPhiBins(720), dNum2DBCALZBins(450), dNum2DPhiBins(360), dNumHitEnergyBins(500),
		dNum2DHitEnergyBins(250), dNum2DThetaBins(280), dNumFOMBins(500), dNum2DFOMBins(200), dNum2DPBins(250), dNum2DDeltaTBins(800),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinPhi(-180.0), dMaxPhi(180.0), dMinHitEnergy(0.0),
		dMaxHitEnergy(50.0), dMinTheta(0.0), dMaxTheta(140.0), dMinP(0.0), dMaxP(10.0), dMinDeltaT(-20.0), dMaxDeltaT(20.0),
		dGoodTrackFOM(5.73303E-7), dHighTrackFOM(0.98)
		{
			dTargetCenter.SetXYZ(0.0, 0.0, -9.9E9);
		}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent);

		unsigned int dNumFCALTOFXYBins, dNumShowerEnergyBins, dNumPhiBins, dNum2DBCALZBins, dNum2DPhiBins;
		unsigned int dNumHitEnergyBins, dNum2DHitEnergyBins, dNum2DThetaBins, dNumFOMBins, dNum2DFOMBins, dNum2DPBins, dNum2DDeltaTBins;
		double dMinShowerEnergy, dMaxShowerEnergy, dMaxBCALP, dMinPhi, dMaxPhi, dMinHitEnergy;
		double dMaxHitEnergy, dMinTheta, dMaxTheta, dMinP, dMaxP, dMinDeltaT, dMaxDeltaT;

		double dGoodTrackFOM, dHighTrackFOM;

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		DVector3 dTargetCenter;

		TH2I* dHist_FCALShowerYVsX = nullptr;
		TH1I* dHist_FCALShowerEnergy = nullptr;

		TH2I* dHist_CCALShowerYVsX = nullptr;
		TH1I* dHist_CCALShowerEnergy = nullptr;

		TH1I* dHist_BCALShowerEnergy = nullptr;
		TH1I* dHist_BCALShowerPhi = nullptr;
		TH2I* dHist_BCALShowerPhiVsZ = nullptr;

		TH1I* dHist_TOFPointEnergy = nullptr;
		TH1I* dHist_TOFPointEnergyP1 = nullptr;
		TH1I* dHist_TOFPointEnergyP2 = nullptr;
		TH2I* dHist_TOFPointYVsX = nullptr;

		TH1I* dHist_SCHitSector = nullptr;
		TH1I* dHist_SCHitEnergy = nullptr;
		TH2I* dHist_SCHitEnergyVsSector = nullptr;
		TH2I *dHist_SCRFDeltaTVsSector = nullptr;

		TH2I *dHist_TAGMRFDeltaTVsColumn = nullptr;
		TH2I *dHist_TAGHRFDeltaTVsCounter = nullptr;

		TH1I* dHist_NumDCHitsPerTrack = nullptr;
		TH1I* dHist_NumPossDCHitsPerTrack = nullptr;
		TH1I* dHist_TrackHitFraction = nullptr;
		TH2I* dHist_NumDCHitsPerTrackVsTheta = nullptr;
		TH2I* dHist_NumPossDCHitsPerTrackVsTheta = nullptr;
		TH2I* dHist_TrackHitFractionVsTheta = nullptr;
		TH1I* dHist_TrackingFOM = nullptr;
		TH1I* dHist_TrackingFOM_WireBased = nullptr;
		TH2I* dHist_TrackingFOMVsTheta = nullptr;
		TH2I* dHist_TrackingFOMVsP = nullptr;
		TH2I* dHist_TrackingFOMVsNumHits = nullptr;
		map<int, TH2I*> dHistMap_PVsTheta_Candidates; //int is charge
		map<int, TH2I*> dHistMap_PVsTheta_WireBased; //int is charge
		map<int, TH2I*> dHistMap_PVsTheta_TimeBased; //int is charge
		map<int, TH2I*> dHistMap_PVsTheta_TimeBased_GoodTrackFOM; //int is charge
		map<int, TH2I*> dHistMap_PVsTheta_TimeBased_LowTrackFOM; //int is charge
		map<int, TH2I*> dHistMap_PVsTheta_TimeBased_HighTrackFOM; //int is charge

		map<int, TH2I*> dHistMap_PVsTheta_GoodWireBased_GoodTimeBased; //int is charge
		map<int, TH2I*> dHistMap_PVsTheta_GoodWireBased_BadTimeBased; //int is charge

		TH2I* dHist_CDCRingVsTheta_Candidates = nullptr;
		TH2I* dHist_CDCRingVsTheta_WireBased = nullptr;
		TH2I* dHist_CDCRingVsTheta_TimeBased = nullptr;
		TH2I* dHist_CDCRingVsTheta_TimeBased_GoodTrackFOM = nullptr;
		TH2I* dHist_FDCPlaneVsTheta_Candidates = nullptr;
		TH2I* dHist_FDCPlaneVsTheta_WireBased = nullptr;
		TH2I* dHist_FDCPlaneVsTheta_TimeBased = nullptr;
		TH2I* dHist_FDCPlaneVsTheta_TimeBased_GoodTrackFOM = nullptr;

		TH2I* dHist_MCMatchedHitsVsTheta = nullptr;
		TH2I* dHist_MCMatchedHitsVsP = nullptr;
};

class DHistogramAction_DetectorMatching : public DAnalysisAction
{
	public:

		//user can call any of these three constructors
		DHistogramAction_DetectorMatching(const DReaction* locReaction, string locActionUniqueString = "") :
		DAnalysisAction(locReaction, "Hist_DetectorMatching", false, locActionUniqueString),
		dNum2DThetaBins(280), dNumPBins(500), dNum2DPBins(250), dNum2DPhiBins(360), dNum2DDeltaPhiBins(300), dNum2DDeltaZBins(250), dNum2DTrackDOCABins(200),
		dNumTrackDOCABins(400), dNumFCALTOFXYBins(260), dNum2DBCALZBins(450), dNum2DSCZBins(240), dNumTOFRBins(360), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0),
		dMinPhi(-180.0), dMaxPhi(180.0), dSCMatchMinDeltaPhi(-60.0), dSCMatchMaxDeltaPhi(60.0), dMinTrackDOCA(0.0), dMaxTrackMatchDOCA(20.0),
		dMinDeltaPhi(-30.0), dMaxDeltaPhi(30.0), dMinDeltaZ(-50.0), dMaxDeltaZ(50.0),
		dMinTrackingFOM(0.0027), dMinTOFPaddleMatchDistance(9.0), dMinHitRingsPerCDCSuperlayer(2), dMinHitPlanesPerFDCPackage(4) {}

		DHistogramAction_DetectorMatching(string locActionUniqueString) :
		DAnalysisAction(NULL, "Hist_DetectorMatching", false, locActionUniqueString),
		dNum2DThetaBins(280), dNumPBins(500), dNum2DPBins(250), dNum2DPhiBins(360), dNum2DDeltaPhiBins(300), dNum2DDeltaZBins(250), dNum2DTrackDOCABins(200),
		dNumTrackDOCABins(400), dNumFCALTOFXYBins(260), dNum2DBCALZBins(450), dNum2DSCZBins(240), dNumTOFRBins(360), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0),
		dMinPhi(-180.0), dMaxPhi(180.0), dSCMatchMinDeltaPhi(-60.0), dSCMatchMaxDeltaPhi(60.0), dMinTrackDOCA(0.0), dMaxTrackMatchDOCA(20.0),
		dMinDeltaPhi(-30.0), dMaxDeltaPhi(30.0), dMinDeltaZ(-50.0), dMaxDeltaZ(50.0),
		dMinTrackingFOM(0.0027), dMinTOFPaddleMatchDistance(9.0), dMinHitRingsPerCDCSuperlayer(2), dMinHitPlanesPerFDCPackage(4) {}

		DHistogramAction_DetectorMatching(void) :
		DAnalysisAction(NULL, "Hist_DetectorMatching", false, ""),
		dNum2DThetaBins(280), dNumPBins(500), dNum2DPBins(250), dNum2DPhiBins(360), dNum2DDeltaPhiBins(300), dNum2DDeltaZBins(250), dNum2DTrackDOCABins(200),
		dNumTrackDOCABins(400), dNumFCALTOFXYBins(260), dNum2DBCALZBins(450), dNum2DSCZBins(240), dNumTOFRBins(360), dMinP(0.0), dMaxP(10.0), dMinTheta(0.0), dMaxTheta(140.0),
		dMinPhi(-180.0), dMaxPhi(180.0), dSCMatchMinDeltaPhi(-60.0), dSCMatchMaxDeltaPhi(60.0), dMinTrackDOCA(0.0), dMaxTrackMatchDOCA(20.0),
		dMinDeltaPhi(-30.0), dMaxDeltaPhi(30.0), dMinDeltaZ(-50.0), dMaxDeltaZ(50.0),
		dMinTrackingFOM(0.0027), dMinTOFPaddleMatchDistance(9.0), dMinHitRingsPerCDCSuperlayer(2), dMinHitPlanesPerFDCPackage(4) {}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent);

		unsigned int dNum2DThetaBins, dNumPBins, dNum2DPBins, dNum2DPhiBins, dNum2DDeltaPhiBins, dNum2DDeltaZBins;
		unsigned int dNum2DTrackDOCABins, dNumTrackDOCABins, dNumFCALTOFXYBins, dNum2DBCALZBins, dNum2DSCZBins, dNumTOFRBins;
		double dMinP, dMaxP, dMinTheta, dMaxTheta, dMinPhi, dMaxPhi, dSCMatchMinDeltaPhi, dSCMatchMaxDeltaPhi, dMinTrackDOCA, dMaxTrackMatchDOCA;
		double dMinDeltaPhi, dMaxDeltaPhi, dMinDeltaZ, dMaxDeltaZ;

		double dMinTrackingFOM, dMinTOFPaddleMatchDistance;
		unsigned int dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage;

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);
		void Fill_MatchingHists(const std::shared_ptr<const JEvent>& locEvent, bool locIsTimeBased);

		bool Get_Extrapolations(const DKinematicData* locTrack,map<DetectorSystem_t,vector<DTrackFitter::Extrapolation_t> >&extrapolations) const{
		  const DTrackTimeBased* locTrackTimeBased = dynamic_cast<const DTrackTimeBased*>(locTrack);
		  const DTrackWireBased* locTrackWireBased = dynamic_cast<const DTrackWireBased*>(locTrack);
		  if((locTrackTimeBased == NULL) && (locTrackWireBased == NULL))
		    return false;
		  (locTrackTimeBased != NULL) ? (extrapolations=locTrackTimeBased->extrapolations) : (extrapolations=locTrackWireBased->extrapolations);
		  return true;
		}

		double TOF_E_THRESHOLD;
        bool IGNORE_START_COUNTER;

		//bool is time/wire-based for true/false
		map<DetectorSystem_t, map<bool, TH2I*> > dHistMap_PVsTheta_HasHit;
		map<DetectorSystem_t, map<bool, TH2I*> > dHistMap_PVsTheta_NoHit;
		map<DetectorSystem_t, map<bool, TH2I*> > dHistMap_PhiVsTheta_HasHit;
		map<DetectorSystem_t, map<bool, TH2I*> > dHistMap_PhiVsTheta_NoHit;

		map<bool, TH2I*> dHistMap_SCPaddleVsTheta_HasHit;
		map<bool, TH2I*> dHistMap_SCPaddleVsTheta_NoHit;
		map<bool, TH2I*> dHistMap_SCPaddleVsZ_HasHit;
		map<bool, TH2I*> dHistMap_SCPaddleVsZ_NoHit;
		map<bool, TH1I*> dHistMap_SCPaddle_BarrelRegion_HasHit;
		map<bool, TH1I*> dHistMap_SCPaddle_BarrelRegion_NoHit;
		map<bool, TH1I*> dHistMap_SCPaddle_NoseRegion_HasHit; //includes bend
		map<bool, TH1I*> dHistMap_SCPaddle_NoseRegion_NoHit; //includes bend

		map<bool, TH2I*> dHistMap_TrackTOFYVsX_HasHit;
		map<bool, TH2I*> dHistMap_TrackTOFYVsX_NoHit;
		map<bool, TH2I*> dHistMap_TrackTOF2DPaddles_HasHit;
		map<bool, TH2I*> dHistMap_TrackTOF2DPaddles_NoHit;
		map<bool, TH1I*> dHistMap_TrackTOFR_HasHit;
		map<bool, TH1I*> dHistMap_TrackTOFR_NoHit;
		map<bool, TH1I*> dHistMap_TrackTOFP_HasHit;
		map<bool, TH1I*> dHistMap_TrackTOFP_NoHit;

		map<bool, TH2I*> dHistMap_TrackFCALYVsX_HasHit;
		map<bool, TH2I*> dHistMap_TrackFCALYVsX_NoHit;
		map<bool, TH1I*> dHistMap_TrackFCALR_HasHit;
		map<bool, TH1I*> dHistMap_TrackFCALR_NoHit;
		map<bool, TH1I*> dHistMap_TrackFCALP_HasHit;
		map<bool, TH1I*> dHistMap_TrackFCALP_NoHit;
		map<bool, TH2I*> dHistMap_TrackFCALRowVsColumn_HasHit;
		map<bool, TH2I*> dHistMap_TrackFCALRowVsColumn_NoHit;

		map<bool, TH2I*> dHistMap_TrackBCALModuleVsZ_HasHit;
		map<bool, TH2I*> dHistMap_TrackBCALModuleVsZ_NoHit;
		map<bool, TH2I*> dHistMap_TrackBCALPhiVsZ_HasHit;
		map<bool, TH2I*> dHistMap_TrackBCALPhiVsZ_NoHit;

		map<bool, TH2I*> dHistMap_TrackPVsTheta_NoHitMatch;
		map<bool, TH2I*> dHistMap_TrackPVsTheta_HitMatch;
		map<bool, TH2I*> dHistMap_SCTrackDeltaPhiVsP;
		map<bool, TH2I*> dHistMap_SCTrackDeltaPhiVsZ;
		map<bool, TH2I*> dHistMap_SCTrackDeltaPhiVsTheta;
		map<bool, TH2I*> dHistMap_FCALTrackDistanceVsP;
		map<bool, TH2I*> dHistMap_FCALTrackDistanceVsTheta;
		map<bool, TH2I*> dHistMap_BCALDeltaPhiVsP;
		map<bool, TH2I*> dHistMap_BCALDeltaPhiVsZ;
		map<bool, TH2I*> dHistMap_BCALDeltaZVsTheta;
		map<bool, TH2I*> dHistMap_BCALDeltaZVsZ;

		//DTOFPaddle Matching
		map<bool, TH1I*> dHistMap_TOFPaddleTrackDeltaX;
		map<bool, TH1I*> dHistMap_TOFPaddleTrackDeltaY;
		map<bool, TH2I*> dHistMap_TOFPaddleTrackYVsVerticalPaddle_HasHit;
		map<bool, TH2I*> dHistMap_TOFPaddleTrackYVsVerticalPaddle_NoHit;
		map<bool, TH2I*> dHistMap_TOFPaddleHorizontalPaddleVsTrackX_HasHit;
		map<bool, TH2I*> dHistMap_TOFPaddleHorizontalPaddleVsTrackX_NoHit;

		//DTOFPoint Matching
		map<bool, TH2I*> dHistMap_TOFPointTrackDistanceVsP;
		map<bool, TH2I*> dHistMap_TOFPointTrackDistanceVsTheta;
		map<bool, TH2I*> dHistMap_TOFPointTrackDeltaXVsHorizontalPaddle;
		map<bool, TH2I*> dHistMap_TOFPointTrackDeltaXVsVerticalPaddle;
		map<bool, TH2I*> dHistMap_TOFPointTrackDeltaYVsHorizontalPaddle;
		map<bool, TH2I*> dHistMap_TOFPointTrackDeltaYVsVerticalPaddle;
		map<bool, TH1I*> dHistMap_TOFPointTrackDistance_BothPlanes;
		map<bool, TH1I*> dHistMap_TOFPointTrackDistance_OnePlane;
};

class DHistogramAction_DetectorPID : public DAnalysisAction
{
	//Delta-t plots can be found in DHistogramAction_DetectorMatchParams
	public:

		//user can call any of these three constructors
		DHistogramAction_DetectorPID(const DReaction* locReaction, string locActionUniqueString = "") :
		DAnalysisAction(locReaction, "Hist_DetectorPID", false, locActionUniqueString),
		dNum2DPBins(250), dNum2DdEdxBins(400), dNum2DBetaBins(400), dNum2DBCALThetaBins(260), dNum2DFCALThetaBins(120), dNum2DCCALThetaBins(120), dNum2DEOverPBins(300),
		dNum2DDeltaBetaBins(400), dNum2DDeltadEdxBins(300), dNum2DDeltaTBins(400), dNum2DPullBins(200), dNum2DFOMBins(200), dMinP(0.0), dMaxP(10.0), dMaxBCALP(3.0), 
		dMindEdX(0.0), dMaxdEdX(25.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinBCALTheta(10.0), dMaxBCALTheta(140.0), dMinFCALTheta(0.0), dMaxFCALTheta(12.0), 
		dMinCCALTheta(0.0), dMaxCCALTheta(2.0), 
		dMinEOverP(0.0), dMaxEOverP(4.0), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0), dMinDeltadEdx(-30.0), dMaxDeltadEdx(30.0), dMinDeltaT(-10.0), dMaxDeltaT(10.0), 
		dMinPull(-10.0), dMaxPull(10.0), dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag"),
		dDIRCNumPhotonsBins(100), dDIRCThetaCBins(100), dDIRCLikelihoodBins(100), 
		dDIRCMinNumPhotons(0), dDIRCMaxNumPhotons(100),
                dDIRCMinThetaC(0.6), dDIRCMaxThetaC(1.0), dDIRCMinLikelihood(0), dDIRCMaxLikelihood(1000)
		{
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);

			// used for indexing into maps
			SYS_CDC_AMP = static_cast<DetectorSystem_t>(SYS_CDC + SYS_FDC);
		}

		DHistogramAction_DetectorPID(string locActionUniqueString) :
		DAnalysisAction(NULL, "Hist_DetectorPID", false, locActionUniqueString),
		dNum2DPBins(250), dNum2DdEdxBins(400), dNum2DBetaBins(400), dNum2DBCALThetaBins(260), dNum2DFCALThetaBins(120), dNum2DCCALThetaBins(120), dNum2DEOverPBins(300), 
		dNum2DDeltaBetaBins(400), dNum2DDeltadEdxBins(300), dNum2DDeltaTBins(400), dNum2DPullBins(200), dNum2DFOMBins(200), dMinP(0.0), dMaxP(10.0), dMaxBCALP(3.0), 
		dMindEdX(0.0), dMaxdEdX(25.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinBCALTheta(10.0), dMaxBCALTheta(140.0), dMinFCALTheta(0.0), dMaxFCALTheta(12.0), 
		dMinCCALTheta(0.0), dMaxCCALTheta(2.0), 
		dMinEOverP(0.0), dMaxEOverP(4.0), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0), dMinDeltadEdx(-30.0), dMaxDeltadEdx(30.0), dMinDeltaT(-10.0), dMaxDeltaT(10.0), 
		dMinPull(-10.0), dMaxPull(10.0), dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag"),
		dDIRCNumPhotonsBins(100), dDIRCThetaCBins(100), dDIRCLikelihoodBins(100),
		dDIRCMinNumPhotons(0), dDIRCMaxNumPhotons(100),
                dDIRCMinThetaC(0.6), dDIRCMaxThetaC(1.0), dDIRCMinLikelihood(0), dDIRCMaxLikelihood(1000)
		{
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);

			// used for indexing into maps
			SYS_CDC_AMP = static_cast<DetectorSystem_t>(SYS_CDC + SYS_FDC);
		}

		DHistogramAction_DetectorPID(void) :
		DAnalysisAction(NULL, "Hist_DetectorPID", false, ""),
		dNum2DPBins(250), dNum2DdEdxBins(400), dNum2DBetaBins(400), dNum2DBCALThetaBins(260), dNum2DFCALThetaBins(120),dNum2DCCALThetaBins(120), dNum2DEOverPBins(300), 
		dNum2DDeltaBetaBins(400), dNum2DDeltadEdxBins(300), dNum2DDeltaTBins(400), dNum2DPullBins(200), dNum2DFOMBins(200), dMinP(0.0), dMaxP(10.0), dMaxBCALP(3.0), 
		dMindEdX(0.0), dMaxdEdX(25.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinBCALTheta(10.0), dMaxBCALTheta(140.0), dMinFCALTheta(0.0), dMaxFCALTheta(12.0), 
		dMinCCALTheta(0.0), dMaxCCALTheta(2.0), 
		dMinEOverP(0.0), dMaxEOverP(4.0), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0), dMinDeltadEdx(-30.0), dMaxDeltadEdx(30.0), dMinDeltaT(-10.0), dMaxDeltaT(10.0), 
		dMinPull(-10.0), dMaxPull(10.0), dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag"),
		dDIRCNumPhotonsBins(100), dDIRCThetaCBins(100), dDIRCLikelihoodBins(100),
		dDIRCMinNumPhotons(0), dDIRCMaxNumPhotons(100),
                dDIRCMinThetaC(0.6), dDIRCMaxThetaC(1.0), dDIRCMinLikelihood(0), dDIRCMaxLikelihood(1000)
		{
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);

			// used for indexing into maps
			SYS_CDC_AMP = static_cast<DetectorSystem_t>(SYS_CDC + SYS_FDC);
		}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent) {}

		unsigned int dNum2DPBins, dNum2DdEdxBins, dNum2DBetaBins, dNum2DBCALThetaBins, dNum2DFCALThetaBins, dNum2DCCALThetaBins;
		unsigned int dNum2DEOverPBins, dNum2DDeltaBetaBins, dNum2DDeltadEdxBins, dNum2DDeltaTBins, dNum2DPullBins, dNum2DFOMBins;
		double dMinP, dMaxP, dMaxBCALP, dMindEdX, dMaxdEdX, dMinBeta, dMaxBeta, dMinBCALTheta, dMaxBCALTheta, dMinFCALTheta, dMaxFCALTheta, dMinCCALTheta, dMaxCCALTheta;
		double dMinEOverP, dMaxEOverP, dMinDeltaBeta, dMaxDeltaBeta, dMinDeltadEdx, dMaxDeltadEdx, dMinDeltaT, dMaxDeltaT, dMinPull, dMaxPull;
		string dTrackSelectionTag, dShowerSelectionTag; //In Initialize, will default to "PreSelect" unless otherwise specified
		unsigned int dDIRCNumPhotonsBins, dDIRCThetaCBins, dDIRCLikelihoodBins, dDIRCMinNumPhotons, dDIRCMaxNumPhotons;
                double dDIRCMinThetaC, dDIRCMaxThetaC;
                double dDIRCMinLikelihood, dDIRCMaxLikelihood;

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		DetectorSystem_t SYS_CDC_AMP;
		vector<Particle_t> dFinalStatePIDs;

		//int is charge: -1, 0, 1
		map<int, TH2I*> dHistMap_BCALEOverPVsP;
		map<int, TH2I*> dHistMap_BCALEOverPVsTheta;

		map<int, TH2I*> dHistMap_FCALEOverPVsP;
		map<int, TH2I*> dHistMap_FCALEOverPVsTheta;

		map<DetectorSystem_t, map<int, TH2I*> > dHistMap_dEdXVsP;
		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_DeltadEdXVsP;
//		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_dEdXPullVsP;
//		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_dEdXFOMVsP;

		map<DetectorSystem_t, map<int, TH2I*> > dHistMap_BetaVsP;
		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_DeltaBetaVsP;

		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_DeltaTVsP;
//		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_TimePullVsP;
//		map<DetectorSystem_t, map<Particle_t, TH2I*> > dHistMap_TimeFOMVsP;

		map<Particle_t, TH1I*> dHistMap_NumPhotons_DIRC;
                map<Particle_t, TH2I*> dHistMap_ThetaCVsP_DIRC;
                map<Particle_t, TH2I*> dHistMap_Ldiff_kpiVsP_DIRC;
                map<Particle_t, TH2I*> dHistMap_Ldiff_pkVsP_DIRC;
};

class DHistogramAction_Neutrals : public DAnalysisAction
{
	public:

		//user can call any of these three constructors
		DHistogramAction_Neutrals(const DReaction* locReaction, string locActionUniqueString = "") :
		DAnalysisAction(locReaction, "Hist_Neutrals", false, locActionUniqueString),
		dNumTrackDOCABins(400), dNumDeltaPhiBins(600), dNumShowerEnergyBins(800), dNumDeltaTBins(400), dNum2DShowerEnergyBins(400),
		dNum2DDeltaTBins(400), dNum2DBCALZBins(450), dMinTrackDOCA(0.0), dMaxTrackDOCA(400.0), dMinDeltaPhi(-30.0), dMaxDeltaPhi(30.0),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinDeltaT(-10.0), dMaxDeltaT(10.0)
		{
			dTargetCenter.SetXYZ(0.0, 0.0, -9.9E9);
		}

		DHistogramAction_Neutrals(string locActionUniqueString) :
		DAnalysisAction(NULL, "Hist_Neutrals", false, locActionUniqueString),
		dNumTrackDOCABins(400), dNumDeltaPhiBins(600), dNumShowerEnergyBins(800), dNumDeltaTBins(400), dNum2DShowerEnergyBins(400),
		dNum2DDeltaTBins(400), dNum2DBCALZBins(450), dMinTrackDOCA(0.0), dMaxTrackDOCA(400.0), dMinDeltaPhi(-30.0), dMaxDeltaPhi(30.0),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinDeltaT(-10.0), dMaxDeltaT(10.0)
		{
			dTargetCenter.SetXYZ(0.0, 0.0, -9.9E9);
		}

		DHistogramAction_Neutrals(void) :
		DAnalysisAction(NULL, "Hist_Neutrals", false, ""),
		dNumTrackDOCABins(400), dNumDeltaPhiBins(600), dNumShowerEnergyBins(800), dNumDeltaTBins(400), dNum2DShowerEnergyBins(400),
		dNum2DDeltaTBins(400), dNum2DBCALZBins(450), dMinTrackDOCA(0.0), dMaxTrackDOCA(400.0), dMinDeltaPhi(-30.0), dMaxDeltaPhi(30.0),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinDeltaT(-10.0), dMaxDeltaT(10.0)
		{
			dTargetCenter.SetXYZ(0.0, 0.0, -9.9E9);
		}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent);

		unsigned int dNumTrackDOCABins, dNumDeltaPhiBins, dNumShowerEnergyBins, dNumDeltaTBins, dNum2DShowerEnergyBins, dNum2DDeltaTBins, dNum2DBCALZBins;
		double dMinTrackDOCA, dMaxTrackDOCA, dMinDeltaPhi, dMaxDeltaPhi, dMinShowerEnergy, dMaxShowerEnergy, dMaxBCALP, dMinDeltaT, dMaxDeltaT;

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		DVector3 dTargetCenter;

		TH1I* dHist_BCALTrackDOCA = nullptr;
		TH1I* dHist_BCALTrackDeltaPhi = nullptr;
		TH1I* dHist_BCALTrackDeltaZ = nullptr;
		TH1I* dHist_BCALNeutralShowerEnergy = nullptr;
		TH1I* dHist_BCALNeutralShowerDeltaT = nullptr;
		TH2I* dHist_BCALNeutralShowerDeltaTVsE = nullptr;
		TH2I* dHist_BCALNeutralShowerDeltaTVsZ = nullptr;

		TH1I* dHist_FCALTrackDOCA = nullptr;
		TH1I* dHist_FCALNeutralShowerEnergy = nullptr;
		TH1I* dHist_FCALNeutralShowerDeltaT = nullptr;
		TH2I* dHist_FCALNeutralShowerDeltaTVsE = nullptr;

		TH1I* dHist_CCALNeutralShowerEnergy = nullptr;
		TH1I* dHist_CCALNeutralShowerDeltaT = nullptr;
		TH2I* dHist_CCALNeutralShowerDeltaTVsE = nullptr;
};

class DHistogramAction_DetectorMatchParams : public DAnalysisAction
{
	public:

		//user can call any of these three constructors
		DHistogramAction_DetectorMatchParams(const DReaction* locReaction, string locActionUniqueString = "") :
		DAnalysisAction(locReaction, "Hist_DetectorMatchParams", false, locActionUniqueString),
		dNumShowerEnergyBins(800), dNumShowerDepthBins(400), dNum2DPBins(250), dNum2DThetaBins(280), dNum2DHitEnergyBins(250),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinShowerDepth(0.0), dMaxShowerDepth(20.0), dMinP(0.0), dMaxP(10.0), 
		dMinTheta(0.0), dMaxTheta(140.0), dMinHitEnergy(0.0), dMaxHitEnergy(50.0), dNum2DPhiBins(360), dMinPhi(-180.0), dMaxPhi(180.0),
		dTrackSelectionTag("NotATag")
		{
			dTrackingPIDs.push_back(PiPlus);  dTrackingPIDs.push_back(KPlus);  dTrackingPIDs.push_back(Proton);
			dTrackingPIDs.push_back(PiMinus);  dTrackingPIDs.push_back(KMinus);
			dTargetCenterZ = -9.9E9;
		}

		DHistogramAction_DetectorMatchParams(string locActionUniqueString) :
		DAnalysisAction(NULL, "Hist_DetectorMatchParams", false, locActionUniqueString),
		dNumShowerEnergyBins(800), dNumShowerDepthBins(400), dNum2DPBins(250), dNum2DThetaBins(280), dNum2DHitEnergyBins(250),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinShowerDepth(0.0), dMaxShowerDepth(20.0), dMinP(0.0), dMaxP(10.0), 
		dMinTheta(0.0), dMaxTheta(140.0), dMinHitEnergy(0.0), dMaxHitEnergy(50.0), dNum2DPhiBins(360), dMinPhi(-180.0), dMaxPhi(180.0),
		dTrackSelectionTag("NotATag")
		{
			dTrackingPIDs.push_back(PiPlus);  dTrackingPIDs.push_back(KPlus);  dTrackingPIDs.push_back(Proton);
			dTrackingPIDs.push_back(PiMinus);  dTrackingPIDs.push_back(KMinus);
			dTargetCenterZ = -9.9E9;
		}

		DHistogramAction_DetectorMatchParams(void) :
		DAnalysisAction(NULL, "Hist_DetectorMatchParams", false, ""),
		dNumShowerEnergyBins(800), dNumShowerDepthBins(400), dNum2DPBins(250), dNum2DThetaBins(280), dNum2DHitEnergyBins(250),
		dMinShowerEnergy(0.0), dMaxShowerEnergy(8.0), dMaxBCALP(1.5), dMinShowerDepth(0.0), dMaxShowerDepth(20.0), dMinP(0.0), dMaxP(10.0), 
		dMinTheta(0.0), dMaxTheta(140.0), dMinHitEnergy(0.0), dMaxHitEnergy(50.0), dNum2DPhiBins(360), dMinPhi(-180.0), dMaxPhi(180.0),
		dTrackSelectionTag("NotATag")
		{
			dTrackingPIDs.push_back(PiPlus);  dTrackingPIDs.push_back(KPlus);  dTrackingPIDs.push_back(Proton);
			dTrackingPIDs.push_back(PiMinus);  dTrackingPIDs.push_back(KMinus);
			dTargetCenterZ = -9.9E9;
		}

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent);

		unsigned int dNumShowerEnergyBins, dNumShowerDepthBins, dNum2DPBins, dNum2DThetaBins, dNum2DHitEnergyBins;
		double dMinShowerEnergy, dMaxShowerEnergy, dMaxBCALP, dMinShowerDepth, dMaxShowerDepth, dMinP, dMaxP;
		double dMinTheta, dMaxTheta, dMinHitEnergy, dMaxHitEnergy, dNum2DPhiBins, dMinPhi, dMaxPhi;
		string dTrackSelectionTag; //In Initialize, will default to "PreSelect" unless otherwise specified

		vector<Particle_t> dTrackingPIDs;

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);
		void Fill_Hists(const std::shared_ptr<const JEvent>& locEvent, bool locUseTruePIDFlag);

		double dTargetCenterZ;

		//Delta-T: shower/hit_t - tflight - RF_t (if present)
		//bool in pair: true/false for true/best-reconstructed PID
		//int in pair: -2 for q-, -1 for q+, 0+ for Particle_t

		map<pair<int, bool>, TH1I*> dHistMap_BCALShowerEnergy;
		map<pair<int, bool>, TH1I*> dHistMap_BCALShowerTrackDepth;
		map<pair<int, bool>, TH2I*> dHistMap_BCALShowerTrackDepthVsP;

		map<pair<int, bool>, TH1I*> dHistMap_FCALShowerEnergy;
		map<pair<int, bool>, TH1I*> dHistMap_FCALShowerTrackDepth;
		map<pair<int, bool>, TH2I*> dHistMap_FCALShowerTrackDepthVsP;

		map<pair<int, bool>, TH1I*> dHistMap_CCALShowerEnergy;

		map<pair<int, bool>, TH2I*> dHistMap_SCEnergyVsTheta;
		map<pair<int, bool>, TH2I*> dHistMap_SCPhiVsTheta;
};

class DHistogramAction_EventVertex : public DAnalysisAction
{
	public:
		DHistogramAction_EventVertex(const DReaction* locReaction, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Hist_EventVertex", false, locActionUniqueString), 
		dNumConfidenceLevelBins(400), dNumPullBins(200), dNumVertexZBins(750), dNumTBins(400), dNumRFTBins(300), dNumVertexXYBins(400),
		dMinVertexZ(-50.0), dMaxVertexZ(200.0), dMinT(-20.0), dMaxT(20.0), dMinVertexXY(-10.0), dMaxVertexXY(10.0), 
		dMinPull(-4.0), dMaxPull(4.0), dPullHistConfidenceLevelCut(0.05), dTrackSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(Proton);  dFinalStatePIDs.push_back(PiMinus);
		}

		DHistogramAction_EventVertex(string locActionUniqueString) : 
		DAnalysisAction(NULL, "Hist_EventVertex", false, locActionUniqueString), 
		dNumConfidenceLevelBins(400), dNumPullBins(200), dNumVertexZBins(750), dNumTBins(400), dNumRFTBins(300), dNumVertexXYBins(400),
		dMinVertexZ(-50.0), dMaxVertexZ(200.0), dMinT(-20.0), dMaxT(20.0), dMinVertexXY(-10.0), dMaxVertexXY(10.0), 
		dMinPull(-4.0), dMaxPull(4.0), dPullHistConfidenceLevelCut(0.05), dTrackSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(Proton);  dFinalStatePIDs.push_back(PiMinus);
		}

		DHistogramAction_EventVertex(void) : 
		DAnalysisAction(NULL, "Hist_EventVertex", false, ""), 
		dNumConfidenceLevelBins(400), dNumPullBins(200), dNumVertexZBins(750), dNumTBins(400), dNumRFTBins(300), dNumVertexXYBins(400),
		dMinVertexZ(-50.0), dMaxVertexZ(200.0), dMinT(-20.0), dMaxT(20.0), dMinVertexXY(-10.0), dMaxVertexXY(10.0), 
		dMinPull(-4.0), dMaxPull(4.0), dPullHistConfidenceLevelCut(0.05), dTrackSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(Proton);  dFinalStatePIDs.push_back(PiMinus);
		}

		unsigned int dNumConfidenceLevelBins, dNumPullBins, dNumVertexZBins, dNumTBins, dNumRFTBins, dNumVertexXYBins;
		double dMinVertexZ, dMaxVertexZ, dMinT, dMaxT, dMinVertexXY, dMaxVertexXY, dMinPull, dMaxPull;

		double dPullHistConfidenceLevelCut;
		string dTrackSelectionTag; //In Initialize, will default to "PreSelect" unless otherwise specified

		deque<Particle_t> dFinalStatePIDs;

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		TH1I* dRFTrackDeltaT = nullptr;
		TH1I* dEventVertexZ_AllEvents = nullptr;
		TH2I* dEventVertexYVsX_AllEvents = nullptr;
		TH1I* dEventVertexT_AllEvents = nullptr;

		TH1I* dEventVertexZ_2OrMoreGoodTracks = nullptr;
		TH2I* dEventVertexYVsX_2OrMoreGoodTracks = nullptr;
		TH1I* dEventVertexT_2OrMoreGoodTracks = nullptr;

		TH1I* dHist_KinFitConfidenceLevel = nullptr;
		map<Particle_t, map<DKinFitPullType, TH1I*> > dHistMap_KinFitPulls;
};

class DHistogramAction_DetectedParticleKinematics : public DAnalysisAction
{
	public:
		DHistogramAction_DetectedParticleKinematics(const DReaction* locReaction, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Hist_DetectedParticleKinematics", false, locActionUniqueString), 
		dMinPIDFOM(5.73303E-7), dNumPBins(600), dNumThetaBins(560), dNumPhiBins(360), dNumVertexZBins(750), dNumTBins(400),
		dNumVertexXYBins(400), dNumBetaBins(400), dNum2DDeltaBetaBins(400), dNum2DPBins(300), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumBeamEBins(650),
		dMinT(-20.0), dMaxT(20.0), dMinP(0.0), dMaxP(12.0), dMaxBeamE(13.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(-50.0), dMaxVertexZ(200.0),
		dMinVertexXY(-10.0), dMaxVertexXY(10.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);  dFinalStatePIDs.push_back(Neutron);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);
		}

		DHistogramAction_DetectedParticleKinematics(string locActionUniqueString) : 
		DAnalysisAction(NULL, "Hist_DetectedParticleKinematics", false, locActionUniqueString), 
		dMinPIDFOM(5.73303E-7), dNumPBins(600), dNumThetaBins(560), dNumPhiBins(360), dNumVertexZBins(750), dNumTBins(400),
		dNumVertexXYBins(400), dNumBetaBins(400), dNum2DDeltaBetaBins(400), dNum2DPBins(300), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumBeamEBins(650),
		dMinT(-20.0), dMaxT(20.0), dMinP(0.0), dMaxP(12.0), dMaxBeamE(13.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(-50.0), dMaxVertexZ(200.0),
		dMinVertexXY(-10.0), dMaxVertexXY(10.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);  dFinalStatePIDs.push_back(Neutron);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);
		}

		DHistogramAction_DetectedParticleKinematics(void) : 
		DAnalysisAction(NULL, "Hist_DetectedParticleKinematics", false, ""), 
		dMinPIDFOM(5.73303E-7), dNumPBins(600), dNumThetaBins(560), dNumPhiBins(360), dNumVertexZBins(750), dNumTBins(400),
		dNumVertexXYBins(400), dNumBetaBins(400), dNum2DDeltaBetaBins(400), dNum2DPBins(300), dNum2DThetaBins(140), dNum2DPhiBins(180), dNumBeamEBins(650),
		dMinT(-20.0), dMaxT(20.0), dMinP(0.0), dMaxP(12.0), dMaxBeamE(13.0), dMinTheta(0.0), dMaxTheta(140.0), dMinPhi(-180.0), dMaxPhi(180.0), dMinVertexZ(-50.0), dMaxVertexZ(200.0),
		dMinVertexXY(-10.0), dMaxVertexXY(10.0), dMinBeta(-0.2), dMaxBeta(1.2), dMinDeltaBeta(-1.0), dMaxDeltaBeta(1.0),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);  dFinalStatePIDs.push_back(Neutron);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);
		}

		double dMinPIDFOM;
		unsigned int dNumPBins, dNumThetaBins, dNumPhiBins, dNumVertexZBins, dNumTBins, dNumVertexXYBins, dNumBetaBins, dNum2DDeltaBetaBins, dNum2DPBins;
		unsigned int dNum2DThetaBins, dNum2DPhiBins, dNumBeamEBins;
		double dMinT, dMaxT, dMinP, dMaxP, dMaxBeamE, dMinTheta, dMaxTheta, dMinPhi, dMaxPhi, dMinVertexZ, dMaxVertexZ, dMinVertexXY, dMaxVertexXY, dMinBeta;
		double dMaxBeta, dMinDeltaBeta, dMaxDeltaBeta;
		string dTrackSelectionTag, dShowerSelectionTag; //In Initialize, will default to "PreSelect" unless otherwise specified

		deque<Particle_t> dFinalStatePIDs;

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		TH1I* dBeamParticle_P = nullptr;

		//PID
		map<int, TH2I*> dHistMap_QBetaVsP; //int is charge: -1, 1

		map<Particle_t, TH2I*> dHistMap_PVsTheta;
		map<Particle_t, TH2I*> dHistMap_PhiVsTheta;
		map<Particle_t, TH2I*> dHistMap_BetaVsP;
		map<Particle_t, TH2I*> dHistMap_DeltaBetaVsP;
		map<Particle_t, TH1I*> dHistMap_P;
		map<Particle_t, TH1I*> dHistMap_Theta;
		map<Particle_t, TH1I*> dHistMap_Phi;
		map<Particle_t, TH1I*> dHistMap_VertexZ;
		map<Particle_t, TH2I*> dHistMap_VertexYVsX;
		map<Particle_t, TH1I*> dHistMap_VertexT;
		map<Particle_t, TH1I*> dHistMap_ShowerE;
};

class DHistogramAction_TrackShowerErrors : public DAnalysisAction
{
	public:
		DHistogramAction_TrackShowerErrors(const DReaction* locReaction, string locActionUniqueString = "") :
		DAnalysisAction(locReaction, "Hist_TrackShowerErrors", false, locActionUniqueString),
		dMinPIDFOM(5.73303E-7), dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180),
		dNum2DXYErrorBins(300), dNum2DZErrorBins(400), dNum2DShowerZErrorBins(300), dNum2DPxyErrorBins(200), dNum2DPzErrorBins(400), dNum2DEErrorBins(200), dNum2DTErrorBins(200),
		dMinP(0.0), dMaxP(10.0), dMaxPBCAL(2.0), dMinTheta(0.0), dMinThetaBCAL(10.0), dMaxTheta(140.0), dMaxThetaFCAL(15.0), dMaxThetaCCAL(3.0), dMinPhi(-180.0), dMaxPhi(180.0),
		dMaxPxyError(0.1), dMaxPzError(0.5), dMaxXYError(1.5), dMaxZError(10.0), dMaxShowerZError(15.0), dMaxEError(0.5), dMaxTError(2.0),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(Proton);
		}

		DHistogramAction_TrackShowerErrors(string locActionUniqueString) :
		DAnalysisAction(NULL, "Hist_TrackShowerErrors", false, locActionUniqueString),
		dMinPIDFOM(5.73303E-7), dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180),
		dNum2DXYErrorBins(300), dNum2DZErrorBins(400), dNum2DShowerZErrorBins(300), dNum2DPxyErrorBins(200), dNum2DPzErrorBins(400), dNum2DEErrorBins(200), dNum2DTErrorBins(200),
		dMinP(0.0), dMaxP(10.0), dMaxPBCAL(2.0), dMinTheta(0.0), dMinThetaBCAL(10.0), dMaxTheta(140.0), dMaxThetaFCAL(15.0), dMaxThetaCCAL(3.0), dMinPhi(-180.0), dMaxPhi(180.0),
		dMaxPxyError(0.1), dMaxPzError(0.5), dMaxXYError(1.5), dMaxZError(10.0), dMaxShowerZError(15.0), dMaxEError(0.5), dMaxTError(2.0),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(Proton);
		}

		DHistogramAction_TrackShowerErrors(void) :
		DAnalysisAction(NULL, "Hist_TrackShowerErrors", false, ""),
		dMinPIDFOM(5.73303E-7), dNum2DPBins(250), dNum2DThetaBins(140), dNum2DPhiBins(180),
		dNum2DXYErrorBins(300), dNum2DZErrorBins(400), dNum2DShowerZErrorBins(300), dNum2DPxyErrorBins(200), dNum2DPzErrorBins(400), dNum2DEErrorBins(200), dNum2DTErrorBins(200),
		dMinP(0.0), dMaxP(10.0), dMaxPBCAL(2.0), dMinTheta(0.0), dMinThetaBCAL(10.0), dMaxTheta(140.0), dMaxThetaFCAL(15.0), dMaxThetaCCAL(3.0), dMinPhi(-180.0), dMaxPhi(180.0),
		dMaxPxyError(0.1), dMaxPzError(0.5), dMaxXYError(1.5), dMaxZError(10.0), dMaxShowerZError(15.0), dMaxEError(0.5), dMaxTError(2.0),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(Proton);
		}

		double dMinPIDFOM;
		unsigned int dNum2DPBins, dNum2DThetaBins, dNum2DPhiBins, dNum2DXYErrorBins, dNum2DZErrorBins, dNum2DShowerZErrorBins;
		unsigned int dNum2DPxyErrorBins, dNum2DPzErrorBins, dNum2DEErrorBins, dNum2DTErrorBins;
		double dMinP, dMaxP, dMaxPBCAL, dMinTheta, dMinThetaBCAL, dMaxTheta, dMaxThetaFCAL, dMaxThetaCCAL, dMinPhi, dMaxPhi;
		double dMaxPxyError, dMaxPzError, dMaxXYError, dMaxZError, dMaxShowerZError, dMaxEError, dMaxTError;
		string dTrackSelectionTag, dShowerSelectionTag; //In Initialize, will default to "PreSelect" unless otherwise specified

		deque<Particle_t> dFinalStatePIDs;

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo);

		//tracks
		map<Particle_t, TH2I*> dHistMap_TrackPxErrorVsP;
		map<Particle_t, TH2I*> dHistMap_TrackPyErrorVsP;
		map<Particle_t, TH2I*> dHistMap_TrackPzErrorVsP;
		map<Particle_t, TH2I*> dHistMap_TrackXErrorVsP;
		map<Particle_t, TH2I*> dHistMap_TrackYErrorVsP;
		map<Particle_t, TH2I*> dHistMap_TrackZErrorVsP;

		map<Particle_t, TH2I*> dHistMap_TrackPxErrorVsTheta;
		map<Particle_t, TH2I*> dHistMap_TrackPyErrorVsTheta;
		map<Particle_t, TH2I*> dHistMap_TrackPzErrorVsTheta;
		map<Particle_t, TH2I*> dHistMap_TrackXErrorVsTheta;
		map<Particle_t, TH2I*> dHistMap_TrackYErrorVsTheta;
		map<Particle_t, TH2I*> dHistMap_TrackZErrorVsTheta;

		map<Particle_t, TH2I*> dHistMap_TrackPxErrorVsPhi;
		map<Particle_t, TH2I*> dHistMap_TrackPyErrorVsPhi;
		map<Particle_t, TH2I*> dHistMap_TrackPzErrorVsPhi;
		map<Particle_t, TH2I*> dHistMap_TrackXErrorVsPhi;
		map<Particle_t, TH2I*> dHistMap_TrackYErrorVsPhi;
		map<Particle_t, TH2I*> dHistMap_TrackZErrorVsPhi;

		//shower //bool: true/false for BCAL/FCAL
		map<bool, TH2I*> dHistMap_ShowerEErrorVsP;
		map<bool, TH2I*> dHistMap_ShowerXErrorVsP;
		map<bool, TH2I*> dHistMap_ShowerYErrorVsP;
		map<bool, TH2I*> dHistMap_ShowerZErrorVsP;
		map<bool, TH2I*> dHistMap_ShowerTErrorVsP;

		map<bool, TH2I*> dHistMap_ShowerEErrorVsTheta;
		map<bool, TH2I*> dHistMap_ShowerXErrorVsTheta;
		map<bool, TH2I*> dHistMap_ShowerYErrorVsTheta;
		map<bool, TH2I*> dHistMap_ShowerZErrorVsTheta;
		map<bool, TH2I*> dHistMap_ShowerTErrorVsTheta;

		map<bool, TH2I*> dHistMap_ShowerEErrorVsPhi;
		map<bool, TH2I*> dHistMap_ShowerXErrorVsPhi;
		map<bool, TH2I*> dHistMap_ShowerYErrorVsPhi;
		map<bool, TH2I*> dHistMap_ShowerZErrorVsPhi;
		map<bool, TH2I*> dHistMap_ShowerTErrorVsPhi;
};

class DHistogramAction_NumReconstructedObjects : public DAnalysisAction
{
	public:
		DHistogramAction_NumReconstructedObjects(const DReaction* locReaction, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Hist_NumReconstructedObjects", false, locActionUniqueString),
		dMaxNumObjects(40), dMaxNumMatchObjects(20), dMaxNumCDCHits(400), dMaxNumFDCHits(1000), dMaxNumFMWPCHits(400), dMaxNumTOFCalorimeterHits(400), dMaxNumBeamPhotons(100){}

		DHistogramAction_NumReconstructedObjects(string locActionUniqueString) : 
		DAnalysisAction(NULL, "Hist_NumReconstructedObjects", false, ""),
		dMaxNumObjects(40), dMaxNumMatchObjects(20), dMaxNumCDCHits(400), dMaxNumFDCHits(1000), dMaxNumFMWPCHits(400), dMaxNumTOFCalorimeterHits(400), dMaxNumBeamPhotons(100){}

		DHistogramAction_NumReconstructedObjects(void) : 
		DAnalysisAction(NULL, "Hist_NumReconstructedObjects", false, ""),
		dMaxNumObjects(40), dMaxNumMatchObjects(20), dMaxNumCDCHits(400), dMaxNumFDCHits(1000), dMaxNumFMWPCHits(400), dMaxNumTOFCalorimeterHits(400), dMaxNumBeamPhotons(100){}

		unsigned int dMaxNumObjects;
		unsigned int dMaxNumMatchObjects;
		unsigned int dMaxNumCDCHits;
		unsigned int dMaxNumFDCHits;
		unsigned int dMaxNumFMWPCHits;
		unsigned int dMaxNumTOFCalorimeterHits;
		unsigned int dMaxNumBeamPhotons;

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		TH2D* dHist_NumHighLevelObjects = nullptr;

		TH1D* dHist_NumChargedTracks = nullptr;
		TH1D* dHist_NumPosChargedTracks = nullptr;
		TH1D* dHist_NumNegChargedTracks = nullptr;

		TH1D* dHist_NumTimeBasedTracks = nullptr;
		TH1D* dHist_NumPosTimeBasedTracks = nullptr;
		TH1D* dHist_NumNegTimeBasedTracks = nullptr;
		TH1D* dHist_NumWireBasedTracks = nullptr;
		TH1D* dHist_NumPosWireBasedTracks = nullptr;
		TH1D* dHist_NumNegWireBasedTracks = nullptr;
		TH1D* dHist_NumTrackCandidates = nullptr;
		TH1D* dHist_NumPosTrackCandidates = nullptr;
		TH1D* dHist_NumNegTrackCandidates = nullptr;
		TH1D* dHist_NumPosTrackCandidates_CDC = nullptr;
		TH1D* dHist_NumNegTrackCandidates_CDC = nullptr;
		TH1D* dHist_NumPosTrackCandidates_FDC = nullptr;
		TH1D* dHist_NumNegTrackCandidates_FDC = nullptr;

		TH1D* dHist_NumBeamPhotons = nullptr;
		TH1D* dHist_NumFCALShowers = nullptr;
		TH1D* dHist_NumCCALShowers = nullptr;
		TH1D* dHist_NumBCALShowers = nullptr;
		TH1D* dHist_NumNeutralShowers = nullptr;
		TH1D* dHist_NumTOFPoints = nullptr;
		TH1D* dHist_NumSCHits = nullptr;
		TH1D* dHist_NumTAGMHits = nullptr;
		TH1D* dHist_NumTAGHHits = nullptr;

		TH1D* dHist_NumTrackBCALMatches = nullptr;
		TH1D* dHist_NumTrackFCALMatches = nullptr;
		TH1D* dHist_NumTrackTOFMatches = nullptr;
		TH1D* dHist_NumTrackSCMatches = nullptr;

		TH1I* dHist_NumCDCHits = nullptr;
		TH1I* dHist_NumFDCWireHits = nullptr;
		TH1I* dHist_NumFDCCathodeHits = nullptr;
		TH1I* dHist_NumFDCPseudoHits = nullptr;
		TH1I* dHist_NumFMWPCHits = nullptr;
		TH1I* dHist_NumTOFHits = nullptr;
		TH1I* dHist_NumBCALHits = nullptr;
		TH1I* dHist_NumFCALHits = nullptr;
		TH1I* dHist_NumCCALHits = nullptr;

		TH1I* dHist_NumRFSignals = nullptr; //all sources
};

class DHistogramAction_TrackMultiplicity : public DAnalysisAction
{
	public:
		DHistogramAction_TrackMultiplicity(const DReaction* locReaction, string locActionUniqueString = "") : 
		DAnalysisAction(locReaction, "Hist_TrackMultiplicity", false, locActionUniqueString),
		dMaxNumTracks(40), dMinTrackingFOM(0.0027), dMinPIDFOM(5.73303E-7), dRequireDetectorMatchFlag(true),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);
		}

		DHistogramAction_TrackMultiplicity(string locActionUniqueString) : 
		DAnalysisAction(NULL, "Hist_TrackMultiplicity", false, ""),
		dMaxNumTracks(40), dMinTrackingFOM(0.0027), dMinPIDFOM(5.73303E-7), dRequireDetectorMatchFlag(true),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);
		}

		DHistogramAction_TrackMultiplicity(void) : 
		DAnalysisAction(NULL, "Hist_TrackMultiplicity", false, ""),
		dMaxNumTracks(40), dMinTrackingFOM(0.0027), dMinPIDFOM(5.73303E-7), dRequireDetectorMatchFlag(true),
		dTrackSelectionTag("NotATag"), dShowerSelectionTag("NotATag")
		{
			dFinalStatePIDs.push_back(Gamma);
			dFinalStatePIDs.push_back(PiPlus);  dFinalStatePIDs.push_back(KPlus);  dFinalStatePIDs.push_back(Proton);
			dFinalStatePIDs.push_back(PiMinus);  dFinalStatePIDs.push_back(KMinus);
		}

		unsigned int dMaxNumTracks;
		double dMinTrackingFOM;
		double dMinPIDFOM;
		bool dRequireDetectorMatchFlag;
		string dTrackSelectionTag, dShowerSelectionTag; //In Initialize, will default to "PreSelect" unless otherwise specified

		deque<Particle_t> dFinalStatePIDs;

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		TH2D* dHist_NumReconstructedParticles = nullptr;
		TH2D* dHist_NumGoodReconstructedParticles = nullptr;
};

class DHistogramAction_TriggerStudies : public DAnalysisAction
{
	public:
		DHistogramAction_TriggerStudies(const DReaction* locReaction, string locActionUniqueString = "", double locKinFitCLCut=-1.) : 
		DAnalysisAction(locReaction, "Hist_TriggerStudies", false, locActionUniqueString),
		dBCALBins(240), dFCALBins(240), dMaxBCALEnergy(12.), dMaxFCALEnergy(12.), dKinFitCLCut(locKinFitCLCut) { }

		DHistogramAction_TriggerStudies(string locActionUniqueString) : 
		DAnalysisAction(NULL, "Hist_TriggerStudies", false, ""),
		dBCALBins(240), dFCALBins(240), dMaxBCALEnergy(12.), dMaxFCALEnergy(12.), dKinFitCLCut(-1.)  { }
			
		DHistogramAction_TriggerStudies(void) : 
		DAnalysisAction(NULL, "Hist_TriggerStudies", false, ""),
		dBCALBins(240), dFCALBins(240), dMaxBCALEnergy(12.), dMaxFCALEnergy(12.), dKinFitCLCut(-1.)  { }

		int dBCALBins, dFCALBins;
		double dMaxBCALEnergy;
		double dMaxFCALEnergy;
		
		double dKinFitCLCut;

		void Initialize(const std::shared_ptr<const JEvent>& locEvent);
		void Run_Update(const std::shared_ptr<const JEvent>& locEvent){}

	private:
		bool Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo = NULL);

		TH2D* dHist_Trigger_FCALBCAL_Energy = nullptr;
};

#endif // _DHistogramActions_Independent_

