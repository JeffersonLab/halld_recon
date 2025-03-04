// $Id$
//
//    File: DTrackWireBased_factory_StraightLine.cc
// Created: Wed Mar 13 10:00:25 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DTrackWireBased_factory_StraightLine.h"
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>


//------------------
// Init
//------------------
void DTrackWireBased_factory_StraightLine::Init()
{
  auto app = GetApplication();
  CDC_MATCH_CUT=2.;
  app->SetDefaultParameter("TRKFIT:CDC_MATCH_CUT",CDC_MATCH_CUT); 
  FDC_MATCH_CUT=1.25;
  app->SetDefaultParameter("TRKFIT:FDC_MATCH_CUT",FDC_MATCH_CUT); 
}

//------------------
// BeginRun
//------------------
void DTrackWireBased_factory_StraightLine::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // Get the particle ID algorithms
  event->GetSingle(dPIDAlgorithm);
  
  // Get pointer to TrackFinder object 
  vector<const DTrackFinder *> finders;
  event->Get(finders);
  
  if(finders.size()<1){
    _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
    return; // RESOURCE_UNAVAILABLE;
  }
  
   // Drop the const qualifier from the DTrackFinder pointer
  finder = const_cast<DTrackFinder*>(finders[0]);
  
  // Get pointer to DTrackFitter object that actually fits a track
  vector<const DTrackFitter *> fitters;
  event->Get(fitters,"StraightTrack");
  if(fitters.size()<1){
    _DBG_<<"Unable to get a DTrackFitter object!"<<endl;
    return; // RESOURCE_UNAVAILABLE;
  }
  
  // Drop the const qualifier from the DTrackFitter pointer
  fitter = const_cast<DTrackFitter*>(fitters[0]);
}

//------------------
// Process
//------------------
void DTrackWireBased_factory_StraightLine::Process(const std::shared_ptr<const JEvent>& event)
{
  // Get candidates
   vector<const DTrackCandidate*> candidates;
   event->Get(candidates);

   // Get hits
   vector<const DCDCTrackHit *>cdchits;
   event->Get(cdchits);
   vector<const DFDCPseudo *>fdchits;
   event->Get(fdchits);
  
   for (unsigned int i=0;i<candidates.size();i++){
     // Reset the fitter
     fitter->Reset();

     const DTrackCandidate *cand=candidates[i];
     DVector3 pos=cand->position();
     DVector3 dir=cand->momentum();
     // Select hits that belong to the track
     for (unsigned int j=0;j<cdchits.size();j++){
       double d=finder->FindDoca(pos,dir,cdchits[j]->wire->origin,
				 cdchits[j]->wire->udir);
       if (d<CDC_MATCH_CUT) fitter->AddHit(cdchits[j]);
     }
     for (unsigned int i=0;i<fdchits.size();i++){
       double pz=dir.z();
       double tx=dir.x()/pz;
       double ty=dir.y()/pz;
       double dz=fdchits[i]->wire->origin.z()-pos.z();
       DVector2 predpos(pos.x()+tx*dz,pos.y()+ty*dz);
       DVector2 diff=predpos-fdchits[i]->xy;
       if (diff.Mod()<FDC_MATCH_CUT) fitter->AddHit(fdchits[i]);
     }

     // Fit the track using the list of hits we gathered above
     if (fitter->FitTrack(pos,dir,1.,0.,0.)==DTrackFitter::kFitSuccess){
       // Make a new wire-based track
       DTrackWireBased *track = new DTrackWireBased();
       *static_cast<DTrackingData*>(track) = fitter->GetFitParameters();

       track->chisq = fitter->GetChisq();
       track->Ndof = fitter->GetNdof();
       track->setPID(PiPlus);
       track->FOM = TMath::Prob(track->chisq, track->Ndof);
       track->pulls =std::move(fitter->GetPulls());  
       track->extrapolations=std::move(fitter->GetExtrapolations());  
       track->IsSmoothed = fitter->GetIsSmoothed();

       // candidate id
       track->candidateid=i+1;

       // Add hits used as associated objects
       vector<const DCDCTrackHit*> cdchits_on_track = fitter->GetCDCFitHits();
       vector<const DFDCPseudo*> fdchits_on_track = fitter->GetFDCFitHits();

       // ...also find minimum drift time...
       double tmin=1e6;
       DetectorSystem_t detector=SYS_NULL;
       for (unsigned int k=0;k<cdchits_on_track.size();k++){
	 if (cdchits_on_track[k]->tdrift<tmin){
	   tmin=cdchits_on_track[k]->tdrift;
	   detector=SYS_CDC;
	 }
	 track->AddAssociatedObject(cdchits_on_track[k]);
       }
       for (unsigned int k=0;k<fdchits_on_track.size();k++){
	 if (fdchits_on_track[k]->time<tmin){
	   tmin=fdchits_on_track[k]->time;
	   detector=SYS_FDC;
	 }
	 track->AddAssociatedObject(fdchits_on_track[k]);
       }
       track->setT0(tmin,10.0,detector);
       
       track->dCDCRings = dPIDAlgorithm->Get_CDCRingBitPattern(cdchits_on_track);
       track->dFDCPlanes = dPIDAlgorithm->Get_FDCPlaneBitPattern(fdchits_on_track);

       Insert(track);
     }
   }
}

//------------------
// EndRun
//------------------
void DTrackWireBased_factory_StraightLine::EndRun()
{
}

//------------------
// Finish
//------------------
void DTrackWireBased_factory_StraightLine::Finish()
{
}

