// $Id$
//
//    File: DTrackHitSelectorALT2.cc
// Created: Fri Feb  6 08:22:58 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#include <cmath>
using namespace std;

#include <TROOT.h>

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Compatibility/JLockService.h>

#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"
#include <TRACKING/DReferenceTrajectory.h>

#include "DTrackHitSelectorALT2.h"

#ifndef ansi_escape
#define ansi_escape			((char)0x1b)
#define ansi_bold 			ansi_escape<<"[1m"
#define ansi_normal			ansi_escape<<"[0m"
#define ansi_red			ansi_escape<<"[31m"
#define ansi_green			ansi_escape<<"[32m"
#define ansi_blue			ansi_escape<<"[34m"
#endif // ansi_escape

#define ONE_OVER_SQRT12  0.288675
#define ONE_OVER_12 0.08333333333333
#define EPS 1e-6

bool static DTrackHitSelector_cdchit_cmp(pair<double,const DCDCTrackHit *>a,
				      pair<double,const DCDCTrackHit *>b){
  if (a.second->wire->ring!=b.second->wire->ring) 
    return (a.second->wire->ring>b.second->wire->ring);
  return (a.first>b.first);
}
bool static DTrackHitSelector_fdchit_cmp(pair<double,const DFDCPseudo *>a,
				      pair<double,const DFDCPseudo *>b){
  if (a.second->wire->layer!=b.second->wire->layer) 
    return (a.second->wire->layer>b.second->wire->layer);
  return (a.first>b.first);
}
bool static DTrackHitSelector_gemhit_cmp(pair<double,const DGEMPoint *>a,
				      pair<double,const DGEMPoint *>b){
  if (a.second->detector!=b.second->detector) 
    return (a.second->detector>b.second->detector);
  return (a.first>b.first);
}
bool static DTrackHitSelector_trdhit_cmp(pair<double,const DTRDPoint *>a,
				      pair<double,const DTRDPoint *>b){
  if (a.second->detector!=b.second->detector) 
    return (a.second->detector>b.second->detector);
  return (a.first>b.first);
}

bool static DTrackHitSelector_cdchit_in_cmp(const DCDCTrackHit *a, const DCDCTrackHit *b){
  if (a->wire->ring != b->wire->ring) return (a->wire->ring < b->wire->ring);
  return (a->wire->straw < b->wire->straw);
}

bool static DTrackHitSelector_fdchit_in_cmp(const DFDCPseudo *a, const DFDCPseudo *b){
  if (a->wire->layer != b->wire->layer) return (a->wire->layer < b->wire->layer);
  return (a->wire->wire < b->wire->wire);
}


//---------------------------------
// DTrackHitSelectorALT2    (Constructor)
//---------------------------------
DTrackHitSelectorALT2::DTrackHitSelectorALT2(const std::shared_ptr<const JEvent>& event):DTrackHitSelector(event)
{
	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto root_lock = app->GetService<JLockService>();
	auto geo_manager = app->GetService<DGeometryManager>();

	HS_DEBUG_LEVEL = 0;
	MAKE_DEBUG_TREES = false;
	MIN_HIT_PROB_CDC = 0.01;
	MIN_HIT_PROB_FDC = 0.01;
	MIN_HIT_PROB_TRD = 0.01;
	MIN_HIT_PROB_GEM = 0.01;
	MIN_FDC_SIGMA_ANODE_CANDIDATE = 0.1000;
	MIN_FDC_SIGMA_CATHODE_CANDIDATE = 0.1000;
	MIN_FDC_SIGMA_ANODE_WIREBASED = 0.0100;
	MIN_FDC_SIGMA_CATHODE_WIREBASED = 0.0100;
	MAX_DOCA=2.5;

	app->SetDefaultParameter("TRKFIT:MAX_DOCA",MAX_DOCA,"Maximum doca for associating hit with track");

	app->SetDefaultParameter("TRKFIT:HS_DEBUG_LEVEL", HS_DEBUG_LEVEL, "Debug verbosity level for hit selector used in track fitting (0=no debug messages)");
	app->SetDefaultParameter("TRKFIT:MAKE_DEBUG_TREES", MAKE_DEBUG_TREES, "Create a TTree with debugging info on hit selection for the FDC and CDC");
	app->SetDefaultParameter("TRKFIT:MIN_HIT_PROB_CDC", MIN_HIT_PROB_CDC, "Minimum probability a CDC hit may have to be associated with a track to be included in list passed to fitter");
	app->SetDefaultParameter("TRKFIT:MIN_HIT_PROB_FDC", MIN_HIT_PROB_FDC, "Minimum probability a FDC hit may have to be associated with a track to be included in list passed to fitter");
	app->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_ANODE_CANDIDATE", MIN_FDC_SIGMA_ANODE_CANDIDATE, "Minimum sigma used for FDC anode hits on track candidates");
	app->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_CATHODE_CANDIDATE", MIN_FDC_SIGMA_CATHODE_CANDIDATE, "Minimum sigma used for FDC cathode hits on track candidates");
	app->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_ANODE_WIREBASED", MIN_FDC_SIGMA_ANODE_WIREBASED, "Minimum sigma used for FDC anode hits on wire-based tracks");
	app->SetDefaultParameter("TRKFIT:MIN_FDC_SIGMA_CATHODE_WIREBASED", MIN_FDC_SIGMA_CATHODE_WIREBASED, "Minimum sigma used for FDC cathode hits on wire-based tracks");
	
	cdchitsel = NULL;
	fdchitsel = NULL;
	if(MAKE_DEBUG_TREES){
		root_lock->RootWriteLock();

		cdchitsel= (TTree*)gROOT->FindObject("cdchitsel");
		if(!cdchitsel){
			cdchitsel = new TTree("cdchitsel", "CDC Hit Selector");
			cdchitsel->Branch("H", &cdchitdbg, "fit_type/I:p/F:theta:mass:sigma:x:y:z:s:itheta02:itheta02s:itheta02s2:dist:doca:resi:chisq:prob:sig_phi:sig_lambda:sig_pt");
		}else{
			_DBG__;
			jerr<<" !!! WARNING !!!"<<endl;
			jerr<<"It appears that the cdchitsel TTree is already defined."<<endl;
			jerr<<"This is probably means you are running with multiple threads."<<endl;
			jerr<<"To avoid complication, filling of the hit selector debug"<<endl;
			jerr<<"trees will be disabled for this thread."<<endl;
			_DBG__;
			cdchitsel = NULL;
		}

		fdchitsel= (TTree*)gROOT->FindObject("fdchitsel");
		if(!fdchitsel){
			fdchitsel = new TTree("fdchitsel", "FDC Hit Selector");
			fdchitsel->Branch("H", &fdchitdbg, "fit_type/I:p/F:theta:mass:sigma_anode:sigma_cathode:x:y:z:s:itheta02:itheta02s:itheta02s2:dist:doca:resi:u:u_cathodes:resic:chisq:prob:sig_phi:sig_lambda:sig_pt");
		}else{
			_DBG__;
			jerr<<" !!! WARNING !!!"<<endl;
			jerr<<"It appears that the fdchitsel TTree is already defined."<<endl;
			jerr<<"This is probably means you are running with multiple threads."<<endl;
			jerr<<"To avoid complication, filling of the hit selector debug"<<endl;
			jerr<<"trees will be disabled for this thread."<<endl;
			_DBG__;
			fdchitsel = NULL;
		}

		root_lock->RootUnLock();
	}

	bfield = geo_manager->GetBfield(runnumber); // this should be run number based!
	
}

//---------------------------------
// ~DTrackHitSelectorALT2    (Destructor)
//---------------------------------
DTrackHitSelectorALT2::~DTrackHitSelectorALT2()
{

}


//---------------------------------
// GetTRDHits
//---------------------------------
void DTrackHitSelectorALT2::GetTRDHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTRDPoint*> &trdhits_in, vector<const DTRDPoint*> &trdhits_out) const {
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DTRDPoint*> >trdhits_tmp;
  
  for (unsigned int k=0;k<extrapolations.size();k++){
    DVector3 pos=extrapolations[k].position;
    for (unsigned int j=0;j<trdhits_in.size();j++){  
      const DTRDPoint *hit=trdhits_in[j];
      if (int(k)==hit->detector){
	double dx=hit->x-pos.X();
	double dy=hit->y-pos.Y();
	double varx=1.,vary=1.;
	double chisq=dx*dx/varx+dy*dy/vary;
	// Use chi-sq probability function with Ndof=2 to calculate probability
	double probability = TMath::Prob(chisq, 2); 
	if(probability>=MIN_HIT_PROB_GEM){
	  pair<double,const DTRDPoint*>myhit;
	  myhit.first=probability;
	  myhit.second=hit;
	  trdhits_tmp.push_back(myhit);
	}
      }
    }
  }
  // Order according to layer number and probability,then put the hits in the 
  // output list with the following algorithm:  put hits with the highest 
  // probability in a given layer in the output list. 
  sort(trdhits_tmp.begin(),trdhits_tmp.end(),DTrackHitSelector_trdhit_cmp);
  int old_layer=1000;
  for (unsigned int i=0;i<trdhits_tmp.size();i++){
    if (trdhits_tmp[i].second->detector!=old_layer){
      trdhits_out.push_back(trdhits_tmp[i].second);   
    }
    old_layer=trdhits_tmp[i].second->detector;
  }
}

//---------------------------------
// GetGEMHits
//---------------------------------
void DTrackHitSelectorALT2::GetGEMHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DGEMPoint*> &gemhits_in, vector<const DGEMPoint*> &gemhits_out) const {
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DGEMPoint*> >gemhits_tmp;

  for (unsigned int k=0;k<extrapolations.size();k++){
    DVector3 pos=extrapolations[k].position;
    for (unsigned int j=0;j<gemhits_in.size();j++){
      const DGEMPoint *hit=gemhits_in[j];
      if (int(k)==hit->detector){
	double dx=hit->x-pos.X();
	double dy=hit->y-pos.Y();
	double varx=1.,vary=1.;
	double chisq=dx*dx/varx+dy*dy/vary;
	// Use chi-sq probability function with Ndof=2 to calculate probability
	double probability = TMath::Prob(chisq, 2);
	if(probability>=MIN_HIT_PROB_GEM){
	  pair<double,const DGEMPoint*>myhit;
	  myhit.first=probability;
	  myhit.second=hit;
	  gemhits_tmp.push_back(myhit);
	}
      }
    }

  }
  // Order according to layer number and probability,then put the hits in the 
  // output list with the following algorithm:  put hits with the highest 
  // probability in a given layer in the output list. 
  sort(gemhits_tmp.begin(),gemhits_tmp.end(),DTrackHitSelector_gemhit_cmp);
  int old_layer=1000;
  for (unsigned int i=0;i<gemhits_tmp.size();i++){
    if (gemhits_tmp[i].second->detector!=old_layer){
      gemhits_out.push_back(gemhits_tmp[i].second);   
    }
    old_layer=gemhits_tmp[i].second->detector;
  }
  
}

//---------------------------------
// GetCDCHits
//---------------------------------
void DTrackHitSelectorALT2::GetCDCHits(double Bz,double q,
				       const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out, int N) const
{
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DCDCTrackHit*> >cdchits_tmp;

  // Sort so innermost ring is first and outermost is last
  vector<const DCDCTrackHit*> cdchits_in_sorted = cdchits_in;
  sort(cdchits_in_sorted.begin(),cdchits_in_sorted.end(),DTrackHitSelector_cdchit_in_cmp);
  
  // The variance on the residual due to measurement error.
  double var=0.25*2.4336*ONE_OVER_12;
   
  // To estimate the impact of errors in the track momentum on the variance 
  // of the residual, use a helical approximation.
  double a=-0.003*Bz*q;
  DVector3 mom=extrapolations[extrapolations.size()/2].momentum;
  double p=mom.Mag();
  double p_over_a=p/a;
  double a_over_p=1./p_over_a;
  double lambda=M_PI_2-mom.Theta();
  double tanl=tan(lambda);
  double tanl2=tanl*tanl;
  double sinl=sin(lambda);
  double cosl=cos(lambda);
  double cosl2=cosl*cosl;
  double sinl2=sinl*sinl;
  double pt_over_a=cosl*p_over_a;

  // variances
  double var_lambda_res=0.;
  double var_phi_radial=0.;
  double var_x0=0.0,var_y0=0.0;
  double var_k=0.;

  // Loop over all the CDC hits looking for matches with the track
  bool outermost_hit=true;
  vector<const DCDCTrackHit*>::const_reverse_iterator iter;
  for(iter=cdchits_in_sorted.rbegin(); iter!=cdchits_in_sorted.rend(); iter++){
    const DCDCTrackHit *hit = *iter;
    DVector3 origin=hit->wire->origin;
    DVector3 dir=hit->wire->udir;
    double ux=dir.x();
    double uy=dir.y();
    double uz=dir.z();
    double z0=origin.z();
    double d2=0.,d2_old=1.e6;
    double s=0.;
    DVector3 old_trackpos;
    for (unsigned int i=0;i<extrapolations.size();i++){
      DVector3 trackpos=extrapolations[i].position;
      double dz=trackpos.z()-z0;  
      DVector3 wirepos=origin+dz/uz*dir;      
      d2=(trackpos-wirepos).Perp2();
      if (d2>d2_old){
	if (d2<4){ // has to be reasonably close to the wire in question
	  double phi=extrapolations[i].momentum.Phi();
	  double sinphi=sin(phi);
	  double cosphi=cos(phi);
	  // Variables relating wire direction and track direction
	  double my_ux=ux*sinl-cosl*cosphi;
	  double my_uy=uy*sinl-cosl*sinphi;
	  double denom=my_ux*my_ux+my_uy*my_uy;
	  // For simplicity make a linear approximation for the path increment
	  double ds=((old_trackpos.x()-origin.x()-ux*dz)*my_ux
		     +(old_trackpos.y()-origin.y()-uy*dz)*my_uy)/denom;
	  wirepos+=ds*dir;
	  double dx=old_trackpos.x()+mom.X()/mom.Mag()*ds-wirepos.x();
	  double dy=old_trackpos.y()+mom.Y()/mom.Mag()*ds-wirepos.y();
	  d2_old=d2;
	  d2=dx*dx+dy*dy;
	  if (d2>d2_old){
	    // linear approximation did not work...
	    d2=d2_old;
	  }
	  // Variance in dip angle due to multiple scattering
	  double var_lambda = extrapolations[i].theta2ms_sum/3.;
	  // variance in phi due to multiple scattering
	  double var_phi=var_lambda*(1.+tanl2);
	
	  if (outermost_hit){
	    // variance in the curvature k due to resolution and multiple scattering
	    double s_sq=s*s;
	    double s_4=s_sq*s_sq;
	    double sum_s_theta_ms=extrapolations[i].s_theta_ms_sum;
	    double var_k_ms=(4./3.)*sum_s_theta_ms*sum_s_theta_ms/s_4;
	    double var_k_res=var*0.0720/double(N+4)/s_4/(cosl2*cosl2);
	    var_k=var_k_ms+var_k_res;

	    // Variance in dip angle due to measurement error
	    var_lambda_res=12.0*var*double(N-1)/double(N*(N+1))
	      *sinl2*sinl2/s_sq;
	    
	    // Variance in phi, crude approximation
	    double tan_s_2R=tan(s*cosl/(2.*pt_over_a));
	    var_phi_radial=tan_s_2R*tan_s_2R*pt_over_a*pt_over_a*var_k;
	    
	    outermost_hit=false;
	  } 
	  // Include error in lambda due to measurements
	  var_lambda+=var_lambda_res;

	  // Include uncertainty in phi due to uncertainty in the center of 
	  // the circle
	  var_phi+=var_phi_radial;

	  // Variance in position due to multiple scattering
	  double var_pos_ms=extrapolations[i].s_theta_ms_sum/3.;

	  // Hit doca and residual
	  double doca=sqrt(d2);
	  double dist=0.39;
	  double resi=dist-doca;

	  // Variances in x and y due to uncertainty in track parameters
	  double as_over_p=s*a_over_p;
	  double sin_as_over_p=sin(as_over_p);
	  double cos_as_over_p=cos(as_over_p);
	  double one_minus_cos_as_over_p=1-cos_as_over_p;
	  double diff1=sin_as_over_p-as_over_p*cos_as_over_p;
	  double diff2=one_minus_cos_as_over_p-as_over_p*sin_as_over_p;
	  double dx_dk=2.*pt_over_a*pt_over_a*(sinphi*diff2-cosphi*diff1);
	  double dx_dcosl=p_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
	  double dx_dphi=-pt_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
	  double var_x=var_x0+dx_dk*dx_dk*var_k+var_pos_ms
	    +dx_dcosl*dx_dcosl*sinl*sinl*var_lambda+dx_dphi*dx_dphi*var_phi;
    
	  double dy_dk=-2.*pt_over_a*pt_over_a*(sinphi*diff1+cosphi*diff2);
	  double dy_dcosl=p_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
	  double dy_dphi=pt_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
	  double var_y=var_y0+dy_dk*dy_dk*var_k+var_pos_ms
	    +dy_dcosl*dy_dcosl*sinl*sinl*var_lambda+dy_dphi*dy_dphi*var_phi;
	  
	  double dd_dx=dx/doca;
	  double dd_dy=dy/doca;
	  double var_d=dd_dx*dd_dx*var_x+dd_dy*dd_dy*var_y;
	  double chisq=resi*resi/(var+var_d);
    
	  // Use chi-sq probability function with Ndof=1 to calculate probability
	  double probability = TMath::Prob(chisq, 1);
	  if(probability>=MIN_HIT_PROB_CDC){
	    pair<double,const DCDCTrackHit*>myhit;
	    myhit.first=probability;
	    myhit.second=hit;
	    cdchits_tmp.push_back(myhit);
	  }

	  // Optionally fill debug tree
	  if(cdchitsel){
	    cdchitdbg.fit_type = kWireBased;
	    cdchitdbg.p = p;
	    cdchitdbg.theta = extrapolations[i].momentum.Theta();
	    //cdchitdbg.mass = mass;
	    cdchitdbg.sigma = sqrt(var+var_d);
	    cdchitdbg.x = old_trackpos.X();
	    cdchitdbg.y = old_trackpos.Y();
	    cdchitdbg.z = old_trackpos.Z();
	    cdchitdbg.s = s;
	    cdchitdbg.itheta02 = extrapolations[i].theta2ms_sum;
	    //cdchitdbg.itheta02s = last_step->itheta02s;
	    //cdchitdbg.itheta02s2 = last_step->itheta02s2;
	    cdchitdbg.dist = dist;
	    cdchitdbg.doca = doca;
	    cdchitdbg.resi = resi;
	    cdchitdbg.chisq = chisq;
	    cdchitdbg.prob = probability;
	    cdchitdbg.sig_phi=sqrt(var_phi);
	    cdchitdbg.sig_lambda=sqrt(var_lambda);
	    cdchitdbg.sig_pt=fabs(2.*pt_over_a)*sqrt(var_k);	
	    
	    cdchitsel->Fill();
	  }
    
	  if(HS_DEBUG_LEVEL>10){
	    _DBG_;
	    if (probability>=MIN_HIT_PROB_CDC) jerr<<"---> ";
	    jerr<<"s="<<s<<" t=" <<hit->tdrift   << " doca="<<doca<<" dist="<<dist<<" resi="<<resi<<" sigma="/*<<sigma_total*/<<" prob="<<probability<<endl;
	  }
	}
	break;
      }
      d2_old=d2;
      old_trackpos=trackpos;
      s=extrapolations[i].s;
    }
  
  }

   // Order according to ring number and probability, then put the hits in the 
  // output list with the following algorithm:  hits with the highest 
  // probability in a given ring are automatically put in the output list, 
  // but if there is more than one hit in a given ring, only those hits 
  // that are within +/-1 of the straw # of the most probable hit are added 
  // to the list.
  sort(cdchits_tmp.begin(),cdchits_tmp.end(),DTrackHitSelector_cdchit_cmp);
  int old_straw=1000,old_ring=1000;
  for (unsigned int i=0;i<cdchits_tmp.size();i++){
    if (cdchits_tmp[i].second->wire->ring!=old_ring || 
	abs(cdchits_tmp[i].second->wire->straw-old_straw)==1){
      cdchits_out.push_back(cdchits_tmp[i].second);   
    }
    old_straw=cdchits_tmp[i].second->wire->straw;
    old_ring=cdchits_tmp[i].second->wire->ring;
  }
}

//---------------------------------
// GetCDCHits
//---------------------------------
void DTrackHitSelectorALT2::GetCDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out, int N) const
{
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DCDCTrackHit*> >cdchits_tmp;

  /// Determine the probability that for each CDC hit that it came from the 
  /// track with the given trajectory.
  ///
  /// This will calculate a probability for each CDC hit that
  /// it came from the track represented by the given
  /// DReference trajectory. The probability is based on
  /// the residual between the distance of closest approach
  /// of the trajectory to the wire and the drift time for
  /// time-based tracks and the distance to the wire for
  /// wire-based tracks.

  // Sort so innermost ring is first and outermost is last
  vector<const DCDCTrackHit*> cdchits_in_sorted = cdchits_in;
  sort(cdchits_in_sorted.begin(),cdchits_in_sorted.end(),DTrackHitSelector_cdchit_in_cmp);
  
  // Calculate beta of particle.
  //double my_mass=rt->GetMass();
  //  double one_over_beta =sqrt(1.0+my_mass*my_mass/rt->swim_steps[0].mom.Mag2());
  
  // The variance on the residual due to measurement error.
  double var=0.25*2.4336*ONE_OVER_12;
   
  // To estimate the impact of errors in the track momentum on the variance of the residual,
  // use a helical approximation.
  const DReferenceTrajectory::swim_step_t *my_step=&rt->swim_steps[0];
  double Bz0=my_step->B.z();
  double a=-0.003*Bz0*rt->q;
  double p=my_step->mom.Mag();
  double p_over_a=p/a;
  double a_over_p=1./p_over_a;
  double lambda=M_PI_2-my_step->mom.Theta();
  double cosl=cos(lambda);
  double sinl=sin(lambda);
  double sinl2=sinl*sinl;
  double cosl2=cosl*cosl;
  double tanl=tan(lambda);
  double tanl2=tanl*tanl;
  double pt_over_a=cosl*p_over_a;
  double phi=my_step->mom.Phi();
  double cosphi=cos(phi);
  double sinphi=sin(phi);
  
  double var_lambda_res=0.;
  double var_phi_radial=0.;
  double var_x0=0.0,var_y0=0.0;
  double mass=rt->GetMass();
  double one_over_beta=sqrt(1.+mass*mass/(p*p));
  double var_pt_factor=0.016*one_over_beta/(cosl*a);
  double var_pt_over_pt_sq=0.;

  // Keep track of straws and rings
  int old_straw=1000,old_ring=1000;

  // Loop over hits
  bool outermost_hit=true;
  vector<const DCDCTrackHit*>::const_reverse_iterator iter;
  for(iter=cdchits_in_sorted.rbegin(); iter!=cdchits_in_sorted.rend(); iter++){
    const DCDCTrackHit *hit = *iter;
    
    // Skip hit if it is on the same wire as the previous hit
    if (hit->wire->ring == old_ring && hit->wire->straw==old_straw){
      //_DBG_ << "ring " << hit->wire->ring << " straw " << hit->wire->straw << endl;
      continue;
    }
    old_ring=hit->wire->ring;
    old_straw=hit->wire->straw;

    // Find the DOCA to this wire
    double s=0.;
    double doca = rt->DistToRT(hit->wire, &s);
    
    if(!isfinite(doca)) continue;
    if(!isfinite(s))continue;
    if (s<1.) continue;
    if (doca>MAX_DOCA) continue;
    
    const DReferenceTrajectory::swim_step_t *last_step = rt->GetLastSwimStep();
    
    // Position along trajectory
    DVector3 pos=rt->GetLastDOCAPoint();
    
    // Compensate for the fact that the field at the "origin" of the 
    // track does not correspond to the average Bz used to compute pt
    double Bratio=last_step->B.z()/Bz0;
    double invBratio=1./Bratio;
    pt_over_a*=invBratio;
    p_over_a*=invBratio;
    a_over_p*=Bratio;
  
    // Variances in angles due to multiple scattering
    double var_lambda = last_step->itheta02/3.;
    double var_phi=var_lambda*(1.+tanl2);
    
    if (outermost_hit){   
      // Fractional variance in the curvature k due to resolution and multiple scattering
      double s_sq=s*s;
      double var_k_over_k_sq_res=var*p_over_a*p_over_a*0.0720/double(N+4)/(s_sq*s_sq*sinl2)/cosl2;
      double var_k_over_k_sq_ms=var_pt_factor*var_pt_factor*last_step->invX0/s;
      // Fractional variance in pt
      var_pt_over_pt_sq=var_k_over_k_sq_ms+var_k_over_k_sq_res;
      
      // Variance in dip angle due to measurement error
      var_lambda_res=12.0*var*double(N-1)/double(N*(N+1))*cosl2*cosl2/s_sq;

      // Variance in phi, crude approximation
      double tan_s_2R=tan(s*cosl/(2.*pt_over_a));
      var_phi_radial=tan_s_2R*tan_s_2R*var_pt_over_pt_sq;

      outermost_hit=false;
    }
    // Include error in lambda due to measurements
    var_lambda+=var_lambda_res;
    
    // Include uncertainty in phi due to uncertainty in the radius of 
    // the circle
    var_phi+=var_phi_radial;

    // Get "measured" distance to wire. 
    double dist=0.39;
    
    // Residual
    double resi = dist - doca;

    // Variance in position due to multiple scattering
    double var_pos_ms=last_step->itheta02s2/3.;
	    
    // Variances in x and y due to uncertainty in track parameters
    double as_over_p=s*a_over_p;
    double sin_as_over_p=sin(as_over_p);
    double cos_as_over_p=cos(as_over_p);
    double one_minus_cos_as_over_p=1-cos_as_over_p;
    double diff1=sin_as_over_p-as_over_p*cos_as_over_p;
    double diff2=one_minus_cos_as_over_p-as_over_p*sin_as_over_p;
    double pdx_dp=pt_over_a*(cosphi*diff1-sinphi*diff2);
    double dx_dcosl=p_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
    double dx_dphi=-pt_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
    double var_x=var_x0+pdx_dp*pdx_dp*var_pt_over_pt_sq+var_pos_ms
      +dx_dcosl*dx_dcosl*sinl*sinl*var_lambda+dx_dphi*dx_dphi*var_phi;
    
    double pdy_dp=pt_over_a*(sinphi*diff1+cosphi*diff2);
    double dy_dcosl=p_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);
    double dy_dphi=pt_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
    double var_y=var_y0+pdy_dp*pdy_dp*var_pt_over_pt_sq+var_pos_ms
      +dy_dcosl*dy_dcosl*sinl*sinl*var_lambda+dy_dphi*dy_dphi*var_phi;
    
    
    DVector3 origin=hit->wire->origin;
    DVector3 dir=hit->wire->udir;
    double uz=dir.z();
    double z0=origin.z();
    DVector3 wirepos=origin+(pos.z()-z0)/uz*dir;
    double dd_dx=(pos.x()-wirepos.x())/doca;
    double dd_dy=(pos.y()-wirepos.y())/doca;
    double var_d=dd_dx*dd_dx*var_x+dd_dy*dd_dy*var_y;

    double chisq=resi*resi/(var+var_d);
    
    // Use chi-sq probability function with Ndof=1 to calculate probability
    double probability = TMath::Prob(chisq, 1);
    if(probability>=MIN_HIT_PROB_CDC){
      pair<double,const DCDCTrackHit*>myhit;
      myhit.first=probability;
      myhit.second=hit;
      cdchits_tmp.push_back(myhit);
    }

    // Optionally fill debug tree
    if(cdchitsel){
      cdchitdbg.fit_type = fit_type;
      cdchitdbg.p = p;
      cdchitdbg.theta = rt->swim_steps[0].mom.Theta();
      cdchitdbg.mass = mass;
      cdchitdbg.sigma = sqrt(var+var_d);
      cdchitdbg.x = pos.X();
      cdchitdbg.y = pos.Y();
      cdchitdbg.z = pos.Z();
      cdchitdbg.s = s;
      cdchitdbg.itheta02 = last_step->itheta02;
      cdchitdbg.itheta02s = last_step->itheta02s;
      cdchitdbg.itheta02s2 = last_step->itheta02s2;
      cdchitdbg.dist = dist;
      cdchitdbg.doca = doca;
      cdchitdbg.resi = resi;
      cdchitdbg.chisq = chisq;
      cdchitdbg.prob = probability;
      cdchitdbg.sig_phi=sqrt(var_phi);
      cdchitdbg.sig_lambda=sqrt(var_lambda);
      cdchitdbg.sig_pt=sqrt(var_pt_over_pt_sq);	

      cdchitsel->Fill();
    }
    
    if(HS_DEBUG_LEVEL>10){
      _DBG_;
      if (probability>=MIN_HIT_PROB_CDC) jerr<<"---> ";
      jerr<<"s="<<s<<" t=" <<hit->tdrift   << " doca="<<doca<<" dist="<<dist<<" resi="<<resi<<" sigma="/*<<sigma_total*/<<" prob="<<probability<<endl;
    }
  }

  // Order according to ring number and probability, then put the hits in the 
  // output list with the following algorithm:  hits with the highest 
  // probability in a given ring are automatically put in the output list, 
  // but if there is more than one hit in a given ring, only those hits 
  // that are within +/-1 of the straw # of the most probable hit are added 
  // to the list.
  sort(cdchits_tmp.begin(),cdchits_tmp.end(),DTrackHitSelector_cdchit_cmp);
  old_straw=1000,old_ring=1000;
  for (unsigned int i=0;i<cdchits_tmp.size();i++){
    if (cdchits_tmp[i].second->wire->ring!=old_ring || 
	abs(cdchits_tmp[i].second->wire->straw-old_straw)==1){
      cdchits_out.push_back(cdchits_tmp[i].second);   
    }
    old_straw=cdchits_tmp[i].second->wire->straw;
    old_ring=cdchits_tmp[i].second->wire->ring;
  }
}

//---------------------------------
// GetFDCHits
//---------------------------------
void DTrackHitSelectorALT2::GetFDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out,int N) const
{
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DFDCPseudo*> >fdchits_tmp;
  
  /// Determine the probability that for each FDC hit that it came from the 
  /// track with the given trajectory.
  ///
  /// This will calculate a probability for each FDC hit that
  /// it came from the track represented by the given
  /// DReference trajectory. The probability is based on
  /// the residual between the distance of closest approach
  /// of the trajectory to the wire and the drift distance
  /// and the distance along the wire.

  // Sort so innermost ring is first and outermost is last
  vector<const DFDCPseudo*> fdchits_in_sorted = fdchits_in;
  sort(fdchits_in_sorted.begin(),fdchits_in_sorted.end(),DTrackHitSelector_fdchit_in_cmp);

  // The variance on the residual due to measurement error.
  double var_anode = 0.25*ONE_OVER_12;
  const double VAR_CATHODE_STRIPS=0.000225;
  double var_cathode = VAR_CATHODE_STRIPS; 

  // To estimate the impact of errors in the track momentum on the variance of the residual,
  // use a helical approximation. 
  const DReferenceTrajectory::swim_step_t *my_step=&rt->swim_steps[0];
  double Bz0=my_step->B.z();
  double z0=my_step->origin.z();
  double a=-0.003*Bz0*rt->q;
  double p=my_step->mom.Mag();
  double p_sq=p*p;
  double p_over_a=p/a;
  double a_over_p=1./p_over_a;
  double lambda=M_PI_2-my_step->mom.Theta();
  double cosl=cos(lambda);
  double cosl2=cosl*cosl;
  double sinl=sin(lambda);
  double sinl2=sinl*sinl;
  double tanl=tan(lambda);
  double tanl2=tanl*tanl;
  double pt_over_a=cosl*p_over_a;
  double phi=my_step->mom.Phi();
  double cosphi=cos(phi);
  double sinphi=sin(phi);
  double var_lambda=0.,var_phi=0.,var_lambda_res=0.;
  double var_phi_radial=0.;
  double mass=rt->GetMass();

  double var_x0=0.0,var_y0=0.0; 
  double var_pt_over_pt_sq=0.;

  // Loop over hits
  bool most_downstream_hit=true;
  vector<const DFDCPseudo*>::const_reverse_iterator iter;
  for(iter=fdchits_in_sorted.rbegin(); iter!=fdchits_in_sorted.rend(); iter++){
    const DFDCPseudo *hit = *iter;
    
    // Find the DOCA to this wire
    double s=0.;
    double doca = rt->DistToRT(hit->wire, &s); 

    if(!isfinite(doca)) continue;
    if(!isfinite(s))continue;
    if (s<1.) continue;
    if (doca>MAX_DOCA)continue;

    const DReferenceTrajectory::swim_step_t *last_step = rt->GetLastSwimStep();
     
    // Position along trajectory
    DVector3 pos=rt->GetLastDOCAPoint();
    double dz=pos.z()-z0;

    // Direction variables for wire
    double cosa=hit->wire->udir.y();
    double sina=hit->wire->udir.x();

    // Cathode Residual
    double u=rt->GetLastDistAlongWire();
    double u_cathodes = hit->s;
    double resic = u - u_cathodes;

    // Get "measured" distance to wire.
    double dist=0.25;
    
    // Take into account non-normal incidence to FDC plane
    double pz=last_step->mom.z();
    double tx=last_step->mom.x()/pz;
    double ty=last_step->mom.y()/pz;
    double tu=tx*cosa-ty*sina;
    double alpha=atan(tu);
    double cosalpha=cos(alpha);

    // Compensate for the fact that the field at the "origin" of the 
    // track does not correspond to the average Bz used to compute pt
    double Bz=last_step->B.z();
    double Bratio=Bz/Bz0;
    double invBratio=1./Bratio;
    pt_over_a*=invBratio;
    p_over_a*=invBratio;
    a_over_p*=Bratio;

    // Anode Residual
    double resi = dist - doca/cosalpha;

    // Initialize some probability-related variables needed later
    double probability=0.,chisq=0.;
 
    if (fit_type!=kHelical){
      // Correct for deflection of avalanche position due to Lorentz force
      double sign=(pos.x()*cosa-pos.y()*sina-hit->w)<0?1:-1.;	
      double ucor=0.1458*Bz*(1.-0.048*last_step->B.Perp())*sign*doca;
      resic-=ucor;
    }
    else{   
      // Cathode variance due to Lorentz deflection
      double max_deflection=0.1458*Bz*(1.-0.048*last_step->B.Perp())*0.5;
      var_cathode=VAR_CATHODE_STRIPS+max_deflection*max_deflection/12.;
    }
    double var_tot=var_anode+var_cathode;

    // Variance in angles due to multiple scattering
    var_lambda = last_step->itheta02/3.;
    var_phi=var_lambda*(1.+tanl2);

    if (most_downstream_hit){
      // Fractional variance in the curvature k due to resolution and multiple scattering
      double s_sq=s*s;
      double var_k_over_k_sq_res=var_tot*p_over_a*p_over_a
	*0.0720/double(N+4)/(s_sq*s_sq)/cosl2;
      
	
       double one_over_beta=sqrt(1.+mass*mass/p_sq);
       double var_pt_factor=0.016*one_over_beta/(cosl*0.003*last_step->B.z());
       double var_k_over_k_sq_ms=var_pt_factor*var_pt_factor*last_step->invX0/s;
       // Fractional variance in pt
       var_pt_over_pt_sq=var_k_over_k_sq_ms+var_k_over_k_sq_res;
	
       // Variance in dip angle due to measurement error
       var_lambda_res=12.0*var_tot*double(N-1)/double(N*(N+1))
	 *sinl2*sinl2/s_sq;

       // Variance in phi, crude approximation
       double tan_s_2R=tan(s*cosl/(2.*pt_over_a));
       var_phi_radial=tan_s_2R*tan_s_2R*var_pt_over_pt_sq;
       
       most_downstream_hit=false;
    }
    
    // Include error in lambda due to measurements
    var_lambda+=var_lambda_res;

    // Include uncertainty in phi due to uncertainty in the center of 
    // the circle
    var_phi+=var_phi_radial;

    var_phi=0.;
    var_lambda=0.;
    
    // Variance in position due to multiple scattering
    double var_pos_ms=last_step->itheta02s2/3.;

    // Variances in x and y due to uncertainty in track parameters
    double as_over_p=s*a_over_p;
    double sin_as_over_p=sin(as_over_p);
    double cos_as_over_p=cos(as_over_p);
    double one_minus_cos_as_over_p=1-cos_as_over_p;
    double diff1=sin_as_over_p-as_over_p*cos_as_over_p;
    double diff2=one_minus_cos_as_over_p-as_over_p*sin_as_over_p;
    double pdx_dp=pt_over_a*(cosphi*diff1-sinphi*diff2);
    double dx_ds=cosl*(cosphi*cos_as_over_p-sinphi*sin_as_over_p);
    double ds_dcosl=dz*cosl/(sinl*sinl2);
    double dx_dcosl
      =p_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p)
      +dx_ds*ds_dcosl;
    double dx_dphi=-pt_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);  
    double var_z0=2.*tanl2*(var_tot)*double(2*N-1)/double(N*(N+1));
   
    double var_x=var_x0+pdx_dp*pdx_dp*var_pt_over_pt_sq+var_pos_ms
      +dx_dcosl*dx_dcosl*sinl2*var_lambda+dx_dphi*dx_dphi*var_phi
      +dx_ds*dx_ds*var_z0/sinl2;
    
    double pdy_dp=pt_over_a*(sinphi*diff1+cosphi*diff2);
    double dy_ds=cosl*(sinphi*cos_as_over_p+cosphi*sin_as_over_p);
    double dy_dcosl
      =p_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p)
      +dy_ds*ds_dcosl;
    double dy_dphi=pt_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
    double var_y=var_y0+pdy_dp*pdy_dp*var_pt_over_pt_sq+var_pos_ms
      +dy_dcosl*dy_dcosl*sinl2*var_lambda+dy_dphi*dy_dphi*var_phi
      +dy_ds*dy_ds*var_z0/sinl2;

    // Rotate from global coordinate system into FDC local system
    double cos2a=cosa*cosa;
    double sin2a=sina*sina;
    double var_d=(cos2a*var_x+sin2a*var_y)/(cosalpha*cosalpha);
    double var_u=cos2a*var_y+sin2a*var_x;    

    if (fit_type!=kHelical){ 
      // Factors take into account improved resolution after wire-based fit
      var_d*=0.1;
      var_u*=0.1;
    }

    // Calculate chisq
    chisq = resi*resi/(var_d+var_anode)+resic*resic/(var_u+var_cathode);
    
    // Probability of this hit being on the track
    probability = TMath::Prob(chisq,2);
  
    if(probability>=MIN_HIT_PROB_FDC){
      pair<double,const DFDCPseudo*>myhit;
      myhit.first=probability;
      myhit.second=hit;
      fdchits_tmp.push_back(myhit);
    }
      
    // Optionally fill debug tree
    if(fdchitsel){
      fdchitdbg.fit_type = fit_type;
      fdchitdbg.p = p;
      fdchitdbg.theta = rt->swim_steps[0].mom.Theta();
      fdchitdbg.mass = mass;
      fdchitdbg.sigma_anode = sqrt(var_d+var_anode);
      fdchitdbg.sigma_cathode = sqrt(var_u+var_cathode);
      fdchitdbg.x = pos.X();
      fdchitdbg.y = pos.Y();
      fdchitdbg.z = pos.Z();
      fdchitdbg.s = s;
      fdchitdbg.itheta02 = last_step->itheta02;
      fdchitdbg.itheta02s = last_step->itheta02s;
      fdchitdbg.itheta02s2 = last_step->itheta02s2;
      fdchitdbg.dist = dist;
      fdchitdbg.doca = doca;
      fdchitdbg.resi = resi;
      fdchitdbg.u = u;
      fdchitdbg.u_cathodes = u_cathodes;
      fdchitdbg.resic = resic;
      fdchitdbg.chisq = chisq;
      fdchitdbg.prob = probability;
      fdchitdbg.sig_phi=sqrt(var_phi);
      fdchitdbg.sig_lambda=sqrt(var_lambda);
      fdchitdbg.sig_pt=sqrt(var_pt_over_pt_sq);
      
      fdchitsel->Fill();
    }
    
    if(HS_DEBUG_LEVEL>10){
      _DBG_;
      if(probability>=MIN_HIT_PROB_FDC)jerr<<"----> ";
      jerr<<"s="<<s<<" doca="<<doca<<" dist="<<dist<<" resi="<<resi<<" resic="<<resic<<" chisq="<<chisq<<" prob="<<probability<<endl;
    }
  }
  // Order according to layer number and probability,then put the hits in the 
  // output list with the following algorithm:  hits with the highest 
  // probability in a given layer are automatically put in the output list, 
  // but if there is more than one hit in a given layer, only those hits 
  // that are within +/-1 of the wire # of the most probable hit are added 
  // to the list.
  sort(fdchits_tmp.begin(),fdchits_tmp.end(),DTrackHitSelector_fdchit_cmp);
  int old_layer=1000,old_wire=1000;
  for (unsigned int i=0;i<fdchits_tmp.size();i++){
    if (fdchits_tmp[i].second->wire->layer!=old_layer || 
	abs(fdchits_tmp[i].second->wire->wire-old_wire)==1){
      fdchits_out.push_back(fdchits_tmp[i].second);   
    }
    old_wire=fdchits_tmp[i].second->wire->wire;
    old_layer=fdchits_tmp[i].second->wire->layer;
  }
}
void DTrackHitSelectorALT2::GetFDCHits(double Bz,double q,
				       const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out,int N) const
{
  // Vector of pairs storing the hit with the probability it is on the track
  vector<pair<double,const DFDCPseudo*> >fdchits_tmp;
  
  /// Determine the probability that for each FDC hit that it came from the 
  /// track with the given trajectory.  The probability is based on
  /// the residual between the distance of closest approach
  /// of the trajectory to the wire and the drift distance
  /// and the distance along the wire.

  // Sort so innermost plane is first and outermost is last
  vector<const DFDCPseudo*> fdchits_in_sorted = fdchits_in;
  sort(fdchits_in_sorted.begin(),fdchits_in_sorted.end(),DTrackHitSelector_fdchit_in_cmp);

  // The variance on the residual due to measurement error.
  double var_anode = 0.25*ONE_OVER_12;
  const double VAR_CATHODE_STRIPS=0.000225;
  double var_cathode = VAR_CATHODE_STRIPS+0.15*0.15*ONE_OVER_12; 
  double var_tot=var_anode+var_cathode;

  // To estimate the impact of errors in the track momentum on the variance of the residual,
  // use a helical approximation. 
  double a=-0.003*Bz*q;
  DVector3 mom=extrapolations[extrapolations.size()/2].momentum;
  double p=mom.Mag();
  double p_over_a=p/a;
  double a_over_p=1./p_over_a;
  double lambda=M_PI_2-mom.Theta();
  double tanl=tan(lambda);
  double tanl2=tanl*tanl;
  double sinl=sin(lambda);
  double cosl=cos(lambda);  
  double cosl2=cosl*cosl;
  double sinl2=sinl*sinl;
  double pt_over_a=cosl*p_over_a;
  double z0=extrapolations[0].position.z();
  // variances
  double var_lambda=0.,var_phi=0.,var_lambda_res=0.,var_k=0.;
  double var_phi_radial=0.;
  double var_x0=0.0,var_y0=0.0; 
  double var_z0=2.*tanl2*(var_tot)*double(2*N-1)/double(N*(N+1));

  // Loop over hits
  bool most_downstream_hit=true;
  int last_extrapolation_index=extrapolations.size()-1;
  vector<const DFDCPseudo*>::const_reverse_iterator iter;
  for(iter=fdchits_in_sorted.rbegin(); iter!=fdchits_in_sorted.rend(); iter++){
    const DFDCPseudo *hit = *iter;
    for (int k=last_extrapolation_index;k>=0;k--){
      // Position along trajectory
      DVector3 pos=extrapolations[k].position;
      double dz=pos.z()-z0;
      double s=extrapolations[k].s;
      if (fabs(pos.z()-hit->wire->origin.z())<0.5){
	// Variance in dip angle due to multiple scattering
	var_lambda = extrapolations[k].theta2ms_sum/3.;
	// the above expression seems to lead to overestimation of  the uncertainty in the dip angle after the wire-based pass..
	var_lambda*=0.01;
	// Variance in phi due to multiple scattering
	var_phi=var_lambda*(1.+tanl2); 

	// azimuthal angle
	double phi=extrapolations[k].momentum.Phi();
	double sinphi=sin(phi);
	double cosphi=cos(phi);
	  
	if (most_downstream_hit){
	  // variance in the curvature k due to resolution and multiple scattering
	  double s_sq=s*s;
	  double s_4=s_sq*s_sq;
	  double sum_s_theta_ms=extrapolations[k].s_theta_ms_sum;
	  double var_k_ms=(4./3.)*sum_s_theta_ms*sum_s_theta_ms/s_4;
	  double var_k_res=var_tot*0.0720/double(N+4)/s_4/(cosl2*cosl2);
	  var_k=var_k_ms+var_k_res;

	  // Variance in dip angle due to measurement error
	  var_lambda_res=12.0*var_tot*double(N-1)/double(N*(N+1))
	    *sinl2*sinl2/s_sq;

	  // Variance in phi, crude approximation
	  double tan_s_2R=tan(s*cosl/(2.*pt_over_a));
	  var_phi_radial=tan_s_2R*tan_s_2R*pt_over_a*pt_over_a*var_k;
	  
	  most_downstream_hit=false;
	}
	// Include error in lambda due to measurements
	var_lambda+=var_lambda_res;	

	// Include uncertainty in phi due to uncertainty in the center of 
	// the circle
	var_phi+=var_phi_radial;

	// Variance in position due to multiple scattering
	double var_pos_ms=extrapolations[k].s_theta_ms_sum/3.;
	
	// doca to wire
	double x=pos.x();
	double y=pos.y();
	double dx=hit->xy.X()-x;
	double dy=hit->xy.Y()-y;
	double doca=sqrt(dx*dx+dy*dy);

	// Direction variables for wire
	double cosa=hit->wire->udir.y();
	double sina=hit->wire->udir.x();
	
	// Cathode Residual
	double u=x*sina+y*cosa;
	double u_cathodes = hit->s;
	double resic = u - u_cathodes;

	// Get "measured" distance to wire.
	double dist=0.25;
	
	// Take into account non-normal incidence to FDC plane
	double pz=extrapolations[k].momentum.z();
	double tx=extrapolations[k].momentum.x()/pz;
	double ty=extrapolations[k].momentum.y()/pz;
	double tu=tx*cosa-ty*sina;
	double alpha=atan(tu);
	double cosalpha=cos(alpha);

	// Anode Residual
	double resi = dist - doca/cosalpha;
	
	// Initialize some probability-related variables needed later
	double probability=0.,chisq=0.;  

	// Variances in x and y due to uncertainty in track parameters
	double as_over_p=s*a_over_p;
	double sin_as_over_p=sin(as_over_p);
	double cos_as_over_p=cos(as_over_p);
	double one_minus_cos_as_over_p=1-cos_as_over_p;
	double diff1=sin_as_over_p-as_over_p*cos_as_over_p;
	double diff2=one_minus_cos_as_over_p-as_over_p*sin_as_over_p;
	double dx_ds=cosl*(cosphi*cos_as_over_p-sinphi*sin_as_over_p);
	double ds_dcosl=dz*cosl/(sinl*sinl2);
	double dx_dcosl
	  =p_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p)
	  +dx_ds*ds_dcosl;
	double dx_dphi=-pt_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p);  
	double dx_dk=2.*pt_over_a*pt_over_a*(sinphi*diff2-cosphi*diff1);

	double var_x=var_x0+dx_dk*dx_dk*var_k+var_pos_ms
	  +dx_dcosl*dx_dcosl*sinl2*var_lambda+dx_dphi*dx_dphi*var_phi
	  +dx_ds*dx_ds*var_z0/sinl2;

    	double dy_dk=-2.*pt_over_a*pt_over_a*(sinphi*diff1+cosphi*diff2);
	double dy_ds=cosl*(sinphi*cos_as_over_p+cosphi*sin_as_over_p);
	double dy_dcosl
	  =p_over_a*(sinphi*sin_as_over_p+cosphi*one_minus_cos_as_over_p)
	  +dy_ds*ds_dcosl;
	double dy_dphi=pt_over_a*(cosphi*sin_as_over_p-sinphi*one_minus_cos_as_over_p);
	double var_y=var_y0+dy_dk*dy_dk*var_k+var_pos_ms
	  +dy_dcosl*dy_dcosl*sinl2*var_lambda+dy_dphi*dy_dphi*var_phi
	  +dy_ds*dy_ds*var_z0/sinl2;

	// Rotate from global coordinate system into FDC local system
	double cos2a=cosa*cosa;
	double sin2a=sina*sina;
	double var_d=(cos2a*var_x+sin2a*var_y)/(cosalpha*cosalpha);
	double var_u=cos2a*var_y+sin2a*var_x;    

	// Factors take into account improved resolution after wire-based fit
	//var_d*=0.1;
	//var_u*=0.1;
	
	// Calculate chisq
	chisq = resi*resi/(var_d+var_anode)+resic*resic/(var_u+var_cathode);
    
	// Probability of this hit being on the track
	probability = TMath::Prob(chisq,2);
  
	if(probability>=MIN_HIT_PROB_FDC){
	  pair<double,const DFDCPseudo*>myhit;
	  myhit.first=probability;
	  myhit.second=hit;
	  fdchits_tmp.push_back(myhit);
	}  
	// Optionally fill debug tree
	if(fdchitsel){
	  fdchitdbg.fit_type = kWireBased;
	  fdchitdbg.p = p;
	  fdchitdbg.theta = extrapolations[k].momentum.Theta();
	  //fdchitdbg.mass = mass;
	  fdchitdbg.sigma_anode = sqrt(var_d+var_anode);
	  fdchitdbg.sigma_cathode = sqrt(var_u+var_cathode);
	  fdchitdbg.x = pos.X();
	  fdchitdbg.y = pos.Y();
	  fdchitdbg.z = pos.Z();
	  fdchitdbg.s = s;
	  fdchitdbg.itheta02 = extrapolations[k].theta2ms_sum;
	  //fdchitdbg.itheta02s = last_step->itheta02s;
	  //fdchitdbg.itheta02s2 = last_step->itheta02s2;
	  fdchitdbg.dist = dist;
	  fdchitdbg.doca = doca;
	  fdchitdbg.resi = resi;
	  fdchitdbg.u = u;
	  fdchitdbg.u_cathodes = u_cathodes;
	  fdchitdbg.resic = resic;
	  fdchitdbg.chisq = chisq;
	  fdchitdbg.prob = probability;
	  fdchitdbg.sig_phi=sqrt(var_phi);
	  fdchitdbg.sig_lambda=sqrt(var_lambda);
	  fdchitdbg.sig_pt=fabs(2.*pt_over_a)*sqrt(var_k);
      
	  fdchitsel->Fill();
	}
    
	if(HS_DEBUG_LEVEL>10){
	  _DBG_;
	  if(probability>=MIN_HIT_PROB_FDC)jerr<<"----> ";
	  jerr<<"s="<<s<<" doca="<<doca<<" dist="<<dist<<" resi="<<resi<<" resic="<<resic<<" chisq="<<chisq<<" prob="<<probability<<endl;
	}
	
      }
    }

  }
   // Order according to layer number and probability,then put the hits in the 
  // output list with the following algorithm:  hits with the highest 
  // probability in a given layer are automatically put in the output list, 
  // but if there is more than one hit in a given layer, only those hits 
  // that are within +/-1 of the wire # of the most probable hit are added 
  // to the list.
  sort(fdchits_tmp.begin(),fdchits_tmp.end(),DTrackHitSelector_fdchit_cmp);
  int old_layer=1000,old_wire=1000;
  for (unsigned int i=0;i<fdchits_tmp.size();i++){
    if (fdchits_tmp[i].second->wire->layer!=old_layer || 
	abs(fdchits_tmp[i].second->wire->wire-old_wire)==1){
      fdchits_out.push_back(fdchits_tmp[i].second);   
    }
    old_wire=fdchits_tmp[i].second->wire->wire;
    old_layer=fdchits_tmp[i].second->wire->layer;
  }
}
