// $Id$
//
//    File: DTrackCandidate_factory_FDCCathodes.cc
// Created: Tue Nov  6 13:37:08 EST 2007
// Creator: staylor (on Linux ifarml1.jlab.org 2.4.21-47.0.1.ELsmp i686)
//
/// This factory links segments in the FDC packages into track candidates 
/// by swimming through the field from one package to the next.

#include "DTrackCandidate_factory_FDCCathodes.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Compatibility/JLockService.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

#include "FDC/DFDCPseudo.h"
#include "FDC/DFDCSegment.h"
#include "DHelicalFit.h"
#include "DHoughFind.h"
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>

///
/// DTrackCandidate_factory_FDCCathodes::BeginRun
///
void DTrackCandidate_factory_FDCCathodes::BeginRun(const std::shared_ptr<const JEvent>& event) {

  auto event_number = event->GetEventNumber();
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

  FDC_HOUGH_THRESHOLD=10.;
  app->SetDefaultParameter("TRKFIND:FDC_HOUGH_THRESHOLD",FDC_HOUGH_THRESHOLD);
  ADD_VERTEX_POINT=true;
  app->SetDefaultParameter("TRKFIND:ADD_VERTEX_POINT", ADD_VERTEX_POINT);
 
  if(DEBUG_HISTS) {
    root_lock->RootWriteLock();
    match_dist_fdc=(TH2F*)gROOT->FindObject("match_dist_fdc");
    if (!match_dist_fdc){ 
      match_dist_fdc=new TH2F("match_dist_fdc",
			      "Matching distance for connecting FDC segments",
			      50,0.,7,500,0,100.);
    }
    match_center_dist2=(TH2F*)gROOT->FindObject("match_center_dist2");
    if (!match_center_dist2){
      match_center_dist2=new TH2F("match_center_dist2","matching distance squared between two circle centers vs p",50,0,1.5,100,0,100);
      match_center_dist2->SetXTitle("p [GeV/c]");
      match_center_dist2->SetYTitle("(#Deltad)^{2} [cm^{2}]");
    }

    root_lock->RootUnLock();
  }
    
  // Initialize the stepper
  stepper=new DMagneticFieldStepper(bfield);
  stepper->SetStepSize(1.0);
}


//------------------
// EndRun
//------------------
void DTrackCandidate_factory_FDCCathodes::EndRun()
{
  if (stepper) {
    delete stepper;
    stepper = nullptr;
  }
}
//------------------
// Finish
//------------------
void DTrackCandidate_factory_FDCCathodes::Finish()
{
  if (stepper) {
    delete stepper;
    stepper = nullptr;
  }  
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
    rc=0.,xc=0.,yc=0.,tanl=0.; //initialize helix variables
    q=mytracks[i][0]->q;
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

    // Do the fit
    if (fit.FitTrackRiemann(rc)==NOERROR){    
      // New track parameters
      tanl=fit.tanl;
      xc=fit.x0;
      yc=fit.y0;
      rc=fit.r0;
      q=FactorForSenseOfRotation*fit.h;

      // Look for cases where the momentum is unrealistically large...
      const DFDCPseudo *myhit=mytracks[0][0]->hits[0];
      double Bz=fabs(bfield->GetBz(myhit->xy.X(),myhit->xy.Y(),myhit->wire->origin.z()));
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
      if (rc<0.5*max_r && max_r<10.0){
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
    GetPositionAndMomentum(mytracks[i],pos,mom);
    
    track->chisq=fit.chisq;
    track->Ndof=fit.ndof;
    track->setPID((q > 0.0) ? PiPlus : PiMinus);
    track->setPosition(pos);
    track->setMomentum(mom);
    
    for (unsigned int m=0;m<mytracks[i].size();m++){
      track->AddAssociatedObject(mytracks[i][m]);
    }
    
    Insert(track); 
 
  }

  
  // Now try to attach stray segments to existing tracks
  for (unsigned int i=0;i<4;i++){
    for (unsigned int k=0;k<packages[i].size();k++){
      DFDCSegment *segment=packages[i][k];
      if (is_paired[i][k]==0 && LinkStraySegment(segment)) is_paired[i][k]=1;
    }
  }
 
  vector<pair<unsigned int,unsigned int> >unused_segments;
  for (unsigned int j=0;j<4;j++){
    for (unsigned int i=0;i<packages[j].size();i++){
      if (is_paired[j][i]==0){
	unused_segments.push_back(make_pair(j,i));
      }
    }
  }

  // Find track candidates using Hough transform
  if (unused_segments.size()>1){
    if (LinkSegmentsHough(unused_segments,packages,is_paired)){
      unused_segments.clear();
      for (unsigned int j=0;j<4;j++){
	for (unsigned int i=0;i<packages[j].size();i++){
	  if (is_paired[j][i]==0){
	    unused_segments.push_back(make_pair(j,i));
	  }
	}
      }
      if (unused_segments.size()>1){
	LinkSegmentsHough(unused_segments,packages,is_paired);
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
	
	Insert(track);
      }
    }
  }

  return;
}


// Routine to do a crude match between fdc points and a helical approximation to
// the trajectory
double DTrackCandidate_factory_FDCCathodes::DocaSqToHelix(const DFDCPseudo *hit){
  double sperp=(hit->wire->origin.z()-zs)*cotl;
  double twoks=twokappa*sperp;
  double sin2ks=sin(twoks);
  double cos2ks=cos(twoks);
  double one_minus_cos2ks=1.-cos2ks;
  double one_over_twokappa=1./twokappa;
 
  double x=xs+(cosphi*sin2ks-sinphi*one_minus_cos2ks)*one_over_twokappa;
  double y=ys+(sinphi*sin2ks+cosphi*one_minus_cos2ks)*one_over_twokappa;
  double dx=x-hit->xy.X();
  double dy=y-hit->xy.Y();

  return (dx*dx+dy*dy);
}

// Propagate track from one package to the next and look for a match to a 
// segment in the new package
DFDCSegment *DTrackCandidate_factory_FDCCathodes::GetTrackMatch(DFDCSegment *segment,
								vector<DFDCSegment*>package,
								unsigned int &match_id){
  DFDCSegment *match=NULL;

  // Get the position and momentum at the exit of the package for the 
  // current segment
  GetPositionAndMomentum(segment);
  
  // Match to the next package
  double doca2_min=1e6,doca2;
  for (unsigned int j=0;j<package.size();j++){
    DFDCSegment *segment2=package[j];
    doca2=DocaSqToHelix(segment2->hits[0]);

    if (doca2<doca2_min){
      doca2_min=doca2;
      if(doca2<Match(p)){
	match=segment2;
	match_id=j;
      }
    }
  }

  if(DEBUG_HISTS){
    match_dist_fdc->Fill(p,doca2_min);
  }
  if (match!=NULL) return match;

  // If matching in the forward direction did not work, try 
  // matching backwards...
  doca2_min=1e6;
  for (unsigned int i=0;i<package.size();i++){
    DFDCSegment *segment2=package[i];
    GetPositionAndMomentum(segment2);
    doca2=DocaSqToHelix(segment->hits[segment->hits.size()-1]);
    if (doca2<doca2_min){
      doca2_min=doca2;
      if (doca2<Match(p)){
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
    match_center_dist2->Fill(p,circle_center_diff2_min);
  }  
  return match;
}

// Obtain position and momentum at the exit of a given package using the 
// helical track model.
//
jerror_t DTrackCandidate_factory_FDCCathodes::GetPositionAndMomentum(const DFDCSegment *segment){
  // Position of track segment at last hit plane of package
  xs=segment->xc+segment->rc*cos(segment->Phi1);
  ys=segment->yc+segment->rc*sin(segment->Phi1);
  zs=segment->hits[0]->wire->origin.z();

  // Track parameters
  //double kappa=segment->q/(2.*segment->rc);
  double my_phi0=segment->phi0;
  double my_tanl=segment->tanl;
  double z0=segment->z_vertex; 
  twokappa=FactorForSenseOfRotation*segment->q/segment->rc;

  cotl=1./my_tanl;

  // Useful intermediate variables
  double cosp=cos(my_phi0);
  double sinp=sin(my_phi0);
  double twoks=twokappa*(zs-z0)*cotl;
  double sin2ks=sin(twoks);
  double cos2ks=cos(twoks); 
  
  // Get Bfield
  double Bz=fabs(bfield->GetBz(xs,ys,zs));

  // Momentum
  p=0.003*Bz*segment->rc/cos(atan(my_tanl));

  cosphi=cosp*cos2ks-sinp*sin2ks;
  sinphi=sinp*cos2ks+cosp*sin2ks;

  return NOERROR;
}

// Routine to return momentum and position given the helical parameters and the
// z-component of the magnetic field
jerror_t 
DTrackCandidate_factory_FDCCathodes::GetPositionAndMomentum(
				        vector<const DFDCSegment *>segments,
							    DVector3 &pos,
							    DVector3 &mom){
  // Hit in the most upstream package
  const DFDCPseudo *hit=segments[0]->hits[segments[0]->hits.size()-1];
  double zhit=hit->wire->origin.z();
  double xhit=hit->xy.X();
  double yhit=hit->xy.Y();
  
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

  // Find the average Bz
  double Bz=0.;
  double z=zmin;
  unsigned int num_segments=segments.size();
  double zmax=segments[num_segments-1]->hits[0]->wire->origin.z();
  unsigned int num_samples=20*num_segments;
  double one_over_denom=1./double(num_samples);
  dz=(zmax-zmin)*one_over_denom;
  for (unsigned int i=0;i<num_samples;i++){
    double my_dphi=phi1+(z-zmin)*q_over_rc_tanl;
    x=xc+rc*cos(my_dphi);
    y=yc+rc*sin(my_dphi);
    Bz+=bfield->GetBz(x,y,z);

    z+=dz;
  }
  Bz=fabs(Bz)*one_over_denom;
  
  
  // Momentum 
  double pt=0.003*Bz*rc; 
  double px=pt*sin(dphi1);
  double py=pt*cos(dphi1);
  double pz=pt*tanl;
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

// Routine for matching to a segment using the stepper
bool DTrackCandidate_factory_FDCCathodes::GetTrackMatch(double q,
							DVector3 &pos,
							DVector3 &mom,
							const DFDCSegment *segment){
  const DVector3 norm(0,0,1.);
  stepper->SetCharge(q);
  
  const DFDCPseudo *hit=segment->hits[0];
  if (stepper->SwimToPlane(pos,mom,hit->wire->origin,norm,NULL)==false){
    double dx=hit->xy.X()-pos.x();
    double dy=hit->xy.Y()-pos.y();
    double d2=dx*dx+dy*dy;
    if (d2<Match(mom.Mag())) return true;
  }
  return false;
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
      // Try to link this segment to an existing candidate
      DVector3 pos=mData[i]->position();
      DVector3 mom=mData[i]->momentum();

      // Switch the direction of the momentum if we would need to backtrack to 
      // get to the segment
      if (segment->hits[0]->wire->origin.z()<pos.z()){
	mom=-1.0*mom;
      }
      // Match by swimming to a plane in the stray segment
      bool got_match=GetTrackMatch(mData[i]->charge(),pos,mom,segment);      
      // if this does not work, try to match using the centers of the circles
      if (got_match==false){
	double dx=segment->xc-mData[i]->xc;
	double dy=segment->yc-mData[i]->yc;
	if (dx*dx+dy*dy<9.0) got_match=true;
      }
      if (got_match){
	// Add the segment as an associated object to mData[i]
	mData[i]->AddAssociatedObject(segment);
   
	// Add the new segment and sort by z
	segments.push_back(segment);
	stable_sort(segments.begin(),segments.end(),DTrackCandidate_segment_cmp_by_z);
	
	// Create fit object and add hits
	DHelicalFit fit;  
	// Fake point at origin
	if (ADD_VERTEX_POINT){
	  fit.AddHitXYZ(0.,0.,TARGET_Z,BEAM_VAR,BEAM_VAR,0.);
	}
	double max_r=0.;
	for (unsigned int m=0;m<segments.size();m++){ 
	  for (unsigned int k=0;k<segments[m]->hits.size();k++){
	    const DFDCPseudo *hit=segments[m]->hits[k];
	    fit.AddHit(hit);

	    double r=hit->xy.Mod();
	    if (r>max_r){
	      max_r=r;
	    }
	  }
	}
	
	// Redo the helical fit with the additional hits
	if (fit.FitTrackRiemann(mData[i]->rc)==NOERROR){      	
	  rc=fit.r0;
	  tanl=fit.tanl;
	  xc=fit.x0;
	  yc=fit.y0;
	  q=FactorForSenseOfRotation*fit.h;
	  
	   // Look for cases where the momentum is unrealistically large...
	  const DFDCPseudo *myhit=segments[0]->hits[0];
	  double Bz=fabs(bfield->GetBz(myhit->xy.X(),myhit->xy.Y(),myhit->wire->origin.z()));
	  double p=0.003*fit.r0*Bz/cos(atan(fit.tanl));
	  
	  // Prune the fake hit at the origin in case we need to use an 
	  // alternate fit
	  if (ADD_VERTEX_POINT){
	    fit.PruneHit(0);
	  }
	  if (p>10.){//... try alternate circle fit 
	    fit.FitCircle();
	    rc=fit.r0;
	    xc=fit.x0;
	    yc=fit.y0;
	  } 
	  if (rc<0.5*max_r && max_r<10.0){
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
	  
	  // Get position and momentum just upstream of first hit
	  GetPositionAndMomentum(segments,pos,mom);
	  
	  mData[i]->chisq=fit.chisq;
	  mData[i]->Ndof=fit.ndof;
	  mData[i]->setPID((q > 0.0) ? PiPlus : PiMinus);
	  mData[i]->setPosition(pos);
	  mData[i]->setMomentum(mom); 
	}

	return true;
      }
    }
  }
  return false;
}

// Find circles using Hough transform
bool DTrackCandidate_factory_FDCCathodes::LinkSegmentsHough(vector<pair<unsigned int,unsigned int> >&unused_segments,
							    vector<DFDCSegment *>packages[4],
							    vector<vector<int> >&is_paired){
  DHoughFind hough(-400.0, +400.0, -400.0, +400.0, 100, 100);
    
  vector<pair<unsigned int, unsigned int> >associated_segments;
  for (unsigned int i=0;i<unused_segments.size();i++){
    unsigned int packNum=unused_segments[i].first;
    unsigned int segmentNum=unused_segments[i].second;    
    const DFDCSegment* segment=packages[packNum][segmentNum];
    for (unsigned int m=0;m<segment->hits.size();m++){
      hough.AddPoint(segment->hits[m]->xy);
      associated_segments.push_back(unused_segments[i]);
    }
  }
        
  DVector2 Ro = hough.Find();
  if(hough.GetMaxBinContent()>FDC_HOUGH_THRESHOLD){	
    // Zoom in on resonance a little
    double width = 60.0;
    hough.SetLimits(Ro.X()-width, Ro.X()+width, Ro.Y()-width, Ro.Y()+width, 
		    100, 100);
    Ro = hough.Find();
    
    // Zoom in on resonance once more
    width = 8.0;
    hough.SetLimits(Ro.X()-width, Ro.X()+width, Ro.Y()-width, Ro.Y()+width, 100, 100);
    Ro = hough.Find();
    
    vector<DVector2> points=hough.GetPoints();
    set<pair<unsigned int, unsigned int> >associated_segments_to_use;
    unsigned int num_hits_to_use=0;
    for (unsigned int m=0;m<points.size();m++){
      // Calculate distance between Hough transformed line (i.e.
      // the line on which a circle that passes through both the
      // origin and the point at hit->pos) and the circle center.
      DVector2 h=0.5*points[m];
      DVector2 g(h.Y(), -h.X()); 
      g /= g.Mod();
      DVector2 Ro_minus_h=Ro-h;	
      double dist = fabs(g.X()*Ro_minus_h.Y() - g.Y()*Ro_minus_h.X());
      
      // If this is not close enough to the found circle's center,
      // reject it for this track candidate
      if(dist < 2.0){
	num_hits_to_use++;
	associated_segments_to_use.emplace(associated_segments[m]);
      }
    }
    if (num_hits_to_use>5&&associated_segments_to_use.size()>1){
      bool same_package=false;
      set<pair<unsigned int,unsigned int> >::iterator it=associated_segments_to_use.begin();
      for (; it!=associated_segments_to_use.end(); ++it){
	unsigned int first_packNo=(*it).first;
	unsigned int first_segmentNo=(*it).second;
	set<pair<unsigned int,unsigned int> >::iterator it2=associated_segments_to_use.begin();
	for (; it2!=associated_segments_to_use.end(); ++it2){
	  unsigned int packNo=(*it2).first;
	  unsigned int segmentNo=(*it2).second;
	  if (packNo==first_packNo){
	    if (segmentNo==first_segmentNo) continue;
	    
	    same_package=true;
	    break;
	  }
	}
      }
      if (same_package==false){
	DHelicalFit fit;
	
	set<pair<unsigned int,unsigned int> >::iterator it=associated_segments_to_use.begin();
	const DFDCSegment *first_segment=NULL;
	bool got_first_segment=false;
	for (; it!=associated_segments_to_use.end(); ++it){
	  unsigned int packNo=(*it).first;
	  unsigned int segmentNo=(*it).second;	    
	    DFDCSegment *segment=packages[packNo][segmentNo];
	    if (got_first_segment==false){
	      first_segment=segment;
	      got_first_segment=true;
	    }
	    for (unsigned int m=0;m<segment->hits.size();m++){
	      fit.AddHit(segment->hits[m]);
	    }
	    
	}
	if (fit.FitTrackRiemann(Ro.Mod())==NOERROR){
	  rc=fit.r0;
	  tanl=fit.tanl;
	  xc=fit.x0;
	  yc=fit.y0;
	  q=FactorForSenseOfRotation*fit.h;
	  
	  // Get the momentum and position at a specific z position
	  DVector3 mom, pos;
	  GetPositionAndMomentum(first_segment,pos,mom);
	  
	  // Create new track
	  DTrackCandidate *track = new DTrackCandidate;
	  track->rc=rc;
	  track->xc=xc;
	  track->yc=yc;
	  
	  track->setPosition(pos);
	  track->setMomentum(mom);
	  track->setPID((q > 0.0) ? PiPlus : PiMinus);
	  track->Ndof=fit.ndof;
	  track->chisq=fit.chisq;
	  
	  for (it=associated_segments_to_use.begin(); 
	       it!=associated_segments_to_use.end(); ++it){
	    unsigned int packNo=(*it).first;
	    unsigned int segmentNo=(*it).second;	    
	    DFDCSegment *segment=packages[packNo][segmentNo];
	    track->AddAssociatedObject(segment);
	    is_paired[packNo][segmentNo]=1;
	  }
	  
	  Insert(track);
	
	  return true;
	}
      }
    }
  } // got resonance
  
  return false;
}
