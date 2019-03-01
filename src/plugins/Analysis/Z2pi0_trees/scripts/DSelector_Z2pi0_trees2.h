#ifndef DSelector_Z2pi0_trees2_h
#define DSelector_Z2pi0_trees2_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"
#include "TLorentzVector.h"
#include "TLorentzRotation.h"


class DSelector_Z2pi0_trees2 : public DSelector
{
	public:

		DSelector_Z2pi0_trees2(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_Z2pi0_trees2(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

		// ANALYZE CUT ACTIONS
		// // Automatically makes mass histograms where one cut is missing
		// DHistogramAction_AnalyzeCutActions* dAnalyzeCutActions;

		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS

		//Step 0
		DParticleComboStep* dStep0Wrapper;
		DBeamParticle* dComboBeamWrapper;
		DKinematicData* dMissingPb208Wrapper;

		//Step 1
		DParticleComboStep* dStep1Wrapper;
		DKinematicData* dDecayingPi01Wrapper;
		DNeutralParticleHypothesis* dPhoton1Wrapper;
		DNeutralParticleHypothesis* dPhoton2Wrapper;

		//Step 2
		DParticleComboStep* dStep2Wrapper;
		DKinematicData* dDecayingPi02Wrapper;
		DNeutralParticleHypothesis* dPhoton3Wrapper;
		DNeutralParticleHypothesis* dPhoton4Wrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		TH1I* dHist_MissingMassSquared;
		TH1I* dHist_BeamEnergy;
		TH1I* dHist_pMomentumMeasured;
		TH1I* dHist_pi01MomentumMeasured;
		TH1I* dHist_pi02MomentumMeasured;
		TH2I* dHist_Proton_dEdx_P;
		TH1I* dHist_KinFitChiSq;
		TH1I* dHist_KinFitCL;
		TH1I* dHist_M2pigen;
		TH1I* dHist_M2pikin;
		TH1I* dHist_M2pidiff;
		TH1I* dHist_tgen;
		TH1I* dHist_tkin;
		TH1I* dHist_tdiff;
		TH1I* dHist_thetapipigen;
		TH1I* dHist_thetapipikin;
		TH1I* dHist_thetapipidiff;

		TH2I* dHist_tkin_tgen;
		TH2I* dHist_thetapipikin_thetapipigen;
		TH1I* dHist_CosTheta;
		TH1I* dHist_CosThetadiff;
		TH2I* dHist_CosTheta_psi;
		TH2I* dHist_CosThetakin_CosThetagen;
		TH2I* dHist_phimeas_phigen;
		TH2I* dHist_phikin_phigen;
		TH2I* dHist_Phimeas_Phigen;
		TH2I* dHist_Phikin_Phigen;
		TH2I* dHist_phikin_Phikin;
		TH2I* dHist_phigen_Phigen;
		TH2I* dHist_Delta_phi;
		TH2I* dHist_Delta_Phi;
		TH2I* dHist_Delta_phimeas;
		TH2I* dHist_Delta_Phimeas;
		TH2I* dHist_thetap_thetam;
		TH2I* dHist_thetap_thetam_Mcut;

		TH1I* dHist_Phigen;
		TH1I* dHist_phigen;
		TH1I* dHist_Phikin;
		TH1I* dHist_phikin;
		TH1I* dHist_Phimeas;
		TH1I* dHist_phimeas;
		TH1I* dHist_psigen;
		TH1I* dHist_psidiff;
		TH1I* dHist_Phidiff;
		TH1I* dHist_phidiff;
		TH1I* dHist_psikin;
		TH1I* dHist_pDeltap;
		TH1I* dHist_pi01Deltap;
		TH1I* dHist_pi02Deltap;
		TH1I* dHist_pDeltap_Measured;
		TH1I* dHist_pi01Deltap_Measured;
		TH1I* dHist_pi02Deltap_Measured;
		TH1I* dHist_TaggerAccidentals;

		// Cut parameters
		TF1* fMinProton_dEdx;
		TF1* fMaxPion_dEdx;
		Double_t dMinKinFitCL;
		Double_t dMaxKinFitChiSq;
		Double_t dMinBeamEnergy;
		Double_t dMaxBeamEnergy;
		Double_t dMin2piMass;
		Double_t dMax2piMass;
		Double_t dMinMissingMassSquared;
		Double_t dMaxMissingMassSquared;
		
		Double_t AccWeight;    // used to store weights due to accidental tagger subtraction

	ClassDef(DSelector_Z2pi0_trees2, 0);
};

void DSelector_Z2pi0_trees2::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dMissingPb208Wrapper = dStep0Wrapper->Get_FinalParticle(2);

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dDecayingPi01Wrapper = dStep1Wrapper->Get_InitialParticle();
	dPhoton1Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));
	dPhoton2Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(1));

	//Step 2
	dStep2Wrapper = dComboWrapper->Get_ParticleComboStep(2);
	dDecayingPi02Wrapper = dStep2Wrapper->Get_InitialParticle();
	dPhoton3Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep2Wrapper->Get_FinalParticle(0));
	dPhoton4Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep2Wrapper->Get_FinalParticle(1));
}

#endif // DSelector_Z2pi0_trees2_h
