// $Id$
//
//    File: DTrackCandidate_factory_FDCCathodes.cc
// Created: Tue Nov  6 13:37:08 EST 2007
// Creator: staylor (on Linux ifarml1.jlab.org 2.4.21-47.0.1.ELsmp i686)
//
/// This factory links segments in the FDC packages into track candidates 
/// by projecting through the field from one package to the next.

#include "DTrackCandidate_factory_FDCCathodes.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Services/JLockService.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

#include "FDC/DFDCPseudo.h"
#include "FDC/DFDCSegment.h"
#include "DHelicalFit.h"
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>

//------------------
// Init
//------------------
void DTrackCandidate_factory_FDCCathodes::Init(void)
{ 
}

///
/// DTrackCandidate_factory_FDCCathodes::BeginRun
///
void DTrackCandidate_factory_FDCCathodes::BeginRun(const std::shared_ptr<const JEvent>& event) {

  auto run_number = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);
  auto root_lock = app->GetService<JLockService>();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto dgeom = geo_manager->GetDGeometry(run_number);
  bfield = geo_manager->GetBfield(run_number);

  FactorForSenseOfRotation=(bfield->GetBz(0.,0.,65.)>0.)?-1.:1.;

  USE_FDC=true;
  if (!dgeom->GetFDCZ(z_wires)){
    _DBG_<< "FDC geometry not available!" <<endl;
    USE_FDC=false;
  }

  map<string, double> targetparms;
  if (jcalib->Get("TARGET/target_parms",targetparms)==false){
    TARGET_Z = targetparms["TARGET_Z_POSITION"];
  }
  else{
    dgeom->GetTargetZ(TARGET_Z);
  }

  DEBUG_HISTS=false;
  app->SetDefaultParameter("TRKFIND:DEBUG_HISTS", DEBUG_HISTS);

  BEAM_VAR=1.;
  app->SetDefaultParameter("TRKFIND:BEAM_VAR",BEAM_VAR);

  ADD_VERTEX_POINT=true;
  app->SetDefaultParameter("TRKFIND:ADD_VERTEX_POINT", ADD_VERTEX_POINT);
 
  if(DEBUG_HISTS) {
    root_lock->RootWriteLock();
    
    match_dist_fdc=(TH2F*)gROOT->FindObject("match_dist_fdc");
    if (!match_dist_fdc){ 
      match_dist_fdc=new TH2F("match_dist_fdc",
			      "Matching distance for connecting FDC segments",
			      500,0.,500,500,0,100.);
    }
    match_center_dist2=(TH2F*)gROOT->FindObject("match_center_dist2");
    if (!match_center_dist2){
      match_center_dist2=new TH2F("match_center_dist2","matching distance squared between two circle centers vs rc",500,0,500.,100,0,100);
      match_center_dist2->SetXTitle("rc [cm]");
      match_center_dist2->SetYTitle("(#Deltad)^{2} [cm^{2}]");
    }

    root_lock->RootUnLock();
  }
  
  // For matching
  app->SetDefaultParameter("TRKFIND:SEGMENT_MATCH_SCALE",
			      SEGMENT_MATCH_SCALE);
  app->SetDefaultParameter("TRKFIND:SEGMENT_MATCH_HI_CUT",
			      SEGMENT_MATCH_HI_CUT);
  app->SetDefaultParameter("TRKFIND:SEGMENT_MATCH_LO_CUT",
			      SEGMENT_MATCH_LO_CUT);
  app->SetDefaultParameter("TRKFIND:CENTER_MATCH_CUT",
			      CENTER_MATCH_CUT);
}


//------------------
// EndRun
//------------------
void DTrackCandidate_factory_FDCCathodes::EndRun()
{
}
//------------------
// Finish
//------------------
void DTrackCandidate_factory_FDCCathodes::Finish()
{
}

// Local routine for sorting segments by charge and curvature
inline bool DTrackCandidate_segment_cmp(const DFDCSegment *a, const DFDCSegment *b){
  //  double k1=a->S(0,0),k2=b->S(0,0);
  //double q1=k1/fabs(k1),q2=k2/fabs(k2);
  //if (q1!=q2) return q1<q2;
  //return fabs(k1)<fabs(k2); 
  if (a->q!=b->q) return a->q<b->q;
  return a->rc>b->rc;
}


inline bool DTrackCandidate_segment_cmp_by_z(const DFDCSegment *a, 
					     const DFDCSegment *b){  
  return (a->hits[0]->wire->origin.z()<b->hits[0]->wire->origin.z());
}


//------------------
// Process:  main segment linking routine
//------------------
void DTrackCandidate_factory_FDCCathodes::Process(const std::shared_ptr<const JEvent>& event)
{
  if (!USE_FDC) return;
  
  vector<const DFDCSegment*>segments;
  event->Get(segments);

  // abort if there are no segments
  if (segments.size()==0.) return;
   
  std::stable_sort(segments.begin(), segments.end(), DTrackCandidate_segment_cmp);

  // Group segments by package
  vector<DFDCSegment*>packages[4];
  for (unsigned int i=0;i<segments.size();i++){
    const DFDCSegment *segment=segments[i];
    packages[segment->package].push_back((DFDCSegment*)segment);
  }
  // Keep track of the segments in each package that have been paired with
  // other segments
  vector<vector<int> >is_paired;
  for (unsigned int i=0;i<4;i++){
    vector<int>temp(packages[i].size());
    is_paired.push_back(temp);
  } 
  // Loop over all the packages to match to segments in packages downstream
  // of the current package 
  vector<pair<const DFDCSegment*,const DFDCSegment*> >paired_segments;
  for (unsigned int i=0;i<3;i++){
    if (packages[i].size()>0) LinkSegments(i,packages,paired_segments,is_paired);
  }
  
  // Link pairs of segments into groups of three linked segments
  vector<vector<const DFDCSegment *> >triplets;
  vector<int>is_tripled(paired_segments.size());
  for (unsigned int i=0;i<paired_segments.size();i++){
    for (unsigned int j=0;j<paired_segments.size();j++){
      if (i==j) continue;
      if (is_tripled[i] || is_tripled[j]) continue;
      if (paired_segments[i].second==paired_segments[j].first){
	is_tripled[i]=1;
	is_tripled[j]=1;

	vector<const DFDCSegment *>triplet;
	triplet.push_back(paired_segments[i].first);
	triplet.push_back(paired_segments[i].second);
	triplet.push_back(paired_segments[j].second);
	triplets.push_back(triplet);
      }  
    }
  } 
  
  // Link triplets with pairs to form groups of four linked segments
  vector<int>is_quadrupled(triplets.size());
  vector<vector<const DFDCSegment *> >quadruplets;
  for (unsigned int i=0;i<triplets.size();i++){
    for (unsigned int j=0;j<paired_segments.size();j++){  
      if (is_tripled[j] || is_quadrupled[i]) continue;
      if (triplets[i][2]==paired_segments[j].first){
	is_quadrupled[i]=1;
	is_tripled[j]=1;

	vector<const DFDCSegment*>quadruplet=triplets[i];
	quadruplet.push_back(paired_segments[j].second);
	quadruplets.push_back(quadruplet);
      } 
    }
  }

  // Elevate quadruplets to track candidates
  for (unsigned int i=0;i<quadruplets.size();i++){
    MakeCandidate(quadruplets[i]);
  }

  // if we could not link some of the triplets to other segments, create
  // three-segment track candidates
  for (unsigned int i=0;i<triplets.size();i++){
    if (is_quadrupled[i]==0){
      MakeCandidate(triplets[i]);
    }
  }

  // If we could not link some of the pairs together, create two-segment 
  // "tracks"
  vector<vector<const DFDCSegment *> >mytracks;
  for (unsigned int i=0;i<is_tripled.size();i++){
    if (is_tripled[i]==0){
      vector<const DFDCSegment *>mytrack;
      mytrack.push_back(paired_segments[i].first);
      mytrack.push_back(paired_segments[i].second);
      mytracks.push_back(mytrack);
    }
  }

  // For each set of matched segments, redo the helical fit with all the hits,
  // create a new track candidate and try to match to another pair of matched
  // segments
  vector<int>is_matched(mytracks.size());
  for (unsigned int i=0;i<mytracks.size();i++){
    if (is_matched[i]==0){
      MakeCandidate(mytracks[i]);
      for (unsigned int j=i+1;j<mytracks.size();j++){
	if (mytracks[j][0]->package > mytracks[i][1]->package){
	  // Try to connect the track stubs together
	  const DFDCPseudo *myhit=mytracks[j][0]->hits[0];
	  unsigned int index=mData.size()-1;
	  double doca2=DocaSqToHelix(mData[index],myhit);
	  bool got_match=(doca2<MatchR(mData[index]->rc));
	  // if this does not work, try to match using the centers of the
	  // circles
	  if (got_match==false){
	    double dx=mytracks[j][0]->xc-mData[index]->xc;
	    double dy=mytracks[j][0]->yc-mData[index]->yc;
	    if (dx*dx+dy*dy<CENTER_MATCH_CUT) got_match=true;
	  }
	  if (got_match==false){
	    double dx=mytracks[j][1]->xc-mData[index]->xc;
	    double dy=mytracks[j][1]->yc-mData[index]->yc;
	    if (dx*dx+dy*dy<CENTER_MATCH_CUT) got_match=true;
	  }
	  if (got_match){
	    is_matched[j]=1;

	    // Add the new segments as associated objects to mData[index]
	    mData[index]->AddAssociatedObject(mytracks[j][0]);
	    mData[index]->AddAssociatedObject(mytracks[j][1]);

	    vector<const DFDCSegment*>segments=mytracks[i];
	    segments.insert(segments.end(),mytracks[j].begin(),mytracks[j].end());

	    // Create fit object and perform helical fit
	    DHelicalFit fit;
	    DoHelicalFit(segments,fit);
    
	    //circle fit parameters
	    mData[index]->rc=fit.r0;
	    mData[index]->xc=fit.x0;
	    mData[index]->yc=fit.y0;

	    // Get position and momentum just upstream of first hit
	    DVector3 pos,mom;
	    GetPositionAndMomentum(fit,segments[0],pos,mom);
	    
	    mData[index]->chisq=fit.chisq;
	    mData[index]->Ndof=fit.ndof;
	    double q=FactorForSenseOfRotation*fit.h;
	    mData[index]->setPID((q > 0.0) ? PiPlus : PiMinus);
	    mData[index]->setPosition(pos);
	    mData[index]->setMomentum(mom);
	  }
	}
      }
    }
  }

  // Now try to attach stray segments to existing tracks
  for (unsigned int i=0;i<4;i++){
    for (unsigned int k=0;k<packages[i].size();k++){
      DFDCSegment *segment=packages[i][k];
      if (is_paired[i][k]==0 && LinkStraySegment(segment)){
	is_paired[i][k]=1;
      }
    }
  }
    
  // Create track stubs for unused segments
  for (unsigned int j=0;j<4;j++){
    for (unsigned int i=0;i<packages[j].size();i++){
      if (is_paired[j][i]==0){
	const DFDCSegment* segment=packages[j][i];

	// Get the momentum and position at a specific z position
	DVector3 mom, pos;
	GetPositionAndMomentum(segment,pos,mom);
	
	// Create new track, starting with the current segment
	DTrackCandidate *track = new DTrackCandidate;
	track->rc=segment->rc;
	track->xc=segment->xc;
	track->yc=segment->yc;
	
	track->setPosition(pos);
	track->setMomentum(mom);
	track->setPID((segment->q > 0.0) ? PiPlus : PiMinus);
	track->Ndof=segment->Ndof;
	track->chisq=segment->chisq;
      
	track->AddAssociatedObject(segment);
	double t0=1e9;
	for (unsigned int k=0;k<segment->hits.size();k++){
	  if (segment->hits[k]->time<t0) t0=segment->hits[k]->time;
	}
	track->setTime(t0);
	track->setT0(t0,5.,SYS_FDC);
	
	Insert(track);
      }
    }
  }

  return;
}

// Project segment1 to a hit in segment 2 and compute the square of the doca
// between the projection and the hit
double DTrackCandidate_factory_FDCCathodes::DocaSqToHelix(const DFDCSegment *segment1,const DFDCSegment *segment2) const {
   const DFDCPseudo *hit1=segment1->hits[0]; 
   const DFDCPseudo *hit2=segment2->hits[0];
   double z1=hit1->wire->origin.z();
   double z2=hit2->wire->origin.z();
   double x2=hit2->xy.X();
   double y2=hit2->xy.Y();
   double phi=segment1->Phi1+(z1-z2)*segment1->q/(segment1->rc*segment1->tanl);
   double dx=segment1->xc+segment1->rc*cos(phi)-x2;
   double dy=segment1->yc+segment1->rc*sin(phi)-y2;
   
   return dx*dx+dy*dy;
}

// Project track candidate to a hit in a segment and compute the square of the 
// doca between the projection and the hit
double DTrackCandidate_factory_FDCCathodes::DocaSqToHelix(const DTrackCandidate *candidate,const DFDCPseudo *hit) const {
  DVector3 pos=candidate->position();
  DVector3 mom=candidate->momentum();
  double z2=hit->wire->origin.z();
  double x2=hit->xy.X();
  double y2=hit->xy.Y();
  double Phi1=atan2(pos.y()-candidate->yc,pos.x()-candidate->xc);
  double mytanl=tan(M_PI_2-mom.Theta());
  double phi=Phi1+(pos.z()-z2)*candidate->charge()/(candidate->rc*mytanl);
  double dx=candidate->xc+candidate->rc*cos(phi)-x2;
  double dy=candidate->yc+candidate->rc*sin(phi)-y2;

  return dx*dx+dy*dy;
}

// Propagate track from one package to the next and look for a match to a 
// segment in the new package
DFDCSegment *DTrackCandidate_factory_FDCCathodes::GetTrackMatch(DFDCSegment *segment,
								vector<DFDCSegment*>package,
								unsigned int &match_id){
  DFDCSegment *match=NULL;
  
  // Match to the next package
  double doca2_min=1e6,doca2;
  for (unsigned int j=0;j<package.size();j++){
    DFDCSegment *segment2=package[j];
    doca2=DocaSqToHelix(segment,segment2);

    if (doca2<doca2_min){
      doca2_min=doca2;
      match_id=j;
    }
  }
  if (doca2_min<MatchR(segment->rc)){
    match=package[match_id];
  }

  if(DEBUG_HISTS){
    match_dist_fdc->Fill(segment->rc,doca2_min);
  }
  if (match!=NULL) return match;

  // If matching in the forward direction did not work, try 
  // matching backwards...
  doca2_min=1e6;
  for (unsigned int i=0;i<package.size();i++){
    DFDCSegment *segment2=package[i];
    doca2=DocaSqToHelix(segment2,segment);

    if (doca2<doca2_min){
      doca2_min=doca2;	
      if (doca2<MatchR(segment2->rc)){
	match=segment2;
	match_id=i;
      }
    }       
  } 
  if (match!=NULL) return match;

  // Match by centers of circles
  double circle_center_diff2_min=1e6;
  for (unsigned int j=0;j<package.size();j++){
    DFDCSegment *segment2=package[j];
    
    double dx=segment->xc-segment2->xc;
    double dy=segment->yc-segment2->yc;
    double circle_center_diff2=dx*dx+dy*dy;
    
    if (circle_center_diff2<circle_center_diff2_min){
      circle_center_diff2_min=circle_center_diff2;
      if (circle_center_diff2_min<CENTER_MATCH_CUT){
	match=segment2;
	match_id=j;
      }
    }
  }
  if (DEBUG_HISTS){
    match_center_dist2->Fill(segment->rc,circle_center_diff2_min);
  }  
  return match;
}

// Routine to return momentum and position given the helical parameters and the
// z-component of the magnetic field
jerror_t 
DTrackCandidate_factory_FDCCathodes::GetPositionAndMomentum(const DHelicalFit &fit,							    
							    const DFDCSegment *segment,
							    DVector3 &pos,
							    DVector3 &mom){
  // Hit in the most upstream package
  const DFDCPseudo *hit=segment->hits[segment->hits.size()-1];
  double zhit=hit->wire->origin.z();
  double xhit=hit->xy.X();
  double yhit=hit->xy.Y();
  
  // Position
  double dz=1.;
  double zmin=zhit-dz;
  double phi1=atan2(yhit-fit.y0,xhit-fit.x0);
  double q=FactorForSenseOfRotation*fit.h;    
  double q_over_rc_tanl=q/(fit.r0*fit.tanl);
  double dphi_s=dz*q_over_rc_tanl;
  double dphi1=phi1-dphi_s;// was -
  double x=fit.x0+fit.r0*cos(dphi1);
  double y=fit.y0+fit.r0*sin(dphi1);
  pos.SetXYZ(x,y,zmin);

  dphi1*=-1.;
  if (FactorForSenseOfRotation*q<0) dphi1+=M_PI;
  
  // Momentum
  double Bz=fabs(bfield->GetBz(xhit,yhit,zhit));
  double pt=0.003*Bz*fit.r0; 
  double px=pt*sin(dphi1);
  double py=pt*cos(dphi1);
  double pz=pt*fit.tanl;
  mom.SetXYZ(px,py,pz);

  return NOERROR;
}

// Routine to return momentum and position given the helical parameters and the
// z-component of the magnetic field
jerror_t 
DTrackCandidate_factory_FDCCathodes::GetPositionAndMomentum(
							    const DFDCSegment *segment,
							    DVector3 &pos,
							    DVector3 &mom){
  // Hit in the most upstream package
  const DFDCPseudo *hit=segment->hits[segment->hits.size()-1];
  double zhit=hit->wire->origin.z();
  double xhit=hit->xy.X();
  double yhit=hit->xy.Y();
  
  // Position
  double dz=1.;
  double zmin=zhit-dz;
  double phi1=atan2(yhit-segment->yc,xhit-segment->xc);
  double q_over_rc_tanl=segment->q/(segment->rc*segment->tanl);
  double dphi_s=dz*q_over_rc_tanl;
  double dphi1=phi1-dphi_s;// was -
  double x=segment->xc+segment->rc*cos(dphi1);
  double y=segment->yc+segment->rc*sin(dphi1);
  pos.SetXYZ(x,y,zmin);

  dphi1*=-1.;
  if (FactorForSenseOfRotation*segment->q<0) dphi1+=M_PI;

  // Find Bz at x,y,zmin
  double Bz=bfield->GetBz(x,y,zmin);
  
  // Momentum 
  double pt=0.003*Bz*segment->rc; 
  double px=pt*sin(dphi1);
  double py=pt*cos(dphi1);
  double pz=pt*segment->tanl;
  mom.SetXYZ(px,py,pz);

  return NOERROR;
}


// Routine to loop over segments in one of the packages, linking them with 
// segments in the package downstream of this package
void DTrackCandidate_factory_FDCCathodes::LinkSegments(unsigned int pack1,
						       vector<DFDCSegment *>packages[4], vector<pair<const DFDCSegment*,const DFDCSegment*> >&paired_segments,
						       vector<vector<int> >&is_paired){ 
  unsigned int match_id=0;
  unsigned int pack2=pack1+1;

  // Loop over the segments in package "pack1"
  for (unsigned int i=0;i<packages[pack1].size();i++){
    DFDCSegment *segment=packages[pack1][i];
    DFDCSegment *match2=NULL;

    // Try matching to the next package
    if (packages[pack2].size()>0 
	&& (match2=GetTrackMatch(segment,packages[pack2],match_id))!=NULL){
      if (is_paired[pack2][match_id]) continue;

      pair<const DFDCSegment*,const DFDCSegment*> mypair;
      mypair.first=segment;
      mypair.second=match2;
      paired_segments.push_back(mypair);
      is_paired[pack2][match_id]=1;
      is_paired[pack1][i]=1;
    }
  }
}

// Routine that tries to link a stray segment with an already existing track
// candidate
bool DTrackCandidate_factory_FDCCathodes::LinkStraySegment(const DFDCSegment *segment){
  // Loop over existing candidates looking for potential holes
  for (unsigned int i=0;i<mData.size();i++){
    bool got_segment_in_package=false;

    // Get the segments already associated with this track 
    vector<const DFDCSegment*>segments;
    mData[i]->GetT(segments);
    // Flag if segment is in a package that has already been used for this 
    // candidate
    for (unsigned int j=0;j<segments.size();j++){
      if (segments[j]->package==segment->package){
	got_segment_in_package=true;
	break;
      }
    }
    if (got_segment_in_package==false){
      // Sort segments already associated with the track and try to match
      // stray segments beyond one end or the other of the existing track
      sort(segments.begin(),segments.end(),DTrackCandidate_segment_cmp_by_z);
      unsigned int max_i=segments.size()-1;
      if (segment->package<segments[0]->package 
	  || segment->package>segments[max_i]->package){
	const DFDCPseudo *myhit=segment->hits[0];
	double doca2=DocaSqToHelix(mData[i],myhit);
	bool got_match=(doca2<MatchR(mData[i]->rc));
	// if this does not work, try to match using the centers of the circles
	if (got_match==false){
	  double dx=segment->xc-mData[i]->xc;
	  double dy=segment->yc-mData[i]->yc;
	  if (dx*dx+dy*dy<CENTER_MATCH_CUT) got_match=true;
	}
	if (got_match){
	  // Add the segment as an associated object to mData[i]
	  mData[i]->AddAssociatedObject(segment);

	  // Modify t0 if necessary
	  double t0=mData[i]->t0();
	  boold got_lower_t0=false;
	  for (unsigned int k=0;k<segment->hits.size();k++){
	    if (segment->hit[k]->time<t0){
	      t0=segment->hit[k]->time;
	      got_lower_t0=true;
	    }
	  }
	  if (got_lower_t0){
	    mData[i]->setTime(t0);
	    mData[i]->setT0(t0,5.,SYS_FDC);
	  }
	  
	  // Add the new segment and sort by z
	  segments.push_back(segment);
	  stable_sort(segments.begin(),segments.end(),DTrackCandidate_segment_cmp_by_z);
	  
	  // Create fit object and perform helical fit
	  DHelicalFit fit;
	  DoHelicalFit(segments,fit);
	     
	  //circle fit parameters
	  mData[i]->rc=fit.r0;
	  mData[i]->xc=fit.x0;
	  mData[i]->yc=fit.y0;
	
	  // Get position and momentum just upstream of first hit
	  DVector3 pos,mom;
	  GetPositionAndMomentum(fit,segments[0],pos,mom);
	  
	  mData[i]->chisq=fit.chisq;
	  mData[i]->Ndof=fit.ndof;
	  double q=FactorForSenseOfRotation*fit.h;
	  mData[i]->setPID((q > 0.0) ? PiPlus : PiMinus);
	  mData[i]->setPosition(pos);
	  mData[i]->setMomentum(mom); 
	
	  return true;
	} // got a match to a stray segment
      }
    }
  } // loop over track candidates

  return false;
}

// For a given sent of linked segments, perform a helical fit using all the hits
// and make a new track candidate object
void DTrackCandidate_factory_FDCCathodes::MakeCandidate(vector<const DFDCSegment *>&mytrack){
  // Create the fit object and perform the fit
  DHelicalFit fit;
  DoHelicalFit(mytrack,fit);
  
  // Create new track, starting with the most upstream segment
  DTrackCandidate *track = new DTrackCandidate;

  //circle fit parameters
  track->rc=fit.r0;
  track->xc=fit.x0;
  track->yc=fit.y0;
  
  // Get the momentum and position just upstream of first hit
  DVector3 mom,pos;
  GetPositionAndMomentum(fit,mytrack[0],pos,mom);
    
  track->chisq=fit.chisq;
  track->Ndof=fit.ndof;
  double q=FactorForSenseOfRotation*fit.h;
  track->setPID((q > 0.0) ? PiPlus : PiMinus);
  track->setPosition(pos);
  track->setMomentum(mom);

  double t0=1e9;
  for (unsigned int m=0;m<mytrack.size();m++){
    track->AddAssociatedObject(mytrack[m]);
    for (unsigned int k=0;k<mytrack[m]->hits.size();k++){
      if (mytrack[m]->hits[k]->time<t0) t0=mytrack[m]->hits[k]->time;
    }
  }
  track->setTime(t0);
  track->setT0(t0,5.,SYS_FDC);
  
  Insert(track); 
}

// Perform a Riemann Helical fit using the set of hits in the track candidate 
// "mytrack"
void DTrackCandidate_factory_FDCCathodes::DoHelicalFit(vector<const DFDCSegment *>&mytrack,DHelicalFit &fit){
  // Fake point at origin
  if (ADD_VERTEX_POINT){
    fit.AddHitXYZ(0.,0.,TARGET_Z,BEAM_VAR,BEAM_VAR,0.);
  }
  double max_r=0.;
  double rc=0.;
  // create a guess for rc and add hits
  for (unsigned int m=0;m<mytrack.size();m++){
    rc+=mytrack[m]->rc;
    for (unsigned int n=0;n<mytrack[m]->hits.size();n++){
      const DFDCPseudo *hit=mytrack[m]->hits[n];
      fit.AddHit(hit);
      
      double r=hit->xy.Mod();
      if (r>max_r){
	max_r=r;
      }
    }
  }
  double mysize=double(mytrack.size());
  rc/=mysize;

  // Do the fit
  if (fit.FitTrackRiemann(rc)==NOERROR){    
    // Look for cases where the momentum is unrealistically large...
    const DFDCPseudo *myhit=mytrack[0]->hits[0];
    double Bz=fabs(bfield->GetBz(myhit->xy.X(),myhit->xy.Y(),myhit->wire->origin.z()));
    double p=0.003*fit.r0*Bz/cos(atan(fit.tanl));
    
    // Prune the fake hit at the origin in case we need to use an alternate
    // fit
    if (ADD_VERTEX_POINT){
      fit.PruneHit(0);
    }
    if (p>10.){//... try alternate circle fit
      fit.FitCircle();
      fit.FindSenseOfRotation();
    } 
    if (fit.r0<0.5*max_r && max_r<10.0){
      // ... we can also have issues near the beam line:
      // Try to fix relatively high momentum tracks in the very forward 
      // direction that look like low momentum tracks due to small pt.
      // Assume that the particle came from the center of the target.
      fit.FitTrack_FixedZvertex(TARGET_Z);
      fit.FindSenseOfRotation();
    }
  }
  else{
    // Set tanl to guess from one of the segments
    fit.tanl=mytrack[0]->tanl;
    fit.h=FactorForSenseOfRotation*mytrack[0]->q;
    if (ADD_VERTEX_POINT){
      fit.PruneHit(0);
    }
    fit.FitCircle();
  }
}
