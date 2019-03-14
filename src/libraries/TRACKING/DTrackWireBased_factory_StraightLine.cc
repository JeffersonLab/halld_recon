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
jerror_t DTrackWireBased_factory_StraightLine::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // Get candidates
   vector<const DTrackCandidate*> candidates;
   loop->Get(candidates);

   // Get hits
   vector<const DCDCTrackHit *>cdchits;
   loop->Get(cdchits);
  
   for (unsigned int i=0;i<candidates.size();i++){
     // Reset the fitter
     fitter->Reset();

     const DTrackCandidate *cand=candidates[i];
     DVector3 pos=cand->position();
     DVector3 dir=cand->momentum();
     for (unsigned int j=0;j<cdchits.size();j++){
       double d=finder->FindDoca(pos,dir,cdchits[j]->wire->origin,
				 cdchits[j]->wire->udir);
       if (d<CDC_MATCH_CUT) fitter->AddHit(cdchits[j]);
     }

     // Fit the track using the list of hits we gathered above
     if (fitter->FitTrack(pos,dir,1.,0.,0.)==DTrackFitter::kFitSuccess){       
       // Make a new wire-based track
       DTrackWireBased *track = new DTrackWireBased(); //share the memory: isn't changed below
       *static_cast<DTrackingData*>(track) = fitter->GetFitParameters();

       track->chisq = fitter->GetChisq();
       track->Ndof = fitter->GetNdof();
       track->setPID(PiPlus);
       track->FOM = TMath::Prob(track->chisq, track->Ndof);
       track->pulls =std::move(fitter->GetPulls());        
       track->IsSmoothed = fitter->GetIsSmoothed();
     
       // candidate id
       track->candidateid=i+1;

       /*
	 for (unsigned int k=0;k<cdchits.size();k++){
       track->AddAssociatedObject(cdchits[k]);
       }
       for (unsigned int k=0;k<fdchits.size();k++){
       track->AddAssociatedObject(fdchits[k]);
       }
       track->dCDCRings = dPIDAlgorithm->Get_CDCRingBitPattern(cdchits);
       track->dFDCPlanes = dPIDAlgorithm->Get_FDCPlaneBitPattern(fdchits);
       */

       // Create the extrapolation vectors
       vector<DTrackFitter::Extrapolation_t>myvector;
       track->extrapolations.emplace(SYS_BCAL,myvector);
       track->extrapolations.emplace(SYS_TOF,myvector);
       track->extrapolations.emplace(SYS_FCAL,myvector);
       track->extrapolations.emplace(SYS_FDC,myvector);
       track->extrapolations.emplace(SYS_CDC,myvector);
       track->extrapolations.emplace(SYS_DIRC,myvector);
       track->extrapolations.emplace(SYS_START,myvector);	
       
       // Extrapolate to TOF
       DVector3 pos0=track->position();
       double z0=track->position().z();
       double z=z0;
       double uz=dir.z();
       // Extrapolate to DIRC
       DVector3 diff=((dDIRCz-z0)/uz)*dir;
       pos=pos0+diff;
       double s=diff.Mag();
       double t=s/29.98;
       track->extrapolations[SYS_DIRC].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
       // Extrapolate to TOF
       diff=((dTOFz-z0)/uz)*dir;
       pos=pos0+diff;
       s=diff.Mag();
       t=s/29.98;
       track->extrapolations[SYS_TOF].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));	 
       // Extrapolate to FCAL
       diff=((dFCALz-z0)/uz)*dir;
       pos=pos0+diff;
       s=diff.Mag();
       t=s/29.98;
       track->extrapolations[SYS_FCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));  
       // extrapolate to exit of FCAL
       diff=((dFCALz+45.-z0)/uz)*dir;
       pos=pos0+diff;
       s=diff.Mag();
       t=s/29.98;
       track->extrapolations[SYS_FCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
       
       // Extrapolate to Start Counter and BCAL
       double R=pos0.Perp();
       diff.SetMag(0.);
       while (R<89.0 && z>17. && z<410.){
	 diff+=(1./dir.z())*dir;
	 pos=pos0+diff;
	 R=pos.Perp();
	 z=pos.z();
	 s=diff.Mag();
	 t=s/29.98;
	 //	   printf("R %f z %f\n",R,z);
	 // start counter
	 if (sc_pos.empty()==false && R<SC_BARREL_R && z<SC_END_NOSE_Z){
	   double d_old=1000.,d=1000.;
	 unsigned int index=0;
	 for (unsigned int m=0;m<12;m++){
	   double dphi=pos.Phi()-SC_PHI_SECTOR1;
	   if (dphi<0) dphi+=2.*M_PI;
	   index=int(floor(dphi/(2.*M_PI/30.)));
	   if (index>29) index=0;
	   d=sc_norm[index][m].Dot(pos-sc_pos[index][m]);
	   if (d*d_old<0){ // break if we cross the current plane  
	     // Find the new distance to the start counter (which could 
	     // now be to a plane in the one adjacent to the one before the
	     // step...)
	     int count=0;
	     while (fabs(d)>0.05 && count<20){ 
	       // Find the index for the nearest start counter paddle
	       double dphi=pos.Phi()-SC_PHI_SECTOR1;
	       if (dphi<0) dphi+=2.*M_PI;
	       index=int(floor(dphi/(2.*M_PI/30.)));
	       d=sc_norm[index][m].Dot(pos-sc_pos[index][m]);
	       pos+=d*dir;
	       count++;
	     }
	     track->extrapolations[SYS_START].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
	     break;
	       }
	   d_old=d;
	 }
	 }
	 if (R>64.){	 
	   track->extrapolations[SYS_BCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
	 }
       }
       
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

