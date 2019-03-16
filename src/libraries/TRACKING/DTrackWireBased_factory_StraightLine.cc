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
using namespace jana;

//------------------
// init
//------------------
jerror_t DTrackWireBased_factory_StraightLine::init(void)
{
  CDC_MATCH_CUT=1.25;
  gPARMS->SetDefaultParameter("TRKFIT:CDC_MATCH_CUT",CDC_MATCH_CUT); 
  FDC_MATCH_CUT=1.25;
  gPARMS->SetDefaultParameter("TRKFIT:FDC_MATCH_CUT",FDC_MATCH_CUT); 

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DTrackWireBased_factory_StraightLine::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  // Get the particle ID algorithms
  eventLoop->GetSingle(dPIDAlgorithm);
  
  // Get pointer to TrackFinder object 
  vector<const DTrackFinder *> finders;
  eventLoop->Get(finders);
  
  if(finders.size()<1){
    _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
    return RESOURCE_UNAVAILABLE;
  }
  
   // Drop the const qualifier from the DTrackFinder pointer
  finder = const_cast<DTrackFinder*>(finders[0]);
  
  // Get pointer to DTrackFitter object that actually fits a track
  vector<const DTrackFitter *> fitters;
  eventLoop->Get(fitters,"StraightTrack");
  if(fitters.size()<1){
    _DBG_<<"Unable to get a DTrackFitter object!"<<endl;
    return RESOURCE_UNAVAILABLE;
  }
  
  // Drop the const qualifier from the DTrackFitter pointer
  fitter = const_cast<DTrackFitter*>(fitters[0]);

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DTrackWireBased_factory_StraightLine::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // Get candidates
   vector<const DTrackCandidate*> candidates;
   loop->Get(candidates);

   // Get hits
   vector<const DCDCTrackHit *>cdchits;
   loop->Get(cdchits);
   vector<const DFDCPseudo *>fdchits;
   loop->Get(fdchits);
  
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

       for (unsigned int k=0;k<cdchits_on_track.size();k++){
	 track->AddAssociatedObject(cdchits_on_track[k]);
       }
       for (unsigned int k=0;k<fdchits_on_track.size();k++){
	 track->AddAssociatedObject(fdchits_on_track[k]);
       }
       track->dCDCRings = dPIDAlgorithm->Get_CDCRingBitPattern(cdchits_on_track);
       track->dFDCPlanes = dPIDAlgorithm->Get_FDCPlaneBitPattern(fdchits_on_track);

       _data.push_back(track);
     }
   }

   return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DTrackWireBased_factory_StraightLine::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DTrackWireBased_factory_StraightLine::fini(void)
{
	return NOERROR;
}

