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
using namespace jana;

//------------------
// init
//------------------
jerror_t DTrackTimeBased_factory_StraightLine::init(void)
{
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

  // Copy wire-based results 
  for (unsigned int i=0;i<tracks.size();i++){
    const DTrackWireBased *track = tracks[i];
    
    // Copy over the results of the wire-based fit to DTrackTimeBased
    DTrackTimeBased *timebased_track = new DTrackTimeBased(); //share the memory (isn't changed below)
      *static_cast<DTrackingData*>(timebased_track) = *static_cast<const DTrackingData*>(track);
      
      timebased_track->chisq = track->chisq;
      timebased_track->Ndof = track->Ndof;
      timebased_track->FOM =  TMath::Prob(timebased_track->chisq, timebased_track->Ndof);
      timebased_track->pulls = track->pulls;
      timebased_track->extrapolations = track->extrapolations;
      timebased_track->trackid = track->id;
      timebased_track->candidateid=track->candidateid;
      timebased_track->IsSmoothed = track->IsSmoothed;
      timebased_track->flags=DTrackTimeBased::FLAG__USED_WIREBASED_FIT;
      
      // Lists of hits used in the previous pass
      vector<const DCDCTrackHit *>cdchits;
      track->GetT(cdchits);
      vector<const DFDCPseudo *>fdchits;
      track->GetT(fdchits);
      
      for (unsigned int k=0;k<cdchits.size();k++){
	timebased_track->AddAssociatedObject(cdchits[k]);
      }
      for (unsigned int k=0;k<fdchits.size();k++){
	timebased_track->AddAssociatedObject(fdchits[k]);
      }
      timebased_track->measured_cdc_hits_on_track = cdchits.size();
      timebased_track->measured_fdc_hits_on_track = fdchits.size();
      
      timebased_track->AddAssociatedObject(track);
      timebased_track->dCDCRings = dPIDAlgorithm->Get_CDCRingBitPattern(cdchits);
      timebased_track->dFDCPlanes = dPIDAlgorithm->Get_FDCPlaneBitPattern(fdchits);
      
      // TODO: figure out the potential hits on straight line tracks
      timebased_track->potential_cdc_hits_on_track = 0;
      timebased_track->potential_fdc_hits_on_track = 0;
      
      _data.push_back(timebased_track);
      
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

