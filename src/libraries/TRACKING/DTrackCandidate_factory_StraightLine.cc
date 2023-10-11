// $Id$
//
//    File: DTrackCandidate_factory_StraightLine.cc
// Created: Fri Aug 15 09:14:04 EDT 2014
// Creator: staylor (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include <map>

#include "DTrackCandidate_factory_StraightLine.h"

//------------------
// Init
//------------------
void DTrackCandidate_factory_StraightLine::Init()
{
}

//------------------
// BeginRun
//------------------
void DTrackCandidate_factory_StraightLine::BeginRun(const std::shared_ptr<const JEvent>& event)
{
   auto app = GetApplication();

   COSMICS=false;
   app->SetDefaultParameter("TRKFIND:COSMICS",COSMICS);

   CHI2CUT = 15.0; 
   app->SetDefaultParameter("TRKFIT:CHI2CUT",CHI2CUT);    

   DO_PRUNING = 1;
   app->SetDefaultParameter("TRKFIT:DO_PRUNING",DO_PRUNING);

   DEBUG_HISTS=false;
   app->SetDefaultParameter("TRKFIND:DEBUG_HISTS",DEBUG_HISTS);


   USE_FDC_DRIFT_TIMES=true;
   app->SetDefaultParameter("TRKFIT:USE_FDC_DRIFT_TIMES",
         USE_FDC_DRIFT_TIMES);

   PLANE_TO_SKIP=0;
   app->SetDefaultParameter("TRKFIT:PLANE_TO_SKIP",PLANE_TO_SKIP);

   SKIP_CDC=false;
   app->SetDefaultParameter("TRKFIT:SKIP_CDC",SKIP_CDC);

   SKIP_FDC=false;
   app->SetDefaultParameter("TRKFIT:SKIP_FDC",SKIP_FDC);

   // Get pointer to TrackFinder object 
   vector<const DTrackFinder *> finders;
   event->Get(finders);

   if(finders.size()<1){
      _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
      return; // RESOURCE_UNAVAILABLE;
   }

   // Drop the const qualifier from the DTrackFinder pointer
   finder = const_cast<DTrackFinder*>(finders[0]);

   if (DEBUG_HISTS){
      Hvres=(TH2F *)gROOT->FindObject("Hvres");
      if (!Hvres) Hvres=new TH2F("Hvres","Residual along wire",100,-0.25,0.25,24,0.5,24.5);
      hFDCOccTrkFind=new TH1I("Occ form track finding", "Occ per plane", 24,0.5,24.5);
      hFDCOccTrkFit=new TH1I("Occ form track fitting", "Occ per plane", 24,0.5,24.5);
      hFDCOccTrkSmooth=new TH1I("Occ form track smoothing", "Occ per plane", 24,0.5,24.5);
   }

   // TMatrix pool
   dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>();
   dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 50);
}

//------------------
// Process
//------------------
void DTrackCandidate_factory_StraightLine::Process(const std::shared_ptr<const JEvent>& event)
{

   // Reset the track finder
   finder->Reset();

   vector<const DCDCTrackHit*>cdcs;
   vector<const DFDCPseudo*>pseudos;
   event->Get(cdcs);
   event->Get(pseudos);

   set<unsigned int> used_cdc;

   // Look for tracks in the FDC.
   if(!SKIP_FDC){
     if (pseudos.size()>4){
       for (size_t i=0;i<pseudos.size();i++) finder->AddHit(pseudos[i]);
       finder->FindFDCSegments();
       finder->LinkFDCSegments();
       
       // Get the list of linked segments and fit the hits to lines
       const vector<DTrackFinder::fdc_segment_t>tracks=finder->GetFDCTracks();
       for (size_t i=0;i<tracks.size();i++){
	 // Create new track candidate
	 DTrackCandidate *cand = new DTrackCandidate;
	 cand->setPID(PiPlus);
	 
	 // Add hits as associated objects
	 vector<const DFDCPseudo *>hits=tracks[i].hits;
	 for (unsigned int k=0;k<hits.size();k++){
	   cand->AddAssociatedObject(hits[k]);
	   if(DEBUG_HISTS){
	     hFDCOccTrkFind->Fill(hits[k]->wire->layer);
	   }
	 }

	 // Set position and momentum
	 double tx=tracks[i].S(state_tx),ty=tracks[i].S(state_ty);
	 double phi=atan2(ty,tx);
	 double tanl=1./sqrt(tx*tx+ty*ty);
	 double pt=cos(atan(tanl)); // arbitrary magnitude...    
	 cand->setMomentum(DVector3(pt*cos(phi),pt*sin(phi),pt*tanl));

	 DVector3 pos,origin,dir(0,0,1.);
	 finder->FindDoca(0,tracks[i].S,dir,origin,&pos);
	 cand->setPosition(pos);
	 
	 Insert(cand);

         }
      }
   }

   if(!SKIP_CDC){
      if (cdcs.size()>4){
         for (size_t i=0;i<cdcs.size();i++) {
            // If the CDC hit had not been grabbed by the FDC fit, try to find CDC only tracks.
            if(used_cdc.find(i) == used_cdc.end()) finder->AddHit(cdcs[i]);
         }
         finder->FindAxialSegments();
         finder->LinkCDCSegments();

         // Get the list of linked segments and fit the hits to lines
         const vector<DTrackFinder::cdc_track_t>tracks=finder->GetCDCTracks();
         for (size_t i=0;i<tracks.size();i++){
	   // Create new track candidate
	   DTrackCandidate *cand = new DTrackCandidate;
	   cand->setPID(PiPlus);
	   
	   // Add hits as associated objects
	   // list of axial and stereo hits for this track
	   vector<const DCDCTrackHit *>hits=tracks[i].axial_hits;
	   for (unsigned int k=0;k<hits.size();k++){
	     cand->AddAssociatedObject(hits[k]);
	   }
	   hits=tracks[i].stereo_hits; 
	   for (unsigned int k=0;k<hits.size();k++){
	     cand->AddAssociatedObject(hits[k]);
	   }
	
	   // state vector
	   DMatrix4x1 S(tracks[i].S);
	   double x=S(state_x);
	   double y=S(state_y);
	   double z=tracks[i].z;
	   double tx=S(state_tx),ty=S(state_ty);
	  
	   if (COSMICS==false){
	     double phi=atan2(ty,tx);
	     double tanl=1./sqrt(tx*tx+ty*ty);
	     double phi_diff=phi-hits[0]->wire->origin.Phi()-M_PI;
	     if (phi_diff<-M_PI) phi_diff+=2.*M_PI;
	     if (phi_diff> M_PI) phi_diff-=2.*M_PI;
	     if (fabs(phi_diff)<M_PI_2){
	       phi+=M_PI;
	     }
	     double pt=cos(atan(tanl)); // set magnitude to 1.
	     DVector3 tdir(pt*cos(phi),pt*sin(phi),pt*tanl);
	     cand->setMomentum(tdir);

	     DVector3 tpos(x,y,z),origin,dir(0.,0.,1.),pos;
	     finder->FindDoca(tpos,tdir,origin,dir,&pos);
	     cand->setPosition(pos);
	   }
	   else{
	     DVector3 mom(tx,ty,1.);
	     double delta_y=-1.;
	     double delta_z=delta_y/ty;
	     // Go slightly beyond the first hit in y
	     x+=tx*delta_z;
	     y+=delta_y;
	     z+=delta_z;

	     mom.SetMag(1.);
	     cand->setMomentum(mom);
	     cand->setPosition(DVector3(x,y,z));
	   }
	 
	   Insert(cand);
         }
      }
   }

   // Set CDC ring & FDC plane hit patterns
   for(size_t loc_i = 0; loc_i < mData.size(); ++loc_i)
   {
      vector<const DCDCTrackHit*> locCDCTrackHits;
      mData[loc_i]->Get(locCDCTrackHits);

      vector<const DFDCPseudo*> locFDCPseudos;
      mData[loc_i]->Get(locFDCPseudos);

      mData[loc_i]->dCDCRings = dParticleID->Get_CDCRingBitPattern(locCDCTrackHits);
      mData[loc_i]->dFDCPlanes = dParticleID->Get_FDCPlaneBitPattern(locFDCPseudos);
   }
}

//------------------
// EndRun
//------------------
void DTrackCandidate_factory_StraightLine::EndRun()
{
}

//------------------
// Finish
//------------------
void DTrackCandidate_factory_StraightLine::Finish()
{
}
