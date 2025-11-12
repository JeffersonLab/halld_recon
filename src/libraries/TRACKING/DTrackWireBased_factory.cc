// $Id: DTrackWireBased_factory.cc 5612 2009-10-15 20:51:25Z staylor $
//
//    File: DTrackWireBased_factory.cc
// Created: Wed Sep  3 09:33:40 EDT 2008
// Creator: davidl (on Darwin harriet.jlab.org 8.11.1 i386)
//


#include <iostream>
#include <iomanip>
#include <set>
#include <cmath>
#include <mutex>
using namespace std;

#include "DTrackWireBased_factory.h"
#include <TRACKING/DTrackCandidate.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <BCAL/DBCALPoint.h>
#include <DANA/DObjectID.h>
#include <SplitString.h>

#include <TROOT.h>


//------------------
// CDCSortByRincreasing
//------------------
bool CDCSortByRincreasing(const DCDCTrackHit* const &hit1, const DCDCTrackHit* const &hit2) {
   // use the ring number to sort by R(decreasing) and then straw(increasing)
   if(hit1->wire->ring == hit2->wire->ring){
      return hit1->wire->straw < hit2->wire->straw;
   }
   return hit1->wire->ring < hit2->wire->ring;
}

//------------------
// FDCSortByZincreasing
//------------------
bool FDCSortByZincreasing(const DFDCPseudo* const &hit1, const DFDCPseudo* const &hit2) {
   // use the layer number to sort by Z(decreasing) and then wire(increasing)
   if(hit1->wire->layer == hit2->wire->layer){
      return hit1->wire->wire < hit2->wire->wire;
   }
   return hit1->wire->layer < hit2->wire->layer;
}

//------------------
// TRDSortByZincreasing
//------------------
bool TRDSortByZincreasing(const DTRDPoint* const &hit1, const DTRDPoint* const &hit2) {

   return hit1->z < hit2->z;
}

//------------------
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

bool DTrackWireBased_cmp(DTrackWireBased *a,DTrackWireBased *b){
  if (a->candidateid==b->candidateid) return a->mass()<b->mass();
  return a->candidateid<b->candidateid;
}

//------------------
// Init
//------------------
void DTrackWireBased_factory::Init()
{
  fitter = NULL;
  
  //DEBUG_HISTS = true;	
  DEBUG_HISTS = false;
  DEBUG_LEVEL = 0;
  
  auto app = GetApplication();
  
  app->SetDefaultParameter("TRKFIT:DEBUG_LEVEL",DEBUG_LEVEL);
  
  PROTON_MOM_THRESH=0.8; // GeV 
  app->SetDefaultParameter("TRKFIT:PROTON_MOM_THRESH",PROTON_MOM_THRESH);

  BCAL_CUT=3.; // cm^2 
  app->SetDefaultParameter("TRKFIT:BCAL_CUT",BCAL_CUT);

  SC_DPHI_CUT=0.25; // radians
  app->SetDefaultParameter("TRKFIT:SC_DPHI_CUT",SC_DPHI_CUT);

  MIN_BCAL_MATCHES=1;
  app->SetDefaultParameter("TRKFIT:MIN_BCAL_MATCHES",MIN_BCAL_MATCHES);
  
  MIN_FIT_P = 0.050; // GeV
  app->SetDefaultParameter("TRKFIT:MIN_FIT_P", MIN_FIT_P, "Minimum fit momentum in GeV/c for fit to be considered successful");
}

//------------------
// BeginRun
//------------------
void DTrackWireBased_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto run_number = event->GetRunNumber();
  auto app = GetApplication();
  
  auto geo_manager = app->GetService<DGeometryManager>();
  geom = geo_manager->GetDGeometry(run_number);
  vector<vector<DVector3> >sc_pos;
  vector<vector<DVector3> >sc_norm;
  if (geom->GetStartCounterGeom(sc_pos, sc_norm)){
    for (unsigned int i=0;i<30;i++){
      sc_phi.push_back(sc_pos[i][1].Phi());
    }
  }

   // Check for magnetic field
   const DMagneticFieldMap *bfield = geo_manager->GetBfield(run_number);
   dIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField*>(bfield) != NULL);
   
   if(dIsNoFieldFlag){
     //Setting this flag makes it so that JANA does not delete the objects in 
     //mData.  This factory will manage this memory.
     //This is all of these pointers are just copied from the "StraightLine" 
     //factory, and should not be re-deleted.
     SetFactoryFlag(NOT_OBJECT_OWNER);
   }
   else{
     ClearFactoryFlag(NOT_OBJECT_OWNER); //This factory will create it's own obje
   }

   // Get pointer to DTrackFitter object that actually fits a track
   vector<const DTrackFitter *> fitters;
   event->Get(fitters);
   
   if(fitters.size()<1){
      _DBG_<<"Unable to get a DTrackFitter object! NO Charged track fitting will be done!"<<endl;
      return; // RESOURCE_UNAVAILABLE;
   }

   // Drop the const qualifier from the DTrackFitter pointer (I'm surely going to hell for this!)
   fitter = const_cast<DTrackFitter*>(fitters[0]);

   // Warn user if something happened that caused us NOT to get a fitter object pointer
   if(!fitter){
      _DBG_<<"Unable to get a DTrackFitter object! NO Charged track fitting will be done!"<<endl;
      return; // RESOURCE_UNAVAILABLE;
   }
   
   if(DEBUG_HISTS){
      // dapp->Lock();

      // Histograms may already exist. (Another thread may have created them)
      // Try and get pointers to the existing ones.

      // dapp->Unlock();
   }

   // Get the particle ID algorithms
   event->Get(&dPIDAlgorithm);
}

//------------------
// Process
//------------------
void DTrackWireBased_factory::Process(const std::shared_ptr<const JEvent>& event)
{

  std::vector<DTrackWireBased*> data;
  if(!fitter)return; 
  
  if(dIsNoFieldFlag){
    //Clear previous objects: 
    //JANA doesn't do it because NOT_OBJECT_OWNER was set
    //It DID delete them though, in the "StraightLine" factory
    mData.clear();
     
    vector<const DTrackWireBased*> locWireBasedTracks;
    event->Get(locWireBasedTracks, "StraightLine");
    for(size_t loc_i = 0; loc_i < locWireBasedTracks.size(); ++loc_i)
      mData.push_back(const_cast<DTrackWireBased*>(locWireBasedTracks[loc_i]));
    return;
  }

  // Get candidates and hits
  vector<const DTrackCandidate*> candidates;
  event->Get(candidates);
  
  if (candidates.size()==0) return;
  
  // Get SC hits and BCAL points for crude t0 estimate
  vector<const DSCHit*>schits;
  event->Get(schits);
  vector<const DBCALPoint*>bcalpoints;
  event->Get(bcalpoints);
  
  // Loop over candidates
  for(unsigned int i=0; i<candidates.size(); i++){
    const DTrackCandidate *candidate = candidates[i];

    // Skip candidates with momentum below some cutoff
    if (candidate->dMomentum.Mag()<MIN_FIT_P){
      continue;
    }

    // Get crude t0 estimate
    double min_dphi=1e9;
    double phi=candidate->dPosition.Phi();
    double t0=candidate->dMinimumDriftTime;
    unsigned int sc_hit_index=0;
    for (unsigned int j=0;j<schits.size();j++){
      double dphi=sc_phi[schits[j]->sector-1]-phi;
      if (dphi<-M_PI) dphi+=2.*M_PI;
      if (dphi>M_PI) dphi-=2.*M_PI;
      if (fabs(dphi)<min_dphi){
	min_dphi=fabs(dphi);
	sc_hit_index=j;
      }
    }
    DetectorSystem_t t0_detector=candidate->dDetector;
    if (min_dphi<SC_DPHI_CUT){
      t0_detector=SYS_START;
      t0=schits[sc_hit_index]->t;
    }
    else {
      // circle parameters from the candidate
      double xc=candidate->xc;
      double yc=candidate->yc;
      double rc=candidate->rc;
      double rc2=rc*rc;
      double xc2=xc*xc;
      double yc2=yc*yc;
      double xc2_plus_yc2=xc2+yc2;
      double scale=1./(2.*xc2_plus_yc2);
      // loop over BCAL points looking for matches
      vector<const DBCALPoint *>matched_points;
      for (unsigned int k=0;k<bcalpoints.size();k++){
	const DBCALPoint *point=bcalpoints[k];
	double phi_b=point->phi();
	double r_b=point->r();
	double r2=r_b*r_b;
	double A=r2+xc2_plus_yc2-rc2;
	double B=4*r2*xc2_plus_yc2-A*A;
	if (B<0) continue;
	
	double sqrtB=sqrt(B);
	double my_xplus=(xc*A+yc*sqrtB)*scale;
	double my_xminus=(xc*A-yc*sqrtB)*scale;
	double my_yplus=(yc*A-xc*sqrtB)*scale;
	double my_yminus=(yc*A+xc*sqrtB)*scale;
	
	double x_b=r_b*cos(phi_b);
	double y_b=r_b*sin(phi_b);
	double dx_b=my_xplus-x_b;
	double dy_b=my_yplus-y_b;
	double d2min=dx_b*dx_b+dy_b*dy_b;
	if (d2min<BCAL_CUT) matched_points.push_back(point);
	dx_b=my_xminus-x_b;
	dy_b=my_yminus-y_b;
	d2min=dx_b*dx_b+dy_b*dy_b;
	if (d2min<BCAL_CUT) matched_points.push_back(point);
      }
      if (matched_points.size()>=MIN_BCAL_MATCHES){
	t0_detector=SYS_BCAL;
	t0=matched_points[0]->t();
      }
    }
      
    DoFit(i,candidate,ParticleMass(PiPlus),t0,t0_detector);
    // Only do fit for proton mass hypothesis for low momentum particles
    if (candidate->dMomentum.Mag()<PROTON_MOM_THRESH){
      DoFit(i,candidate,ParticleMass(Proton),t0,t0_detector);
    }
  }
  
  // Filter out duplicate tracks
  FilterDuplicates();

  return;
}


//------------------
// EndRun
//------------------
void DTrackWireBased_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DTrackWireBased_factory::Finish()
{
}

//------------------
// FilterDuplicates
//------------------
void DTrackWireBased_factory::FilterDuplicates(void)
{
   /// Look through all current DTrackWireBased objects and remove any
   /// that have all of their hits in common with another track

   if(mData.size()==0)return;

   if(DEBUG_LEVEL>2)_DBG_<<"Looking for clones of wire-based tracks ..."<<endl;

   set<unsigned int> indexes_to_delete;
   for(unsigned int i=0; i<mData.size()-1; i++){
      DTrackWireBased *dtrack1 = mData[i];

      auto cdchits1 = dtrack1->Get<DCDCTrackHit>();
	  auto fdchits1 = dtrack1->Get<DFDCPseudo>();

      oid_t cand1=dtrack1->candidateid;
      for(unsigned int j=i+1; j<mData.size(); j++){
         DTrackWireBased *dtrack2 = mData[j];
         if (dtrack2->candidateid==cand1) continue;
         if (dtrack2->mass() != dtrack1->mass())continue;

         auto cdchits2 = dtrack2->Get<DCDCTrackHit>();
         auto fdchits2 = dtrack2->Get<DFDCPseudo>();

         // Count number of cdc and fdc hits in common
         unsigned int Ncdc = count_common_members(cdchits1, cdchits2);
         unsigned int Nfdc = count_common_members(fdchits1, fdchits2);
         unsigned int total = Ncdc + Nfdc;

         if (total==0) continue;
         if(Ncdc!=cdchits1.size() && Ncdc!=cdchits2.size())continue;
         if(Nfdc!=fdchits1.size() && Nfdc!=fdchits2.size())continue;

         unsigned int total1 = cdchits1.size()+fdchits1.size();
         unsigned int total2 = cdchits2.size()+fdchits2.size();
         if(total!=total1 && total!=total2)continue;

         if(total1<total2){
            // The two track candidates actually correspond to 
            // a single track.  Set the candidate id for this 
            // track to the candidate id from the clone match to 
            // prevent multiple clone tracks confusing matters 
            // at a later stage of the reconstruction...
            mData[j]->candidateid=cand1;
            indexes_to_delete.insert(i);
         }else{
            indexes_to_delete.insert(j);
         }

      }
   }

   if(DEBUG_LEVEL>2)_DBG_<<"Found "<<indexes_to_delete.size()<<" wire-based clones"<<endl;

   // Return now if we're keeping everyone
   if(indexes_to_delete.size()==0)return;

   // Copy pointers that we want to keep to a new container and delete
   // the clone objects
   vector<DTrackWireBased*> newmData;
   for(unsigned int i=0; i<mData.size(); i++){
      if(indexes_to_delete.find(i)==indexes_to_delete.end()){
         newmData.push_back(mData[i]);
      }else{
         delete mData[i];
         if(DEBUG_LEVEL>1)_DBG_<<"Deleting clone wire-based track "<<i<<endl;
      }
   }	
   mData = newmData;
}

// Routine to do the fit and fill the list of wire-based tracks
void DTrackWireBased_factory::DoFit(unsigned int c_id,
				    const DTrackCandidate *candidate,
				    double mass,
				    double t0,DetectorSystem_t t0_detector){
   // Get the hits from the candidate
  vector<const DFDCPseudo*>myfdchits=candidate->fdchits;
  vector<const DCDCTrackHit *>mycdchits=candidate->cdchits;
  
  // Do the fit
  fitter->Reset();
  fitter->SetFitType(DTrackFitter::kWireBased);	
  
  fitter->AddHits(myfdchits);
  fitter->AddHits(mycdchits);
  
  DTrackFitter::fit_status_t status=fitter->FitTrack(candidate->dPosition,
						     candidate->dMomentum,
						     candidate->dCharge,
						     mass,t0,t0_detector);
  if (status==DTrackFitter::fit_status_t::kFitSuccess && fitter->GetChisq()>0){
    // Make a new wire-based track
    DTrackWireBased *track = new DTrackWireBased();
    *static_cast<DTrackingData*>(track) = fitter->GetFitParameters();
    
    track->chisq = fitter->GetChisq();
    track->Ndof = fitter->GetNdof();
    track->FOM = TMath::Prob(track->chisq, track->Ndof);
    track->pulls =std::move(fitter->GetPulls()); 
    track->extrapolations=std::move(fitter->GetExtrapolations());
    track->candidateid = c_id+1;

    // Add hits used as associated objects
    vector<const DCDCTrackHit*> cdchits = fitter->GetCDCFitHits();
    vector<const DFDCPseudo*> fdchits = fitter->GetFDCFitHits();
    sort(cdchits.begin(), cdchits.end(), CDCSortByRincreasing);
    sort(fdchits.begin(), fdchits.end(), FDCSortByZincreasing);
    for(unsigned int m=0; m<cdchits.size(); m++)track->AddAssociatedObject(cdchits[m]);
    for(unsigned int m=0; m<fdchits.size(); m++)track->AddAssociatedObject(fdchits[m]);
    
    // Set CDC ring & FDC plane hit patterns before candidate tracks are associated
    vector<const DCDCTrackHit*> tempCDCTrackHits = track->Get<DCDCTrackHit>();
    vector<const DFDCPseudo*> tempFDCPseudos = track->Get<DFDCPseudo>();
    
    track->dCDCRings = dPIDAlgorithm->Get_CDCRingBitPattern(tempCDCTrackHits);
    track->dFDCPlanes = dPIDAlgorithm->Get_FDCPlaneBitPattern(tempFDCPseudos);
    
    // Add DTrackCandidate as associated object
    track->AddAssociatedObject(candidate);
    
    Insert(track);
  }
}
