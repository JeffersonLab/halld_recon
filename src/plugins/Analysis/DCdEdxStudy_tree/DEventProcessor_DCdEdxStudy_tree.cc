#include "DEventProcessor_DCdEdxStudy_tree.h"

// The executable should define the ROOTfile global variable. It will
// be automatically linked when dlopen is called.
extern TFile *ROOTfile;

// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_DCdEdxStudy_tree());
}
} // "C"


//------------------
// Init
//------------------
void DEventProcessor_DCdEdxStudy_tree::Init()
{

	dDCdEdxInformation = new DCdEdxInformation();
	dPluginTree_DCdEdxInformation = new TTree("dPluginTree_DCdEdxInformation", "DC dEdx Information");
	dPluginTree_DCdEdxInformation->Branch("dPluginBranch_DCdEdxInformation", "DCdEdxInformation", &dDCdEdxInformation);
}

//------------------
// BeginRun
//------------------
void DEventProcessor_DCdEdxStudy_tree::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventProcessor_DCdEdxStudy_tree::Process(const std::shared_ptr<const JEvent>& event)
{
  // Get the particle ID algorithms
	vector<const DParticleID *> locPIDAlgorithms;
	event->Get(locPIDAlgorithms);
	if(locPIDAlgorithms.size() < 1){
		_DBG_<<"Unable to get a DParticleID object! NO PID will be done!"<<endl;
		return RESOURCE_UNAVAILABLE;
	}
	// Drop the const qualifier from the DParticleID pointer (I'm surely going to hell for this!)
	locPIDAlgorithm = const_cast<DParticleID*>(locPIDAlgorithms[0]);

	// Warn user if something happened that caused us NOT to get a locPIDAlgorithm object pointer
	if(!locPIDAlgorithm){
		_DBG_<<"Unable to get a DParticleID object! NO PID will be done!"<<endl;
		return RESOURCE_UNAVAILABLE;
	}

	vector<const DMCThrown*> locDMCThrownVector;
	const DMCThrown *locDMCThrown;
	event->Get(locDMCThrownVector);

	vector<const DChargedTrackHypothesis*> locChargedTrackHypothesisVector;
	const DChargedTrackHypothesis *locChargedTrackHypothesis;
	event->Get(locChargedTrackHypothesisVector);

	const DTrackTimeBased *locTrackTimeBased;

	Particle_t locThrownPID, locReconstructedPID;
	float locThrownMass, locMomentum, locBeta;

	if(locDMCThrownVector.size() != 1)
		return; //routine assumes only one thrown track (matching not performed!)
	if(locChargedTrackHypothesisVector.size() == 0)
		return;
	locDMCThrown = locDMCThrownVector[0];
	locThrownPID = Particle_t(locDMCThrown->type);
	locThrownMass = ParticleMass(locThrownPID);

	bool locPIDMatchFlag = false;
	for(unsigned int loc_i = 0; loc_i < locChargedTrackHypothesisVector.size(); loc_i++){
		locChargedTrackHypothesis = locChargedTrackHypothesisVector[loc_i];
		locTrackTimeBased = locChargedTrackHypothesis->dTrackTimeBased;
		locReconstructedPID = locChargedTrackHypothesis->dPID;
		if(locReconstructedPID == locThrownPID){
			locPIDMatchFlag = true;
			break;
		}
	}
	if(locPIDMatchFlag == false)
		locTrackTimeBased = locChargedTrackHypothesisVector[0]->dTrackTimeBased;

	locMomentum = locTrackTimeBased->momentum().Mag();
	locBeta = locMomentum/sqrt(locThrownMass*locThrownMass + locMomentum*locMomentum);

	// Although we are only filling objects local to this plugin, TTree::Fill() periodically writes to file: Global ROOT lock
	GetLockService(locEvent)->RootWriteLock(); //ACQUIRE ROOT LOCK

	//low-momentum & beta protons are reconstructed as low-momentum but high-beta pions
	dDCdEdxInformation->dBeta = locBeta;
	dDCdEdxInformation->dMomentum = locMomentum;
	dDCdEdxInformation->dTheta = locTrackTimeBased->momentum().Theta();
	dDCdEdxInformation->dVertexZ = locTrackTimeBased->z();

	dDCdEdxInformation->ddEdx_FDC = locTrackTimeBased->ddEdx_FDC; //units of GeV/cm
	dDCdEdxInformation->ddx_FDC = locTrackTimeBased->ddx_FDC;
	dDCdEdxInformation->dNumHitsUsedFordEdx_FDC = locTrackTimeBased->dNumHitsUsedFordEdx_FDC;
	dDCdEdxInformation->ddEdx_CDC = locTrackTimeBased->ddEdx_CDC; //units of GeV/cm
	dDCdEdxInformation->ddx_CDC = locTrackTimeBased->ddx_CDC;
	dDCdEdxInformation->dNumHitsUsedFordEdx_CDC = locTrackTimeBased->dNumHitsUsedFordEdx_CDC;

	dDCdEdxInformation->dChiSq_DCdEdx = locChargedTrackHypothesis->dChiSq_DCdEdx;
	dDCdEdxInformation->dNDF_DCdEdx = locChargedTrackHypothesis->dNDF_DCdEdx;
	dDCdEdxInformation->dFOM = TMath::Prob(dDCdEdxInformation->dChiSq_DCdEdx, dDCdEdxInformation->dNDF_DCdEdx);
	dPluginTree_DCdEdxInformation->Fill();

	GetLockService(locEvent)->RootUnLock(); //RELEASE ROOT LOCK
}

//------------------
// EndRun
//------------------
void DEventProcessor_DCdEdxStudy_tree::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_DCdEdxStudy_tree::Finish()
{
}

