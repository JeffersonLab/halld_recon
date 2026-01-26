// $Id$
//
//    File: DCustomAction_cpp_hists.cc
// Created: Thu May 12 12:29:15 EDT 2022
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#include "DCustomAction_cpp_hists.h"
#include <DANA/DEvent.h>


void DCustomAction_cpp_hists::Run_Update(const std::shared_ptr<const JEvent>& locEvent)
{
	JCalibration *jcalib = DEvent::GetJCalibration(locEvent);

	// Parameters for event selection to fill histograms
	endpoint_energy = 12.;
	map<string, double> photon_endpoint_energy;
	if(jcalib->Get("/PHOTON_BEAM/endpoint_energy", photon_endpoint_energy) == false) {
		endpoint_energy = photon_endpoint_energy["PHOTON_BEAM_ENDPOINT_ENERGY"];
	}
	else {
		jout<<"No /PHOTON_BEAM/endpoint_energy for this run number: using default of 12 GeV"<<endl;
	}
	endpoint_energy_bins = (int)(20*endpoint_energy);

	cohmin_energy = 0.;
	cohedge_energy = 12.;
	map<string, double> photon_beam_param;
	if(jcalib->Get("/PHOTON_BEAM/coherent_energy", photon_beam_param) == false) {
		cohmin_energy = photon_beam_param["cohmin_energy"];
		cohedge_energy = photon_beam_param["cohedge_energy"];
	}
	else {
		jout<<"No /PHOTON_BEAM/coherent_energy for this run number: using default range of 0-12 GeV"<<endl;
	}

	locEvent->GetSingle(dAnalysisUtilities);

}

void DCustomAction_cpp_hists::Initialize(const std::shared_ptr<const JEvent>& locEvent)
{
  
	Run_Update(locEvent);

	//Optional: Create histograms and/or modify member variables.
	//Create any histograms/trees/etc. within a ROOT lock. 
		//This is so that when running multithreaded, only one thread is writing to the ROOT file at a time. 
	//NEVER: Get anything from the JEventLoop while in a lock: May deadlock

	//CREATE THE HISTOGRAMS
	//Since we are creating histograms, the contents of gDirectory will be modified: must use JANA-wide ROOT lock
	DEvent::GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK!!
	{
		//Required: Create a folder in the ROOT output file that will contain all of the output ROOT objects (if any) for this action.
			//If another thread has already created the folder, it just changes to it. 
		CreateAndChangeTo_ActionDirectory();

		// Optional: Create a ROOT subfolder.
			//If another thread has already created the folder, it just changes to it. 
		// CreateAndChangeTo_Directory("MyDirName", "MyDirTitle");
			//make sub-directory content here
		// gDirectory->cd(".."); //return to the action directory

		//	(Optional) Example: Create a histogram. 
			// This function will return the histogram if already created by another thread. If not pre-existing, it will create and return it. 
			// Function arguments are identical to those used for the histogram constructors
		// dMyHist = GetOrCreate_Histogram<TH1I>("MyHistName", "MyHistTitle", 100, 0.0, 1.0);

		dEgamma = GetOrCreate_Histogram<TH1I>("Egamma", "TAGGER photon energy; E_{#gamma}", endpoint_energy_bins, 0., endpoint_energy);
		dcosTheta_rho = GetOrCreate_Histogram<TH1I>("cosTheta_rho","cos#theta",200,-1,1);	
		dphi_rho = GetOrCreate_Histogram<TH1I>("phi_rho","#phi",360,-180,180);	
		dPhi_rho = GetOrCreate_Histogram<TH1I>("Phi_rho","#Phi",360,-180,180);	
		dpsi_rho = GetOrCreate_Histogram<TH1I>("Psi_rho","#psi",360,-180,180);
		dpsi_vs_E_rho = GetOrCreate_Histogram<TH2I>("Psi_vs_E_rho","#psi",360,-180,180,endpoint_energy_bins, 0., endpoint_energy);
		dt_rho = GetOrCreate_Histogram<TH1I>("t_rho","-t; -t (GeV/c)^{2}",400,0.,.1);	
		dcosTheta_cpp = GetOrCreate_Histogram<TH1I>("cosTheta_cpp","cos#theta",200,-1,1);	
		dphi_cpp = GetOrCreate_Histogram<TH1I>("phi_cpp","#phi",360,-180,180);	
		dPhi_cpp = GetOrCreate_Histogram<TH1I>("Phi_cpp","#Phi",360,-180,180);	
		dpsi_cpp = GetOrCreate_Histogram<TH1I>("Psi_cpp","#psi",360,-180,180);	
		dt_cpp = GetOrCreate_Histogram<TH1I>("t_cpp","-t; -t (GeV/c)^{2}",400,0.,.1);

		//Return to the base directory
		ChangeTo_BaseDirectory();
	}
	DEvent::GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK!!
	
}

bool DCustomAction_cpp_hists::Perform_Action(const std::shared_ptr<const JEvent>& locEvent, const DParticleCombo* locParticleCombo)
{
	//Write custom code to perform an action on the INPUT DParticleCombo (DParticleCombo)
	//NEVER: Grab DParticleCombo or DAnalysisResults objects (of any tag!) from the JEventLoop within this function
	//NEVER: Grab objects that are created post-kinfit (e.g. DKinFitResults, etc.) from the JEventLoop if Get_UseKinFitResultsFlag() == false: CAN CAUSE INFINITE DEPENDENCY LOOP
	//NEVER: Get anything from the JEventLoop while in a lock: May deadlock



	// Optional: Useful utility functions.
	// const DAnalysisUtilities* locAnalysisUtilities;
	// locEventLoop->GetSingle(locAnalysisUtilities);

        //Optional: check whether the user wanted to use the kinematic fit results when performing this action
	bool locUseKinFitResultsFlag = Get_UseKinFitResultsFlag();

  	// should only have one reaction step
	const DParticleComboStep* locParticleComboStep = locParticleCombo->Get_ParticleComboStep(0);

	// get beam photon energy and final state particles
	auto locBeamPhoton = locUseKinFitResultsFlag ? locParticleComboStep->Get_InitialParticle() : locParticleComboStep->Get_InitialParticle_Measured();
	auto locParticles = locUseKinFitResultsFlag ? locParticleComboStep->Get_FinalParticles() : locParticleComboStep->Get_FinalParticles_Measured();
        double locBeamPhotonEnergy = locBeamPhoton->energy();
	
	// calculate 2pi P4
	DLorentzVector locP4_2pi;
	for(size_t loc_i = 0; loc_i < 3; ++loc_i) {
		if(locParticles[loc_i] == NULL) continue; // missing particle
                if(locParticles[loc_i]->PID() == PiPlus || locParticles[loc_i]->PID() == PiMinus)
                        locP4_2pi += locParticles[loc_i]->lorentzMomentum();
        }
	
	DLorentzVector locBeamP4 = locBeamPhoton->lorentzMomentum();
	DLorentzVector locPiP4 = locParticles[0]->lorentzMomentum();
	
	// calculate missing P4
	DLorentzVector locMissingPb208P4 = dAnalysisUtilities->Calc_MissingP4(Get_Reaction(), locParticleCombo,Get_UseKinFitResultsFlag());
	
	// production kinematics
	DLorentzVector locDelta = (locBeamP4 - locP4_2pi);
	double t = fabs(locDelta.M2());

	// boost into 2pi frame
	DLorentzVector beam_res = locBeamP4;
	beam_res.Boost(-locP4_2pi.BoostVector());
	DLorentzVector recoil_res = locMissingPb208P4;
	recoil_res.Boost(-locP4_2pi.BoostVector());
	DLorentzVector pi_res = locPiP4;
	pi_res.Boost(-locP4_2pi.BoostVector());

	// choose helicity frame: z-axis opposite recoil target in rho rest frame. Note that for Primakoff recoil is missing P4, including target.
	DVector3 y = (locBeamP4.Vect().Unit().Cross(-locMissingPb208P4.Vect().Unit())).Unit();
	
	// choose helicity frame: z-axis opposite recoil nucleus in rho rest frame
	DVector3 z = -1. * recoil_res.Vect().Unit();
	DVector3 x = y.Cross(z).Unit();
	DVector3 angles( (pi_res.Vect()).Dot(x),
			 (pi_res.Vect()).Dot(y),
			 (pi_res.Vect()).Dot(z) );
	
	double cosTheta = angles.CosTheta();
	double phi = angles.Phi();

	double phipol = 0;                           // *** Note assumes horizontal polarization plane.
	DVector3 eps(cos(phipol), sin(phipol), 0.0); // beam polarization vector in lab
	double Phi = atan2(y.Dot(eps), locBeamP4.Vect().Unit().Dot(eps.Cross(y)));
	
	double psi = phi - Phi;
	if(psi < -3.14159) psi += 2*3.14159;
	if(psi > 3.14159) psi -= 2*3.14159;
	

	//Optional: FILL HISTOGRAMS
	//Since we are filling histograms local to this action, it will not interfere with other ROOT operations: can use action-wide ROOT lock
	//Note, the mutex is unique to this DReaction + action_string combo: actions of same class with different hists will have a different mutex
	Lock_Action(); //ACQUIRE ROOT LOCK!!
	{
	  // Fill any histograms here
	  dEgamma->Fill(locBeamPhotonEnergy);
	  if (locP4_2pi.M() > 0.6 && locP4_2pi.M() < 0.88){ // rho(770)
	    dt_rho->Fill(t);
	    dpsi_vs_E_rho->Fill(psi*TMath::RadToDeg(),locBeamPhotonEnergy);
	    if(locBeamPhotonEnergy > cohmin_energy && locBeamPhotonEnergy < cohedge_energy){
	      dphi_rho->Fill(phi*TMath::RadToDeg());
	      dPhi_rho->Fill(Phi*TMath::RadToDeg());
	      dpsi_rho->Fill(psi*TMath::RadToDeg());
	      dcosTheta_rho->Fill(cosTheta);
	    }
	  }
	  else if (locP4_2pi.M() > 0.2 && locP4_2pi.M() < 0.5){ // threshold region
	    dt_cpp->Fill(t);
	    if(locBeamPhotonEnergy > cohmin_energy && locBeamPhotonEnergy < cohedge_energy){
	      dphi_cpp->Fill(phi*TMath::RadToDeg());
	      dPhi_cpp->Fill(Phi*TMath::RadToDeg());
	      dpsi_cpp->Fill(psi*TMath::RadToDeg());
	      dcosTheta_cpp->Fill(cosTheta);
	    }
	  }
	}
	Unlock_Action(); //RELEASE ROOT LOCK!!
	

	return true; //return false if you want to use this action to apply a cut (and it fails the cut!)
}
