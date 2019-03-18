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
using namespace jana;
#include <JANA/JApplication.h>
#include <JANA/JCalibration.h>
#include <DANA/DApplication.h>
#include "HDGEOMETRY/DGeometry.h"

//------------------
// init
//------------------
jerror_t DTrackCandidate_factory_StraightLine::init(void)
{
   return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DTrackCandidate_factory_StraightLine::brun(jana::JEventLoop *loop, int runnumber)
{

   COSMICS=false;
   gPARMS->SetDefaultParameter("TRKFIND:COSMICS",COSMICS);

   CHI2CUT = 15.0; 
   gPARMS->SetDefaultParameter("TRKFIT:CHI2CUT",CHI2CUT);    

   DO_PRUNING = 1;
   gPARMS->SetDefaultParameter("TRKFIT:DO_PRUNING",DO_PRUNING);

   DEBUG_HISTS=false;
   gPARMS->SetDefaultParameter("TRKFIND:DEBUG_HISTS",DEBUG_HISTS);


   USE_FDC_DRIFT_TIMES=true;
   gPARMS->SetDefaultParameter("TRKFIT:USE_FDC_DRIFT_TIMES",
         USE_FDC_DRIFT_TIMES);

   PLANE_TO_SKIP=0;
   gPARMS->SetDefaultParameter("TRKFIT:PLANE_TO_SKIP",PLANE_TO_SKIP);

   SKIP_CDC=false;
   gPARMS->SetDefaultParameter("TRKFIT:SKIP_CDC",SKIP_CDC);

   SKIP_FDC=false;
   gPARMS->SetDefaultParameter("TRKFIT:SKIP_FDC",SKIP_FDC);

   // Get pointer to TrackFinder object 
   vector<const DTrackFinder *> finders;
   loop->Get(finders);

   if(finders.size()<1){
      _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
      return RESOURCE_UNAVAILABLE;
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
  
   return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DTrackCandidate_factory_StraightLine::evnt(JEventLoop *loop, uint64_t eventnumber)
{

   // Reset the track finder
   finder->Reset();

   vector<const DCDCTrackHit*>cdcs;
   vector<const DFDCPseudo*>pseudos;
   loop->Get(cdcs);
   loop->Get(pseudos);

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
	 double pt=10.0*cos(atan(tanl)); // arbitrary magnitude...    
	 cand->setMomentum(DVector3(pt*cos(phi),pt*sin(phi),pt*tanl));

	 DVector3 pos,origin,dir(0,0,1.);
	 finder->FindDoca(0,tracks[i].S,dir,origin,&pos);
	 cand->setPosition(pos);
	 
	 _data.push_back(cand);

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

	   double z=tracks[i].z;
	   // state vector
	   DMatrix4x1 S(tracks[i].S);
	   double tx=tracks[i].S(state_tx),ty=tracks[i].S(state_ty);
	   double phi=atan2(ty,tx);
	   double tanl=1./sqrt(tx*tx+ty*ty);
	   // Check for tracks heading upstream
	   double phi_diff=phi-hits[0]->wire->origin.Phi()-M_PI;
	   if (phi_diff<-M_PI) phi_diff+=2.*M_PI;
	   if (phi_diff> M_PI) phi_diff-=2.*M_PI;
	   if (fabs(phi_diff)<M_PI_2){
	     tanl*=-1;
	     phi+=M_PI;
	   }
	   double pt=cos(atan(tanl)); //only direction is known...    
	   cand->setMomentum(DVector3(pt*cos(phi),pt*sin(phi),pt*tanl));

	   if (COSMICS==false){
	     DVector3 pos,origin,dir(0,0,1.);
	     finder->FindDoca(z,tracks[i].S,dir,origin,&pos);
	     cand->setPosition(pos);
	   }
	   else{
	     cand->setPosition(DVector3(tracks[i].S(state_x),
					tracks[i].S(state_y),z));
	   }
	 
	   _data.push_back(cand);
         }
      }
   }

   // Set CDC ring & FDC plane hit patterns
   for(size_t loc_i = 0; loc_i < _data.size(); ++loc_i)
   {
      vector<const DCDCTrackHit*> locCDCTrackHits;
      _data[loc_i]->Get(locCDCTrackHits);

      vector<const DFDCPseudo*> locFDCPseudos;
      _data[loc_i]->Get(locFDCPseudos);

      _data[loc_i]->dCDCRings = dParticleID->Get_CDCRingBitPattern(locCDCTrackHits);
      _data[loc_i]->dFDCPlanes = dParticleID->Get_FDCPlaneBitPattern(locFDCPseudos);
   }

   return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DTrackCandidate_factory_StraightLine::erun(void)
{
   return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DTrackCandidate_factory_StraightLine::fini(void)
{
   return NOERROR;
}
