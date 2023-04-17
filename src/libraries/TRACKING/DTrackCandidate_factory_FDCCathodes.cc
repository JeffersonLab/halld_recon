// $Id$
//
//    File: DTrackCandidate_factory_FDCCathodes.cc
// Created: Tue Nov  6 13:37:08 EST 2007
// Creator: staylor (on Linux ifarml1.jlab.org 2.4.21-47.0.1.ELsmp i686)
//
/// This factory links segments in the FDC packages into track candidates 
/// by swimming through the field from one package to the next.

#include "DTrackCandidate_factory_FDCCathodes.h"
#include "DANA/DApplication.h"
#include <JANA/JCalibration.h>
#include "FDC/DFDCPseudo_factory.h"
#include "FDC/DFDCSegment_factory.h"
#include "DHelicalFit.h"
#include "DHoughFind.h"
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>

///
/// DTrackCandidate_factory_FDCCathodes::brun():
///
jerror_t DTrackCandidate_factory_FDCCathodes::brun(JEventLoop* eventLoop, 
						   int32_t runnumber) {
  DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  bfield = dapp->GetBfield(runnumber);
  FactorForSenseOfRotation=(bfield->GetBz(0.,0.,65.)>0.)?-1.:1.;

  const DGeometry *dgeom  = dapp->GetDGeometry(runnumber);
  
  USE_FDC=true;
  if (!dgeom->GetFDCZ(z_wires)){
    _DBG_<< "FDC geometry not available!" <<endl;
    USE_FDC=false;
  }

  JCalibration *jcalib = dapp->GetJCalibration(runnumber);
  map<string, double> targetparms;
  if (jcalib->Get("TARGET/target_parms",targetparms)==false){
    TARGET_Z = targetparms["TARGET_Z_POSITION"];
  }
  else{
    dgeom->GetTargetZ(TARGET_Z);
  }

  DEBUG_HISTS=false;
  gPARMS->SetDefaultParameter("TRKFIND:DEBUG_HISTS", DEBUG_HISTS);

  BEAM_VAR=1.;
  gPARMS->SetDefaultParameter("TRKFIND:BEAM_VAR",BEAM_VAR);

  FDC_HOUGH_THRESHOLD=10.;
  gPARMS->SetDefaultParameter("TRKFIND:FDC_HOUGH_THRESHOLD",FDC_HOUGH_THRESHOLD);
  ADD_VERTEX_POINT=true;
  gPARMS->SetDefaultParameter("TRKFIND:ADD_VERTEX_POINT", ADD_VERTEX_POINT);
 
  if(DEBUG_HISTS) {
    dapp->Lock();
    match_dist_fdc=(TH2F*)gROOT->FindObject("match_dist_fdc");
    if (!match_dist_fdc){ 
      match_dist_fdc=new TH2F("match_dist_fdc",
			      "Matching distance for connecting FDC segments",
			      100,0.,500,500,0,100.);
    }
    match_center_dist2=(TH2F*)gROOT->FindObject("match_center_dist2");
    if (!match_center_dist2){
      match_center_dist2=new TH2F("match_center_dist2","matching distance squared between two circle centers vs p",50,0,50.0,100,0,100);
      match_center_dist2->SetXTitle("p [GeV/c]");
      match_center_dist2->SetYTitle("(#Deltad)^{2} [cm^{2}]");
    }
    
    dapp->Unlock();
  }
    
  // Initialize the stepper
  stepper=new DMagneticFieldStepper(bfield);
  stepper->SetStepSize(1.0);

  return NOERROR;
}


//------------------
// erun
//------------------
jerror_t DTrackCandidate_factory_FDCCathodes::erun(void)
{
  if (stepper) {
    delete stepper;
    stepper = nullptr;
  }

  return NOERROR;
}
//------------------
// fini
//------------------
jerror_t DTrackCandidate_factory_FDCCathodes::fini(void)
{
  if (stepper) {
    delete stepper;
    stepper = nullptr;
  }  

  return NOERROR;
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
// evnt:  main segment linking routine
//------------------
jerror_t DTrackCandidate_factory_FDCCathodes::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  if (!USE_FDC) return NOERROR;

  vector<const DFDCSegment*>segments;
  eventLoop->Get(segments);

  // abort if there are no segments
  if (segments.size()==0.) return NOERROR;

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

  // Start gathering groups into a list of linked segments to elevate to track
  // candidates
  vector<vector<const DFDCSegment *> >mytracks;
  for (unsigned int i=0;i<quadruplets.size();i++){
    mytracks.push_back(quadruplets[i]);
  }
  // If we could not link some of the pairs together, create two-segment 
  // "tracks"
  for (unsigned int i=0;i<is_tripled.size();i++){
    if (is_tripled[i]==0){
      vector<const DFDCSegment *>mytrack;
      mytrack.push_back(paired_segments[i].first);
      mytrack.push_back(paired_segments[i].second);
      mytracks.push_back(mytrack);
    }
  }
  // if we could not link some of the triplets to other segments, create 
  // three-segment "tracks"
  for (unsigned int i=0;i<triplets.size();i++){
    if (is_quadrupled[i]==0){
      mytracks.push_back(triplets[i]);
    }
  }

  // For each set of matched segments, redo the helical fit with all the hits 
  // and create a new track candidate
  for (unsigned int i=0;i<mytracks.size();i++){  
    // Create the fit object and add the hits
    DHelicalFit fit; 
    // Fake point at origin
    if (ADD_VERTEX_POINT){
      fit.AddHitXYZ(0.,0.,TARGET_Z,BEAM_VAR,BEAM_VAR,0.);
    }
    double max_r=0.;
    double rc=0.,xc=0.,yc=0.,tanl=0.; //initialize helix variables
    double q=mytracks[i][0]->q;
    // create a guess for rc and add hits
    for (unsigned int m=0;m<mytracks[i].size();m++){
      rc+=mytracks[i][m]->rc;
      xc+=mytracks[i][m]->xc;
      yc+=mytracks[i][m]->yc;
      tanl+=mytracks[i][m]->tanl;
      for (unsigned int n=0;n<mytracks[i][m]->hits.size();n++){
	const DFDCPseudo *hit=mytracks[i][m]->hits[n];
	fit.AddHit(hit);
	
	double r=hit->xy.Mod();
	if (r>max_r){
	  max_r=r;
	}
      }
    }
    double mysize=double(mytracks[i].size());
    rc/=mysize;
    xc/=mysize;
    yc/=mysize;
    tanl/=mysize;

    // Use a hit on the track to get the magnetic field
    const DFDCPseudo *myhit=mytracks[0][0]->hits[mytracks[0][0]->hits.size()-1];
    double xhit=myhit->xy.X();
    double yhit=myhit->xy.Y();
    double zhit=myhit->wire->origin.z();
    double Bz=fabs(bfield->GetBz(xhit,yhit,zhit));

    // Do the fit
    if (fit.FitTrackRiemann(rc)==NOERROR){    
      // New track parameters
      tanl=fit.tanl;
      xc=fit.x0;
      yc=fit.y0;
      rc=fit.r0;
      q=FactorForSenseOfRotation*fit.h;

      // Look for cases where the momentum is unrealistically large...
      double p=0.003*fit.r0*Bz/cos(atan(fit.tanl));

      // Prune the fake hit at the origin in case we need to use an alternate
      // fit
      if (ADD_VERTEX_POINT){
	fit.PruneHit(0);
      }
      if (p>10.){//... try alternate circle fit
	fit.FitCircle();
	rc=fit.r0;
	xc=fit.x0;
	yc=fit.y0;
      } 
      if (rc<max_r && max_r<10.0){
	// ... we can also have issues near the beam line:
	// Try to fix relatively high momentum tracks in the very forward 
	// direction that look like low momentum tracks due to small pt.
	// Assume that the particle came from the center of the target.
	fit.FitTrack_FixedZvertex(TARGET_Z);
	tanl=fit.tanl;
	rc=fit.r0;
	xc=fit.x0;
	yc=fit.y0;
	fit.FindSenseOfRotation();
	q=FactorForSenseOfRotation*fit.h;      
      }
    }
    
    // Create new track, starting with the most upstream segment
    DTrackCandidate *track = new DTrackCandidate;
    //circle fit parameters
    track->rc=rc;
    track->xc=xc;
    track->yc=yc;

    // Get the momentum and position just upstream of first hit
    DVector3 mom,pos;
    GetPositionAndMomentum(xhit,yhit,zhit,Bz,q,xc,yc,rc,tanl,pos,mom);
    
    track->chisq=fit.chisq;
    track->Ndof=fit.ndof;
    track->setPID((q > 0.0) ? PiPlus : PiMinus);
    track->setPosition(pos);
    track->setMomentum(mom);
    
    for (unsigned int m=0;m<mytracks[i].size();m++){
      track->AddAssociatedObject(mytracks[i][m]);
    }
    
    _data.push_back(track); 
 
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
	
	_data.push_back(track);
      }
    }
  }

  return NOERROR;
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
   
   return (dx*dx+dy*dy);
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
      if (doca2<MatchR(segment->rc)){
	match=segment2;
	match_id=j;
      }
    }
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
      if (doca2<MatchR(segment->rc)){
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
      if (circle_center_diff2_min<9.0){
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
DTrackCandidate_factory_FDCCathodes::GetPositionAndMomentum(
							    const DFDCSegment *segment,
							    DVector3 &pos,
							    DVector3 &mom) const {
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

// Routine to return momentum and position given the helical parameters and the
// z-component of the magnetic field
jerror_t 
DTrackCandidate_factory_FDCCathodes::GetPositionAndMomentum(double xhit,
							    double yhit,
							    double zhit,
							    double Bz,
							    double q,
							    double xc,
							    double yc,
							    double rc,
							    double tanl,
							    DVector3 &pos,
							    DVector3 &mom) const {
  // Position
  double dz=1.;
  double zmin=zhit-dz;
  double phi1=atan2(yhit-yc,xhit-xc);
  double q_over_rc_tanl=q/(rc*tanl);
  double dphi_s=dz*q_over_rc_tanl;
  double dphi1=phi1-dphi_s;// was -
  double x=xc+rc*cos(dphi1);
  double y=yc+rc*sin(dphi1);
  pos.SetXYZ(x,y,zmin);
 
  dphi1*=-1.;
  if (FactorForSenseOfRotation*q<0) dphi1+=M_PI;
  
  // Momentum 
  double pt=0.003*Bz*rc; 
  double px=pt*sin(dphi1);
  double py=pt*cos(dphi1);
  double pz=pt*tanl;
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
