// $Id$
//
//    File: DTrackFitterStraightTrack.cc
// Created: Tue Mar 12 10:22:32 EDT 2019
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DTrackFitterStraightTrack.h"

//---------------------------------
// DTrackFitterStraightTrack    (Constructor)
//---------------------------------
DTrackFitterStraightTrack::DTrackFitterStraightTrack(JEventLoop *loop):DTrackFitter(loop){
  int runnumber=(loop->GetJEvent()).GetRunNumber();
  DApplication* dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
  JCalibration *jcalib = dapp->GetJCalibration(runnumber);
 
  //****************
  // CDC parameters
  //****************
  map<string, double> cdc_res_parms;
  jcalib->Get("CDC/cdc_resolution_parms_v2", cdc_res_parms);
  CDC_RES_PAR1 = cdc_res_parms["res_par1"];
  CDC_RES_PAR2 = cdc_res_parms["res_par2"];
  CDC_RES_PAR3 = cdc_res_parms["res_par3"];
  
  vector< map<string, double> > tvals;
  cdc_drift_table.clear();  
  if (jcalib->Get("CDC/cdc_drift_table::NoBField", tvals)==false){    
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

   //*************************
   // Command-line parameters
   //*************************
   VERBOSE=0;
   gPARMS->SetDefaultParameter("TRKFIT:VERBOSE",VERBOSE);
   COSMICS=false;
   gPARMS->SetDefaultParameter("TRKFIND:COSMICS",COSMICS);
   CHI2CUT = 15.0; 
   gPARMS->SetDefaultParameter("TRKFIT:CHI2CUT",CHI2CUT);
   DO_PRUNING = true;
   gPARMS->SetDefaultParameter("TRKFIT:DO_PRUNING",DO_PRUNING);
}

//---------------------------------
// ~DTrackFitterStraightTrack    (Destructor)
//---------------------------------
DTrackFitterStraightTrack::~DTrackFitterStraightTrack()
{

}


// Smearing function derived from fitting residuals
inline double DTrackFitterStraightTrack::CDCDriftVariance(double t) const { 
   if (t<0.) t=0.;
   double sigma=CDC_RES_PAR1/(t+1.)+CDC_RES_PAR2 + CDC_RES_PAR3*t;
   return sigma*sigma;
}

// Convert time to distance for the cdc
double DTrackFitterStraightTrack::CDCDriftDistance(double dphi,double delta,
						   double t) const{
  double d=0.;
  if (t>0){
    double f_0=0.;
    double f_delta=0.;

    if (delta>0){
      double a1=long_drift_func[0][0];
      double a2=long_drift_func[0][1];
      double b1=long_drift_func[1][0];
      double b2=long_drift_func[1][1];
      double c1=long_drift_func[2][0];
      double c2=long_drift_func[2][1];
      double c3=long_drift_func[2][2];
      
      // use "long side" functional form
      double my_t=0.001*t;
      double sqrt_t=sqrt(my_t);
      double t3=my_t*my_t*my_t;
      double delta_mag=fabs(delta);
      f_delta=(a1+a2*delta_mag)*sqrt_t+(b1+b2*delta_mag)*my_t
	+(c1+c2*delta_mag+c3*delta*delta)*t3;
      f_0=a1*sqrt_t+b1*my_t+c1*t3;
      }
    else{
      double my_t=0.001*t;
      double sqrt_t=sqrt(my_t);
      double delta_mag=fabs(delta);
      
      // use "short side" functional form
      double a1=short_drift_func[0][0];
      double a2=short_drift_func[0][1];
      double a3=short_drift_func[0][2];
      double b1=short_drift_func[1][0];
      double b2=short_drift_func[1][1];
      double b3=short_drift_func[1][2];
      
      double delta_sq=delta*delta;
      f_delta= (a1+a2*delta_mag+a3*delta_sq)*sqrt_t
	+(b1+b2*delta_mag+b3*delta_sq)*my_t;
      f_0=a1*sqrt_t+b1*my_t;
    }
    
    unsigned int max_index=cdc_drift_table.size()-1;
    if (t>cdc_drift_table[max_index]){
      //_DBG_ << "t: " << t <<" d " << f_delta <<endl;
      d=f_delta;
      
      return d;
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
  return d;
}

// Perform the Kalman Filter for the current set of cdc hits
jerror_t DTrackFitterStraightTrack::KalmanFilter(DMatrix4x1 &S,DMatrix4x4 &C,
						 vector<int>&used_hits,
						 vector<cdc_update_t>&updates,
						 double &chi2,
						 unsigned int &ndof,
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
      
      // The next measurement and its variance
      double tdrift=cdchits[cdc_index]->tdrift-trajectory[k].t;
      double dmeas=0.39; 
      double delta=0.0;
      if (fit_type==kTimeBased){
	V=CDCDriftVariance(tdrift);	
	
	double phi_d=diff.Phi();
	double dphi=phi_d-origin.Phi();  
	while (dphi>M_PI) dphi-=2*M_PI;
	while (dphi<-M_PI) dphi+=2*M_PI;
	
	int ring_index=cdchits[cdc_index]->wire->ring-1;
	int straw_index=cdchits[cdc_index]->wire->straw-1;
	double dz=t*wdir.z();
	delta=max_sag[ring_index][straw_index]*(1.-dz*dz/5625.)
	  *cos(phi_d+sag_phi_offset[ring_index][straw_index]);
	dmeas=CDCDriftDistance(dphi,delta,tdrift);
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
	
	// Flag that we used this hit
	used_hits[cdc_index]=1;
	
	// fill updates
	updates[cdc_index].resi=res;
	updates[cdc_index].d=d;
	updates[cdc_index].delta=delta;
	updates[cdc_index].S=S;
	updates[cdc_index].C=C;
	updates[cdc_index].V=V;
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
  unsigned int max=trajectory.size()-1;
  DMatrix4x1 S=(trajectory[max].Skk);
  DMatrix4x4 C=(trajectory[max].Ckk);
  DMatrix4x4 JT=trajectory[max].J.Transpose();
  DMatrix4x1 Ss=S;
  DMatrix4x4 Cs=C;
  DMatrix4x4 A,dC;
  DMatrix1x4 H;  // Track projection matrix
  DMatrix4x1 H_T; // Transpose of track projection matrix
  
  const double d_EPS=1e-8;
  
  for (unsigned int m=max-1;m>0;m--){
    if (trajectory[m].id>0){
      unsigned int id=trajectory[m].id-1;
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
      DVector3 wirepos=origin+(trajectory[m].z-z0)*wdir;
      // Position and direction from state vector
      double x=Ss(state_x);
      double y=Ss(state_y);
      double tx=Ss(state_tx);
      double ty=Ss(state_ty);
      
      DVector3 pos0(x,y,trajectory[m].z);
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
      V=V-H*Cs*H_T;
      
      if (V<0) return VALUE_OUT_OF_RANGE;

      // Add the pull
      double myscale=1./sqrt(1.+tx*tx+ty*ty);
      double cosThetaRel=wire->udir.Dot(DVector3(myscale*tx,myscale*ty,myscale));
      pull_t thisPull(resi,sqrt(V),
		      trajectory[m].t*SPEED_OF_LIGHT,
		      cdc_updates[id].tdrift,
		      d,cdchits[id], NULL,
		      diff.Phi(), //docaphi
		      trajectory[m].z,cosThetaRel,
		      cdc_updates[id].tdrift);
      
      // Derivatives for alignment
      double wtx=wire->udir.X(), wty=wire->udir.Y(), wtz=wire->udir.Z();
      double wx=wire->origin.X(), wy=wire->origin.Y(), wz=wire->origin.Z();
      
      double z=trajectory[m].z;
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
      A=trajectory[m].Ckk*JT*C.Invert();
      Ss=trajectory[m].Skk+A*(Ss-S);
      Cs=trajectory[m].Ckk+A*(Cs-C)*A.Transpose();
    }
    
    S=trajectory[m].Skk;
    C=trajectory[m].Ckk;
    JT=trajectory[m].J.Transpose();
  }
  
  return NOERROR;
}

//Reference trajectory for the track for cdc tracks
jerror_t DTrackFitterStraightTrack::SetReferenceTrajectory(double t0,double z,
							   DMatrix4x1 &S,
							   const DCDCTrackHit *last_cdc,
							   double &dzsign){ 
  DMatrix4x4 J(1.,0.,1.,0., 0.,1.,0.,1., 0.,0.,1.,0., 0.,0.,0.,1.);

  double ds=1.0;
  double t=t0;
  
  // last y position of hit (approximate, using center of wire)
  double last_y=last_cdc->wire->origin.y();
  double last_r2=last_cdc->wire->origin.Perp2();
  // Check that track points towards last wire, otherwise swap deltaz
  DVector3 dir(S(state_tx),S(state_ty),dzsign);
  if (!COSMICS){
    double dphi=dir.Phi()-last_cdc->wire->origin.Phi(); 
    while (dphi>M_PI) dphi-=2*M_PI;
    while (dphi<-M_PI) dphi+=2*M_PI;
    //if (fabs(dphi) > M_PI/2.) dzsign*=-1.;
  }
  if (fabs(dir.Theta() - M_PI/2.) < 0.2) ds = 0.1;
  
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
  double upstreamEndplate = cdc_endplate_z - cdc_length;
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
    if (z > cdc_endplate_z && dz < 0) continue;
    if (z < upstreamEndplate && dz > 0) continue;
    trajectory.push_front(trajectory_t(z,t,S,J,DMatrix4x1(),DMatrix4x4()));
    
    if (COSMICS) done=((z>cdc_endplate_z) | (z<cdc_endplate_z-cdc_length));
    else{
      double r2=S(state_x)*S(state_x)+S(state_y)*S(state_y);
      if (VERBOSE >= 5) jout << " r2=" << r2 << endl;
      done=(r2>last_r2) | (z>cdc_endplate_z) | (z<cdc_endplate_z-cdc_length); 
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


DTrackFitter::fit_status_t DTrackFitterStraightTrack::FitTrack(void){
  unsigned int numhits=cdchits.size();
  unsigned int maxindex=numhits-1;
  
  // vectors of indexes to cdc hits used in the fit
  vector<int> used_cdc_hits(numhits);
  vector<int> used_cdc_hits_best_fit(numhits);
  
  // vectors of residual information 
  vector<cdc_update_t>updates(numhits);
  vector<cdc_update_t>best_updates(numhits);
  
  // deque to store reference trajectory
  deque<trajectory_t>trajectory;
  deque<trajectory_t>best_trajectory;
  
  // Initial guess for state vector
  DVector3 input_pos=input_params.position();
  DVector3 input_mom=input_params.momentum();
  double pz=input_mom.z();
  DMatrix4x1 S(input_pos.x(),input_pos.y(),input_pos.x()/pz,input_pos.y()/pz);
  
  // State vector to store "best" values
  DMatrix4x1 Sbest;
  
  // Covariance matrix
  DMatrix4x4 C0,C,Cbest;
  C0(state_x,state_x)=C0(state_y,state_y)=10.0;     
  C0(state_tx,state_tx)=C0(state_ty,state_ty)=0.1;
  
  double chi2=1e16,chi2_old=1e16;
  unsigned int ndof=0,ndof_old=0;
  unsigned int iter=0;
  double z0=input_pos.z();
  double t0=input_params.t0();
  
  double dzsign=1.;

  // Perform the fit
  for(iter=0;iter<20;iter++){
    if (VERBOSE) jout << " Performing Pass iter " << iter << endl;
    chi2_old=chi2; 
    ndof_old=ndof;
    
    trajectory.clear();
    if (SetReferenceTrajectory(t0,z0,S,cdchits[maxindex],dzsign)
	!=NOERROR) break;
    
    if (VERBOSE) jout << " Reference Trajectory Set " << endl;
    C=C0;
    if (KalmanFilter(S,C,used_cdc_hits,updates,chi2,ndof,iter)!=NOERROR) break;
    if (VERBOSE) jout << " Filter returns NOERROR" << endl;
     if (fabs(chi2_old-chi2)<0.1 || chi2>chi2_old) break;  
     
     // Save the current state and covariance matrixes
     Cbest=C;
     Sbest=S; 
  }
  
  DVector3 pos(Sbest(state_x),Sbest(state_y),trajectory[0].z);
  double tx=Sbest(state_tx),ty=Sbest(state_ty);
  double denom=sqrt(1.+tx*tx+ty*ty);
  DVector3 mom(tx/denom,ty/denom,1./denom);
  
  fit_params.setPosition(pos);
  fit_params.setMomentum(mom);
  
  auto locTrackingCovarianceMatrix = dResourcePool_TMatrixFSym->Get_SharedResource();
  locTrackingCovarianceMatrix->ResizeTo(5, 5); 
  locTrackingCovarianceMatrix->Zero();
  for(unsigned int loc_i = 0; loc_i < 4; ++loc_i)
    {
      for(unsigned int loc_j = 0; loc_j < 4; ++loc_j)
	(*locTrackingCovarianceMatrix)(loc_i, loc_j) = Cbest(loc_i, loc_j);
      
    }
  (*locTrackingCovarianceMatrix)(4,4)=1.;
  fit_params.setTrackingErrorMatrix(locTrackingCovarianceMatrix);
  
  this->chisq = chi2_old;
  this->Ndof = ndof_old;
  
  return kFitSuccess;
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

