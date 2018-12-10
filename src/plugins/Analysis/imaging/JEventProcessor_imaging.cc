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
#include <TRACKING/DMCThrown.h>

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
  MC_RECON_CHECK=false;
  gPARMS->SetDefaultParameter("IMAGING:MC_RECON_CHECK",MC_RECON_CHECK, "Turn on/off comparison to MC"); 
  DEBUG_LEVEL=0;
  gPARMS->SetDefaultParameter("IMAGING:DEBUG_LEVEL",DEBUG_LEVEL);
  FIT_CL_CUT=0.05;
  gPARMS->SetDefaultParameter("IMAGING:FIT_CL_CUT",FIT_CL_CUT, "CL cut for vertex fit"); 
  TRACK_CL_CUT=1e-3;
  gPARMS->SetDefaultParameter("IMAGING:TRACK_CL_CUT",TRACK_CL_CUT, "CL cut for tracks");
  DOCA_CUT=1.0; 
  gPARMS->SetDefaultParameter("IMAGING:DOCA_CUT",DOCA_CUT, "Maximum doca between tracks");


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

  if (MC_RECON_CHECK){
    MCVertexDiff= new TH3I("MCVertexDiff","dz vs dy vs dx",400,-10,10,
			   400,-10,10,400,-10,10); 
    MCVertexDxVsZ= new TH2F("MCVertexDxVsZ","dx vs z",400,0,200,400,-10,10);
    MCVertexDyVsZ= new TH2F("MCVertexDyVsZ","dy vs z",400,0,200,400,-10,10); 
    MCVertexDzVsZ= new TH2F("MCVertexDzVsZ","dz vs z",400,0,200,400,-10,10);
    MCVertexDxVsR= new TH2F("MCVertexDxVsR","dx vs R",120,0,60,400,-10,10);
    MCVertexDyVsR= new TH2F("MCVertexDyVsR","dy vs R",120,0,60,400,-10,10); 
    MCVertexDzVsR= new TH2F("MCVertexDzVsR","dz vs R",120,0,60,400,-10,10);
  }
    
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
  bfield=dapp->GetBfield(runnumber);

  eventLoop->GetSingle(dAnalysisUtilities);
  
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

  vector<const DMCThrown*>mcthrowns;
  if (MC_RECON_CHECK){
    loop->Get(mcthrowns);
  }

  japp->RootWriteLock();

  if (MC_RECON_CHECK && tracks.size()==2){
    // Check estimate of vertex position relative to thrown vertex for simple
    // reactions
    DVector3 truevertex;
    for (unsigned int i=0;i<mcthrowns.size();i++){
      if (mcthrowns[i]->parentid==1){
	truevertex=mcthrowns[i]->position();
	break;
      }
    }
    const DTrackTimeBased *track1=tracks[0]->Get_BestTrackingFOM()->Get_TrackTimeBased();
    const DTrackTimeBased *track2=tracks[1]->Get_BestTrackingFOM()->Get_TrackTimeBased();
    double q1=track1->charge();
    double q2=track2->charge();
    DVector3 mom1_in=track1->momentum();
    DVector3 mom2_in=track2->momentum();
    DVector3 pos1_in=track1->position();
    DVector3 pos2_in=track2->position();
    double doca=0.,ds1=0.,ds2=0.;
    DVector3 pos1_out,pos2_out,mom1_out,mom2_out;
    if (dAnalysisUtilities->Calc_DOCA(q1,q2,mom1_in,pos1_in,mom2_in,pos2_in,
				      mom1_out, pos1_out,mom2_out,pos2_out,
				      doca,ds1,ds2)==NOERROR){  
      DVector3 vertex=0.5*(pos1_out+pos2_out);
      DVector3 diff=vertex-truevertex;
      double ztrue=truevertex.z();
      double rtrue=truevertex.Perp();
      MCVertexDiff->Fill(diff.x(),diff.y(),diff.z());
      MCVertexDxVsZ->Fill(ztrue,diff.x());   
      MCVertexDyVsZ->Fill(ztrue,diff.y());
      MCVertexDzVsZ->Fill(ztrue,diff.z()); 
      MCVertexDxVsR->Fill(rtrue,diff.x());   
      MCVertexDyVsR->Fill(rtrue,diff.y());
      MCVertexDzVsR->Fill(rtrue,diff.z());
      
      
    }
  }

  // For each track make a helical approximation to the trajectory near the 
  // point of closest approach to the beam line, so that the intersection point
  // of each pair of tracks can be approximated by finding the point of
  // intersection of two circles closest to the measured POCAs.
  for (unsigned int i=0;i<tracks.size()-1;i++){
    const DTrackTimeBased *track1=tracks[i]->Get_BestTrackingFOM()->Get_TrackTimeBased();						  
    if (TMath::Prob(track1->chisq,track1->Ndof)>TRACK_CL_CUT){
      DVector3 pos1=track1->position();
      DVector3 mom1=track1->momentum();
      double q1=track1->charge();

      for (unsigned int j=i+1;j<tracks.size();j++){
	const DTrackTimeBased *track2=tracks[j]->Get_BestTrackingFOM()->Get_TrackTimeBased();

	if (TMath::Prob(track2->chisq,track2->Ndof)>TRACK_CL_CUT){
	  DVector3 mom2=track2->momentum();
	  double q2=track2->charge();
	  DVector3 pos2=track2->position();

	  // Find the positions corresponding to the doca between the two
	  // tracks
	  double doca=0.,s1=0,s2=0.;
	  DVector3 pos2_out,pos1_out,mom2_out,mom1_out;
	  if (dAnalysisUtilities->Calc_DOCA(q1,q2,mom1,pos1,mom2,pos2,
					    mom1_out,pos1_out,mom2_out,
					    pos2_out,doca,s1,s2)==NOERROR){  
	    TwoTrackDoca->Fill(doca);
	    TwoTrackDz->Fill(pos1_out.z()-pos2_out.z());
	    
	    if (doca<DOCA_CUT){
	      // Use the positions corresponding to the doca of the tracks to 
	      // refine the estimate for the vertex position
	      DVector3 myvertex=0.5*(pos1_out+pos2_out);
	      
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
	    } // doca cut
	  } // found the doca
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


