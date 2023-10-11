// $Id$
//
//    File: DTrackFitterStraightTrack.cc
// Created: Tue Mar 12 10:22:32 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DTrackFitterStraightTrack.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include "DANA/DGeometryManager.h"

//---------------------------------
// DTrackFitterStraightTrack    (Constructor)
//---------------------------------
DTrackFitterStraightTrack::DTrackFitterStraightTrack(const std::shared_ptr<const JEvent>& event):DTrackFitter(event){

  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(runnumber);
  auto geo_manager = app->GetService<DGeometryManager>();
  auto geom = geo_manager->GetDGeometry(runnumber);

  // Get pointer to TrackFinder object 
  vector<const DTrackFinder *> finders;
  event->Get(finders);
    
   // Drop the const qualifier from the DTrackFinder pointer
  finder = const_cast<DTrackFinder*>(finders[0]);

  // Resource pool for matrices
  dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>
    ();
  dResourcePool_TMatrixFSym->Set_ControlParams(20, 20, 50);
  
  //****************
  // FDC parameters
  //****************

  map<string,double>drift_res_parms;
  jcalib->Get("FDC/drift_resolution_parms",drift_res_parms);
  DRIFT_RES_PARMS[0]=drift_res_parms["p0"];   
  DRIFT_RES_PARMS[1]=drift_res_parms["p1"];
  DRIFT_RES_PARMS[2]=drift_res_parms["p2"]; 

  // Time-to-distance function parameters for FDC
  map<string,double>drift_func_parms;
  jcalib->Get("FDC/drift_function_parms",drift_func_parms); 
  DRIFT_FUNC_PARMS[0]=drift_func_parms["p0"];   
  DRIFT_FUNC_PARMS[1]=drift_func_parms["p1"];
  DRIFT_FUNC_PARMS[2]=drift_func_parms["p2"]; 
  DRIFT_FUNC_PARMS[3]=drift_func_parms["p3"];
  DRIFT_FUNC_PARMS[4]=1000.;
  DRIFT_FUNC_PARMS[5]=0.;
  map<string,double>drift_func_ext;
  if (jcalib->Get("FDC/drift_function_ext",drift_func_ext)==false){
    DRIFT_FUNC_PARMS[4]=drift_func_ext["p4"]; 
    DRIFT_FUNC_PARMS[5]=drift_func_ext["p5"]; 
  }

  PLANE_TO_SKIP=0;
  app->SetDefaultParameter("TRKFIT:PLANE_TO_SKIP",PLANE_TO_SKIP); 

  //****************
  // CDC parameters
  //****************
  RING_TO_SKIP=0;
  app->SetDefaultParameter("TRKFIT:RING_TO_SKIP",RING_TO_SKIP);
  
  map<string, double> cdc_res_parms;
  jcalib->Get("CDC/cdc_resolution_parms_v2", cdc_res_parms);
  CDC_RES_PAR1 = cdc_res_parms["res_par1"];
  CDC_RES_PAR2 = cdc_res_parms["res_par2"];
  CDC_RES_PAR3 = cdc_res_parms["res_par3"];
  
  vector< map<string, double> > tvals;
  cdc_drift_table.clear();  
  if (jcalib->Get("CDC/cdc_drift_table", tvals)==false){    
    for(unsigned int i=0; i<tvals.size(); i++){
      map<string, double> &row = tvals[i];
      cdc_drift_table.push_back(1000.*row["t"]);
    }
  }
  else{
    jerr << " CDC time-to-distance table not available... bailing..." << endl;
    exit(0);
  }

  if (jcalib->Get("CDC/drift_parameters", tvals)==false){
    map<string, double> &row = tvals[0]; //long drift side
    long_drift_func[0][0]=row["a1"]; 
    long_drift_func[0][1]=row["a2"];
    long_drift_func[0][2]=row["a3"];  
    long_drift_func[1][0]=row["b1"];
    long_drift_func[1][1]=row["b2"];
    long_drift_func[1][2]=row["b3"];
    long_drift_func[2][0]=row["c1"];
    long_drift_func[2][1]=row["c2"];
    long_drift_func[2][2]=row["c3"];
    
    row = tvals[1]; // short drift side
    short_drift_func[0][0]=row["a1"];
    short_drift_func[0][1]=row["a2"];
    short_drift_func[0][2]=row["a3"];  
    short_drift_func[1][0]=row["b1"];
    short_drift_func[1][1]=row["b2"];
    short_drift_func[1][2]=row["b3"];
    short_drift_func[2][0]=row["c1"];
    short_drift_func[2][1]=row["c2"];
    short_drift_func[2][2]=row["c3"];
  }

   // Get the straw sag parameters from the database
   max_sag.clear();
   sag_phi_offset.clear();
   unsigned int numstraws[28]={42,42,54,54,66,66,80,80,93,93,106,106,123,123,
      135,135,146,146,158,158,170,170,182,182,197,197,209,209};
   unsigned int straw_count=0,ring_count=0;
   if (jcalib->Get("CDC/sag_parameters", tvals)==false){
     vector<double>temp,temp2;
     for(unsigned int i=0; i<tvals.size(); i++){
       map<string, double> &row = tvals[i];
       
       temp.push_back(row["offset"]);
       temp2.push_back(row["phi"]);
       
       straw_count++;
       if (straw_count==numstraws[ring_count]){
	 max_sag.push_back(temp);
	 sag_phi_offset.push_back(temp2);
	 temp.clear();
	 temp2.clear();
	 straw_count=0;
	 ring_count++;
       }
     }
   }

   // CDC Geometry
   vector<double>cdc_origin;
   vector<double>cdc_center;
   vector<double>cdc_upstream_endplate_pos; 
   vector<double>cdc_endplate_dim;
   geom->Get("//posXYZ[@volume='CentralDC'/@X_Y_Z",cdc_origin);
   geom->Get("//posXYZ[@volume='centralDC']/@X_Y_Z",cdc_center);
   geom->Get("//posXYZ[@volume='CDPU']/@X_Y_Z",cdc_upstream_endplate_pos);
   geom->Get("//tubs[@name='CDPU']/@Rio_Z",cdc_endplate_dim);
   cdc_origin[2]+=cdc_center[2]+cdc_upstream_endplate_pos[2]
      +0.5*cdc_endplate_dim[2];
   upstreamEndplate=cdc_origin[2];

   double endplate_dz=0.;
   geom->GetCDCEndplate(downstreamEndplate,endplate_dz,cdc_endplate_rmin,
			cdc_endplate_rmax);
   downstreamEndplate-=0.5*endplate_dz;

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
   geom->GetTRDZ(dTRDz_vec); // TRD planes
   
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
     SC_BARREL_R=sc_pos[0][0].Perp()+0.5; // radius just outside start counter volume
     SC_PHI_SECTOR1=sc_pos[0][0].Phi();
   }
   

   //*************************
   // Command-line parameters
   //*************************
   VERBOSE=0;
   app->SetDefaultParameter("TRKFIT:VERBOSE",VERBOSE);
   COSMICS=false;
   app->SetDefaultParameter("TRKFIND:COSMICS",COSMICS);
   CHI2CUT = 15.0; 
   app->SetDefaultParameter("TRKFIT:CHI2CUT",CHI2CUT);
   DO_PRUNING = true;
   app->SetDefaultParameter("TRKFIT:DO_PRUNING",DO_PRUNING);
}

//---------------------------------
// ~DTrackFitterStraightTrack    (Destructor)
//---------------------------------
DTrackFitterStraightTrack::~DTrackFitterStraightTrack()
{

}

//----------------------------------------------------
// Comparison routines for sorting
//----------------------------------------------------
bool DTrackFitterStraightTrack_cdc_hit_cmp(const DCDCTrackHit *a,
					   const DCDCTrackHit *b){

   return(a->wire->origin.Y()>b->wire->origin.Y());
}

bool DTrackFitterStraightTrack_cdc_hit_reverse_cmp(const DCDCTrackHit *a,
						   const DCDCTrackHit *b){

   return(a->wire->origin.Y()<b->wire->origin.Y());
}

bool DTrackFitterStraightTrack_cdc_hit_radius_cmp(const DCDCTrackHit *a,
						  const DCDCTrackHit *b){
   if (a==NULL || b==NULL){
      cout << "Null pointer in CDC hit list??" << endl;
      return false;
   }
   const DCDCWire *wire_a= a->wire;
   const DCDCWire *wire_b= b->wire;
   if(wire_a->ring == wire_b->ring){
      return wire_a->straw < wire_b->straw;
   }

   return (wire_a->ring<wire_b->ring);
}

bool DTrackFitterStraightTrack_fdc_hit_cmp(const DFDCPseudo *a,
					   const DFDCPseudo *b){

   return(a->wire->origin.z()<b->wire->origin.z());
}

//----------------------------------------------------
// CDC fitting routines
//----------------------------------------------------

// Smearing function derived from fitting residuals
inline double DTrackFitterStraightTrack::CDCDriftVariance(double t) const { 
   if (t<0.) t=0.;
   double sigma=CDC_RES_PAR1/(t+1.)+CDC_RES_PAR2 + CDC_RES_PAR3*t;
   return sigma*sigma;
}

// Convert time to distance for the cdc.  Also returns the variance in d.
void DTrackFitterStraightTrack::CDCDriftParameters(double dphi,double delta,
						   double t, double &d, 
						   double &V) const{
  
// NSJ 26 May 2020 included long side a3, b3 and short side c1, c2, c3
// Previously these parameters were not used (0 in ccdb) for production runs
// except intensity scan run 72312 by accident 5 May 2020, superseded 8 May.
// They were used in 2015 for runs 0-3050.

d=0.; 
  double dd_dt=0;
  if (t>0){
    double f_0=0.;
    double f_delta=0.;

    if (delta>0){
      double a1=long_drift_func[0][0];
      double a2=long_drift_func[0][1];
      double a3=long_drift_func[0][2];
      double b1=long_drift_func[1][0];
      double b2=long_drift_func[1][1];
      double b3=long_drift_func[1][2];
      double c1=long_drift_func[2][0];
      double c2=long_drift_func[2][1];
      double c3=long_drift_func[2][2];
      
      // use "long side" functional form
      double my_t=0.001*t;
      double sqrt_t=sqrt(my_t);
      double t3=my_t*my_t*my_t;
      double delta_mag=fabs(delta);

      double delta_sq=delta*delta;
      double a=a1+a2*delta_mag+a3*delta_sq;
      double b=b1+b2*delta_mag+b3*delta_sq;
      double c=c1+c2*delta_mag+c3*delta_sq;

      f_delta=a*sqrt_t+b*my_t+c*t3;
      f_0=a1*sqrt_t+b1*my_t+c1*t3;

      dd_dt=0.001*(0.5*a/sqrt_t+b+3.*c*my_t*my_t);

      }
    else{
      double my_t=0.001*t;
      double sqrt_t=sqrt(my_t);
      double t3=my_t*my_t*my_t;
      double delta_mag=fabs(delta);
      
      // use "short side" functional form
      double a1=short_drift_func[0][0];
      double a2=short_drift_func[0][1];
      double a3=short_drift_func[0][2];
      double b1=short_drift_func[1][0];
      double b2=short_drift_func[1][1];
      double b3=short_drift_func[1][2];
      double c1=short_drift_func[2][0];
      double c2=short_drift_func[2][1];
      double c3=short_drift_func[2][2];

      double delta_sq=delta*delta;
      double a=a1+a2*delta_mag+a3*delta_sq;
      double b=b1+b2*delta_mag+b3*delta_sq;
      double c=c1+c2*delta_mag+c3*delta_sq;

      f_delta=a*sqrt_t+b*my_t+c*t3;
      f_0=a1*sqrt_t+b1*my_t+c1*t3;

      dd_dt=0.001*(0.5*a/sqrt_t+b+3.*c*my_t*my_t);
      
    }
    
    unsigned int max_index=cdc_drift_table.size()-1;
    if (t>cdc_drift_table[max_index]){
      //_DBG_ << "t: " << t <<" d " << f_delta <<endl;
      d=f_delta;
    }
    
    // Drift time is within range of table -- interpolate...
    unsigned int index=0;
    index=Locate(cdc_drift_table,t);
    double dt=cdc_drift_table[index+1]-cdc_drift_table[index];
    double frac=(t-cdc_drift_table[index])/dt;
    double d_0=0.01*(double(index)+frac); 

    double P=0.;
    double tcut=250.0; // ns
    if (t<tcut) {
      P=(tcut-t)/tcut;
    }
    d=f_delta*(d_0/f_0*P+1.-P);
  }
  double VarMs=0.001; // kludge for material effects
  V=CDCDriftVariance(t)+mVarT0*dd_dt*dd_dt+VarMs;
}

// Perform the Kalman Filter for the current set of cdc hits
jerror_t DTrackFitterStraightTrack::KalmanFilter(DMatrix4x1 &S,DMatrix4x4 &C,
						 vector<int>&used_hits,
						 vector<cdc_update_t>&updates,
						 double &chi2,
						 int &ndof,
						 unsigned int iter){
  DMatrix1x4 H;  // Track projection matrix
  DMatrix4x1 H_T; // Transpose of track projection matrix 
  DMatrix4x1 K;  // Kalman gain matrix
  DMatrix4x4 J; // Jacobian matrix
  DMatrix4x1 S0; // State vector from reference trajectory
  double V=1.15*(0.78*0.78/12.); // sigma=cell_size/sqrt(12.)*scale_factor
  
  const double d_EPS=1e-8;

  // Zero out the vector of used hit flags
  for (unsigned int i=0;i<used_hits.size();i++) used_hits[i]=0;
  
  //Initialize chi2 and ndof
  chi2=0.;
  ndof=0;
  
  //Save the starting values for C and S
  trajectory[0].Skk=S;
  trajectory[0].Ckk=C;
  
  double doca2=0.;

  // CDC index and wire position variables
  unsigned int cdc_index=cdchits.size()-1;
  bool more_hits=true;
  const DCDCWire *wire=cdchits[cdc_index]->wire;
  DVector3 origin=wire->origin;
  double z0=origin.z();
  double vz=wire->udir.z();
  if (VERBOSE) jout << " Starting in Ring " << wire->ring << endl;
  DVector3 wdir=(1./vz)*wire->udir;
  DVector3 wirepos=origin+(trajectory[0].z-z0)*wdir;

  /// compute initial doca^2 to first wire
  double dx=S(state_x)-wirepos.X();
  double dy=S(state_y)-wirepos.Y();
  double old_doca2=dx*dx+dy*dy;
  
  // Loop over all steps in the trajectory
  S0=trajectory[0].S;
  J=trajectory[0].J;
  for (unsigned int k=1;k<trajectory.size();k++){
    if (!C.IsPosDef()) return UNRECOVERABLE_ERROR;
    
    // Propagate the state and covariance matrix forward in z
    S=trajectory[k].S+J*(S-S0);
    C=J*C*J.Transpose();
    
    // Save the current state and covariance matrix in the deque
    trajectory[k].Skk=S;
    trajectory[k].Ckk=C;
    
    // Save S and J for the next step
    S0=trajectory[k].S;
    J=trajectory[k].J;
    
    // Position along wire
    wirepos=origin+(trajectory[k].z-z0)*wdir;
    
    // New doca^2
    dx=S(state_x)-wirepos.X();
    dy=S(state_y)-wirepos.Y();
    doca2=dx*dx+dy*dy;
    if (VERBOSE > 10) jout<< "At Position " << S(state_x) << " " << S(state_y) << " " << trajectory[k].z << " doca2 " << doca2 << endl;
    
    if (doca2>old_doca2 && more_hits){
       
      // zero-position and direction of line describing particle trajectory
      double tx=S(state_tx),ty=S(state_ty);
      DVector3 pos0(S(state_x),S(state_y),trajectory[k].z);
      DVector3 tdir(tx,ty,1.);
      
      // Find the true doca to the wire
      DVector3 diff=pos0-origin;
      double dx0=diff.x(),dy0=diff.y();
      double wdir_dot_diff=diff.Dot(wdir);
      double tdir_dot_diff=diff.Dot(tdir);
      double tdir_dot_wdir=tdir.Dot(wdir);
      double tdir2=tdir.Mag2();
      double wdir2=wdir.Mag2();
      double D=tdir2*wdir2-tdir_dot_wdir*tdir_dot_wdir;
      double N=tdir_dot_wdir*wdir_dot_diff-wdir2*tdir_dot_diff;
      double N1=tdir2*wdir_dot_diff-tdir_dot_wdir*tdir_dot_diff;
      double scale=1./D;
      double s=scale*N;
      double t=scale*N1;
      diff+=s*tdir-t*wdir;
      double d=diff.Mag()+d_EPS; // prevent division by zero
       
      // The next measurement
      double dmeas=0.39,delta=0.;
      double tdrift=cdchits[cdc_index]->tdrift-trajectory[k].t;
      if (fit_type==kTimeBased){
	double phi_d=diff.Phi();
	double dphi=phi_d-origin.Phi();  
	while (dphi>M_PI) dphi-=2*M_PI;
	while (dphi<-M_PI) dphi+=2*M_PI;
	
	int ring_index=cdchits[cdc_index]->wire->ring-1;
	int straw_index=cdchits[cdc_index]->wire->straw-1;
	double dz=t*wdir.z();
	double delta=max_sag[ring_index][straw_index]*(1.-dz*dz/5625.)
	  *cos(phi_d+sag_phi_offset[ring_index][straw_index]);
	CDCDriftParameters(dphi,delta,tdrift,dmeas,V);
      }

      // residual
      double res=dmeas-d;
      if (VERBOSE>5) jout << " Residual " << res << endl;
      
      // Track projection
      
      double one_over_d=1./d;
      double diffx=diff.x(),diffy=diff.y(),diffz=diff.z();
      double wx=wdir.x(),wy=wdir.y();
      
      double dN1dtx=2.*tx*wdir_dot_diff-wx*tdir_dot_diff-tdir_dot_wdir*dx0;
      double dDdtx=2.*tx*wdir2-2.*tdir_dot_wdir*wx;
      double dtdtx=scale*(dN1dtx-t*dDdtx);
      
      double dN1dty=2.*ty*wdir_dot_diff-wy*tdir_dot_diff-tdir_dot_wdir*dy0;
      double dDdty=2.*ty*wdir2-2.*tdir_dot_wdir*wy;
      double dtdty=scale*(dN1dty-t*dDdty);
      
      double dNdtx=wx*wdir_dot_diff-wdir2*dx0;
      double dsdtx=scale*(dNdtx-s*dDdtx);
      
      double dNdty=wy*wdir_dot_diff-wdir2*dy0;
      double dsdty=scale*(dNdty-s*dDdty);
      
      H(state_tx)=H_T(state_tx)
	=one_over_d*(diffx*(s+tx*dsdtx-wx*dtdtx)+diffy*(ty*dsdtx-wy*dtdtx)
		     +diffz*(dsdtx-dtdtx));
      H(state_ty)=H_T(state_ty)
	=one_over_d*(diffx*(tx*dsdty-wx*dtdty)+diffy*(s+ty*dsdty-wy*dtdty)
		     +diffz*(dsdty-dtdty));
      
      double dsdx=scale*(tdir_dot_wdir*wx-wdir2*tx);
      double dtdx=scale*(tdir2*wx-tdir_dot_wdir*tx);
      double dsdy=scale*(tdir_dot_wdir*wy-wdir2*ty);
      double dtdy=scale*(tdir2*wy-tdir_dot_wdir*ty);
      
      H(state_x)=H_T(state_x)
	=one_over_d*(diffx*(1.+dsdx*tx-dtdx*wx)+diffy*(dsdx*ty-dtdx*wy)
		     +diffz*(dsdx-dtdx));
      H(state_y)=H_T(state_y)
	=one_over_d*(diffx*(dsdy*tx-dtdy*wx)+diffy*(1.+dsdy*ty-dtdy*wy)
		     +diffz*(dsdy-dtdy));

      double InvV=1./(V+H*C*H_T);
      
      // Check how far this hit is from the projection
      double chi2check=res*res*InvV;
      if (chi2check < CHI2CUT || DO_PRUNING == 0 || (COSMICS && iter == 0)){
	if (VERBOSE>5) jout << "Hit Added to track " << endl;
	if (cdchits[cdc_index]->wire->ring!=RING_TO_SKIP){
	  
	  // Compute Kalman gain matrix
	  K=InvV*(C*H_T);
	  
	  // Update state vector covariance matrix
	  DMatrix4x4 Ctest=C-K*(H*C);
	  
	  //C.Print();
	  //K.Print();
	  //Ctest.Print();
	  
	  // Check that Ctest is positive definite
	  if (!Ctest.IsPosDef()) return VALUE_OUT_OF_RANGE;
	  C=Ctest;
	  if(VERBOSE>10) C.Print();
	  // Update the state vector 
	  //S=S+res*K;
	  S+=res*K;
	  if(VERBOSE>10) S.Print();
	  
	  // Compute new residual 
	  //d=finder->FindDoca(trajectory[k].z,S,wdir,origin);
	  res=res-H*K*res;
	  
	  // Update chi2 
	  double fit_V=V-H*C*H_T;
	  chi2+=res*res/fit_V;
	  ndof++;

	  // fill pull vector entry
	  updates[cdc_index].V=fit_V;
	}
	else {
	  updates[cdc_index].V=V;
	}
	
	// Flag that we used this hit
	used_hits[cdc_index]=1;
	
	// fill updates
	updates[cdc_index].resi=res;
	updates[cdc_index].d=d;
	updates[cdc_index].delta=delta;
	updates[cdc_index].S=S;
	updates[cdc_index].C=C;
	updates[cdc_index].tdrift=tdrift;
	updates[cdc_index].ddrift=dmeas;
	updates[cdc_index].s=29.98*trajectory[k].t; // assume beta=1
	trajectory[k].id=cdc_index+1;
      }
      // move to next cdc hit
      if (cdc_index>0){
	cdc_index--;
	
	//New wire position
	wire=cdchits[cdc_index]->wire;
	if (VERBOSE>5) {
	  jout << " Next Wire ring " << wire->ring << " straw " << wire->straw << " origin udir" << endl;
	  wire->origin.Print(); wire->udir.Print();
	 }
	origin=wire->origin;
	z0=origin.z();
	vz=wire->udir.z();
	wdir=(1./vz)*wire->udir;
	wirepos=origin+((trajectory[k].z-z0))*wdir;
	
	// New doca^2
	dx=S(state_x)-wirepos.x();
	dy=S(state_y)-wirepos.y();
	doca2=dx*dx+dy*dy;	
	
      }
      else more_hits=false;
    }
    
    old_doca2=doca2;
  }
  if (ndof<=4) return VALUE_OUT_OF_RANGE;
  
  ndof-=4;
  
  return NOERROR;
}

// Smooth the CDC only tracks
jerror_t DTrackFitterStraightTrack::Smooth(vector<cdc_update_t>&cdc_updates){
  unsigned int max=best_trajectory.size()-1;
  DMatrix4x1 S=(best_trajectory[max].Skk);
  DMatrix4x4 C=(best_trajectory[max].Ckk);
  DMatrix4x4 JT=best_trajectory[max].J.Transpose();
  DMatrix4x1 Ss=S;
  DMatrix4x4 Cs=C;
  DMatrix4x4 A,dC;
  DMatrix1x4 H;  // Track projection matrix
  DMatrix4x1 H_T; // Transpose of track projection matrix
  
  const double d_EPS=1e-8;
  
  for (unsigned int m=max-1;m>0;m--){
    if (best_trajectory[m].id>0){
      unsigned int id=best_trajectory[m].id-1;
      A=cdc_updates[id].C*JT*C.Invert();
      Ss=cdc_updates[id].S+A*(Ss-S);
      
      dC=A*(Cs-C)*A.Transpose();
      Cs=cdc_updates[id].C+dC;
      if (VERBOSE > 10) {
	jout << " In Smoothing Step Using ID " << id << "/" << cdc_updates.size() << " for ring " << cdchits[id]->wire->ring << endl;
	jout << " A cdc_updates[id].C Ss Cs " << endl;
	A.Print(); cdc_updates[id].C.Print(); Ss.Print(); Cs.Print();
      }
      if(!Cs.IsPosDef()) {
	if (VERBOSE) jout << "Cs is not PosDef!" << endl;
	return VALUE_OUT_OF_RANGE;
      }

      const DCDCWire *wire=cdchits[id]->wire;
      DVector3 origin=wire->origin;
      double z0=origin.z();
      double vz=wire->udir.z();
      DVector3 wdir=(1./vz)*wire->udir;
      DVector3 wirepos=origin+(best_trajectory[m].z-z0)*wdir;
      // Position and direction from state vector
      double x=Ss(state_x);
      double y=Ss(state_y);
      double tx=Ss(state_tx);
      double ty=Ss(state_ty);
      
      DVector3 pos0(x,y,best_trajectory[m].z);
      DVector3 tdir(tx,ty,1.);
      
      // Find the true doca to the wire
      DVector3 diff=pos0-origin;
      double dx0=diff.x(),dy0=diff.y();
      double wdir_dot_diff=diff.Dot(wdir);
      double tdir_dot_diff=diff.Dot(tdir);
      double tdir_dot_wdir=tdir.Dot(wdir);
      double tdir2=tdir.Mag2();
      double wdir2=wdir.Mag2();
      double D=tdir2*wdir2-tdir_dot_wdir*tdir_dot_wdir;
      double N=tdir_dot_wdir*wdir_dot_diff-wdir2*tdir_dot_diff;
      double N1=tdir2*wdir_dot_diff-tdir_dot_wdir*tdir_dot_diff;
      double scale=1./D;
      double s=scale*N;
      double t=scale*N1;
      diff+=s*tdir-t*wdir;
      double d=diff.Mag()+d_EPS; // prevent division by zero
      double ddrift = cdc_updates[id].ddrift;
      
      double resi = ddrift - d;
      // Track projection
      
      {
	double one_over_d=1./d;
	double diffx=diff.x(),diffy=diff.y(),diffz=diff.z();
	double wx=wdir.x(),wy=wdir.y();
	
	double dN1dtx=2.*tx*wdir_dot_diff-wx*tdir_dot_diff-tdir_dot_wdir*dx0;
	double dDdtx=2.*tx*wdir2-2.*tdir_dot_wdir*wx;
	double dtdtx=scale*(dN1dtx-t*dDdtx);
	
	double dN1dty=2.*ty*wdir_dot_diff-wy*tdir_dot_diff-tdir_dot_wdir*dy0;
	double dDdty=2.*ty*wdir2-2.*tdir_dot_wdir*wy;
	double dtdty=scale*(dN1dty-t*dDdty);
	
	double dNdtx=wx*wdir_dot_diff-wdir2*dx0;
	double dsdtx=scale*(dNdtx-s*dDdtx);
	
	double dNdty=wy*wdir_dot_diff-wdir2*dy0;
	double dsdty=scale*(dNdty-s*dDdty);
	
	H(state_tx)=H_T(state_tx)
	  =one_over_d*(diffx*(s+tx*dsdtx-wx*dtdtx)+diffy*(ty*dsdtx-wy*dtdtx)
		       +diffz*(dsdtx-dtdtx));
	H(state_ty)=H_T(state_ty)
	  =one_over_d*(diffx*(tx*dsdty-wx*dtdty)+diffy*(s+ty*dsdty-wy*dtdty)
		       +diffz*(dsdty-dtdty));
	
	double dsdx=scale*(tdir_dot_wdir*wx-wdir2*tx);
	double dtdx=scale*(tdir2*wx-tdir_dot_wdir*tx);
	double dsdy=scale*(tdir_dot_wdir*wy-wdir2*ty);
	double dtdy=scale*(tdir2*wy-tdir_dot_wdir*ty);
	
	H(state_x)=H_T(state_x)
	  =one_over_d*(diffx*(1.+dsdx*tx-dtdx*wx)+diffy*(dsdx*ty-dtdx*wy)
		       +diffz*(dsdx-dtdx));
	H(state_y)=H_T(state_y)
	  =one_over_d*(diffx*(dsdy*tx-dtdy*wx)+diffy*(1.+dsdy*ty-dtdy*wy)
		       +diffz*(dsdy-dtdy));
      }
      double V=cdc_updates[id].V;
    
      if (VERBOSE > 10) jout << " d " << d << " H*S " << H*S << endl;
      if (cdchits[id]->wire->ring==RING_TO_SKIP){
	V=V+H*Cs*H_T;
      }
      else{
	V=V-H*Cs*H_T;
      }      
      if (V<0) return VALUE_OUT_OF_RANGE;

      // Add the pull
      double myscale=1./sqrt(1.+tx*tx+ty*ty);
      double cosThetaRel=wire->udir.Dot(DVector3(myscale*tx,myscale*ty,myscale));
      pull_t thisPull(resi,sqrt(V),
		      best_trajectory[m].t*SPEED_OF_LIGHT,
		      cdc_updates[id].tdrift,
		      d,cdchits[id], NULL,
		      diff.Phi(), //docaphi
		      best_trajectory[m].z,cosThetaRel,
		      cdc_updates[id].tdrift);
      
      // Derivatives for alignment
      double wtx=wire->udir.X(), wty=wire->udir.Y(), wtz=wire->udir.Z();
      double wx=wire->origin.X(), wy=wire->origin.Y(), wz=wire->origin.Z();
      
      double z=best_trajectory[m].z;
      double tx2=tx*tx, ty2=ty*ty;
      double wtx2=wtx*wtx, wty2=wty*wty, wtz2=wtz*wtz;
      double denom=(1 + ty2)*wtx2 + (1 + tx2)*wty2 - 2*ty*wty*wtz + (tx2 + ty2)*wtz2 - 2*tx*wtx*(ty*wty + wtz) +d_EPS;
      double denom2=denom*denom;
      double c1=-(wtx - tx*wtz)*(wy - y);
      double c2=wty*(wx - tx*wz - x + tx*z);
      double c3=ty*(-(wtz*wx) + wtx*wz + wtz*x - wtx*z);
      double dscale=0.5*(1./d);
      
      vector<double> derivatives(11);
      
      derivatives[CDCTrackD::dDOCAdOriginX]=dscale*(2*(wty - ty*wtz)*(c1 + c2 + c3))/denom;
      
      derivatives[CDCTrackD::dDOCAdOriginY]=dscale*(2*(-wtx + tx*wtz)*(c1 + c2 + c3))/denom;
      
      derivatives[CDCTrackD::dDOCAdOriginZ]=dscale*(2*(ty*wtx - tx*wty)*(c1 + c2 + c3))/denom;
      
      derivatives[CDCTrackD::dDOCAdDirX]=dscale*(2*(wty - ty*wtz)*(c1 + c2 + c3)*
						 (tx*(ty*wty + wtz)*(wx - x) + (wty - ty*wtz)*(-wy + y + ty*(wz - z)) +
						  wtx*(-((1 + ty2)*wx) + (1 + ty2)*x + tx*(ty*wy + wz - ty*y - z)) + tx2*(wty*(-wy + y) + wtz*(-wz + z))))/denom2;
      
      derivatives[CDCTrackD::dDOCAdDirY]=dscale*(-2*(wtx - tx*wtz)*(c1 + c2 + c3)*
						 (tx*(ty*wty + wtz)*(wx - x) + (wty - ty*wtz)*(-wy + y + ty*(wz - z)) +
						  wtx*(-((1 + ty2)*wx) + (1 + ty2)*x + tx*(ty*wy + wz - ty*y - z)) + tx2*(wty*(-wy + y) + wtz*(-wz + z))))/denom2;
      
      derivatives[CDCTrackD::dDOCAdDirZ]=dscale*(-2*(ty*wtx - tx*wty)*(c1 + c2 + c3)*
						 (-(tx*(ty*wty + wtz)*(wx - x)) + tx2*(wty*(wy - y) + wtz*(wz - z)) + (wty - ty*wtz)*(wy - y + ty*(-wz + z)) +
						  wtx*((1 + ty2)*wx - (1 + ty2)*x + tx*(-(ty*wy) - wz + ty*y + z))))/denom2;
      
      derivatives[CDCTrackD::dDOCAdS0]=-derivatives[CDCTrackD::dDOCAdOriginX];
      
      derivatives[CDCTrackD::dDOCAdS1]=-derivatives[CDCTrackD::dDOCAdOriginY];
      
      derivatives[CDCTrackD::dDOCAdS2]=dscale*(2*(wty - ty*wtz)*(-c1 - c2 - c3)*
					       (-(wtx*wtz*wx) - wty*wtz*wy + wtx2*wz + wty2*wz + wtx*wtz*x + wty*wtz*y - wtx2*z - wty2*z +
						tx*(wty2*(wx - x) + wtx*wty*(-wy + y) + wtz*(wtz*wx - wtx*wz - wtz*x + wtx*z)) +
						ty*(wtx*wty*(-wx + x) + wtx2*(wy - y) + wtz*(wtz*wy - wty*wz - wtz*y + wty*z))))/denom2;
      
      derivatives[CDCTrackD::dDOCAdS3]=dscale*(2*(wtx - tx*wtz)*(c1 + c2 + c3)*
					       (-(wtx*wtz*wx) - wty*wtz*wy + wtx2*wz + wty2*wz + wtx*wtz*x + wty*wtz*y - wtx2*z - wty2*z +
						tx*(wty2*(wx - x) + wtx*wty*(-wy + y) + wtz*(wtz*wx - wtx*wz - wtz*x + wtx*z)) +
						ty*(wtx*wty*(-wx + x) + wtx2*(wy - y) + wtz*(wtz*wy - wty*wz - wtz*y + wty*z))))/denom2;

      thisPull.AddTrackDerivatives(derivatives);
      pulls.push_back(thisPull);
    }
    else{
      A=best_trajectory[m].Ckk*JT*C.Invert();
      Ss=best_trajectory[m].Skk+A*(Ss-S);
      Cs=best_trajectory[m].Ckk+A*(Cs-C)*A.Transpose();
    }
    
    S=best_trajectory[m].Skk;
    C=best_trajectory[m].Ckk;
    JT=best_trajectory[m].J.Transpose();
  }
  
  return NOERROR;
}

//Reference trajectory for the track for cdc tracks
jerror_t DTrackFitterStraightTrack::SetReferenceTrajectory(double t0,double z,
							   DMatrix4x1 &S,
							   const DCDCTrackHit *last_cdc,
							   double &dzsign){ 
  DMatrix4x4 J(1.,0.,1.,0., 0.,1.,0.,1., 0.,0.,1.,0., 0.,0.,0.,1.);

  double ds=0.5;
  double t=t0;
  
  // last y position of hit (approximate, using center of wire)
  double last_y=last_cdc->wire->origin.y();
  double last_r2=last_cdc->wire->origin.Perp2();
  // Check that track points towards last wire, otherwise swap deltaz
  DVector3 dir(S(state_tx),S(state_ty),dzsign);
  if (fabs(dir.Theta() - M_PI_2) < 0.2) ds = 0.1;
  
  //jout << "dPhi " << dphi << " theta " << dir.Theta() << endl;
  double dz=dzsign*ds/sqrt(1.+S(state_tx)*S(state_tx)+S(state_ty)*S(state_ty));
  
  if (VERBOSE) {
    if (COSMICS) jout << " Swimming Reference Trajectory last CDC y " << last_y << " dz "<< dz << endl;
    else jout << " Swimming Reference Trajectory last CDC r2 " << last_r2 << " dz "<< dz << endl;
    jout << " S" << endl; S.Print(); jout << "z= "<< z << endl;
    jout << "  Last CDC ring " << last_cdc->wire->ring << " straw " << last_cdc->wire->straw << endl;
  }
  unsigned int numsteps=0;
  const unsigned int MAX_STEPS=5000;
  bool done=false;
  do{
    numsteps++;
    z+=dz;
    J(state_x,state_tx)=-dz;
    J(state_y,state_ty)=-dz;
    // Flight time: assume particle is moving at the speed of light
    t+=ds/29.98;
    //propagate the state to the next z position
    S(state_x)+=S(state_tx)*dz;
    S(state_y)+=S(state_ty)*dz;

    if (z > downstreamEndplate && dz < 0) continue;
    if (z < upstreamEndplate && dz > 0) continue;
    trajectory.push_front(trajectory_t(z,t,S,J,DMatrix4x1(),DMatrix4x4()));

    done=((z>downstreamEndplate) || (z<upstreamEndplate));
    if (COSMICS==false){
      double r2=S(state_x)*S(state_x)+S(state_y)*S(state_y);
      done |= (r2>last_r2);
    }
  }while (!done && numsteps<MAX_STEPS);
  
  if (VERBOSE)
    {
      if (VERBOSE > 10){
	printf("Trajectory:\n");
	for (unsigned int i=0;i<trajectory.size();i++){
	  printf(" x %f y %f z %f\n",trajectory[i].S(state_x),
		 trajectory[i].S(state_y),trajectory[i].z); 
	}
      }
      else{
	printf("%i step trajectory Begin/End:\n", numsteps);
	printf(" x %f y %f z %f\n",trajectory[0].S(state_x), trajectory[0].S(state_y), trajectory[0].z);
	if (trajectory.size() > 1) printf(" x %f y %f z %f\n",trajectory[trajectory.size()-1].S(state_x),
					  trajectory[trajectory.size()-1].S(state_y),trajectory[trajectory.size()-1].z);
      }
    }
  if (trajectory.size()<2) return UNRECOVERABLE_ERROR;
  return NOERROR;
}

// Routine for steering the fit of the track
DTrackFitter::fit_status_t DTrackFitterStraightTrack::FitTrack(void){
  DTrackFitter::fit_status_t status=DTrackFitter::kFitNotDone;
  if (cdchits.size()+fdchits.size() < 4) return status;

  // Initial guess for state vector
  DVector3 input_pos=input_params.position();
  DVector3 input_mom=input_params.momentum();
  double pz=input_mom.z();
  DMatrix4x1 S(input_pos.x(),input_pos.y(),input_mom.x()/pz,input_mom.y()/pz);

  // Initial guess for covariance matrix
  DMatrix4x4 C;
  C(state_x,state_x)=C(state_y,state_y)=1.0;     
  C(state_tx,state_tx)=C(state_ty,state_ty)=0.01;

  // Starting z-position and time
  double z=input_pos.z();
  double t0=input_params.t0();
  // Variance in start time
  switch(input_params.t0_detector()){
  case SYS_TOF:
    mVarT0=0.01;
    break;
  case SYS_CDC:
    mVarT0=25.;
    break;
  case SYS_FDC:
    mVarT0=25.;
    break;
  case SYS_BCAL:
    mVarT0=0.25;
    break;
  case SYS_START:
    mVarT0=0.09;
    break;
  default:
    mVarT0=0.;
    break;
  }

  // Chisq and ndof
  chisq=1e16;
  Ndof=0;

  // Sort the CDC hits by radius or y (for cosmics)
  if (cdchits.size()>0){
    if (COSMICS){
      stable_sort(cdchits.begin(),cdchits.end(),DTrackFitterStraightTrack_cdc_hit_reverse_cmp);
    }
    else{
      stable_sort(cdchits.begin(),cdchits.end(),DTrackFitterStraightTrack_cdc_hit_radius_cmp);
    }
  }

  if (fdchits.size()>0){
    // Sort FDC hits by z
    stable_sort(fdchits.begin(),fdchits.end(),DTrackFitterStraightTrack_fdc_hit_cmp);
    status=FitForwardTrack(t0,z,S,C,chisq,Ndof);
  }
  else if (cdchits.size()>0){
    double dzsign=(pz>0)?1.:-1.;
    if (COSMICS){
      dzsign=(S(state_ty)>0.)?1.:-1.;
    }
    status=FitCentralTrack(z,t0,dzsign,S,C,chisq,Ndof);
  }

  if (status==DTrackFitter::kFitSuccess){
    // Output fit results
    double tx=S(state_tx),ty=S(state_ty);
    DVector3 mom(tx,ty,1.);
    mom.SetMag(1.);
    
    // Convert 4x4 covariance matrix to a TMatrixFSym for output
    TMatrixFSym errMatrix(5);
    for(unsigned int loc_i = 0; loc_i < 4; ++loc_i){
      for(unsigned int loc_j = 0; loc_j < 4; ++loc_j){
	  errMatrix(loc_i, loc_j) = C(loc_i, loc_j);
      }
    }
    errMatrix(4,4)=1.;
    
    // Add 7x7 covariance matrix to output
    double sign=(mom.Theta()>M_PI_2)?-1.:1.;
    fit_params.setErrorMatrix(Get7x7ErrorMatrix(errMatrix,S,sign));

    DVector3 pos;
    if (COSMICS==false){
      DVector3 beamdir(0.,0.,1.);
      DVector3 beampos(0.,0.,65.);
      finder->FindDoca(z,S,beamdir,beampos,&pos);
      
      // Jacobian matrix
      double dz=pos.z()-z;
      DMatrix4x4 J(1.,0.,1.,0., 0.,1.,0.,1., 0.,0.,1.,0., 0.,0.,0.,1.);
      J(state_x,state_tx)=dz;
      J(state_y,state_ty)=dz;
      // Transform the matrix to the position of the doca
      C=J*C*J.Transpose();  
    }
    else{
      pos.SetXYZ(S(state_x),S(state_y),z);
    }

    fit_params.setForwardParmFlag(true);

    fit_params.setPosition(pos);
    fit_params.setMomentum(mom);

    // Add tracking covariance matrix to output
    auto locTrackingCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
    locTrackingCovarianceMatrix->ResizeTo(5, 5); 
    locTrackingCovarianceMatrix->Zero();
    for(unsigned int loc_i = 0; loc_i < 4; ++loc_i){
      for(unsigned int loc_j = 0; loc_j < 4; ++loc_j){
	(*locTrackingCovarianceMatrix)(loc_i, loc_j) = C(loc_i, loc_j);
      }
    }
    (*locTrackingCovarianceMatrix)(4,4)=1.;
    fit_params.setTrackingErrorMatrix(locTrackingCovarianceMatrix);

    // Get extrapolations to other detectors
    GetExtrapolations(pos,mom);
  
  }

  return status;
}

// Locate a position in vector xx given x
unsigned int DTrackFitterStraightTrack::Locate(const vector<double>&xx,double x) const{
  int n=xx.size();
  if (x==xx[0]) return 0;
  else if (x==xx[n-1]) return n-2;
  
  int jl=-1;
  int ju=n;
  int ascnd=(xx[n-1]>=xx[0]);
  while(ju-jl>1){
    int jm=(ju+jl)>>1;
    if ( (x>=xx[jm])==ascnd)
	jl=jm;
    else
      ju=jm;
  } 
  return jl;
}

// Steering routine for fitting CDC-only tracks
DTrackFitter::fit_status_t DTrackFitterStraightTrack::FitCentralTrack(double &z0,double t0,
						double dzsign,
						DMatrix4x1 &Sbest,
						DMatrix4x4 &Cbest,
						double &chi2_best,
						int &ndof_best){
  // State vector and covariance matrix
  DMatrix4x1 S(Sbest);
  DMatrix4x4 C(Cbest),C0(Cbest);

  double chi2=chi2_best;
  int ndof=ndof_best;

  unsigned int numhits=cdchits.size();
  unsigned int maxindex=numhits-1;
  
  // vectors of indexes to cdc hits used in the fit
  vector<int> used_cdc_hits(numhits);
  vector<int> used_cdc_hits_best_fit(numhits);
  
  // vectors of residual information 
  vector<cdc_update_t>updates(numhits);
  vector<cdc_update_t>best_updates(numhits);
  
  // Rest deque for "best" trajectory
  best_trajectory.clear();
  
  // Perform the fit
  int iter=0;
  for(iter=0;iter<20;iter++){
    if (VERBOSE) jout << " Performing Pass iter " << iter << endl;
       
    trajectory.clear();
    if (SetReferenceTrajectory(t0,z0,S,cdchits[maxindex],dzsign)
	!=NOERROR) break;
    
    if (VERBOSE) jout << " Reference Trajectory Set " << endl;
    C=C0;
    if (KalmanFilter(S,C,used_cdc_hits,updates,chi2,ndof,iter)!=NOERROR) break;
    if (VERBOSE) jout << " Filter returns NOERROR" << endl;
    if (iter>0 && (fabs(chi2_best-chi2)<0.1 || chi2>chi2_best)) break;  
    
    // Save the current state and covariance matrixes
    Cbest=C;
    Sbest=S;

    // Save the current used hit and trajectory information
    best_trajectory.assign(trajectory.begin(),trajectory.end());
    used_cdc_hits_best_fit.assign(used_cdc_hits.begin(),used_cdc_hits.end());
    best_updates.assign(updates.begin(),updates.end());

    chi2_best=chi2; 
    ndof_best=ndof;
  }
  if (iter==0) return DTrackFitter::kFitFailed;

  //Final z position (closest to beam line)
  z0=best_trajectory[best_trajectory.size()-1].z;

  //Run the smoother 
  if (Smooth(best_updates) == NOERROR) IsSmoothed=true;

  // output list of cdc hits used in the fit
  cdchits_used_in_fit.clear();
  for (unsigned int m=0;m<used_cdc_hits_best_fit.size();m++){
    if (used_cdc_hits_best_fit[m]){
      cdchits_used_in_fit.push_back(cdchits[m]);
    }
  }

  return DTrackFitter::kFitSuccess;
}


//----------------------------------------------------
// FDC fitting routines
//----------------------------------------------------

// parametrization of time-to-distance for FDC
double DTrackFitterStraightTrack::fdc_drift_distance(double time) const {
  if (time<0.) return 0.;
  double d=0.; 
  double tsq=time*time;
  double t_high=DRIFT_FUNC_PARMS[4];
  
  if (time<t_high){
    d=DRIFT_FUNC_PARMS[0]*sqrt(time)+DRIFT_FUNC_PARMS[1]*time
      +DRIFT_FUNC_PARMS[2]*tsq+DRIFT_FUNC_PARMS[3]*tsq*time;
  }
  else{
    double t_high_sq=t_high*t_high;
    d=DRIFT_FUNC_PARMS[0]*sqrt(t_high)+DRIFT_FUNC_PARMS[1]*t_high
      +DRIFT_FUNC_PARMS[2]*t_high_sq+DRIFT_FUNC_PARMS[3]*t_high_sq*t_high;
    d+=DRIFT_FUNC_PARMS[5]*(time-t_high);
  }
    
  return d;

}

// Crude approximation for the variance in drift distance due to smearing
double DTrackFitterStraightTrack::fdc_drift_variance(double t) const{
   //return FDC_ANODE_VARIANCE;
   if (t<5.) t=5.;
   double sigma=DRIFT_RES_PARMS[0]/(t+1.)+DRIFT_RES_PARMS[1]+DRIFT_RES_PARMS[2]*t*t;

   return sigma*sigma;
}

// Steering routine for the kalman filter
DTrackFitter::fit_status_t
DTrackFitterStraightTrack::FitForwardTrack(double t0,double &start_z,
      DMatrix4x1 &Sbest,DMatrix4x4 &Cbest,double &chi2_best,int &ndof_best){
  // State vector and covariance matrix
  DMatrix4x1 S(Sbest);
  DMatrix4x4 C(Cbest),C0(Cbest);

  // vectors of indexes to fdc hits used in the fit
  unsigned int numfdchits=fdchits.size();
  vector<int> used_fdc_hits(numfdchits);
  vector<int> used_fdc_hits_best_fit(numfdchits);
  // vectors of indexes to cdc hits used in the fit
  unsigned int numcdchits=cdchits.size();
  vector<int> used_cdc_hits(numcdchits);
  vector<int> used_cdc_hits_best_fit(numcdchits);

  // vectors of residual information 
  vector<fdc_update_t>updates(numfdchits);
  vector<fdc_update_t>best_updates(numfdchits);
  vector<cdc_update_t>cdc_updates(numcdchits);
  vector<cdc_update_t>best_cdc_updates(numcdchits);
  
  vector<const DCDCTrackHit *> matchedCDCHits;
  
  // Chi-squared and degrees of freedom
  double chi2=chi2_best;
  int ndof=ndof_best;
  
  // Rest deque for "best" trajectory
  best_trajectory.clear();
  
  unsigned iter=0;
  // First pass
  for(iter=0;iter<20;iter++){
    trajectory.clear();
    if (SetReferenceTrajectory(t0,start_z,S)!=NOERROR) break;
    
    C=C0;
    if (KalmanFilter(S,C,used_fdc_hits,used_cdc_hits,updates,cdc_updates,
		     chi2,ndof)!=NOERROR) break;
    
    // printf(" == iter %d =====chi2 %f ndof %d \n",iter,chi2,ndof);
    if (iter>0 && (chi2>chi2_best || fabs(chi2_best-chi2)<0.1)) break;  
    
    // Save the current state and covariance matrixes
    Cbest=C;
    Sbest=S;
    
    // Save the current used hit and trajectory information
    best_trajectory.assign(trajectory.begin(),trajectory.end());
    used_cdc_hits_best_fit.assign(used_cdc_hits.begin(),used_cdc_hits.end());
    used_fdc_hits_best_fit.assign(used_fdc_hits.begin(),used_fdc_hits.end());
    best_updates.assign(updates.begin(),updates.end());
    best_cdc_updates.assign(cdc_updates.begin(),cdc_updates.end());
    
    chi2_best=chi2; 
    ndof_best=ndof;
  }
  if (iter==0) return DTrackFitter::kFitFailed;

  //Final z position (closest to beam line)
  start_z=best_trajectory[best_trajectory.size()-1].z;

  //Run the smoother
  if (Smooth(best_updates,best_cdc_updates) == NOERROR) IsSmoothed=true;

  // output list of hits used in the fit
  cdchits_used_in_fit.clear();
  for (unsigned int m=0;m<used_cdc_hits_best_fit.size();m++){
    if (used_cdc_hits_best_fit[m]){
      cdchits_used_in_fit.push_back(cdchits[m]);
    }
  }  
  fdchits_used_in_fit.clear();
  for (unsigned int m=0;m<used_fdc_hits_best_fit.size();m++){
    if (used_fdc_hits_best_fit[m]){
      fdchits_used_in_fit.push_back(fdchits[m]);
    }
  }

  
  return DTrackFitter::kFitSuccess;
}


// Reference trajectory for the track
jerror_t 
DTrackFitterStraightTrack::SetReferenceTrajectory(double t0,double z,
						  DMatrix4x1 &S){
  const double EPS=1e-3;

  // Jacobian matrix 
  DMatrix4x4 J(1.,0.,1.,0., 0.,1.,0.,1., 0.,0.,1.,0., 0.,0.,0.,1.);
  
  double dz=1.1;
  double t=t0;
  trajectory.push_front(trajectory_t(z,t,S,J,DMatrix4x1(),DMatrix4x4()));

  double zhit=z;
  double old_zhit=z;
  for (unsigned int i=0;i<fdchits.size();i++){  
    zhit=fdchits[i]->wire->origin.z();
    dz=1.1;
    
    if (fabs(zhit-old_zhit)<EPS && i>0){
      trajectory[0].numhits++;
      continue;
    }
    // propagate until we would step beyond the FDC hit plane
    bool done=false;
    while (!done){	    
      double new_z=z+dz;	      
      
      if (new_z>zhit){
	dz=zhit-z;
	new_z=zhit;
	done=true;
      }
      J(state_x,state_tx)=-dz;
      J(state_y,state_ty)=-dz;
      // Flight time: assume particle is moving at the speed of light
      t+=dz*sqrt(1+S(state_tx)*S(state_tx)+S(state_ty)*S(state_ty))/29.98;
      //propagate the state to the next z position
      S(state_x)+=S(state_tx)*dz;
      S(state_y)+=S(state_ty)*dz;
      
      trajectory.push_front(trajectory_t(new_z,t,S,J,DMatrix4x1(),
					 DMatrix4x4())); 
      if (done){
	trajectory[0].id=i+1;
	trajectory[0].numhits=1;
      }
      
      z=new_z;
    }	   
    old_zhit=zhit;
  }
  // One last step
  dz=1.1;
  J(state_x,state_tx)=-dz;
  J(state_y,state_ty)=-dz;
  
   // Flight time: assume particle is moving at the speed of light
  t+=dz*sqrt(1+S(state_tx)*S(state_tx)+S(state_ty)*S(state_ty))/29.98;
  
  //propagate the state to the next z position
  S(state_x)+=S(state_tx)*dz;
  S(state_y)+=S(state_ty)*dz;
  trajectory.push_front(trajectory_t(z+dz,t,S,J,DMatrix4x1(),DMatrix4x4()));
  
  if (false)
   {
     printf("Trajectory:\n");
     for (unsigned int i=0;i<trajectory.size();i++){
       printf(" x %f y %f z %f first hit %d num in layer %d\n",trajectory[i].S(state_x),
	      trajectory[i].S(state_y),trajectory[i].z,trajectory[i].id,
	      trajectory[i].numhits); 
     }
   }
  
  return NOERROR;
}

// Perform Kalman Filter for the current trajectory
jerror_t DTrackFitterStraightTrack::KalmanFilter(DMatrix4x1 &S,DMatrix4x4 &C,
						 vector<int>&used_fdc_hits, 
						 vector<int>&used_cdc_hits,
						 vector<fdc_update_t>&updates,
						 vector<cdc_update_t>&cdc_updates,
						 double &chi2,int &ndof){
  DMatrix2x4 H;  // Track projection matrix
  DMatrix4x2 H_T; // Transpose of track projection matrix 
  DMatrix4x2 K;  // Kalman gain matrix
  double VarMs=0.001; // kludge for material
  DMatrix2x2 V(0.0833,0.,0.,0.000256+VarMs);  // Measurement variance 
  DMatrix2x2 Vtemp,InvV;
  DMatrix2x1 Mdiff;
  DMatrix4x4 I; // identity matrix
  DMatrix4x4 J; // Jacobian matrix
  DMatrix4x1 S0; // State vector from reference trajectory
  
  DMatrix1x4 H_CDC;  // Track projection matrix
  DMatrix4x1 H_T_CDC; // Transpose of track projection matrix
  DMatrix4x1 K_CDC;  // Kalman gain matrix
  double V_CDC;

  const double d_EPS=1e-8;
  
  // Zero out the vectors of used hit flags
  for (unsigned int i=0;i<used_fdc_hits.size();i++) used_fdc_hits[i]=0; 
  for (unsigned int i=0;i<used_cdc_hits.size();i++) used_cdc_hits[i]=0;

  //Initialize chi2 and ndof
  chi2=0.;
  ndof=0;
  
  // Save the starting values for C and S in the deque
  trajectory[0].Skk=S;
  trajectory[0].Ckk=C;
  
  // Loop over all steps in the trajectory
  S0=trajectory[0].S;
  J=trajectory[0].J;

  // CDC index and wire position variables
  bool more_hits = cdchits.size() == 0 ? false: true;
  bool firstCDCStep=true;
  unsigned int cdc_index=0;
  const DCDCWire *wire;
  DVector3 origin,wdir,wirepos;
  double doca2=0.0, old_doca2=0.0;
  if(more_hits){
    cdc_index=cdchits.size()-1;
    wire=cdchits[cdc_index]->wire;
    origin=wire->origin;
    double vz=wire->udir.z();
    if (VERBOSE) jout << " Additional CDC Hits in FDC track Starting in Ring " << wire->ring << endl;
    wdir=(1./vz)*wire->udir;
  }
  
  for (unsigned int k=1;k<trajectory.size();k++){
    if (C(0,0)<=0. || C(1,1)<=0. || C(2,2)<=0. || C(3,3)<=0.)
      return UNRECOVERABLE_ERROR;
    
    // Propagate the state and covariance matrix forward in z
    S=trajectory[k].S+J*(S-S0);
    C=J*C*J.Transpose();
    
    // Save the current state and covariance matrix in the deque
    trajectory[k].Skk=S;
    trajectory[k].Ckk=C;
     
    // Save S and J for the next step
    S0=trajectory[k].S;
    J=trajectory[k].J;
    
    // Correct S and C for the hit 
    if (trajectory[k].id>0){
      unsigned int id=trajectory[k].id-1;
      
      double cospsi=cos(fdchits[id]->wire->angle);
      double sinpsi=sin(fdchits[id]->wire->angle);
  
      // Small angle alignment correction
      double x = S(state_x) + fdchits[id]->wire->angles.Z()*S(state_y);
      double y = S(state_y) - fdchits[id]->wire->angles.Z()*S(state_x);
      //tz = 1. + my_fdchits[id]->phiY*tx - my_fdchits[id]->phiX*ty;
      double tx = (S(state_tx) + fdchits[id]->wire->angles.Z()*S(state_ty) - fdchits[id]->wire->angles.Y());
      double ty = (S(state_ty) - fdchits[id]->wire->angles.Z()*S(state_tx) + fdchits[id]->wire->angles.X());
      
      if (std::isnan(x) || std::isnan(y)) return UNRECOVERABLE_ERROR;
      
      // x,y and tx,ty in local coordinate system	
      // To transform from (x,y) to (u,v), need to do a rotation:
      //   u = x*cos(psi)-y*sin(psi)
      //   v = y*cos(psi)+x*sin(psi)
      // (without alignment offsets)
      double vpred_wire_plane=y*cospsi+x*sinpsi;
      double upred_wire_plane=x*cospsi-y*sinpsi;
      double tu=tx*cospsi-ty*sinpsi;
      double tv=tx*sinpsi+ty*cospsi;
      
      // Variables for angle of incidence with respect to the z-direction in
      // the u-z plane
      double alpha=atan(tu);
      double cosalpha=cos(alpha);
      double cos2_alpha=cosalpha*cosalpha;
      double sinalpha=sin(alpha);
      double sin2_alpha=sinalpha*sinalpha;
      double cos2_alpha_minus_sin2_alpha=cos2_alpha-sin2_alpha;
      
      // Difference between measurement and projection
      for (int m=trajectory[k].numhits-1;m>=0;m--){
	unsigned int my_id=id+m;
	double uwire=fdchits[my_id]->w;
	// (signed) distance of closest approach to wire
	double du=upred_wire_plane-uwire;
	double doca=du*cosalpha;
	
	// Predicted avalanche position along the wire
	double vpred=vpred_wire_plane;
	
	// Measured position of hit along wire
	double v=fdchits[my_id]->s; 
	
	// Difference between measurements and predictions
	double drift=0.; // assume hit at wire position
	if (fit_type==kTimeBased){
	  double drift_time=fdchits[my_id]->time-trajectory[k].t; 
	  drift=(du>0.0?1.:-1.)*fdc_drift_distance(drift_time);	 
	  V(0,0)=fdc_drift_variance(drift_time)+VarMs;
	}
	Mdiff(0)=drift-doca;
	Mdiff(1)=v-vpred;
	
	// Matrix for transforming from state-vector space to measurement space
	H_T(state_x,0)=cospsi*cosalpha;
	H_T(state_y,0)=-sinpsi*cosalpha;
	double temp=-du*sinalpha*cos2_alpha;
	H_T(state_tx,0)=cospsi*temp;
	H_T(state_ty,0)=-sinpsi*temp;
	double temp2=cosalpha*sinalpha*tv;
	H_T(state_x,1)=sinpsi-temp2*cospsi;
	H_T(state_y,1)=cospsi+temp2*sinpsi;
	double temp4=sinalpha*doca;
	double temp5=tv*cos2_alpha*du*cos2_alpha_minus_sin2_alpha;
	H_T(state_tx,1)=-sinpsi*temp4-cospsi*temp5;
	H_T(state_ty,1)=-cospsi*temp4+sinpsi*temp5;
	
	// Matrix transpose H_T -> H
	H(0,state_x)=H_T(state_x,0);
	H(0,state_y)=H_T(state_y,0);
	H(0,state_tx)=H_T(state_tx,0);
	H(0,state_ty)=H_T(state_ty,0);
	H(1,state_x)=H_T(state_x,1);
	H(1,state_y)=H_T(state_y,1);
	H(1,state_tx)=H_T(state_tx,1);
	H(1,state_ty)=H_T(state_ty,1);
	
	// Variance for this hit
	InvV=(V+H*C*H_T).Invert();
	
	// Compute Kalman gain matrix
	K=(C*H_T)*InvV;
	
	if (fdchits[my_id]->wire->layer!=PLANE_TO_SKIP){
	  /*
	    if(DEBUG_HISTS){
	    hFDCOccTrkFit->Fill(fdchits[my_id]->wire->layer);
	    }
	  */
	  // Update the state vector 
	  S+=K*Mdiff;
	  if(VERBOSE) S.Print();
	  // Update state vector covariance matrix
	  C=C-K*(H*C);    
	  
	  // Update the filtered measurement covariane matrix and put results in 
	  // update vector
	  DMatrix2x2 RC=V-H*C*H_T;
	  DMatrix2x1 res=Mdiff-H*K*Mdiff;
	  
	  chi2+=RC.Chi2(res);
	  ndof+=2;
	   
	  // fill pull vector entries
	  updates[my_id].V=RC;
	}
	else{
	  updates[my_id].V=V;
	}
	
	used_fdc_hits[my_id]=1;
	
	// fill pull vector
	updates[my_id].d=doca;
	updates[my_id].S=S;
	updates[my_id].C=C;
	updates[my_id].tdrift=fdchits[my_id]->time-trajectory[k].t;
	updates[my_id].s=29.98*trajectory[k].t; // assume beta=1
      } 
    }
    
    if (more_hits && trajectory[k].z < downstreamEndplate){
      // Position along wire
      double z0=origin.Z();
      wirepos=origin+(trajectory[k].z-z0)*wdir;
      
      // New doca^2
      double dx=S(state_x)-wirepos.X();
      double dy=S(state_y)-wirepos.Y();
      doca2=dx*dx+dy*dy;
      if (VERBOSE > 10) jout<< "At Position " << S(state_x) << " " << S(state_y) << " " << trajectory[k].z << " doca2 " << doca2 << endl;
      
      if (doca2>old_doca2 && more_hits && !firstCDCStep){
	
	// zero-position and direction of line describing particle trajectory
	double tx=S(state_tx),ty=S(state_ty);
	DVector3 pos0(S(state_x),S(state_y),trajectory[k].z);
	DVector3 tdir(tx,ty,1.);
	
	// Find the true doca to the wire
	DVector3 diff=pos0-origin;
	double dx0=diff.x(),dy0=diff.y();
	double wdir_dot_diff=diff.Dot(wdir);
	double tdir_dot_diff=diff.Dot(tdir);
	double tdir_dot_wdir=tdir.Dot(wdir);
	double tdir2=tdir.Mag2();
	double wdir2=wdir.Mag2();
	double D=tdir2*wdir2-tdir_dot_wdir*tdir_dot_wdir;
	double N=tdir_dot_wdir*wdir_dot_diff-wdir2*tdir_dot_diff;
	double N1=tdir2*wdir_dot_diff-tdir_dot_wdir*tdir_dot_diff;
	double scale=1./D;
	double s=scale*N;
	double t=scale*N1;
	diff+=s*tdir-t*wdir;
	double d=diff.Mag()+d_EPS; // prevent division by zero
	
	// The next measurement	
	double dmeas=0.39,delta=0.;  
	double tdrift=cdchits[cdc_index]->tdrift-trajectory[k].t;	 
	if (fit_type==kTimeBased){	 
	  double phi_d=diff.Phi();
	  double dphi=phi_d-origin.Phi();
	  while (dphi>M_PI) dphi-=2*M_PI;
	  while (dphi<-M_PI) dphi+=2*M_PI;
	  
	  int ring_index=cdchits[cdc_index]->wire->ring-1;
	  int straw_index=cdchits[cdc_index]->wire->straw-1;
	  double dz=t*wdir.z();
	  double delta=max_sag[ring_index][straw_index]*(1.-dz*dz/5625.)
	    *cos(phi_d+sag_phi_offset[ring_index][straw_index]);
	  CDCDriftParameters(dphi,delta,tdrift,dmeas,V_CDC);
	}

	// residual
	double res=dmeas-d;
	if (VERBOSE>5) jout << " Residual " << res << endl;
	// Track projection
	double one_over_d=1./d;
	double diffx=diff.x(),diffy=diff.y(),diffz=diff.z();
	double wx=wdir.x(),wy=wdir.y();
	
	double dN1dtx=2.*tx*wdir_dot_diff-wx*tdir_dot_diff-tdir_dot_wdir*dx0;
	double dDdtx=2.*tx*wdir2-2.*tdir_dot_wdir*wx;
	double dtdtx=scale*(dN1dtx-t*dDdtx);
	
	double dN1dty=2.*ty*wdir_dot_diff-wy*tdir_dot_diff-tdir_dot_wdir*dy0;
	double dDdty=2.*ty*wdir2-2.*tdir_dot_wdir*wy;
	double dtdty=scale*(dN1dty-t*dDdty);
	
	double dNdtx=wx*wdir_dot_diff-wdir2*dx0;
	double dsdtx=scale*(dNdtx-s*dDdtx);
	
	double dNdty=wy*wdir_dot_diff-wdir2*dy0;
	double dsdty=scale*(dNdty-s*dDdty);
	
	H_CDC(state_tx)=H_T_CDC(state_tx)
	  =one_over_d*(diffx*(s+tx*dsdtx-wx*dtdtx)+diffy*(ty*dsdtx-wy*dtdtx)
		       +diffz*(dsdtx-dtdtx));
	H_CDC(state_ty)=H_T_CDC(state_ty)
	  =one_over_d*(diffx*(tx*dsdty-wx*dtdty)+diffy*(s+ty*dsdty-wy*dtdty)
		       +diffz*(dsdty-dtdty));
	
	double dsdx=scale*(tdir_dot_wdir*wx-wdir2*tx);
	double dtdx=scale*(tdir2*wx-tdir_dot_wdir*tx);
	double dsdy=scale*(tdir_dot_wdir*wy-wdir2*ty);
	double dtdy=scale*(tdir2*wy-tdir_dot_wdir*ty);
	
	H_CDC(state_x)=H_T_CDC(state_x)
	  =one_over_d*(diffx*(1.+dsdx*tx-dtdx*wx)+diffy*(dsdx*ty-dtdx*wy)
		       +diffz*(dsdx-dtdx));
	H_CDC(state_y)=H_T_CDC(state_y)
	  =one_over_d*(diffx*(dsdy*tx-dtdy*wx)+diffy*(1.+dsdy*ty-dtdy*wy)
		       +diffz*(dsdy-dtdy));
	
	double InvV=1./(V_CDC+H_CDC*C*H_T_CDC);
	
	// Check how far this hit is from the projection
	double chi2check=res*res*InvV;
	if (chi2check < CHI2CUT || DO_PRUNING == 0){
	  if (VERBOSE) jout << "CDC Hit Added to FDC track " << endl;
	  if (cdchits[cdc_index]->wire->ring!=RING_TO_SKIP){

	    // Compute Kalman gain matrix
	    K_CDC=InvV*(C*H_T_CDC);
	    // Update state vector covariance matrix
	    DMatrix4x4 Ctest=C-K_CDC*(H_CDC*C);
	    
	    //C.Print();
	    //K.Print();
	    //Ctest.Print();
	    
	    // Check that Ctest is positive definite
	    if (!Ctest.IsPosDef()) return VALUE_OUT_OF_RANGE;
	    C=Ctest;
	    if(VERBOSE>10) C.Print();
	    // Update the state vector
	    //S=S+res*K;
	    S+=res*K_CDC;
	    if(VERBOSE) {jout << "traj[z]=" << trajectory[k].z<< endl; S.Print();} 
	    
	    // Compute new residual
	    //d=finder->FindDoca(trajectory[k].z,S,wdir,origin);
	    res=res-H_CDC*K_CDC*res;
	    
	    // Update chi2
	    double fit_V=V_CDC-H_CDC*C*H_T_CDC;
	    chi2+=res*res/fit_V;
	    ndof++;
	  
	    // fill pull vector entry
	    cdc_updates[cdc_index].V=fit_V;
	  }
	  else {
	    cdc_updates[cdc_index].V=V_CDC;
	  }

	  // fill updates
	  cdc_updates[cdc_index].resi=res;
	  cdc_updates[cdc_index].d=d;
	  cdc_updates[cdc_index].delta=delta;
	  cdc_updates[cdc_index].S=S;
	  cdc_updates[cdc_index].C=C;
	  cdc_updates[cdc_index].tdrift=tdrift;
	  cdc_updates[cdc_index].ddrift=dmeas;
	  cdc_updates[cdc_index].s=29.98*trajectory[k].t; // assume beta=1
	  trajectory[k].id=cdc_index+1000;
	  
	  used_cdc_hits[cdc_index]=1;
	}
	// move to next cdc hit
	if (cdc_index>0){
	  cdc_index--;
	  
	  //New wire position
	  wire=cdchits[cdc_index]->wire;
	  if (VERBOSE>5) jout << " Next Wire ring " << wire->ring << endl;
	  origin=wire->origin;
	  double vz=wire->udir.z();
	  wdir=(1./vz)*wire->udir;
	  wirepos=origin+((trajectory[k].z-z0))*wdir;
	  
	  // New doca^2
	  dx=S(state_x)-wirepos.x();
	  dy=S(state_y)-wirepos.y();
	  doca2=dx*dx+dy*dy;
	  
	}
	else more_hits=false;
      }
      firstCDCStep=false;
      old_doca2=doca2;
    }
  }
  
  ndof-=4;
  
  return NOERROR;
}


// Smoothing algorithm for the forward trajectory.  Updates the state vector
// at each step (going in the reverse direction to the filter) based on the 
// information from all the steps and outputs the state vector at the
// outermost step.

jerror_t 
DTrackFitterStraightTrack::Smooth(vector<fdc_update_t>&fdc_updates,
      vector<cdc_update_t>&cdc_updates){ 
  unsigned int max=best_trajectory.size()-1;
  DMatrix4x1 S=(best_trajectory[max].Skk);
  DMatrix4x4 C=(best_trajectory[max].Ckk);
  DMatrix4x4 JT=best_trajectory[max].J.Transpose();
  DMatrix4x1 Ss=S;
  DMatrix4x4 Cs=C;
  DMatrix4x4 A,dC;
  
  const double d_EPS=1e-8;
  
  for (unsigned int m=max-1;m>0;m--){
    if (best_trajectory[m].id>0 && best_trajectory[m].id<1000){ // FDC Hit
      unsigned int id=best_trajectory[m].id-1;
      A=fdc_updates[id].C*JT*C.Invert();
      Ss=fdc_updates[id].S+A*(Ss-S);

      dC=A*(Cs-C)*A.Transpose();
      Cs=fdc_updates[id].C+dC;

      double cosa=cos(fdchits[id]->wire->angle);
      double cos2a=cos(2*fdchits[id]->wire->angle);
      double sina=sin(fdchits[id]->wire->angle);
      double u=fdchits[id]->w;
      double v=fdchits[id]->s;

      // Small angle alignment correction
      double x = Ss(state_x) + fdchits[id]->wire->angles.Z() * Ss(state_y);
      double y = Ss(state_y) - fdchits[id]->wire->angles.Z() * Ss(state_x);
      // tz = 1.0 + my_fdchits[id]->phiY * tx - my_fdchits[id]->phiX * ty;
      double tx = Ss(state_tx) + fdchits[id]->wire->angles.Z() * Ss(state_ty) - fdchits[id]->wire->angles.Y();
      double ty = Ss(state_ty) - fdchits[id]->wire->angles.Z() * Ss(state_tx) + fdchits[id]->wire->angles.X();

      // Projected position along the wire 
      double vpred=x*sina+y*cosa;
      
      // Projected position in the plane of the wires transverse to the wires
      double upred=x*cosa-y*sina;

      // Direction tangent in the u-z plane
      double tu=tx*cosa-ty*sina;
      double alpha=atan(tu);
      double cosalpha=cos(alpha);
      //double cosalpha2=cosalpha*cosalpha;
      double sinalpha=sin(alpha);

      // (signed) distance of closest approach to wire
      double du=upred-u;
      double doca=du*cosalpha;
      // Difference between measurement and projection for the cathodes
      double tv=tx*sina+ty*cosa;
      double resi_c=v-vpred;

      // Difference between measurement and projection perpendicular to the wire
      double drift = 0.0;  // assume hit at wire position
      int left_right = -999;
      double drift_time = fdc_updates[id].tdrift;
      if (fit_type == kTimeBased) {
        drift = (du > 0.0 ? 1.0 : -1.0) * fdc_drift_distance(drift_time);
        left_right = (du > 0.0 ? +1 : -1);
      }
      double resi_a = drift - doca;

      // Variance from filter step
      DMatrix2x2 V=fdc_updates[id].V;
      // Compute projection matrix and find the variance for the residual
      DMatrix4x2 H_T;
      double temp2=-tv*sinalpha;
      H_T(state_x,1)=sina+cosa*cosalpha*temp2;	
      H_T(state_y,1)=cosa-sina*cosalpha*temp2;	
      
      double cos2_minus_sin2=cosalpha*cosalpha-sinalpha*sinalpha;
      double doca_cosalpha=doca*cosalpha;
      H_T(state_tx,1)=-doca_cosalpha*(tu*sina+tv*cosa*cos2_minus_sin2);
      H_T(state_ty,1)=-doca_cosalpha*(tu*cosa-tv*sina*cos2_minus_sin2);
      
      H_T(state_x,0)=cosa*cosalpha;
      H_T(state_y,0)=-sina*cosalpha;
      double one_plus_tu2=1.+tu*tu;
      double factor=du*tu/sqrt(one_plus_tu2)/one_plus_tu2;
      H_T(state_ty,0)=sina*factor;
      H_T(state_tx,0)=-cosa*factor;

      // Matrix transpose H_T -> H
      DMatrix2x4 H;
      H(0,state_x)=H_T(state_x,0);
      H(0,state_y)=H_T(state_y,0);
      H(0,state_tx)=H_T(state_tx,0);
      H(0,state_ty)=H_T(state_ty,0);
      H(1,state_x)=H_T(state_x,1);
      H(1,state_y)=H_T(state_y,1);
      H(1,state_tx)=H_T(state_tx,1);
      H(1,state_ty)=H_T(state_ty,1);

      if (fdchits[id]->wire->layer == PLANE_TO_SKIP) {
        // V += Cs.SandwichMultiply(H_T);
        V = V + H * Cs * H_T;
      } else {
        // V -= dC.SandwichMultiply(H_T);

        // R. Fruehwirth, Nucl. Instrum. Methods Phys. Res. A 262, 444 (1987).
        // Eq. (9)
        // The following V (lhs) corresponds to R^n_k in the paper.
        // dC corresponds to 'A_k * (C^n_{k+1} - C^k_{k+1}) * A_k^T' in the paper.
        V = V - H * dC * H_T;
      }

      /*
      if(DEBUG_HISTS){
	hFDCOccTrkSmooth->Fill(fdchits[id]->wire->layer);
      }
      */
      // Implement derivatives wrt track parameters needed for millepede alignment
      // Add the pull
      double scale=1./sqrt(1.+tx*tx+ty*ty);
      double cosThetaRel=fdchits[id]->wire->udir.Dot(DVector3(scale*tx,scale*ty,scale));
      DTrackFitter::pull_t thisPull(resi_a,sqrt(V(0,0)),
				    best_trajectory[m].t*SPEED_OF_LIGHT,
				    fdc_updates[id].tdrift,
				    fdc_updates[id].d,
				    NULL,fdchits[id],
				    0.0, //docaphi
				    best_trajectory[m].z,cosThetaRel, 
				    0.0, //tcorr
				    resi_c, sqrt(V(1,1))
				    );
      thisPull.left_right = left_right;

      if (fdchits[id]->wire->layer!=PLANE_TO_SKIP){
	vector<double> derivatives;
	derivatives.resize(FDCTrackD::size);
	
	//dDOCAW/dDeltaX
	derivatives[FDCTrackD::dDOCAW_dDeltaX] = -(1/sqrt(1 + pow(tx*cosa - ty*sina,2)));

	//dDOCAW/dDeltaZ
	derivatives[FDCTrackD::dDOCAW_dDeltaZ] = (tx*cosa - ty*sina)/sqrt(1 + pow(tx*cosa - ty*sina,2));

	//dDOCAW/ddeltaPhiX
	derivatives[FDCTrackD::dDOCAW_dDeltaPhiX] = (sina*(-(tx*cosa) + ty*sina)*(u - x*cosa + y*sina))/pow(1 + pow(tx*cosa - ty*sina,2),1.5);

	//dDOCAW/ddeltaphiY
	derivatives[FDCTrackD::dDOCAW_dDeltaPhiY] = (cosa*(tx*cosa - ty*sina)*(-u + x*cosa - y*sina))/pow(1 + pow(tx*cosa - ty*sina,2),1.5);

	//dDOCAW/ddeltaphiZ
	derivatives[FDCTrackD::dDOCAW_dDeltaPhiZ] = (tx*ty*u*cos2a + (x + pow(ty,2)*x - tx*ty*y)*sina + 
						     cosa*(-(tx*ty*x) + y + pow(tx,2)*y + (pow(tx,2) - pow(ty,2))*u*sina))/
	  pow(1 + pow(tx*cosa - ty*sina,2),1.5);
	
	// dDOCAW/dx
	derivatives[FDCTrackD::dDOCAW_dx] = cosa/sqrt(1 + pow(tx*cosa - ty*sina,2));

	// dDOCAW/dy
	derivatives[FDCTrackD::dDOCAW_dy] = -(sina/sqrt(1 + pow(tx*cosa - ty*sina,2)));

	// dDOCAW/dtx
	derivatives[FDCTrackD::dDOCAW_dtx] = -((cosa*(tx*cosa - ty*sina)*(-u + x*cosa - y*sina))/pow(1 + pow(tx*cosa - ty*sina,2),1.5));

	// dDOCAW/dty
	derivatives[FDCTrackD::dDOCAW_dty] = (sina*(-(tx*cosa) + ty*sina)*(u - x*cosa + y*sina))/pow(1 + pow(tx*cosa - ty*sina,2),1.5); 

    // dDOCAW/dt0
    double t0shift = 4.0;  // ns
    double drift_shift = 0.0;
    if (drift_time < 0.0) {
      drift_shift = drift;
    } else {
      drift_shift =
          (du > 0.0 ? 1.0 : -1.0) *
          fdc_drift_distance(drift_time + t0shift);
    }
    derivatives[FDCTrackD::dW_dt0] = (drift_shift - drift) / t0shift;

	// And the cathodes
	//dDOCAW/ddeltax
	derivatives[FDCTrackD::dDOCAC_dDeltaX] = 0.;

	//dDOCAW/ddeltax
	derivatives[FDCTrackD::dDOCAC_dDeltaZ] = ty*cosa + tx*sina;

	//dDOCAW/ddeltaPhiX
	derivatives[FDCTrackD::dDOCAC_dDeltaPhiX] = 0.;

	//dDOCAW/ddeltaPhiX
	derivatives[FDCTrackD::dDOCAC_dDeltaPhiY] = 0.;

	//dDOCAW/ddeltaPhiX
	derivatives[FDCTrackD::dDOCAC_dDeltaPhiZ] = -(x*cosa) + y*sina;

	// dDOCAW/dx
	derivatives[FDCTrackD::dDOCAC_dx] = sina;

	// dDOCAW/dy
	derivatives[FDCTrackD::dDOCAW_dy] = cosa;

	// dDOCAW/dtx
	derivatives[FDCTrackD::dDOCAW_dtx] = 0.;

	// dDOCAW/dty
	derivatives[FDCTrackD::dDOCAW_dty] = 0.;
	
	thisPull.AddTrackDerivatives(derivatives);
      }
      pulls.push_back(thisPull);  
    }
    else if (best_trajectory[m].id>=1000){ // CDC Hit
      unsigned int id=best_trajectory[m].id-1000;
      A=cdc_updates[id].C*JT*C.Invert();
      Ss=cdc_updates[id].S+A*(Ss-S);
      
      dC=A*(Cs-C)*A.Transpose();
      Cs=cdc_updates[id].C+dC;
      if (VERBOSE > 10) {
	jout << " In Smoothing Step Using ID " << id << "/" << cdc_updates.size() << " for ring " << cdchits[id]->wire->ring << endl;
	jout << " A cdc_updates[id].C Ss Cs " << endl;
	A.Print(); cdc_updates[id].C.Print(); Ss.Print(); Cs.Print();
      }
      if(!Cs.IsPosDef()) {
	if (VERBOSE) jout << "Cs is not PosDef!" << endl;
	return VALUE_OUT_OF_RANGE;
      }
      
      const DCDCWire *wire=cdchits[id]->wire;
      DVector3 origin=wire->origin;
      double z0=origin.z();
      double vz=wire->udir.z();
      DVector3 wdir=(1./vz)*wire->udir;
      DVector3 wirepos=origin+(best_trajectory[m].z-z0)*wdir;
      // Position and direction from state vector
      double x=Ss(state_x);
      double y=Ss(state_y);
      double tx=Ss(state_tx);
      double ty=Ss(state_ty);
      
      DVector3 pos0(x,y,best_trajectory[m].z);
      DVector3 tdir(tx,ty,1.);
      
      // Find the true doca to the wire
      DVector3 diff=pos0-origin;
      double dx0=diff.x(),dy0=diff.y();
      double wdir_dot_diff=diff.Dot(wdir);
      double tdir_dot_diff=diff.Dot(tdir);
      double tdir_dot_wdir=tdir.Dot(wdir);
      double tdir2=tdir.Mag2();
      double wdir2=wdir.Mag2();
      double D=tdir2*wdir2-tdir_dot_wdir*tdir_dot_wdir;
      double N=tdir_dot_wdir*wdir_dot_diff-wdir2*tdir_dot_diff;
      double N1=tdir2*wdir_dot_diff-tdir_dot_wdir*tdir_dot_diff;
      double scale=1./D;
      double s=scale*N;
      double t=scale*N1;
      diff+=s*tdir-t*wdir;
      double d=diff.Mag()+d_EPS; // prevent division by zero
      double ddrift = cdc_updates[id].ddrift;
      
      double resi = ddrift - d;
      
      
      // Track projection
      DMatrix1x4 H; DMatrix4x1 H_T;
      {
	double one_over_d=1./d;
	double diffx=diff.x(),diffy=diff.y(),diffz=diff.z();
	double wx=wdir.x(),wy=wdir.y();
	
	double dN1dtx=2.*tx*wdir_dot_diff-wx*tdir_dot_diff-tdir_dot_wdir*dx0;
	double dDdtx=2.*tx*wdir2-2.*tdir_dot_wdir*wx;
	double dtdtx=scale*(dN1dtx-t*dDdtx);
	
	double dN1dty=2.*ty*wdir_dot_diff-wy*tdir_dot_diff-tdir_dot_wdir*dy0;
	double dDdty=2.*ty*wdir2-2.*tdir_dot_wdir*wy;
	double dtdty=scale*(dN1dty-t*dDdty);
	
	double dNdtx=wx*wdir_dot_diff-wdir2*dx0;
	double dsdtx=scale*(dNdtx-s*dDdtx);
	
	double dNdty=wy*wdir_dot_diff-wdir2*dy0;
	double dsdty=scale*(dNdty-s*dDdty);
	
	H(state_tx)=H_T(state_tx)
	  =one_over_d*(diffx*(s+tx*dsdtx-wx*dtdtx)+diffy*(ty*dsdtx-wy*dtdtx)
		       +diffz*(dsdtx-dtdtx));
	H(state_ty)=H_T(state_ty)
	  =one_over_d*(diffx*(tx*dsdty-wx*dtdty)+diffy*(s+ty*dsdty-wy*dtdty)
		       +diffz*(dsdty-dtdty));
	
	double dsdx=scale*(tdir_dot_wdir*wx-wdir2*tx);
	double dtdx=scale*(tdir2*wx-tdir_dot_wdir*tx);
	double dsdy=scale*(tdir_dot_wdir*wy-wdir2*ty);
	double dtdy=scale*(tdir2*wy-tdir_dot_wdir*ty);

	H(state_x)=H_T(state_x)
	  =one_over_d*(diffx*(1.+dsdx*tx-dtdx*wx)+diffy*(dsdx*ty-dtdx*wy)
		       +diffz*(dsdx-dtdx));
	H(state_y)=H_T(state_y)
	  =one_over_d*(diffx*(dsdy*tx-dtdy*wx)+diffy*(1.+dsdy*ty-dtdy*wy)
		       +diffz*(dsdy-dtdy));
      }
      double V=cdc_updates[id].V;

      if (VERBOSE > 10) jout << " d " << d << " H*S " << H*S << endl;
      if (cdchits[id]->wire->ring==RING_TO_SKIP){
	V=V+H*Cs*H_T;
      }
      else{
	V=V-H*Cs*H_T;
      }
      if (V<0) return VALUE_OUT_OF_RANGE;
      
      // Add the pull
      double myscale=1./sqrt(1.+tx*tx+ty*ty);
      double cosThetaRel=wire->udir.Dot(DVector3(myscale*tx,myscale*ty,myscale));
      DTrackFitter::pull_t thisPull(resi,sqrt(V),
				    best_trajectory[m].t*SPEED_OF_LIGHT,
				    cdc_updates[id].tdrift,
				    d,cdchits[id], NULL,
				    diff.Phi(), //docaphi
				    best_trajectory[m].z,cosThetaRel,
				    cdc_updates[id].tdrift);

      // Derivatives for alignment
      double wtx=wire->udir.X(), wty=wire->udir.Y(), wtz=wire->udir.Z();
      double wx=wire->origin.X(), wy=wire->origin.Y(), wz=wire->origin.Z();
      
      double z=best_trajectory[m].z;
      double tx2=tx*tx, ty2=ty*ty;
      double wtx2=wtx*wtx, wty2=wty*wty, wtz2=wtz*wtz;
      double denom=(1 + ty2)*wtx2 + (1 + tx2)*wty2 - 2*ty*wty*wtz + (tx2 + ty2)*wtz2 - 2*tx*wtx*(ty*wty + wtz)+d_EPS;
      double denom2=denom*denom;
      double c1=-(wtx - tx*wtz)*(wy - y);
      double c2=wty*(wx - tx*wz - x + tx*z);
      double c3=ty*(-(wtz*wx) + wtx*wz + wtz*x - wtx*z);
      double dscale=0.5*(1/d);
      
      vector<double> derivatives(11);
      
      derivatives[CDCTrackD::dDOCAdOriginX]=dscale*(2*(wty - ty*wtz)*(c1 + c2 + c3))/denom;
      
      derivatives[CDCTrackD::dDOCAdOriginY]=dscale*(2*(-wtx + tx*wtz)*(c1 + c2 + c3))/denom;
      
      derivatives[CDCTrackD::dDOCAdOriginZ]=dscale*(2*(ty*wtx - tx*wty)*(c1 + c2 + c3))/denom;
      
      derivatives[CDCTrackD::dDOCAdDirX]=dscale*(2*(wty - ty*wtz)*(c1 + c2 + c3)*
						 (tx*(ty*wty + wtz)*(wx - x) + (wty - ty*wtz)*(-wy + y + ty*(wz - z)) +
						  wtx*(-((1 + ty2)*wx) + (1 + ty2)*x + tx*(ty*wy + wz - ty*y - z)) + tx2*(wty*(-wy + y) + wtz*(-wz + z))))/denom2;
      
      derivatives[CDCTrackD::dDOCAdDirY]=dscale*(-2*(wtx - tx*wtz)*(c1 + c2 + c3)*
						 (tx*(ty*wty + wtz)*(wx - x) + (wty - ty*wtz)*(-wy + y + ty*(wz - z)) +
						  wtx*(-((1 + ty2)*wx) + (1 + ty2)*x + tx*(ty*wy + wz - ty*y - z)) + tx2*(wty*(-wy + y) + wtz*(-wz + z))))/denom2;

      derivatives[CDCTrackD::dDOCAdDirZ]=dscale*(-2*(ty*wtx - tx*wty)*(c1 + c2 + c3)*
						 (-(tx*(ty*wty + wtz)*(wx - x)) + tx2*(wty*(wy - y) + wtz*(wz - z)) + (wty - ty*wtz)*(wy - y + ty*(-wz + z)) +
						  wtx*((1 + ty2)*wx - (1 + ty2)*x + tx*(-(ty*wy) - wz + ty*y + z))))/denom2;

      derivatives[CDCTrackD::dDOCAdS0]=-derivatives[CDCTrackD::dDOCAdOriginX];
      
      derivatives[CDCTrackD::dDOCAdS1]=-derivatives[CDCTrackD::dDOCAdOriginY];

      derivatives[CDCTrackD::dDOCAdS2]=dscale*(2*(wty - ty*wtz)*(-c1 - c2 - c3)*
					       (-(wtx*wtz*wx) - wty*wtz*wy + wtx2*wz + wty2*wz + wtx*wtz*x + wty*wtz*y - wtx2*z - wty2*z +
						tx*(wty2*(wx - x) + wtx*wty*(-wy + y) + wtz*(wtz*wx - wtx*wz - wtz*x + wtx*z)) +
						ty*(wtx*wty*(-wx + x) + wtx2*(wy - y) + wtz*(wtz*wy - wty*wz - wtz*y + wty*z))))/denom2;
      
      derivatives[CDCTrackD::dDOCAdS3]=dscale*(2*(wtx - tx*wtz)*(c1 + c2 + c3)*
					       (-(wtx*wtz*wx) - wty*wtz*wy + wtx2*wz + wty2*wz + wtx*wtz*x + wty*wtz*y - wtx2*z - wty2*z +
						tx*(wty2*(wx - x) + wtx*wty*(-wy + y) + wtz*(wtz*wx - wtx*wz - wtz*x + wtx*z)) +
						ty*(wtx*wty*(-wx + x) + wtx2*(wy - y) + wtz*(wtz*wy - wty*wz - wtz*y + wty*z))))/denom2;
      
      thisPull.AddTrackDerivatives(derivatives);
      
      pulls.push_back(thisPull);
    }
    else{
      A=best_trajectory[m].Ckk*JT*C.Invert();
      Ss=best_trajectory[m].Skk+A*(Ss-S);
      Cs=best_trajectory[m].Ckk+A*(Cs-C)*A.Transpose();
    }
    
    S=best_trajectory[m].Skk;
    C=best_trajectory[m].Ckk;
    JT=best_trajectory[m].J.Transpose();
  }
  
  return NOERROR;
}

shared_ptr<TMatrixFSym> 
DTrackFitterStraightTrack::Get7x7ErrorMatrix(TMatrixFSym C,DMatrix4x1 &S,double sign){
  auto C7x7 = dResourcePool_TMatrixFSym->Get_SharedResource();
  C7x7->ResizeTo(7, 7);
  DMatrix J(7,5);

  double p=5.; // fixed: cannot measure
  double tx_=S(state_tx);
  double ty_=S(state_ty);
  double x_=S(state_x);
  double y_=S(state_y);
  double tanl=sign/sqrt(tx_*tx_+ty_*ty_);
  double tanl2=tanl*tanl;
  double lambda=atan(tanl);
  double sinl=sin(lambda);
  double sinl3=sinl*sinl*sinl;
  
  J(state_X,state_x)=J(state_Y,state_y)=1.;
  J(state_Pz,state_ty)=-p*ty_*sinl3;
  J(state_Pz,state_tx)=-p*tx_*sinl3;
  J(state_Px,state_ty)=J(state_Py,state_tx)=-p*tx_*ty_*sinl3;
  J(state_Px,state_tx)=p*(1.+ty_*ty_)*sinl3;
  J(state_Py,state_ty)=p*(1.+tx_*tx_)*sinl3;
  J(state_Pz,4)=-p*p*sinl;
  J(state_Px,4)=tx_*J(state_Pz,4);
  J(state_Py,4)=ty_*J(state_Pz,4); 
  J(state_Z,state_x)=-tx_*tanl2;
  J(state_Z,state_y)=-ty_*tanl2;
  double diff=tx_*tx_-ty_*ty_;
  double frac=tanl2*tanl2;
  J(state_Z,state_tx)=(x_*diff+2.*tx_*ty_*y_)*frac;
  J(state_Z,state_ty)=(2.*tx_*ty_*x_-y_*diff)*frac;
  
  // C'= JCJ^T
  *C7x7=C.Similarity(J);
  
  return C7x7;
}

// Routine to get extrapolations to other detectors
void DTrackFitterStraightTrack::GetExtrapolations(const DVector3 &pos0,
						  const DVector3 &dir){
  double x0=pos0.x(),y0=pos0.y(),z0=pos0.z();
  double s=0.,t=0.;
  DVector3 pos(0,0,0);
  DVector3 diff(0,0,0);
  ClearExtrapolations();

  double z=z0;
  double dz=0.1;
  double uz=dir.z();
  double ux=dir.x()/uz,ux2=ux*ux;
  double uy=dir.y()/uz,uy2=uy*uy;

  // Extrapolate to start counter
  double Rd=SC_BARREL_R;
  double A=ux*x0 + uy*y0;
  double B=uy2*(Rd - x0)*(Rd + x0) + 2*ux*uy*x0*y0 + ux2*(Rd - y0)*(Rd + y0);
  double C=ux2 + uy2;
  if (sc_pos.empty()==false && z<SC_END_NOSE_Z && B>0){
    dz=-(A-sqrt(B))/C;
    pos=pos0+(dz/uz)*dir;
    z=pos.z();
    bool done=(z<sc_pos[0][0].z()||z>SC_END_NOSE_Z);
    while(!done){
      double d_old=1000.,d=1000.;
      unsigned int index=0;
     
      for (unsigned int m=0;m<12;m++){
	double dphi=pos.Phi()-SC_PHI_SECTOR1;
	if (dphi<0) dphi+=2.*M_PI;
	index=int(floor(dphi/(2.*M_PI/30.)));
	if (index>29) index=0;
	d=sc_norm[index][m].Dot(pos-sc_pos[index][m]);
	if (d*d_old<0){ // break if we cross the current plane
	  pos+=d*dir;
	  s=(pos-pos0).Mag();
	  t=s/29.98;
	  extrapolations[SYS_START].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
	  done=true;
	  break;
	}
	d_old=d;
      }
      pos-=(0.1/uz)*dir;
      z=pos.z();
    }
  }
  // Extrapolate to BCAL
  Rd=65.; // approximate BCAL inner radius
  B=uy2*(Rd - x0)*(Rd + x0) + 2*ux*uy*x0*y0 + ux2*(Rd - y0)*(Rd + y0);
  if (B>0){
    diff=-(A-sqrt(B))/(C*uz)*dir;
    s=diff.Mag();
    t=s/29.98;
    pos=pos0+diff;
    extrapolations[SYS_BCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s)); 
    Rd=89.; // approximate BCAL outer radius
    B=uy2*(Rd - x0)*(Rd + x0) + 2*ux*uy*x0*y0 + ux2*(Rd - y0)*(Rd + y0);
    if (B>0){
      diff=-(A-sqrt(B))/(C*uz)*dir;
      s=diff.Mag();
      t=s/29.98;
      pos=pos0+diff;
      extrapolations[SYS_BCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
    }
  }

  // Extrapolate to TRD
  for (unsigned int i=0;i<dTRDz_vec.size();i++){
    diff=((dTRDz_vec[i]-z0)/uz)*dir;
    pos=pos0+diff;
    if (fabs(pos.x())<130. && fabs(pos.y())<130.){
      s=diff.Mag();
      t=s/29.98;
      extrapolations[SYS_TRD].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));   	
    }
  }

  // Extrapolate to DIRC
  diff=((dDIRCz-z0)/uz)*dir;
  pos=pos0+diff;
  if (fabs(pos.x())<130. && fabs(pos.y())<130.){
    s=diff.Mag();
    t=s/29.98;
    extrapolations[SYS_DIRC].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
  }

  // Extrapolate to TOF
  diff=((dTOFz-z0)/uz)*dir;
  pos=pos0+diff;
  if (fabs(pos.x())<130. && fabs(pos.y())<130.){
    double s=diff.Mag();
    double t=s/29.98;
    s=diff.Mag();
    t=s/29.98;
    extrapolations[SYS_TOF].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));	
  }
  
  // Extrapolate to FCAL
  diff=((dFCALz-z0)/uz)*dir;
  pos=pos0+diff;
  if (fabs(pos.x())<130. && fabs(pos.y())<130.){
    s=diff.Mag();
    t=s/29.98;
    extrapolations[SYS_FCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));  
  
    // extrapolate to exit of FCAL
    diff=((dFCALz+45.-z0)/uz)*dir;
    pos=pos0+diff;
    s=diff.Mag();
    t=s/29.98;
    extrapolations[SYS_FCAL].push_back(DTrackFitter::Extrapolation_t(pos,dir,t,s));
  } 
}
