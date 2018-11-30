// $Id$
//
//    File: JEventProcessor_imaging.cc
// Created: Thu Nov  9 10:49:12 EST 2017
// Creator: staylor (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "JEventProcessor_imaging.h"
using namespace jana;
#include <TDirectory.h>
#include <TMatrix.h>

#include <PID/DChargedTrack.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_imaging());
}
} // "C"


//------------------
// JEventProcessor_imaging (Constructor)
//------------------
JEventProcessor_imaging::JEventProcessor_imaging()
{

}

//------------------
// ~JEventProcessor_imaging (Destructor)
//------------------
JEventProcessor_imaging::~JEventProcessor_imaging()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_imaging::init(void)
{
  gDirectory->mkdir("Vertexes")->cd();

  TwoTrackXYZ= new TH3I("TwoTrackXYZ","z vs y vs x",400,-10,10,
			400,-10,10,140,30,100);
  TwoTrackXYZ->SetXTitle("x (cm)");
  TwoTrackXYZ->SetYTitle("y (cm)");
  TwoTrackXYZ->SetZTitle("z (cm)");
  
  TwoTrackZ=new TH1F("TwoTrackZ","z for r<0.5 cm",1000,0,200);
  TwoTrackZ->SetXTitle("z [cm]");

  TwoTrackPocaCut=new TH2F("TwoTrackPocaCut","2track POCA,doca cut",4000,0,400,650,0,65);
  TwoTrackPocaCut->SetXTitle("z (cm)");
  TwoTrackPocaCut->SetYTitle("r (cm)"); 

  TwoTrackXY_at_65cm=new TH2F("TwoTrackXY_at_65cm","y vs x near 65 cm",400,-5,5,400,-5,5);
  TwoTrackXY_at_65cm->SetXTitle("x [cm]");
  TwoTrackXY_at_65cm->SetYTitle("y [cm]");
  
  //  TwoTrackRelCosTheta=new TH1F("TwoTrackRelCosTheta","relative direction",100,-1.,1.);
  TwoTrackChi2=new TH1F("TwoTrackChi2","vertex #chi^2",1000,0,1000);
 
  TwoTrackProb=new TH1F("TwoTrackProb","vertex probability",100,0,1.);

  TwoTrackZFit=new TH1F("TwoTrackZFit","z for r<0.5 cm",1000,0,200);
  TwoTrackZFit->SetXTitle("z [cm]");

  TwoTrackPocaCutFit=new TH2F("TwoTrackPocaCutFit","2track POCA,doca cut",4000,0,400,650,0,65);
  TwoTrackPocaCutFit->SetXTitle("z (cm)");
  TwoTrackPocaCutFit->SetYTitle("r (cm)"); 

  TwoTrackXYFit_at_65cm=new TH2F("TwoTrackXYFit_at_65cm","y vs x near 65 cm",400,-5,5,400,-5,5);
  TwoTrackXYFit_at_65cm->SetXTitle("x [cm]");
  TwoTrackXYFit_at_65cm->SetYTitle("y [cm]"); 

  TwoTrackXYZFit= new TH3I("TwoTrackXYZFit","z vs y vs x",400,-10,10,
			   400,-10,10,140,30,100);
  TwoTrackXYZFit->SetXTitle("x (cm)");
  TwoTrackXYZFit->SetYTitle("y (cm)");
  TwoTrackXYZFit->SetZTitle("z (cm)");
  
  TwoTrackDz=new TH1F("TwoTrackDz","#deltaz at x,y vertex",100,-10,10);

  TwoTrackDoca=new TH1F("TwoTrackDoca","#deltar",100,0,10);

  gDirectory->cd("../");

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_imaging::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  geom = dapp->GetDGeometry(runnumber);
  bfield=dapp->GetBfield(runnumber);
  
  //Pre-allocate memory for DReferenceTrajectory objects early
  //The swim-step objects of these arrays take up a significant amount of memory, and it can be difficult to find enough free contiguous space for them.
  //Therefore, allocate them at the beginning before the available memory becomes randomly populated
  while(rtv.size() < 50)
    rtv.push_back(new DReferenceTrajectory(bfield)); 

  FIT_VERTEX=true;
  gPARMS->SetDefaultParameter("IMAGING:FIT_VERTEX",FIT_VERTEX, "Turn on/off vertex fitting");
  FIT_CL_CUT=0.01;
  gPARMS->SetDefaultParameter("IMAGING:FIT_CL_CUT",FIT_CL_CUT, "CL cut for vertex fit"); 
  TRACK_CL_CUT=1e-4;
  gPARMS->SetDefaultParameter("IMAGING:TRACK_CL_CUT",TRACK_CL_CUT, "CL cut for tracks");
  DOCA_CUT=3.0; 
  gPARMS->SetDefaultParameter("IMAGING:DOCA_CUT",DOCA_CUT, "Maximum doca between tracks");

  
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_imaging::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // loop->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // loop->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);
  
  vector<const DChargedTrack*>tracks; 
  loop->Get(tracks); 
  if (tracks.size()<2) return NOERROR;

  japp->RootWriteLock();

  // Reset the number of used reference trajectories from the pool
  num_used_rts=0;
 
  // For each track make a helical approximation to the trajectory near the 
  // point of closest approach to the beam line, so that the intersection point
  // of each pair of tracks can be approximated by finding the point of
  // intersection of two circles closest to the measured POCAs.
  for (unsigned int i=0;i<tracks.size()-1;i++){
    const DTrackTimeBased *track1=tracks[i]->Get_BestTrackingFOM()->Get_TrackTimeBased();						  
    if (TMath::Prob(track1->chisq,track1->Ndof)>TRACK_CL_CUT){
      DVector3 pos1=track1->position();
      DVector3 mom1=track1->momentum();
      double phi1=mom1.Phi();
      double B1=fabs(bfield->GetBz(pos1.x(),pos1.y(),pos1.z()));
      double pt1=mom1.Perp();  
      double px1=mom1.x(),py1=mom1.y();
      double q1=track1->charge();
      double a1=-0.003*B1*q1;
      double R1=fabs(pt1/a1);
    
      // Find the center of the circle approximating the trajectory in the 
      // projection to a plane perpendicular to the beam line
      double x1=pos1.x()+q1*R1*sin(phi1);
      double y1=pos1.y()-q1*R1*cos(phi1);

      for (unsigned int j=i+1;j<tracks.size();j++){
	const DTrackTimeBased *track2=tracks[j]->Get_BestTrackingFOM()->Get_TrackTimeBased();

	if (TMath::Prob(track2->chisq,track2->Ndof)>TRACK_CL_CUT){
	  DVector3 mom2=track2->momentum();
	  double phi2=mom2.Phi(); 
	  double q2=track2->charge();
	  DVector3 pos2=track2->position();
	  double B2=fabs(bfield->GetBz(pos2.x(),pos2.y(),pos2.z()));
	  double pt2=mom2.Perp();
	  double px2=mom2.x(),py2=mom2.y();
	  double a2=-0.003*B2*q2;
	  double R2=fabs(pt2/a2);	 
	  // Find the center of the circle approximating the trajectory in 
	  // the projection to a plane perpendicular to the beam line
	  double x2=pos2.x()+q2*R2*sin(phi2);
	  double y2=pos2.y()-q2*R2*cos(phi2);
	  
	  // Find where the two circles intersect
	  double dx=x1-x2;
	  double dy=y1-y2;
	  double R1_sq=R1*R1;
	  double R2_sq=R2*R2;
	  double temp1=dx*dx+dy*dy;
	  double temp2=R2_sq-temp1;
	  double temp3=2.*R1_sq*(R2_sq+temp1)-temp2*temp2-R1_sq*R1_sq;
	  // if temp3 is less than zero, the two circles never intersect
	  if (temp3>0.){
	    double Ax=(R2_sq-R1_sq)*dx+(x1+x2)*temp1;
	    double Bx=dy*sqrt(temp3);
	    double x1_vert=(Ax+Bx)/(2.*temp1);
	    double x2_vert=(Ax-Bx)/(2.*temp1);
	    double Ay=(R2_sq-R1_sq)*dy+(y1+y2)*temp1;
	    double By=dx*sqrt(temp3);
	    double y2_vert=(Ay+By)/(2.*temp1);
	    double y1_vert=(Ay-By)/(2.*temp1);
	    
	    // There are two possible intersection points:  choose the solution
	    // that is closest to one of the POCAs.	   
	    DVector3 myvertex;
	    double dx_test=pos1.x()-x1_vert;
	    double dy_test=pos1.y()-y1_vert;
	    double diff1=dx_test*dx_test+dy_test*dy_test;
	    dx_test=pos1.x()-x2_vert;
	    dy_test=pos1.y()-y2_vert;
	    double diff2=dx_test*dx_test+dy_test*dy_test;
	    if (diff2<diff1){
	      myvertex.SetX(x2_vert);
	      myvertex.SetY(y2_vert);
	    }
	    else{
	      myvertex.SetX(x1_vert);
	      myvertex.SetY(y1_vert);
	    }
	    
	    // Find the z-positions for each track corresponding to
	    // (x,y)=(myvertex.X(),myvertex.Y())
	    double dx=myvertex.x()-pos1.x();
	    double dy=myvertex.y()-pos1.y();
	    double s1=(pt1/a1)*asin(a1*(dx*px1+dy*py1)/(pt1*pt1));
	    double z1=pos1.z()+s1*tan(M_PI_2-mom1.Theta()); 
	    double dx2=myvertex.x()-pos2.x();
	    double dy2=myvertex.y()-pos2.y();
	    double s2=(pt2/a2)*asin(a2*(dx2*px2+dy2*py2)/(pt2*pt2));
	    double z2=pos2.z()+s2*tan(M_PI_2-mom2.Theta());  
	    
	    // Find the doca, starting with the points where the circles 
	    // intersect in the xy plane
	    double doca=0.;
	    DVector3 pos1_in=myvertex;
	    pos1_in.SetZ(z1);
	    DVector3 mom1_in=mom1;
	    double twoks1=a1*s1/pt1;
	    double sin2ks1=sin(twoks1);
	    double cos2ks1=cos(twoks1);
	    mom1_in.SetX(px1*cos2ks1-py1*sin2ks1);
	    mom1_in.SetY(py1*cos2ks1+px1*sin2ks1);
	    DVector3 pos2_in=myvertex;
	    pos2_in.SetZ(z2);
	    DVector3 mom2_in=mom2;
	    double twoks2=a2*s2/pt2;
	    double sin2ks2=sin(twoks2);
	    double cos2ks2=cos(twoks2);
	    mom2_in.SetX(px2*cos2ks2-py2*sin2ks2);
	    mom2_in.SetY(py2*cos2ks2+px2*sin2ks2);
	    DVector3 pos1_out,pos2_out,mom1_out,mom2_out;
	    double ds1=0,ds2=0.;
	    if (FindDoca(q1,q2,mom1_in,pos1_in,mom2_in,pos2_in,mom1_out,
			 pos1_out,mom2_out,pos2_out,doca,ds1,ds2)==NOERROR){  
	      TwoTrackDoca->Fill(doca);
	      TwoTrackDz->Fill(pos1_out.z()-pos2_out.z());
	      
	      if (doca<1.){
		// Use the positions corresponding to the doca of the tracks to 
		// refine the estimate for the vertex position
		myvertex=0.5*(pos1_out+pos2_out);
		s1+=ds1;
		s2+=ds2;
		
		TwoTrackPocaCut->Fill(myvertex.z(),myvertex.Perp());
		TwoTrackXYZ->Fill(myvertex.x(),myvertex.y(),myvertex.z());
		if (myvertex.z()>64.5 && myvertex.z()<65.5){
		  TwoTrackXY_at_65cm->Fill(myvertex.x(),myvertex.y());
		}
		if (myvertex.Perp()<0.5){
		  TwoTrackZ->Fill(myvertex.z());
		}
		
		// Propagate the covariances matrices to myvertex
		DReferenceTrajectory rt(bfield);
		DVector3 B;
		bfield->GetField(myvertex,B);
		TMatrixFSym cov1(7),cov2(7);
		cov1=*(track1->errorMatrix());
		cov2=*(track2->errorMatrix());
		double mass1=track1->mass();
		double mass2=track2->mass();
		rt.PropagateCovariance(s1,q1,mass1*mass1,mom1,pos1,B,cov1);
		rt.PropagateCovariance(s2,q2,mass2*mass2,mom2,pos2,B,cov2);
		
		// Do a fit using the Lagrange multiplier method
		double vertex_chi2=0.;
		rt.FitVertex(pos1_out,mom1_out,pos2_out,mom2_out,cov1,cov2,
			     myvertex,vertex_chi2,q1,q2);
		
		TwoTrackChi2->Fill(vertex_chi2);
		double vertex_prob=TMath::Prob(vertex_chi2,1);
		TwoTrackProb->Fill(vertex_prob);
		if (vertex_prob>FIT_CL_CUT){  
		  TwoTrackPocaCutFit->Fill(myvertex.z(),myvertex.Perp());
		  TwoTrackXYZFit->Fill(myvertex.x(),myvertex.y(),myvertex.z());
		  if (myvertex.z()>64.5 && myvertex.z()<65.5){
		    TwoTrackXYFit_at_65cm->Fill(myvertex.x(),myvertex.y());
		  }
		  if (myvertex.Perp()<0.5){
		  TwoTrackZFit->Fill(myvertex.z());
		  }
		}
	      }// doca cut
	    } // found the doca
	  } // check for valid common vertex
	} // second track
      } // second loop over tracks
    } // first track 
  } // first loop over tracks
  
  japp->RootUnLock();


  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_imaging::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_imaging::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

jerror_t JEventProcessor_imaging::FindDoca(double q1,double q2,
					   const DVector3 &mom1_in, 
					   const DVector3 &pos1_in,
					   const DVector3 &mom2_in, 
					   const DVector3 &pos2_in,
					   DVector3 &mom1_out,
					   DVector3 &pos1_out,
					   DVector3 &mom2_out,
					   DVector3 &pos2_out,
					   double &doca,double &s1, 
					   double &s2) const{
  DVector3 avg_pos=0.5*(pos1_in+pos2_in);
  DVector3 diff_pos=pos1_in-pos2_in;
  double B=fabs(bfield->GetBz(avg_pos.x(),avg_pos.y(),avg_pos.z()));
  double a=-0.003*B*q1;
  double b=-0.003*B*q2;
  double dx=diff_pos.x(),dy=diff_pos.y(),dz=diff_pos.z();
  double px1=mom1_in.x(),py1=mom1_in.y(),pz1=mom1_in.z();
  double tan1=tan(M_PI_2-mom1_in.Theta());
  double phi1=mom1_in.Phi();
  double cos1=cos(phi1),sin1=sin(phi1);
  double px2=mom2_in.x(),py2=mom2_in.y(),pz2=mom2_in.z();
  double tan2=tan(M_PI_2-mom2_in.Theta());
  double phi2=mom2_in.Phi();
  double cos2=cos(phi2),sin2=sin(phi2);
  double dx_sq=dx*dx,dy_sq=dy*dy;
  double cos2sin1_minus_cos1sin2=cos2*sin1 - cos1*sin2;
  double cos1px1_plus_sin1py1_plus_tan1pz1=cos1*px1 + py1*sin1 + pz1*tan1;
  double cos2dy_minus_dxsin2=cos2*dy - dx*sin2;

  double temp1_1=4*cos2sin1_minus_cos1sin2
    *(-(b*(cos1px1_plus_sin1py1_plus_tan1pz1)) 
      + a*(-(b*cos1*dy) + cos1*px2 + b*dx*sin1 + py2*sin1 + pz2*tan1))*
    (cos2*(-(dy*px2*py1) + dy*px1*py2 - dz*px2*pz1 + dz*px1*pz2) 
     + dx*px2*py1*sin2 - dx*px1*py2*sin2 - dz*py2*pz1*sin2 + dz*py1*pz2*sin2 
     + b*(dx*px1 + dy*py1 + dz*pz1)*(cos2dy_minus_dxsin2) + dx*px2*pz1*tan2 
     + dy*py2*pz1*tan2 - dx*px1*pz2*tan2 - dy*py1*pz2*tan2) 
    + pow(-(cos2*px2*py1*sin1) + cos2*px1*py2*sin1 + cos1*px2*py1*sin2 
	  - cos1*px1*py2*sin2 - cos2*px2*pz1*tan1 + cos2*px1*pz2*tan1 
	  - py2*pz1*sin2*tan1 + py1*pz2*sin2*tan1 
	  + b*(cos1*(cos2*dy*px1 - (2*dx*px1 + dy*py1 + dz*pz1)*sin2)
	       - dx*sin2*(py1*sin1 + pz1*tan1) 
	       + cos2*(dx*px1*sin1 + 2*dy*py1*sin1 + dz*pz1*sin1
		       + dy*pz1*tan1)) + cos1*px2*pz1*tan2 - cos1*px1*pz2*tan2
	  + py2*pz1*sin1*tan2 - py1*pz2*sin1*tan2 
	  +  a*(b*(cos1*dy - dx*sin1)*(cos2dy_minus_dxsin2) 
		+ sin1*(-(cos2*dy*py2) - cos2*dz*pz2 + dx*py2*sin2 
			+ dx*pz2*tan2) + cos1*(-(cos2*dy*px2) + dx*px2*sin2 
					       + dz*pz2*sin2 - dy*pz2*tan2)),2);
  if (temp1_1<0){
    //_DBG_ << temp1_1 <<endl;
    return VALUE_OUT_OF_RANGE;
  }
  double temp2_1
    =4*(cos2sin1_minus_cos1sin2)*(-(b*(cos1px1_plus_sin1py1_plus_tan1pz1)) 
				+ a*(-(b*cos1*dy) + cos1*px2 + b*dx*sin1 
				     + py2*sin1 + pz2*tan1))*
    (cos2*(-(dy*px2*py1) + dy*px1*py2 - dz*px2*pz1 + dz*px1*pz2) 
     + dx*px2*py1*sin2 - dx*px1*py2*sin2 - dz*py2*pz1*sin2 + dz*py1*pz2*sin2
     + b*(dx*px1 + dy*py1 + dz*pz1)*(cos2dy_minus_dxsin2) 
     + dx*px2*pz1*tan2 + dy*py2*pz1*tan2 - dx*px1*pz2*tan2 - dy*py1*pz2*tan2) 
    + pow(-(cos2*px2*py1*sin1) + cos2*px1*py2*sin1 + cos1*px2*py1*sin2 
	  - cos1*px1*py2*sin2 - cos2*px2*pz1*tan1 
	  + cos2*px1*pz2*tan1 - py2*pz1*sin2*tan1 + py1*pz2*sin2*tan1 
	  + b*(cos1*(cos2*dy*px1 - (2*dx*px1 + dy*py1 + dz*pz1)*sin2) 
	       - dx*sin2*(py1*sin1 + pz1*tan1)
	       + cos2*(dx*px1*sin1 + 2*dy*py1*sin1 + dz*pz1*sin1 
		       + dy*pz1*tan1)) + cos1*px2*pz1*tan2 - cos1*px1*pz2*tan2
	  + py2*pz1*sin1*tan2 - py1*pz2*sin1*tan2
	  + a*(b*(cos1*dy - dx*sin1)*(cos2dy_minus_dxsin2)
	       + sin1*(-(cos2*dy*py2) - cos2*dz*pz2 + dx*py2*sin2 
		       + dx*pz2*tan2) + cos1*(-(cos2*dy*px2) + dx*px2*sin2 
					      + dz*pz2*sin2 - dy*pz2*tan2)),2);
  if (temp2_1<0){
    //_DBG_ << temp2_1 << endl;
    return VALUE_OUT_OF_RANGE;
  }

  double temp1_2= a*b*cos1*cos2*dy_sq + b*cos1*cos2*dy*px1
    - a*cos1*cos2*dy*px2 - a*b*cos2*dx*dy*sin1 + b*cos2*dx*px1*sin1 
    + 2*b*cos2*dy*py1*sin1 - cos2*px2*py1*sin1 - a*cos2*dy*py2*sin1 
    + cos2*px1*py2*sin1 + b*cos2*dz*pz1*sin1 - a*cos2*dz*pz2*sin1 
    - a*b*cos1*dx*dy*sin2 - 2*b*cos1*dx*px1*sin2 + a*cos1*dx*px2*sin2 
    - b*cos1*dy*py1*sin2 + cos1*px2*py1*sin2 - cos1*px1*py2*sin2 
    - b*cos1*dz*pz1*sin2 + a*cos1*dz*pz2*sin2 + a*b*dx_sq*sin1*sin2 
    - b*dx*py1*sin1*sin2 + a*dx*py2*sin1*sin2 + b*cos2*dy*pz1*tan1 
    - cos2*px2*pz1*tan1 + cos2*px1*pz2*tan1 - b*dx*pz1*sin2*tan1 
    - py2*pz1*sin2*tan1 + py1*pz2*sin2*tan1 + cos1*px2*pz1*tan2 
    - a*cos1*dy*pz2*tan2 - cos1*px1*pz2*tan2 + py2*pz1*sin1*tan2 
    + a*dx*pz2*sin1*tan2 - py1*pz2*sin1*tan2;

  double temp2_2= a*b*cos1*cos2*dy_sq + b*cos1*cos2*dy*px1 
    - a*cos1*cos2*dy*px2 - a*b*cos2*dx*dy*sin1 - b*cos2*dx*px1*sin1 
    + 2*a*cos2*dx*px2*sin1 - cos2*px2*py1*sin1 + a*cos2*dy*py2*sin1 
    + cos2*px1*py2*sin1 - b*cos2*dz*pz1*sin1 + a*cos2*dz*pz2*sin1 
    - a*b*cos1*dx*dy*sin2 - a*cos1*dx*px2*sin2 + b*cos1*dy*py1*sin2 
    + cos1*px2*py1*sin2 - 2*a*cos1*dy*py2*sin2 - cos1*px1*py2*sin2 
    + b*cos1*dz*pz1*sin2 - a*cos1*dz*pz2*sin2 + a*b*dx_sq*sin1*sin2 
    - b*dx*py1*sin1*sin2 + a*dx*py2*sin1*sin2 + b*cos2*dy*pz1*tan1 
    - cos2*px2*pz1*tan1 + cos2*px1*pz2*tan1 - b*dx*pz1*sin2*tan1 
    - py2*pz1*sin2*tan1 + py1*pz2*sin2*tan1 + cos1*px2*pz1*tan2 
    - a*cos1*dy*pz2*tan2 - cos1*px1*pz2*tan2 + py2*pz1*sin1*tan2 
    + a*dx*pz2*sin1*tan2 - py1*pz2*sin1*tan2;

  double denom1
    =2*(cos2sin1_minus_cos1sin2)*(-(b*(cos1px1_plus_sin1py1_plus_tan1pz1))
				+ a*(-(b*cos1*dy) + cos1*px2 + b*dx*sin1 
				     + py2*sin1 + pz2*tan1));
  double denom2=-2*(cos2sin1_minus_cos1sin2)*(b*(cos2*px1 + py1*sin2 + pz1*tan2) + a*(b*cos2*dy - cos2*px2 - b*dx*sin2 - py2*sin2 - pz2*tan2));

  temp2_1=sqrt(temp2_1);
  temp1_1=sqrt(temp1_1);

  // Arc length to POCA between tracks for track 1
  s1=(temp1_2+sqrt(temp1_1))/denom1;
  //s1=(temp1_2-temp1_1)/denom1;
  double pt1=sqrt(px1*px1+py1*py1);
  if (fabs(a*s1/pt1)>0.2){
    //_DBG_ <<a*s1/pt1<< endl;
    return VALUE_OUT_OF_RANGE;
  }

  pos1_out.SetX(pos1_in.x()+cos1*s1);
  pos1_out.SetY(pos1_in.y()+sin1*s1);
  pos1_out.SetZ(pos1_in.z()+tan1*s1); 

  // Arc length to POCA between tracks for track 2
  s2=(temp2_2+sqrt(temp2_1))/denom2;
  //s2=(temp2_2-temp2_1)/denom2;
  double pt2=sqrt(px2*px2+py2*py2);
  if (fabs(b*s2/pt2)>0.2){
    //_DBG_ <<b*s2/pt2 << endl;
    return VALUE_OUT_OF_RANGE;
  }
  
  pos2_out.SetX(pos2_in.x()+cos2*s2);
  pos2_out.SetY(pos2_in.y()+sin2*s2);
  pos2_out.SetZ(pos2_in.z()+tan2*s2);

  mom1_out.SetX(px1-a*sin1*s1);
  mom1_out.SetY(py1+a*cos1*s1);
  mom1_out.SetZ(mom1_in.z());

  mom2_out.SetX(px2-b*sin2*s2);
  mom2_out.SetY(py2+b*cos2*s2);
  mom2_out.SetZ(mom2_in.z());
  
  doca=(pos1_out-pos2_out).Mag();

  return NOERROR;
}
