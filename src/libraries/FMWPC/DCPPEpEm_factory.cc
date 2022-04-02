// $Id$
//
//    File: DCPPEpEm_factory.cc
// Created: Thu Mar 17 14:49:42 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//
/// This factory is intended to aid in the identification of e+/e- events 
/// coming from the Pb target.  For events with one positive and one negative
/// track, it performs kinematic fits for the hypotheses that the pair of tracks
/// is either e+/e-, pi+/pi-, or K+K- and saves the four-vectors for the tracks
/// after the fits along with the FCAL showers associated with the tracks.

#include <iostream>
#include <iomanip>
using namespace std;

#include "DCPPEpEm_factory.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DCPPEpEm_factory::init(void)
{
  SPLIT_CUT=0.5;
  gPARMS->SetDefaultParameter("CPPAnalysis:SPLIT_CUT",SPLIT_CUT); 
  FCAL_THRESHOLD=0.1;
  gPARMS->SetDefaultParameter("CPPAnalysis:FCAL_THRESHOLD",FCAL_THRESHOLD);
  BCAL_THRESHOLD=0.05;
  gPARMS->SetDefaultParameter("CPPAnalysis:BCAL_THRESHOLD",BCAL_THRESHOLD);
  GAMMA_DT_CUT=2.; 

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DCPPEpEm_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DCPPEpEm_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
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
  
  hyp=tracks[ip]->Get_Hypothesis(PiPlus);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *piplus=hyp->Get_TrackTimeBased();

  hyp=tracks[in]->Get_Hypothesis(PiMinus);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *piminus=hyp->Get_TrackTimeBased();

  hyp=tracks[ip]->Get_Hypothesis(KPlus);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *kplus=hyp->Get_TrackTimeBased();

  hyp=tracks[in]->Get_Hypothesis(KMinus);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *kminus=hyp->Get_TrackTimeBased();

  hyp=tracks[ip]->Get_Hypothesis(Positron);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *positron=hyp->Get_TrackTimeBased();
 
   // FCAL shower associated with the e+ track
  shared_ptr<const DFCALShowerMatchParams>fcalparms=hyp->Get_FCALShowerMatchParams();
  const DFCALShower *PositronShower=NULL;
  if (fcalparms!=NULL){
    PositronShower=fcalparms->dFCALShower;
  }
    
  hyp=tracks[in]->Get_Hypothesis(Electron);
  if (hyp==NULL) return RESOURCE_UNAVAILABLE;
  const DTrackTimeBased *electron=hyp->Get_TrackTimeBased();
  
  // FCAL shower associated with the e- track
  fcalparms=hyp->Get_FCALShowerMatchParams();
  const DFCALShower *ElectronShower=NULL;
  if (fcalparms!=NULL){
    ElectronShower=fcalparms->dFCALShower;
  }

  vector<const DNeutralParticle*>neutrals;
  loop->Get(neutrals);
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

      // Create a new CPPEpEm object
      DCPPEpEm *myCPPEpEm=new DCPPEpEm;
      myCPPEpEm->Ebeam=beamphotons[i]->lorentzMomentum().E();
      myCPPEpEm->weight=weight;
      myCPPEpEm->ElectronShower=ElectronShower;
      myCPPEpEm->PositronShower=PositronShower;

      //--------------------------------
      // Kinematic fit for pi+/pi- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],piminus,piplus,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
      // fit quality
      myCPPEpEm->pippim_chisq=dKinFitter->Get_ChiSq();

      // Get the fitted 4-vectors
      set<shared_ptr<DKinFitParticle>>myParticles=dKinFitter->Get_KinFitParticles();
      set<shared_ptr<DKinFitParticle>>::iterator locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
	if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
	  if ((*locParticleIterator)->Get_PID()==211){
	    myCPPEpEm->pip_v4=(*locParticleIterator)->Get_P4();
	  }
	  else{
	    myCPPEpEm->pim_v4=(*locParticleIterator)->Get_P4();
    	  }
	}
      }

       //--------------------------------
      // Kinematic fit for K+/K- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],kminus,kplus,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
        // fit quality
      myCPPEpEm->kpkm_chisq=dKinFitter->Get_ChiSq();

      // Get the fitted 4-vectors
      myParticles=dKinFitter->Get_KinFitParticles();
      locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
	if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
	  if ((*locParticleIterator)->Get_PID()==-321){
	    myCPPEpEm->km_v4=(*locParticleIterator)->Get_P4();
	  }
	  else{
	    myCPPEpEm->kp_v4=(*locParticleIterator)->Get_P4();
    	  }
	}
      }

      //--------------------------------
      // Kinematic fit for e+/e- case
      //--------------------------------
      dKinFitter->Reset_NewFit();
      DoKinematicFit(beamphotons[i],electron,positron,dKinFitUtils,dKinFitter,
		     dAnalysisUtilities);
        // fit quality
      myCPPEpEm->epem_chisq=dKinFitter->Get_ChiSq();

      // Get the fitted 4-vectors
      myParticles=dKinFitter->Get_KinFitParticles();
      locParticleIterator=myParticles.begin();
      for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
	if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
	  if ((*locParticleIterator)->Get_PID()==11){
	    myCPPEpEm->em_v4=(*locParticleIterator)->Get_P4();
	  }
	  else{
	    myCPPEpEm->ep_v4=(*locParticleIterator)->Get_P4();
    	  }
	}
      }

      _data.push_back(myCPPEpEm);
    }
      
    delete dAnalysisUtilities;
    delete dKinFitter;
    delete dKinFitUtils;
 
  }

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DCPPEpEm_factory::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DCPPEpEm_factory::fini(void)
{
  return NOERROR;
}

// Run the kinematic fitter requiring energy and momentum conservation and 
// applying a vertex constraint
void DCPPEpEm_factory::DoKinematicFit(const DBeamPhoton *beamphoton,
				      const DTrackTimeBased *negative,
				      const DTrackTimeBased *positive,
				      DKinFitUtils_GlueX *dKinFitUtils,
				      DKinFitter *dKinFitter,
				      DAnalysisUtilities *dAnalysisUtilities) const {
  set<shared_ptr<DKinFitParticle>> InitialParticles, FinalParticles;
  
  shared_ptr<DKinFitParticle>myBeam=dKinFitUtils->Make_BeamParticle(beamphoton);
  shared_ptr<DKinFitParticle>myTarget=dKinFitUtils->Make_TargetParticle(Pb208);
  
  InitialParticles.insert(myBeam);  
  InitialParticles.insert(myTarget);

  shared_ptr<DKinFitParticle>myNegativeParticle=dKinFitUtils->Make_DetectedParticle(negative);
  FinalParticles.insert(myNegativeParticle);
  shared_ptr<DKinFitParticle>myPositiveParticle=dKinFitUtils->Make_DetectedParticle(positive);
  FinalParticles.insert(myPositiveParticle);
  shared_ptr<DKinFitParticle>myRecoil=dKinFitUtils->Make_MissingParticle(Pb208);
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
bool DCPPEpEm_factory::VetoNeutrals(double t0_rf,const DVector3 &vect,
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
