// -----------------------------------------
// DEventProcessor_dirc_hists.cc
// -----------------------------------------

#include "DEventProcessor_dirc_hists.h"

// Routine used to create our DEventProcessor
extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_dirc_hists());
  }
}

DEventProcessor_dirc_hists::DEventProcessor_dirc_hists() {
}

DEventProcessor_dirc_hists::~DEventProcessor_dirc_hists() {
}

jerror_t DEventProcessor_dirc_hists::init(void) {
  string locOutputFileName = "hd_root.root";
  if(gPARMS->Exists("OUTPUT_FILENAME"))
    gPARMS->GetParameter("OUTPUT_FILENAME", locOutputFileName);

  TDirectory *dir = new TDirectoryFile("DIRC","DIRC");
  dir->cd();

  // timing difference for measured tracks
  hDiff = new TH1I("hDiff",";t_{calc}-t_{measured} [ns];entries [#]", 400,-20,20);
  
  // likelihood plots for each hypothesis
  TString particleName[4] = {"Electron", "Pion", "Kaon", "Proton"};
  for(int loc_i=0; loc_i<4; loc_i++) {
	  hDeltaThetaC[loc_i] = new TH1I(Form("hDeltaThetaC_%s",particleName[loc_i].Data()),  "cherenkov angle; #Delta#theta_{C} [rad]", 200,-0.5,0.5);
	  hLikelihood[loc_i] = new TH1I(Form("hLikelihood_%s",particleName[loc_i].Data()), "lnL;entries [#]",1000,-5000.,5000.);
	  hLikelihoodDiff[loc_i] = new TH1I(Form("hLiklihoodDiff_%s",particleName[loc_i].Data()), "ln L(#pi) - ln L(K);entries [#]",100,-200.,200.);
  }

  gDirectory->cd("/");
 
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::brun(jana::JEventLoop *loop, int32_t runnumber)
{
   // Get the geometry
   DApplication* dapp=dynamic_cast<DApplication*>(loop->GetJApplication());
   DGeometry *geom = dapp->GetDGeometry(runnumber);

   // Outer detector geometry parameters
   vector<double>tof_face;
   geom->Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z", tof_face);
   vector<double>tof_plane;  
   geom->Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='0']", tof_plane);
   double dTOFz=tof_face[2]+tof_plane[2]; 
   geom->Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='1']", tof_plane);
   dTOFz+=tof_face[2]+tof_plane[2];
   dTOFz*=0.5;  // mid plane between tof Planes
   //std::cout<<"dTOFz "<<dTOFz<<std::endl;

   double dDIRCz;
   vector<double>dirc_face;
   vector<double>dirc_plane;
   vector<double>dirc_shift;
   vector<double>bar_plane;
   geom->Get("//section/composition/posXYZ[@volume='DIRC']/@X_Y_Z", dirc_face);
   geom->Get("//composition[@name='DRCC']/mposY[@volume='DCML']/@Z_X/plane[@value='1']", dirc_plane);
   geom->Get("//composition[@name='DIRC']/posXYZ[@volume='DRCC']/@X_Y_Z", dirc_shift);
   geom->Get("//composition[@name='DCBR']/mposX[@volume='QZBL']/@Y_Z", bar_plane);
   
   dDIRCz=dirc_face[2]+dirc_plane[0]+dirc_shift[2]+bar_plane[1]; // 585.862
   //std::cout<<"dDIRCz "<<dDIRCz<<std::endl;

   // get PID algos
   const DParticleID* locParticleID = NULL;
   loop->GetSingle(locParticleID);
   dParticleID = locParticleID;

   return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::evnt(JEventLoop *loop, uint64_t eventnumber) {

  // retrieve tracks and detector matches 
  vector<const DTrackTimeBased*> locTimeBasedTracks;
  const DDetectorMatches* locDetectorMatches = NULL;
  loop->Get(locTimeBasedTracks);
  loop->GetSingle(locDetectorMatches);

  // plot DIRC LUT variables for specific tracks
  for (unsigned int loc_i = 0; loc_i < locTimeBasedTracks.size(); loc_i++){
	  const DTrackTimeBased* locTrackTimeBased = locTimeBasedTracks[loc_i];
	  
	  // expected thetaC
	  vector<DTrackFitter::Extrapolation_t> extrapolations=locTrackTimeBased->extrapolations.at(SYS_DIRC);
	  if(extrapolations.size()==0) continue;
	  DVector3 momInBar = extrapolations[0].momentum;
	  double locMass = locTrackTimeBased->mass();
	  double mAngle = acos(sqrt(momInBar.Mag()*momInBar.Mag() + locMass*locMass)/momInBar.Mag()/1.473);

	  int locHypothesisIndex = -1;
	  Particle_t hypotheses[4] = {Electron, PiPlus, KPlus, Proton};
	  for(int loc_i = 0; loc_i<4; loc_i++) {
		  if( fabs(ParticleMass(hypotheses[loc_i]) - locMass) < 0.01 )
			  locHypothesisIndex = loc_i;	
	  }

	  shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
	  bool foundDIRC = dParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);
	  
	  if(foundDIRC) {

		  // loop over hits associated with track
		  const DDIRCLutPhotons* locDIRCLutPhotons = locDIRCMatchParams->dDIRCLutPhotons;
		  if(locDIRCLutPhotons) {
			  vector< pair<double,double> > locPhotons = locDIRCLutPhotons->dPhoton;
			  if(locPhotons.size() > 0) {
				  for(uint loc_j = 0; loc_j<locPhotons.size(); loc_j++) {
					  double locDeltaT = locPhotons[loc_j].second;
					  hDiff->Fill(locDeltaT);
					  if(fabs(locDeltaT) < 2.0) 
						  hDeltaThetaC[locHypothesisIndex]->Fill(locPhotons[loc_j].first-mAngle);
				  }
			  }
		  }			  
		  
		  // fill histograms with per-track quantities
		  if(locHypothesisIndex == 0) {
			  hLikelihood[0]->Fill(locDIRCMatchParams->dLikelihoodElectron);
			  hLikelihoodDiff[0]->Fill(locDIRCMatchParams->dLikelihoodElectron - locDIRCMatchParams->dLikelihoodPion);
		  }
		  else if(locHypothesisIndex == 1) {
			  hLikelihood[1]->Fill(locDIRCMatchParams->dLikelihoodPion);
			  hLikelihoodDiff[1]->Fill(locDIRCMatchParams->dLikelihoodPion - locDIRCMatchParams->dLikelihoodKaon);
		  }
		  else if(locHypothesisIndex == 2) {
			  hLikelihood[2]->Fill(locDIRCMatchParams->dLikelihoodKaon);
			  hLikelihoodDiff[2]->Fill(locDIRCMatchParams->dLikelihoodKaon - locDIRCMatchParams->dLikelihoodProton);
		  }
		  else if(locHypothesisIndex == 3) {
			  hLikelihood[3]->Fill(locDIRCMatchParams->dLikelihoodProton);
			  hLikelihoodDiff[3]->Fill(locDIRCMatchParams->dLikelihoodProton - locDIRCMatchParams->dLikelihoodKaon);
		  }
	  }
  }
  
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::erun(void) {
  return NOERROR;
}

jerror_t DEventProcessor_dirc_hists::fini(void) {
  return NOERROR;
}
