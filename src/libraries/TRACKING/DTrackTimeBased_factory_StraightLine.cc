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

#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include <TOF/DTOFPoint.h>
#include <START_COUNTER/DSCHit.h>



bool DTrackTimeBased_straightline_cmp(DTrackTimeBased *a,DTrackTimeBased *b){
  return a->candidateid<b->candidateid;
}


// count_common_members
//------------------
template<typename T>
static unsigned int count_common_members(vector<T> &a, vector<T> &b)
{
	unsigned int n=0;
	for(unsigned int i=0; i<a.size(); i++){
		for(unsigned int j=0; j<b.size(); j++){
			if(a[i]==b[j])n++;
		}
	}
	
	return n;
}

//------------------
// init
//------------------
void DTrackTimeBased_factory_StraightLine::Init()
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
void DTrackTimeBased_factory_StraightLine::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // Get the geometry
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto geom = geo_manager->GetDGeometry(runnumber);

  // Get the particle ID algorithms
  event->GetSingle(dPIDAlgorithm);
  
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
void DTrackTimeBased_factory_StraightLine::Process(const std::shared_ptr<const JEvent>& event)
{
  // Get wire-based tracks
  vector<const DTrackWireBased*> tracks;
  event->Get(tracks);

  // Get hits
  vector<const DCDCTrackHit *>cdchits;
  event->Get(cdchits);
  vector<const DFDCPseudo *>fdchits;
  event->Get(fdchits);
  
  // get start counter hits
  vector<const DSCHit*>sc_hits;
  event->Get(sc_hits);
  
  // Get TOF points
  vector<const DTOFPoint*> tof_points;
  event->Get(tof_points);

  // Get BCAL and FCAL showers
  vector<const DBCALShower*>bcal_showers;
  event->Get(bcal_showers);

  vector<const DFCALShower*>fcal_showers;
  event->Get(fcal_showers);

  // Start with wire-based results, refit with drift times 
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
    
    // Estimate t0 for this track
    double t0=0.;
    DetectorSystem_t t0_detector=SYS_NULL;
    GetStartTime(track,sc_hits,tof_points,bcal_showers,fcal_showers,t0,
		 t0_detector);

    // Fit the track using the list of hits we gathered above
    if (fitter->FitTrack(pos,dir,1.,0.,t0,t0_detector)==DTrackFitter::kFitSuccess){
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
      
      Insert(timebased_track);
      
    }
  }

  // Filter out duplicate tracks
  FilterDuplicates();
}

//------------------
// EndRun
//------------------
void DTrackTimeBased_factory_StraightLine::EndRun()
{
}

//------------------
// Finish
//------------------
void DTrackTimeBased_factory_StraightLine::Finish()
{
}

// Get an estimate for the start time for this track
void 
DTrackTimeBased_factory_StraightLine::GetStartTime(const DTrackWireBased *track,
						   vector<const DSCHit*>&sc_hits,
						   vector<const DTOFPoint*>&tof_points,
						   vector<const DBCALShower*>&bcal_showers,	
						   vector<const DFCALShower*>&fcal_showers,
						   double &t0,DetectorSystem_t &t0_detector) const {
  t0=track->t0();
  t0_detector=track->t0_detector();
  double track_t0=t0;
  double locStartTime = track_t0;  // initial guess from tracking
 
  // Get start time estimate from Start Counter
  if (dPIDAlgorithm->Get_StartTime(track->extrapolations.at(SYS_START),sc_hits,locStartTime)){
    t0=locStartTime;
    t0_detector=SYS_START;
    return;
  }

  // Get start time estimate from TOF
  locStartTime = track_t0;  // initial guess from tracking
  if (dPIDAlgorithm->Get_StartTime(track->extrapolations.at(SYS_TOF),tof_points,locStartTime)){
    t0=locStartTime;
    t0_detector=SYS_TOF;
    return;
  }

  // Get start time estimate from FCAL
  locStartTime = track_t0;  // Initial guess from tracking
  if (dPIDAlgorithm->Get_StartTime(track->extrapolations.at(SYS_FCAL),fcal_showers,locStartTime)){
    t0=locStartTime;
    t0_detector=SYS_FCAL;
    return;
  }

  // Get start time estimate from BCAL
  locStartTime=track_t0;
  if (dPIDAlgorithm->Get_StartTime(track->extrapolations.at(SYS_BCAL),bcal_showers,locStartTime)){
    t0=locStartTime;
    t0_detector=SYS_BCAL;
    return;
  }
}

//------------------
// FilterDuplicates
//------------------
void DTrackTimeBased_factory_StraightLine::FilterDuplicates(void)
{
  /// Look through all current DTrackTimeBased objects and remove any
  /// that have most of their hits in common with another track
	
  if(mData.size()==0)return;
  
  if(DEBUG_LEVEL>2) _DBG_<<"Looking for clones of time-based tracks ..."<<endl;
    
  vector<unsigned int> candidates_to_keep;
  vector<unsigned int> candidates_to_delete;
  for(unsigned int i=0; i<mData.size()-1; i++){
    DTrackTimeBased *dtrack1 = mData[i];
    
    vector<const DCDCTrackHit*> cdchits1;
    vector<const DFDCPseudo*> fdchits1;
    dtrack1->Get(cdchits1);
    dtrack1->Get(fdchits1);
    // Total number of hits in this candidate
    unsigned int num_cdc1=cdchits1.size();
    unsigned int num_fdc1=fdchits1.size();
    unsigned int total1 = num_cdc1+num_fdc1;
    
    oid_t cand1=dtrack1->candidateid;
    for(unsigned int j=i+1; j<mData.size(); j++){
      DTrackTimeBased *dtrack2 = mData[j];
      
      vector<const DCDCTrackHit*> cdchits2;
      vector<const DFDCPseudo*> fdchits2;
      dtrack2->Get(cdchits2);
      dtrack2->Get(fdchits2);
      
      // Total number of hits in this candidate
      unsigned int num_cdc2=cdchits2.size();
      unsigned int num_fdc2=fdchits2.size();
      unsigned int total2 = num_cdc2+num_fdc2;
      
      // Count number of cdc and fdc hits in common
      unsigned int Ncdc = count_common_members(cdchits1, cdchits2);
      unsigned int Nfdc = count_common_members(fdchits1, fdchits2);
      
      if(DEBUG_LEVEL>3){
	_DBG_<<"cand1:"<<cand1<<" cand2:"<<dtrack2->candidateid<<endl;
	_DBG_<<"   Ncdc="<<Ncdc<<" num_cdc1="<<num_cdc1<<" num_cdc2="<<num_cdc2<<endl;
	_DBG_<<"   Nfdc="<<Nfdc<<" num_fdc1="<<num_fdc1<<" num_fdc2="<<num_fdc2<<endl;
      }
      unsigned int total = Ncdc + Nfdc;	
      // If the tracks share at most one hit, consider them
      // to be separate tracks
      if (total<=1) continue;
      
      // Deal with the case where there are cdc hits in 
      // common between the tracks but there were no fdc 
      // hits used in one of the tracks.
      if (Ncdc>0 && (num_fdc1>0 || num_fdc2>0) 
	  && (num_fdc1*num_fdc2)==0) continue;
      
      // Deal with the case where there are fdc hits in
      // common between the tracks but no cdc hits used in 
      // one of the tracks.			
      if (Nfdc>0 && (num_cdc1>0 || num_cdc2>0)
	  && (num_cdc1*num_cdc2)==0) continue;
      
      // Look for tracks with many common hits in the CDC
      if (num_cdc1>0 && num_cdc2>0){
	if (double(Ncdc)/double(num_cdc1)<0.9) continue;
	if (double(Ncdc)/double(num_cdc2)<0.9) continue;
      }
      // Look for tracks with many common hits in the FDC
      if (num_fdc1>0 && num_fdc2>0){
	if (double(Nfdc)/double(num_fdc1)<0.9) continue;
	if (double(Nfdc)/double(num_fdc2)<0.9) continue;
      }
      
      if(total1<total2){
	candidates_to_delete.push_back(cand1);
	candidates_to_keep.push_back(dtrack2->candidateid);
      } else {
	candidates_to_delete.push_back(dtrack2->candidateid);
	candidates_to_keep.push_back(cand1);
      }
    }
  }

  if(DEBUG_LEVEL>2)
    _DBG_<<"Found "<<candidates_to_delete.size()<<" time-based clones"<<endl;
  
  // Return now if we're keeping everyone
  if(candidates_to_delete.size()==0)return;
  
  // Copy pointers that we want to keep to a new container and delete
  // the clone objects
  vector<DTrackTimeBased*> new_data;
  sort(mData.begin(),mData.end(),DTrackTimeBased_straightline_cmp);
  for (unsigned int i=0;i<mData.size();i++){
    bool keep_track=true;
    for (unsigned int j=0;j<candidates_to_delete.size();j++){
      if (mData[i]->candidateid==candidates_to_delete[j]){
	keep_track=false;
	if(DEBUG_LEVEL>1)
	  {
	    _DBG_<<"Deleting clone time-based fitted result "<<i<<endl;
	  }
	break;
      }
    }
    if (keep_track){
      new_data.push_back(mData[i]);
    }
    else delete mData[i];
  }
  mData = new_data;
}

