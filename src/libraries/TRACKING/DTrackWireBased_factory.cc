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

   SKIP_MASS_HYPOTHESES_WIRE_BASED=true; 
   app->SetDefaultParameter("TRKFIT:SKIP_MASS_HYPOTHESES_WIRE_BASED",
			       SKIP_MASS_HYPOTHESES_WIRE_BASED); 
   PROTON_MOM_THRESH=0.8; // GeV 
   app->SetDefaultParameter("TRKFIT:PROTON_MOM_THRESH",
			    PROTON_MOM_THRESH);

   BCAL_CUT=3.; // cm^2 
   app->SetDefaultParameter("TRKFIT:BCAL_CUT",BCAL_CUT);

   SC_DPHI_CUT=0.25; // radians
   app->SetDefaultParameter("TRKFIT:SC_DPHI_CUT",SC_DPHI_CUT);
   
   MIN_BCAL_MATCHES=1;
   app->SetDefaultParameter("TRKFIT:MIN_BCAL_MATCHES",MIN_BCAL_MATCHES);
   

   // Make list of mass hypotheses to use in fit
   vector<int> hypotheses;
   hypotheses.push_back(Positron);
   hypotheses.push_back(PiPlus);
   hypotheses.push_back(KPlus);
   hypotheses.push_back(Proton);
   hypotheses.push_back(Electron);
   hypotheses.push_back(PiMinus);
   hypotheses.push_back(KMinus);
   hypotheses.push_back(AntiProton);
   
   ostringstream locMassStream;
   for(size_t loc_i = 0; loc_i < hypotheses.size(); ++loc_i)
     {
	locMassStream << hypotheses[loc_i];
	if(loc_i != (hypotheses.size() - 1))
	  locMassStream << ",";
     }
   
   string HYPOTHESES = locMassStream.str();
   app->SetDefaultParameter("TRKFIT:HYPOTHESES", HYPOTHESES);

   // Parse MASS_HYPOTHESES strings to make list of masses to try
   hypotheses.clear();
   SplitString(HYPOTHESES, hypotheses, ",");
   for(size_t loc_i = 0; loc_i < hypotheses.size(); ++loc_i)
     {
       if(ParticleCharge(Particle_t(hypotheses[loc_i])) > 0)
	 mass_hypotheses_positive.push_back(hypotheses[loc_i]);
       else if(ParticleCharge(Particle_t(hypotheses[loc_i])) < 0)
	 mass_hypotheses_negative.push_back(hypotheses[loc_i]);
     }
   
   if(mass_hypotheses_positive.empty()){
     static once_flag pwarn_flag;
     call_once(pwarn_flag, [](){
	 jout << jendl;
	 jout << "############# WARNING !! ################ " <<jendl;
	 jout << "There are no mass hypotheses for positive tracks!" << jendl;
	 jout << "Be SURE this is what you really want!" << jendl;
	 jout << "######################################### " <<jendl;
	 jout << jendl;
       });
   }
   if(mass_hypotheses_negative.empty()){
     static once_flag nwarn_flag;
     call_once(nwarn_flag, [](){
	 jout << jendl;
	 jout << "############# WARNING !! ################ " <<jendl;
	 jout << "There are no mass hypotheses for negative tracks!" << jendl;
	 jout << "Be SURE this is what you really want!" << jendl;
	 jout << "######################################### " <<jendl;
	 jout << jendl;
       });
   }
   mNumHypPlus=mass_hypotheses_positive.size();
   mNumHypMinus=mass_hypotheses_negative.size();
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

   // set up reference trajectory
   rt = new DReferenceTrajectory(bfield);
   rt->SetDGeometry(geom);

   // Start counter geometry
   haveStartCounter=false;
   vector<vector<DVector3> >sc_pos;
   vector<vector<DVector3> >sc_norm;
   if (geom->GetStartCounterGeom(sc_pos, sc_norm)){
     for (unsigned int i=0;i<30;i++){
       sc_phi.push_back(sc_pos[i][1].Phi());
     }
     haveStartCounter=true;
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

   USE_HITS_FROM_CANDIDATE=false;
   app->SetDefaultParameter("TRKFIT:USE_HITS_FROM_CANDIDATE",
         USE_HITS_FROM_CANDIDATE);

   MIN_FIT_P = 0.050; // GeV
   app->SetDefaultParameter("TRKFIT:MIN_FIT_P", MIN_FIT_P, "Minimum fit momentum in GeV/c for fit to be considered successful");

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
      
      // Get crude t0 estimate. First try to match to start counter.
      double min_dphi=1e9;
      double phi=candidate->dPosition.Phi();
      double t0=candidate->dMinimumDriftTime;
      unsigned int sc_hit_index=0;
      if (haveStartCounter){
	for (unsigned int j=0;j<schits.size();j++){
	  double dphi=sc_phi[schits[j]->sector-1]-phi;
	  if (dphi<-M_PI) dphi+=2.*M_PI;
	  if (dphi>M_PI) dphi-=2.*M_PI;
	  if (fabs(dphi)<min_dphi){
	    min_dphi=fabs(dphi);
	    sc_hit_index=j;
	  }
	}
      }
      DetectorSystem_t t0_detector=candidate->dDetector;
      if (min_dphi<SC_DPHI_CUT){
	t0_detector=SYS_START;
	t0=schits[sc_hit_index]->t;
      }
      else { // If matching to start counter did not work, try to match to BCAL
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
      
      if (SKIP_MASS_HYPOTHESES_WIRE_BASED){
	rt->Reset();
	rt->q = candidate->dCharge;

	DoFit(i,candidate,rt,event,ParticleMass(PiPlus),t0,t0_detector);
	// Only do fit for proton mass hypothesis for low momentum particles
	if (candidate->dMomentum.Mag()<PROTON_MOM_THRESH){
	  rt->Reset();
	  DoFit(i,candidate,rt,event,ParticleMass(Proton),t0,t0_detector);
	}
      }
      else{
         // Choose list of mass hypotheses based on charge of candidate
         vector<int> mass_hypotheses;
         if(candidate->dCharge<0.0){
            mass_hypotheses = mass_hypotheses_negative;
         }else{
            mass_hypotheses = mass_hypotheses_positive;
         }

         if ((!isfinite(candidate->dMomentum.Mag())) || (!isfinite(candidate->dPosition.Mag())))
            _DBG_ << "Invalid seed data for event "<< event->GetEventNumber() <<"..."<<endl;

         // Loop over potential particle masses
         for(unsigned int j=0; j<mass_hypotheses.size(); j++){
            if(DEBUG_LEVEL>1){_DBG__;_DBG_<<"---- Starting wire based fit with id: "<<mass_hypotheses[j]<<endl;}

	    rt->Reset();
            rt->q = candidate->dCharge;
            DoFit(i,candidate,rt,event,ParticleMass(Particle_t(mass_hypotheses[j])),t0,t0_detector);
         }

      }
   }

   // Filter out duplicate tracks
   FilterDuplicates();

   // Add any missing hypotheses
   if (SKIP_MASS_HYPOTHESES_WIRE_BASED==false){
     InsertMissingHypotheses();
   }

   return;
}


//------------------
// EndRun
//------------------
void DTrackWireBased_factory::EndRun()
{
  if (rt) delete rt;
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

// Routine to find the hits, do the fit, and fill the list of wire-based tracks
void DTrackWireBased_factory::DoFit(unsigned int c_id,
				    const DTrackCandidate *candidate,
				    DReferenceTrajectory *rt,
				    const std::shared_ptr<const JEvent>& event,
				    double mass,double t0,
				    DetectorSystem_t t0_detector){
   // Get the hits from the candidate
  vector<const DFDCPseudo*>myfdchits=candidate->fdchits;
  vector<const DCDCTrackHit *>mycdchits=candidate->cdchits;

   // Do the fit
   DTrackFitter::fit_status_t status = DTrackFitter::kFitNotDone;
   if (USE_HITS_FROM_CANDIDATE) {
      fitter->Reset();
      fitter->SetFitType(DTrackFitter::kWireBased);	

      fitter->AddHits(myfdchits);
      fitter->AddHits(mycdchits);

      status=fitter->FitTrack(candidate->dPosition,candidate->dMomentum,
			      candidate->dCharge,mass,t0,t0_detector);
   }
   else{
     fitter->Reset();
      fitter->SetFitType(DTrackFitter::kWireBased);
      // Swim a reference trajectory using the candidate starting momentum
      // and position
      rt->SetMass(mass);
      //rt->Swim(candidate->dPosition,candidate->dMomentum,candidate->dCharge);
      rt->FastSwimForHitSelection(candidate->dPosition,candidate->dMomentum,candidate->dCharge);

      // Kinematic data
      DKinematicData kd;
      kd.setPID(IDTrack(candidate->dCharge,mass));
      kd.setPosition(candidate->dPosition);
      kd.setMomentum(candidate->dMomentum);
      status=fitter->FindHitsAndFitTrack(kd,rt,event,mass,
					 mycdchits.size()+2*myfdchits.size(),
					 t0,t0_detector);
      if (fitter->GetChisq()<0 || status==DTrackFitter::kFitNotDone){
	if (DEBUG_LEVEL>1)
	  _DBG_ << "Using hits from candidate..." << endl;
         fitter->Reset();
        
         fitter->AddHits(myfdchits);
         fitter->AddHits(mycdchits);

         status=fitter->FitTrack(candidate->dPosition,candidate->dMomentum,
				 candidate->dCharge,mass,t0,t0_detector);
      }
   }

   // if the fit returns chisq=-1, something went terribly wrong... 
   if (fitter->GetChisq()<0){
     status=DTrackFitter::kFitFailed;
   }

   // Check the status of the fit
   switch(status){
      case DTrackFitter::kFitNotDone:
         //_DBG_<<"Fitter returned kFitNotDone. This should never happen!!"<<endl;
      case DTrackFitter::kFitFailed:
         break;
      case DTrackFitter::kFitNoImprovement:	
      case DTrackFitter::kFitSuccess:
         if(!isfinite(fitter->GetFitParameters().position().X())) break;
         {    
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
            break;
         }
      default:
         break;
   }
}

// If the fit failed for certain hypotheses, fill in the gaps using data from
// successful fits for each candidate.
bool DTrackWireBased_factory::InsertMissingHypotheses(void){
  if (mData.size()==0) return false;
  
  // Make sure the tracks are ordered by candidate id
  sort(mData.begin(),mData.end(),DTrackWireBased_cmp);
  
  oid_t old_id=mData[0]->candidateid;
  unsigned int mass_bits=0;
  double q=mData[0]->charge();
  vector<DTrackWireBased*>myhypotheses;
  vector<DTrackWireBased*>tracks_to_add;
  for (size_t i=0;i<mData.size();i++){
    if (mData[i]->candidateid!=old_id){
      int num_hyp=myhypotheses.size();
      if ((q<0 && num_hyp!=mNumHypMinus)||(q>0 && num_hyp!=mNumHypPlus)){ 
	AddMissingTrackHypotheses(mass_bits,tracks_to_add,myhypotheses,q);
      }
      
      // Clear the myhypotheses vector for the next track
      myhypotheses.clear();
      // Reset charge 
      q=mData[i]->charge();	
   
      // Set the bit for this mass hypothesis
      mass_bits = 1<<mData[i]->PID();

      // Add the data to the myhypotheses vector
      myhypotheses.push_back(mData[i]);
    }
    else{
      myhypotheses.push_back(mData[i]);
      
      // Set the bit for this mass hypothesis
      mass_bits |= 1<< mData[i]->PID();
    }
    
    old_id=mData[i]->candidateid;
  }
  // Deal with last track candidate	
  int num_hyp=myhypotheses.size();
  if ((q<0 && num_hyp!=mNumHypMinus)||(q>0 && num_hyp!=mNumHypPlus)){
    AddMissingTrackHypotheses(mass_bits,tracks_to_add,myhypotheses,q);
  }
    
  // Add the new list of tracks to the output list
  if (tracks_to_add.size()>0){
    for (size_t i=0;i<tracks_to_add.size();i++){ 
      mData.push_back(tracks_to_add[i]);
    }
    // Make sure the tracks are ordered by candidate id
    sort(mData.begin(),mData.end(),DTrackWireBased_cmp);
  }

  return true;
}

// Create a track with a mass hypothesis that was not present in the list of 
// fitted tracks from an existing fitted track.
void DTrackWireBased_factory::AddMissingTrackHypothesis(vector<DTrackWireBased*>&tracks_to_add,
				      const DTrackWireBased *src_track,
							double my_mass,
							double q){
  // Create a new wire-based track object
  DTrackWireBased *wirebased_track = new DTrackWireBased();
  *static_cast<DTrackingData*>(wirebased_track) = *static_cast<const DTrackingData*>(src_track);

  // Copy over DKinematicData part from the result of a successful fit
  wirebased_track->setPID(IDTrack(q, my_mass));
  wirebased_track->chisq = src_track->chisq;
  wirebased_track->Ndof = src_track->Ndof;
  wirebased_track->pulls = src_track->pulls;
  wirebased_track->extrapolations = src_track->extrapolations;
  wirebased_track->candidateid=src_track->candidateid;
  wirebased_track->FOM=src_track->FOM;
  wirebased_track->IsSmoothed=src_track->IsSmoothed;
  wirebased_track->dCDCRings=src_track->dCDCRings;
  wirebased_track->dFDCPlanes=src_track->dFDCPlanes;

  // (Partially) compensate for the difference in energy loss between the 
  // source track and a particle of mass my_mass 
  DVector3 position,momentum;
  if (wirebased_track->extrapolations.at(SYS_CDC).size()>0){
    unsigned int index=wirebased_track->extrapolations.at(SYS_CDC).size()-1;
    position=wirebased_track->extrapolations[SYS_CDC][index].position;
    momentum=wirebased_track->extrapolations[SYS_CDC][index].momentum;
  }
  else if (wirebased_track->extrapolations.at(SYS_FDC).size()>0){
    unsigned int index=wirebased_track->extrapolations.at(SYS_FDC).size()-1;
    position=wirebased_track->extrapolations[SYS_FDC][index].position;
    momentum=wirebased_track->extrapolations[SYS_FDC][index].momentum;
  }
  if (momentum.Mag()>0.){
    CorrectForELoss(position,momentum,q,my_mass);
    
    wirebased_track->setMomentum(momentum);
    wirebased_track->setPosition(position);
  }

  // Get the hits used in the fit and add them as associated objects 
  vector<const DCDCTrackHit *>cdchits;
  src_track->GetT(cdchits);
  vector<const DFDCPseudo *>fdchits;
  src_track->GetT(fdchits);

  for(unsigned int m=0; m<fdchits.size(); m++)
    wirebased_track->AddAssociatedObject(fdchits[m]); 
  for(unsigned int m=0; m<cdchits.size(); m++)
    wirebased_track->AddAssociatedObject(cdchits[m]);
   
  tracks_to_add.push_back(wirebased_track);
}

// Use the FastSwim method in DReferenceTrajectory to propagate back to the 
// POCA to the beam line, adding a bit of energy at each step that would have 
// been lost had the particle emerged from the target.
void DTrackWireBased_factory::CorrectForELoss(DVector3 &position,DVector3 &momentum,double q,double my_mass){  
  rt->Reset();
  rt->q = q;
  rt->SetMass(my_mass);
  rt->SetPLossDirection(DReferenceTrajectory::kBackward);
  DVector3 last_pos,last_mom;
  DVector3 origin(0.,0.,65.);
  DVector3 dir(0.,0.,1.);
  rt->FastSwim(position,momentum,last_pos,last_mom,rt->q,origin,dir,300.);   
  position=last_pos;
  momentum=last_mom;   
}


// Fill in all missing hypotheses for a given track candidate
void DTrackWireBased_factory::AddMissingTrackHypotheses(unsigned int mass_bits,
							vector<DTrackWireBased*>&tracks_to_add,
							vector<DTrackWireBased *>&myhypotheses,
							double q){ 
  Particle_t negative_particles[3]={KMinus,PiMinus,Electron};
  Particle_t positive_particles[3]={KPlus,PiPlus,Positron};

  unsigned int last_index=myhypotheses.size()-1;
  if (q>0){
    if ((mass_bits & (1<<Proton))==0){
      AddMissingTrackHypothesis(tracks_to_add,myhypotheses[last_index],
				ParticleMass(Proton),+1.);  
    } 
    for (int i=0;i<3;i++){
      if ((mass_bits & (1<<positive_particles[i]))==0){
	AddMissingTrackHypothesis(tracks_to_add,myhypotheses[0],
				  ParticleMass(positive_particles[i]),+1.); 
      } 
    }    
  }
  else{
    if ((mass_bits & (1<<AntiProton))==0){
      AddMissingTrackHypothesis(tracks_to_add,myhypotheses[last_index],
				ParticleMass(Proton),-1.);  
    } 	
    for (int i=0;i<3;i++){
      if ((mass_bits & (1<<negative_particles[i]))==0){
	AddMissingTrackHypothesis(tracks_to_add,myhypotheses[0],
				  ParticleMass(negative_particles[i]),-1.);  
      } 
    }
  }
} 
