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
  DOCA_CUT=1.0; 
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
      double R1=mom1.Perp()/(0.003*fabs(bfield->GetBz(pos1.x(),pos1.y(),pos1.z())));
      double q1=track1->charge();
      // Find the center of the circle approximating the trajectory in the 
      // projection to a plane perpendicular to the beam line
      double x1=pos1.x()+q1*R1*sin(phi1);
      double y1=pos1.y()-q1*R1*cos(phi1);

      for (unsigned int j=i+1;j<tracks.size();j++){
	const DTrackTimeBased *track2=tracks[j]->Get_BestTrackingFOM()->Get_TrackTimeBased();

	if (TMath::Prob(track2->chisq,track2->Ndof)>TRACK_CL_CUT){
	  DVector3 pos2=track2->position();
	  DVector3 mom2=track2->momentum();
	  double phi2=mom2.Phi();
	  double R2=mom2.Perp()/(0.003*fabs(bfield->GetBz(pos2.x(),pos2.y(),pos2.z())));
	  double q2=track2->charge();
	  // Find the center of the circle approximating the trajectory in the 
	  // projection to a plane perpendicular to the beam line
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
	  //printf("temp3 %f\n",temp3);
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
	    //printf("x,y %f %f x,y %f %f \n",x1_vert,y1_vert,x2_vert,y2_vert);
	    
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
	    // (myvertex.X(),myvertex.Y())
	    double chord=(pos1-myvertex).Perp();
	    double s1=2.*R1*asin(0.5*chord/R1);
	    double z1=pos1.z()+s1*tan(M_PI_2-mom1.Theta()); 
	    chord=(pos2-myvertex).Perp();
	    double s2=2.*R2*asin(0.5*chord/R2);
	    double z2=pos2.z()+s2*tan(M_PI_2-mom2.Theta());
	  
	    TwoTrackDz->Fill(z1-z2);
	    if (fabs(z1-z2)<2.){
	      myvertex.SetZ(0.5*(z1+z2));
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
	      rt.FitVertex(pos1,mom1,pos2,mom2,cov1,cov2,myvertex,vertex_chi2);
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
	    }

	  }
	}
      }
    }
  }

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

