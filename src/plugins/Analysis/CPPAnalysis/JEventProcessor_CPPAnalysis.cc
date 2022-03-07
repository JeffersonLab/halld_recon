// $Id$
//
//    File: JEventProcessor_CPPAnalysis.cc
// Created: Fri Mar  4 08:20:58 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_CPPAnalysis.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
  InitJANAPlugin(app);
  app->AddProcessor(new JEventProcessor_CPPAnalysis());
}
} // "C"


//------------------
// JEventProcessor_CPPAnalysis (Constructor)
//------------------
JEventProcessor_CPPAnalysis::JEventProcessor_CPPAnalysis()
{

}

//------------------
// ~JEventProcessor_CPPAnalysis (Destructor)
//------------------
JEventProcessor_CPPAnalysis::~JEventProcessor_CPPAnalysis()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_CPPAnalysis::init(void)
{
  // This is called once at program startup. 
  japp->CreateLock("myCPPlock");

  SPLIT_CUT=0.5;
  gPARMS->SetDefaultParameter("CPPAnalysis:SPLIT_CUT",SPLIT_CUT); 
  FCAL_THRESHOLD=0.1;
  gPARMS->SetDefaultParameter("CPPAnalysis:FCAL_THRESHOLD",FCAL_THRESHOLD);
  BCAL_THRESHOLD=0.05;
  gPARMS->SetDefaultParameter("CPPAnalysis:BCAL_THRESHOLD",BCAL_THRESHOLD);
  GAMMA_DT_CUT=2.; 
  gPARMS->SetDefaultParameter("CPPAnalysis:GAMMA_DT_CUT",GAMMA_DT_CUT);
  EPEM_CL_CUT=1e-4; 
  gPARMS->SetDefaultParameter("CPPAnalysis:EPEM_CL_CUT",EPEM_CL_CUT);
  PIPPIM_CL_CUT=1e-4; 
  gPARMS->SetDefaultParameter("CPPAnalysis:PIPPIM_CL_CUT",PIPPIM_CL_CUT);


  gDirectory->mkdir("CPPFits")->cd();
  H_CL_comp=new TH2F("H_CL_comp",";CL(e^{+}e^{-});CL(#pi^{+}#pi^{-})",
		     100,0,1,100,0,1);
  H_EpEmMass=new TH1F("H_EpEmMass",";M(e^{+}e^{-}) [GeV]",1600,0.,4.);
  H_EpEmMass_Cut=new TH1F("H_EpEmMass_Cut",";M(e^{+}e^{-}) [GeV]",1600,0.,4.);
  H_VertexZ=new TH1F("H_VertexZ",";z [cm]",2000,-20,180);
  H_VertexXY=new TH2F("H_VertexXY",";x [cm]; y [cm]",200,-10,10,200,-10,10);
  H_E_over_P_positron=new TH2F("H_E_over_P_positron",";p [GeV/c];E/p",100,0,10,
			       100,0,1.5);
  H_E_over_P_electron=new TH2F("H_E_over_P_electron",";p [GeV/c];E/p",100,0,10,
			       100,0,1.5);
  H_PipPimMass=new TH1F("H_PipPimMass",";M(#pi^{+}#pi^{-}) [GeV]",1600,0.,4.);

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_CPPAnalysis::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_CPPAnalysis::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  vector<const DBeamPhoton*>beamphotons;
  loop->Get(beamphotons);
  if (beamphotons.size()==0) return RESOURCE_UNAVAILABLE;

  vector<const DChargedTrack*>tracks; 
  loop->Get(tracks);
  if (tracks.size()!=2) return RESOURCE_UNAVAILABLE;

  // Return if we do not have 2 oppositely-charged tracks
  double q1=tracks[0]->Get_Charge(),q2=tracks[1]->Get_Charge();
  if (q1*q2>0) return VALUE_OUT_OF_RANGE;

  // Check that we have fitted tracks for e+/e- and pi+/pi- mass hypotheses
  const DChargedTrackHypothesis *hyp=NULL;
  unsigned int ip=(q1>q2)?0:1;
  unsigned int in=(q1>q2)?1:0;
  
  hyp=tracks[ip]->Get_Hypothesis(Positron);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *positron=hyp->Get_TrackTimeBased();
 
  // Energy in calorimeter associated with the e+ track
  shared_ptr<const DBCALShowerMatchParams>bcalparms=hyp->Get_BCALShowerMatchParams(); 
  shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp->Get_FCALShowerMatchParams();
  double Epositron=0.;
  if (fcalparms!=NULL){
    Epositron=fcalparms->dFCALShower->getEnergy();
  }
  else if (bcalparms!=NULL){
    Epositron=bcalparms->dBCALShower->E;
  }
    
  hyp=tracks[ip]->Get_Hypothesis(PiPlus);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *piplus=hyp->Get_TrackTimeBased();

  hyp=tracks[in]->Get_Hypothesis(Electron);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *electron=hyp->Get_TrackTimeBased();
  
  // Energy in calorimeter associated with the e+ track
  bcalparms=hyp->Get_BCALShowerMatchParams(); 
  fcalparms=hyp->Get_FCALShowerMatchParams();
  double Eelectron=0.;
  if (fcalparms!=NULL){
    Eelectron=fcalparms->dFCALShower->getEnergy();
  }
  else if (bcalparms!=NULL){
    Eelectron=bcalparms->dBCALShower->E;
  }

  hyp=tracks[in]->Get_Hypothesis(PiMinus);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *piminus=hyp->Get_TrackTimeBased();

  vector<const DNeutralParticle*>neutrals;
  loop->Get(neutrals);

  japp->WriteLock("myCPPlock");

  // Find t0 (at "vertex") for event
  double t0_rf=tracks[0]->Get_BestTrackingFOM()->t0();
  DVector3 vertex=tracks[0]->Get_BestTrackingFOM()->position();

  // Veto events that have non-splitoff gammas in time with the tracks
  if (VetoNeutrals(t0_rf,vertex,neutrals)==false){
    // Setup for performing kinematic fits
    DAnalysisUtilities *dAnalysisUtilities=new DAnalysisUtilities(loop);
    DKinFitUtils_GlueX *dKinFitUtils = new DKinFitUtils_GlueX(loop);
    DKinFitter *dKinFitter = new DKinFitter(dKinFitUtils);   
    dKinFitUtils->Reset_NewEvent();
    dKinFitter->Reset_NewEvent();
    
    // Loop over all beam photons
    for (unsigned int i=0;i<beamphotons.size();i++){
      double dt_rf=beamphotons[i]->time()-t0_rf;
      
      double weight=1.;
      bool got_beam_photon=false;
      if (fabs(dt_rf)>6.012&&fabs(dt_rf)<18.036){
	weight=-1./6.;
	got_beam_photon=true;
      }
      if (fabs(dt_rf)<2.004){
	got_beam_photon=true;
      }
      if (got_beam_photon==false) continue;
      
      //--------------------------------
      // Kinematic fit for pi+/pi- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],piminus,piplus,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
      double pippimCL = dKinFitter->Get_ConfidenceLevel();
      if (pippimCL>PIPPIM_CL_CUT){
	PipPimAnalysis(dKinFitter,weight);
      }

      //--------------------------------
      // Kinematic fit for e+/e- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],electron,positron,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
      double epemCL = dKinFitter->Get_ConfidenceLevel();
      if (epemCL>EPEM_CL_CUT && epemCL>pippimCL){
	EpEmAnalysis(dKinFitter,Eelectron,Epositron,weight);
      }
      
      if (weight>0){
	H_CL_comp->Fill(epemCL,pippimCL);
      }
    
    }
      
    delete dAnalysisUtilities;
    delete dKinFitter;
    delete dKinFitUtils;
  }

  japp->Unlock("myCPPlock");

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_CPPAnalysis::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_CPPAnalysis::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}

// Run the kinematic fitter requiring energy and momentum conservation and 
// applying a vertex constraint
void JEventProcessor_CPPAnalysis::DoKinematicFit(const DBeamPhoton *beamphoton,
						 const DTrackTimeBased *negative,
						 const DTrackTimeBased *positive,
						 DKinFitUtils_GlueX *dKinFitUtils,
						 DKinFitter *dKinFitter,
						 DAnalysisUtilities *dAnalysisUtilities) const {
  set<shared_ptr<DKinFitParticle>> InitialParticles, FinalParticles;
  
  shared_ptr<DKinFitParticle>myBeam=dKinFitUtils->Make_BeamParticle(beamphoton);
  shared_ptr<DKinFitParticle>myTarget=dKinFitUtils->Make_TargetParticle(m_TargetParticle);
  
  InitialParticles.insert(myBeam);  
  InitialParticles.insert(myTarget);

  shared_ptr<DKinFitParticle>myNegativeParticle=dKinFitUtils->Make_DetectedParticle(negative);
  FinalParticles.insert(myNegativeParticle);
  shared_ptr<DKinFitParticle>myPositiveParticle=dKinFitUtils->Make_DetectedParticle(positive);
  FinalParticles.insert(myPositiveParticle);
  shared_ptr<DKinFitParticle>myRecoil=dKinFitUtils->Make_MissingParticle(m_TargetParticle);
  FinalParticles.insert(myRecoil);    
  
  // make energy-momentum constraint
  shared_ptr<DKinFitConstraint_P4>locP4Constraint = dKinFitUtils->Make_P4Constraint(InitialParticles, FinalParticles);
  dKinFitter->Add_Constraint(locP4Constraint);

  // Make vertex constraint
  DVector3 vertex;
  dAnalysisUtilities->Calc_DOCAVertex(myPositiveParticle.get(),
				      myNegativeParticle.get(),vertex);
  set<shared_ptr<DKinFitParticle>> ConstrainedParticles;
  set<shared_ptr<DKinFitParticle>> NonConstrainedParticles;
  ConstrainedParticles.insert(myNegativeParticle);
  ConstrainedParticles.insert(myPositiveParticle);
  NonConstrainedParticles.insert(myBeam);  
  NonConstrainedParticles.insert(myTarget);
  NonConstrainedParticles.insert(myRecoil);
  shared_ptr<DKinFitConstraint_Vertex> locVertexConstraint = dKinFitUtils->Make_VertexConstraint(ConstrainedParticles,NonConstrainedParticles,vertex);
  dKinFitter->Add_Constraint(locVertexConstraint);
 
  // PERFORM THE KINEMATIC FIT
  dKinFitter->Fit_Reaction();
}

// Veto events that have neutral particles in time with the tracks that are not
// considered to be hadronic splitoffs
bool JEventProcessor_CPPAnalysis::VetoNeutrals(double t0_rf,
					       const DVector3 &vect,
					       vector<const DNeutralParticle*>&neutrals) const{
  
  for (unsigned int i=0;i<neutrals.size();i++){
    const DNeutralParticleHypothesis *gamHyp=neutrals[i]->Get_Hypothesis(Gamma);
    double tdiff=gamHyp->time()-t0_rf;
    if (fabs(tdiff)<GAMMA_DT_CUT){
      // Shower energy
      double E=gamHyp->lorentzMomentum().E(); 

      // Get shower info corresponding to this hypothesis
      const DNeutralShower *shower=gamHyp->Get_NeutralShower();
      if (shower->dDetectorSystem==SYS_FCAL){
	if (shower->dQuality<SPLIT_CUT) continue;
	if (E>FCAL_THRESHOLD) return true;
      }
      else if (shower->dDetectorSystem==SYS_BCAL){
	if (E>BCAL_THRESHOLD) return true;
      }
    }
  }

  return false;
}

void JEventProcessor_CPPAnalysis::EpEmAnalysis(DKinFitter *dKinFitter,
					      double Eelectron,double Epositron,
					      double weight) const {
  DLorentzVector beam_kf,ep_kf,em_kf;
  DVector3 vertex;
  
  set<shared_ptr<DKinFitParticle>>myParticles=dKinFitter->Get_KinFitParticles();
  set<shared_ptr<DKinFitParticle>>::iterator locParticleIterator=myParticles.begin();
  for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
    if ((*locParticleIterator)->Get_KinFitParticleType()==d_BeamParticle){
      beam_kf=(*locParticleIterator)->Get_P4();
    }
    else if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
      switch ((*locParticleIterator)->Get_PID()){
      case 11: // positron
	ep_kf=(*locParticleIterator)->Get_P4();
	vertex=(*locParticleIterator)->Get_Position();
	break;
      case -11: // electron
	em_kf=(*locParticleIterator)->Get_P4();
	break;	  
      default:
	break;
      }
    }
  }

  H_VertexZ->Fill(vertex.z(),weight);
  H_VertexXY->Fill(vertex.x(),vertex.y(),weight);
  
  DLorentzVector epem_kf=ep_kf+em_kf;
  double epem_mass=epem_kf.M();
  H_EpEmMass->Fill(epem_mass,weight);

  double p_positron=ep_kf.P();
  double E_over_p_positron=Epositron/p_positron;
  H_E_over_P_positron->Fill(p_positron,E_over_p_positron,weight);

  double p_electron=ep_kf.P();
  double E_over_p_electron=Eelectron/p_electron;
  H_E_over_P_electron->Fill(p_electron,E_over_p_electron,weight);
  
  if (E_over_p_electron>0.8 && E_over_p_positron>0.8){
    H_EpEmMass_Cut->Fill(epem_mass,weight);
  }
}

void JEventProcessor_CPPAnalysis::PipPimAnalysis(DKinFitter *dKinFitter,
						 double weight) const {
  DLorentzVector beam_kf,pip_kf,pim_kf;
  
  set<shared_ptr<DKinFitParticle>>myParticles=dKinFitter->Get_KinFitParticles();
  set<shared_ptr<DKinFitParticle>>::iterator locParticleIterator=myParticles.begin();
  for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
    if ((*locParticleIterator)->Get_KinFitParticleType()==d_BeamParticle){
      beam_kf=(*locParticleIterator)->Get_P4();
    }
    else if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
      switch ((*locParticleIterator)->Get_PID()){
      case 211: // pi+
	pip_kf=(*locParticleIterator)->Get_P4();
	break;
      case -211: // pi-
	pim_kf=(*locParticleIterator)->Get_P4();
	break;	  
      default:
	break;
      }
    }
  }

  
  DLorentzVector pippim_kf=pip_kf+pim_kf;
  H_PipPimMass->Fill(pippim_kf.M(),weight);
}
