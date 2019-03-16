// $Id$
//
//    File: DTrackTimeBased_factory_StraightLine.cc
// Created: Wed Mar 13 10:00:17 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DTrackTimeBased_factory_StraightLine.h"
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
using namespace jana;

//------------------
// init
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::init(void)
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
jerror_t DTrackTimeBased_factory_StraightLine::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  // Get the geometry
  DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  const DGeometry *geom = dapp->GetDGeometry(runnumber);

  // Get the particle ID algorithms
  eventLoop->GetSingle(dPIDAlgorithm);
  
  // Outer detector geometry parameters
  if (geom->GetDIRCZ(dDIRCz)==false) dDIRCz=1000.;
  geom->GetFCALZ(dFCALz); 
  vector<double>tof_face;
  geom->Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z",
	    tof_face);
  vector<double>tof_plane;  
  geom->Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='0']", tof_plane);
  dTOFz=tof_face[2]+tof_plane[2]; 
  geom->Get("//composition[@name='ForwardTOF']/posXYZ[@volume='forwardTOF']/@X_Y_Z/plane[@value='1']", tof_plane);
  dTOFz+=tof_face[2]+tof_plane[2];
  dTOFz*=0.5;  // mid plane between tof planes
  
  // Get start counter geometry;
  if (geom->GetStartCounterGeom(sc_pos,sc_norm)){
    // Create vector of direction vectors in scintillator planes
    for (int i=0;i<30;i++){
      vector<DVector3>temp;
      for (unsigned int j=0;j<sc_pos[i].size()-1;j++){
	double dx=sc_pos[i][j+1].x()-sc_pos[i][j].x();
	double dy=sc_pos[i][j+1].y()-sc_pos[i][j].y();
	double dz=sc_pos[i][j+1].z()-sc_pos[i][j].z();
	temp.push_back(DVector3(dx/dz,dy/dz,1.));
      }
      sc_dir.push_back(temp);
    }
    SC_END_NOSE_Z=sc_pos[0][12].z();
    SC_BARREL_R=sc_pos[0][0].Perp();
    SC_PHI_SECTOR1=sc_pos[0][0].Phi();
  }

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
jerror_t DTrackTimeBased_factory_StraightLine::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // Get wire-based tracks
  vector<const DTrackWireBased*> tracks;
  loop->Get(tracks);

  // Get hits
  vector<const DCDCTrackHit *>cdchits;
  loop->Get(cdchits);
  vector<const DFDCPseudo *>fdchits;
  loop->Get(fdchits);

  // Copy wire-based results 
  for (unsigned int i=0;i<tracks.size();i++){
    // Reset the fitter
    fitter->Reset();
    fitter->SetFitType(DTrackFitter::kTimeBased); 

    const DTrackWireBased *track = tracks[i];
    DVector3 pos=track->position();
    DVector3 dir=track->momentum();
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
      DTrackTimeBased *timebased_track = new DTrackTimeBased();
      timebased_track->candidateid=track->candidateid;
      *static_cast<DTrackingData*>(timebased_track) = fitter->GetFitParameters();
      timebased_track->chisq = fitter->GetChisq();
      timebased_track->Ndof = fitter->GetNdof();
      timebased_track->setPID(PiPlus);
      timebased_track->FOM = TMath::Prob(timebased_track->chisq, timebased_track->Ndof);
      timebased_track->pulls =std::move(fitter->GetPulls());
      timebased_track->extrapolations=std::move(fitter->GetExtrapolations());
      timebased_track->IsSmoothed = fitter->GetIsSmoothed();
      
      // Add hits used as associated objects
      vector<const DCDCTrackHit*> cdchits_on_track = fitter->GetCDCFitHits();
      vector<const DFDCPseudo*> fdchits_on_track = fitter->GetFDCFitHits();
      
      for (unsigned int k=0;k<cdchits_on_track.size();k++){
	timebased_track->AddAssociatedObject(cdchits_on_track[k]);
      }
      for (unsigned int k=0;k<fdchits_on_track.size();k++){
	timebased_track->AddAssociatedObject(fdchits_on_track[k]);
      }
      timebased_track->measured_cdc_hits_on_track = cdchits_on_track.size();
      timebased_track->measured_fdc_hits_on_track = fdchits_on_track.size();
      
      timebased_track->AddAssociatedObject(track);
      timebased_track->dCDCRings = dPIDAlgorithm->Get_CDCRingBitPattern(cdchits_on_track);
      timebased_track->dFDCPlanes = dPIDAlgorithm->Get_FDCPlaneBitPattern(fdchits_on_track);
      
      // TODO: figure out the potential hits on straight line tracks
      timebased_track->potential_cdc_hits_on_track = 0;
      timebased_track->potential_fdc_hits_on_track = 0;
      
      _data.push_back(timebased_track);
      
    }
  }

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::fini(void)
{
  return NOERROR;
}

