
// $Id$
//
//    File: JEventProcessor_FCAL_Pi0TOF.cc
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_Pi0TOF.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_FCAL_Pi0TOF());
  }
} // "C"


//------------------
// JEventProcessor_FCAL_Pi0TOF (Constructor)
//------------------
JEventProcessor_FCAL_Pi0TOF::JEventProcessor_FCAL_Pi0TOF()
{

}

//------------------
// ~JEventProcessor_FCAL_Pi0TOF (Destructor)
//------------------
JEventProcessor_FCAL_Pi0TOF::~JEventProcessor_FCAL_Pi0TOF()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::init(void)
{
  // This is called once at program startup. 

  gDirectory->mkdir("FCAL_Pi0TOF");
  gDirectory->cd("FCAL_Pi0TOF");
  hCurrentGainConstants = new TProfile("CurrentGainConstants", "Current Gain Constants", 2800, -0.5, 2799.5);
  gDirectory->cd("..");


     for(Int_t ii = 0; ii < 2800; ii++)
        {
      char title[30];
      sprintf(title,"imgg%d",ii);
      imgg[ii]= new TH1F(title,title,7000, 0.00, 0.7);
        }


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes

  // Put the current gain constants into the output file
  vector< double > raw_gains;
  // This is called whenever the run number changes
  eventLoop->GetCalib("/FCAL/gains", raw_gains);
  for (unsigned int i=0; i<raw_gains.size(); i++){
    hCurrentGainConstants->Fill(i,raw_gains[i]);
  }
  DGeometry* dgeom = NULL;
  DApplication* dapp = dynamic_cast< DApplication* >(eventLoop->GetJApplication());
  if (dapp) dgeom = dapp->GetDGeometry(runnumber);
  if (dgeom) {
    dgeom->GetTargetZ(m_targetZ);
  } else {
    cerr << "No geometry accessbile to ccal_timing monitoring plugin." << endl;
    return RESOURCE_UNAVAILABLE;
  }	
  jana::JCalibration *jcalib = japp->GetJCalibration(runnumber);
  std::map<string, float> beam_spot;
  jcalib->Get("PHOTON_BEAM/beam_spot", beam_spot);
  m_beamSpotX = beam_spot.at("x");
  m_beamSpotY = beam_spot.at("y");
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::evnt(JEventLoop *loop, uint64_t eventnumber)
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
    
  vector<const DNeutralParticle *> neutralParticleVector;
  loop->Get(neutralParticleVector);
  
  vector<const DTOFPoint*> tof_points;
  loop->Get(tof_points);
  
  vector<const DVertex*> kinfitVertex;
  loop->Get(kinfitVertex);
  
  // Cut at most 6 neutral particles
  if (neutralParticleVector.size() > 12 || neutralParticleVector.size() < 2) return NOERROR;
  
  DVector3 vertex;
  vertex.SetXYZ(m_beamSpotX, m_beamSpotY, m_targetZ);
  
  //Use kinfit when available
  double kinfitVertexX = m_beamSpotX;
  double kinfitVertexY = m_beamSpotY;
  double kinfitVertexZ = m_targetZ;
  vector< const JObject* > locObjectsToSave;
  for (unsigned int i = 0 ; i < kinfitVertex.size(); i++) {
    kinfitVertexX = kinfitVertex[i]->dSpacetimeVertex.X();
    kinfitVertexY = kinfitVertex[i]->dSpacetimeVertex.Y();
    kinfitVertexZ = kinfitVertex[i]->dSpacetimeVertex.Z();
  }
  
  for (unsigned int i=0; i< neutralParticleVector.size() - 1; i++){
    const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
    if(photon1 == nullptr) continue;
    // Go into the FCAL shower and find the largest energy deposition
    const DNeutralShower *shower1 = photon1->Get_NeutralShower();
    if(shower1->dDetectorSystem != SYS_FCAL) continue;
    DFCALShower *fcalShower1 = (DFCALShower *) shower1->dBCALFCALShower;
    const DFCALCluster *fcalCluster1;
    fcalShower1->GetSingle(fcalCluster1);
    int ch1 = fcalCluster1->getChannelEmax();
    double xShower1=fcalShower1->getPosition().X();
    double yShower1=fcalShower1->getPosition().Y();
    double zShower1=fcalShower1->getPosition().Z();
    double x1 = xShower1 - kinfitVertexX;
    double y1 = yShower1 - kinfitVertexY;
    double z1 = zShower1 - kinfitVertexZ;
    double radiusShower1=sqrt(pow(xShower1,2)+pow(yShower1,2));
    
    double frac1 = fcalCluster1->getEmax()/fcalCluster1->getEnergy();
    if(fcalCluster1->getEnergy() < 0.25) continue;
    
    int tof_match1 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x1, y1, z1);
    for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
      
      double xtof = (*tof_p)->pos.X() - kinfitVertexX;
      double ytof = (*tof_p)->pos.Y() - kinfitVertexY;
      double ztof = (*tof_p)->pos.Z() - kinfitVertexZ;
      
      //double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
      //double ttof = (*tof_p)->t - (rtof/TMath::C());
      
      xtof = xtof * (z1 / ztof);
      ytof = ytof * (z1 / ztof);
      
      int hbar  = (*tof_p)->dHorizontalBar;			
      int hstat = (*tof_p)->dHorizontalBarStatus;
      int vbar  = (*tof_p)->dVerticalBar;
      int vstat = (*tof_p)->dVerticalBarStatus;
      
      double dx, dy;
      if( hstat==3 && vstat==3 ) {
	dx = x1 - xtof;
	dy = y1 - ytof;
      } else if( vstat==3 ) {
	dx = x1 - bar2x(vbar)*(z1 / ztof);
	dy = y1 - ytof;
      } else if( hstat==3 ) {
	dx = x1 - xtof;
	dy = y1 - bar2x(hbar)*(z1 / ztof);
      } else {
	dx = x1 - bar2x(vbar)*(z1 / ztof);
	dy = y1 - bar2x(hbar)*(z1 / ztof);
      }
      
      if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
	//if( dt > -1. && dt < 3. ) tof_match = 1;
	tof_match1 = 1;
      }
    } // end DTOFPoint loop
        
    for (unsigned int j=i+1; j< neutralParticleVector.size(); j++){
      const DNeutralParticleHypothesis *photon2 = neutralParticleVector[j]->Get_Hypothesis(Gamma);
      if(photon2 == nullptr) continue;
      const DNeutralShower *shower2 = photon2->Get_NeutralShower();
      if(shower2->dDetectorSystem != SYS_FCAL) continue;
      DFCALShower *fcalShower2 = (DFCALShower *) shower2->dBCALFCALShower;
      const DFCALCluster *fcalCluster2;
      fcalShower2->GetSingle(fcalCluster2);
      int ch2 = fcalCluster2->getChannelEmax();
      double xShower2=fcalShower2->getPosition().X();
      double yShower2=fcalShower2->getPosition().Y();
      double zShower2=fcalShower1->getPosition().Z();
      double x2 = xShower2 - kinfitVertexX;
      double y2 = yShower2 - kinfitVertexY;
      double z2 = zShower2 - kinfitVertexZ;
      double radiusShower2=sqrt(pow(xShower2,2)+pow(yShower2,2));
      double frac2 = fcalCluster2->getEmax()/fcalCluster2->getEnergy();
      if(fcalCluster2->getEnergy() < 0.25) continue;
      
      int tof_match2 = 0;//TOF_Match(kinfitVertexX, kinfitVertexY, kinfitVertexZ, x2, y2, z2);
      for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
	
	double xtof = (*tof_p)->pos.X() - kinfitVertexX;
	double ytof = (*tof_p)->pos.Y() - kinfitVertexY;
	double ztof = (*tof_p)->pos.Z() - kinfitVertexZ;
	
	//double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
	//double ttof = (*tof_p)->t - (rtof/TMath::C());
	
	xtof = xtof * (z2 / ztof);
	ytof = ytof * (z2 / ztof);
	
	int hbar  = (*tof_p)->dHorizontalBar;			
	int hstat = (*tof_p)->dHorizontalBarStatus;
	int vbar  = (*tof_p)->dVerticalBar;
	int vstat = (*tof_p)->dVerticalBarStatus;
	
	double dx, dy;
	if( hstat==3 && vstat==3 ) {
	  dx = x2 - xtof;
	  dy = y2 - ytof;
	} else if( vstat==3 ) {
	  dx = x2 - bar2x(vbar)*(z2 / ztof);
	  dy = y2 - ytof;
	} else if( hstat==3 ) {
	  dx = x2 - xtof;
	  dy = y2 - bar2x(hbar)*(z2 / ztof);
	} else {
	  dx = x2 - bar2x(vbar)*(z2 / ztof);
	  dy = y2 - bar2x(hbar)*(z2 / ztof);
	}
	
	if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
	  //if( dt > -1. && dt < 3. ) tof_match = 1;
	  tof_match2 = 1;
	}
      } // end DTOFPoint loop
      
      double pi0Mass = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).M();
      double avgE = 0.5*fcalCluster1->getEnergy() + 0.5*fcalCluster2->getEnergy();
      
      Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass",
		      pi0Mass,
		      "#pi^{0} Mass; #pi^{0} Mass;",
		      500, 0.05, 0.7);
            
      if(frac1 > 0.5){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			ch1, pi0Mass,
			"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			2800, -0.5, 2799.5, 200, 0.00, 0.5);
                
imgg[ch1]->Fill(pi0Mass);

	if(radiusShower2<108.4239 && radiusShower2>20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_Fid",
			  ch1, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_FidBoth",
			    ch1, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  }
	}
	if(radiusShower1<20.785 || radiusShower2<20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_InnerRing",
			  ch1, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	}
                
      }
      if(frac2 > 0.5){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			ch2, pi0Mass,
			"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			2800, -0.5, 2799.5, 200, 0.00, 0.5);
imgg[ch2]->Fill(pi0Mass);

	if(radiusShower1<108.4239 && radiusShower1>20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_Fid",
			  ch2, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_FidBoth",
			    ch2, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  }
	}
	if(radiusShower1<20.785 || radiusShower2<20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum_InnerRing",
			  ch1, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	}
                
      }
      // Energy Weighted Pi0 Mass
      for(auto hit : fcalCluster1->GetHits()){
	Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted",
				hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				2800, -0.5, 2799.5, 200, 0.00, 0.5);
	Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared",
				hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				2800, -0.5, 2799.5, 200, 0.00, 0.5);
	if(radiusShower2<108.4239  && radiusShower2>20.785){
	  Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_Fid",
				  hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_Fid",
				  hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_FidBoth",
				    hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_FidBoth",
				    hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  }
	}
                
      }
            
      for(auto hit : fcalCluster2->GetHits()){
	Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted",
				hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				2800, -0.5, 2799.5, 200, 0.00, 0.5);
	Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared",
				hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				"#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				2800, -0.5, 2799.5, 200, 0.00, 0.5);
	if(radiusShower1<108.4239 && radiusShower1>20.785){
	  Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_Fid",
				  hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_Fid",
				  hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_FidBoth",
				    hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_FidBoth",
				    hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  }
	}
                
      }
            
      if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.25){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE",
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
	if(radiusShower2<108.4239 && radiusShower2>20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_Fiducial",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_FiducialBoth",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	  }
	}
                
      }
      if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.5){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_500",
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
                
	if(radiusShower2<108.4239 && radiusShower2>20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_500_Fiducial",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_500_FiducialBoth",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	  }
	}
                
      }
      if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.1){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100",
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
                
	if(radiusShower2<108.4239 && radiusShower2>20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100_Fiducial",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100_FiducialBoth",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	  }
	}
      }
      if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.05){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_50",
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
	if(radiusShower2<108.4239 && radiusShower2>20.785){
	  Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_50_Fiducial",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_50_FiducialBoth",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	  }
	}
      }
      
      if (tof_match1 == 0 && tof_match2 == 0) {
	
	Fill1DHistogram("FCAL_Pi0TOF","","Pi0Mass",
			pi0Mass,
			"#pi^{0} Mass; #pi^{0} Mass;",
			500, 0.05, 0.7);
	
	if(frac1 > 0.5){
	  Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum",
			  ch1, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_Fid",
			    ch1, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_FidBoth",
			      ch1, pi0Mass,
			      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			      2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    }
	  }
	  if(radiusShower1<20.785 || radiusShower2<20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_InnerRing",
			    ch1, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  }
	  
	}
	if(frac2 > 0.5){
	  Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum",
			  ch2, pi0Mass,
			  "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_Fid",
			    ch2, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_FidBoth",
			      ch2, pi0Mass,
			      "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			      2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    }
	  }
	  if(radiusShower1<20.785 || radiusShower2<20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsChNum_InnerRing",
			    ch1, pi0Mass,
			    "#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  }
	  
	}
	// Energy Weighted Pi0 Mass
	for(auto hit : fcalCluster1->GetHits()){
	  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted",
				  hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared",
				  hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower2<108.4239  && radiusShower2>20.785){
	    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_Fid",
				    hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_Fid",
				    hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_FidBoth",
				      hit.ch, pi0Mass, hit.E / fcalCluster1->getEnergy(),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 200, 0.00, 0.5);
	      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_FidBoth",
				      hit.ch, pi0Mass, (hit.E / fcalCluster1->getEnergy())*(hit.E / fcalCluster1->getEnergy()),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    }
	  }
	  
	}
	
	for(auto hit : fcalCluster2->GetHits()){
	  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted",
				  hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared",
				  hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				  "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				  2800, -0.5, 2799.5, 200, 0.00, 0.5);
	  if(radiusShower1<108.4239 && radiusShower1>20.785){
	    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_Fid",
				    hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_Fid",
				    hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				    2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    if(radiusShower2<108.4239 && radiusShower2>20.785){
	      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeighted_FidBoth",
				      hit.ch, pi0Mass, hit.E / fcalCluster2->getEnergy(),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 200, 0.00, 0.5);
	      Fill2DWeightedHistogram("FCAL_Pi0TOF","","Pi0MassVsChNumWeightedSquared_FidBoth",
				      hit.ch, pi0Mass, (hit.E / fcalCluster2->getEnergy())*(hit.E / fcalCluster2->getEnergy()),
				      "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
				      2800, -0.5, 2799.5, 200, 0.00, 0.5);
	    }
	  }
	  
	}
            
	if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.25){
	  Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_Fiducial",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_FiducialBoth",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 100, 0.05, 0.25);
	    }
	  }
	  
	}
	if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.5){
	  Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_500",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_500_Fiducial",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_500_FiducialBoth",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 100, 0.05, 0.25);
	    }
	  }
	  
	}
	if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.1){
	  Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_100",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_100_Fiducial",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_100_FiducialBoth",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 100, 0.05, 0.25);
	    }
	  }
	}
	if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.05){
	  Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_50",
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  if(radiusShower2<108.4239 && radiusShower2>20.785){
	    Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_50_Fiducial",
			    avgE, pi0Mass,
			    "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			    100, 0.0, 10.0, 100, 0.05, 0.25);
	    if(radiusShower1<108.4239 && radiusShower1>20.785){
	      Fill2DHistogram("FCAL_Pi0TOF","","Pi0MassVsE_50_FiducialBoth",
			      avgE, pi0Mass,
			      "#pi^{0} Mass Vs. Average Shower Energy; Avg. Cluster Energy; #pi^{0} Mass",
			      100, 0.0, 10.0, 100, 0.05, 0.25);
	    }
	  }
	}
      }   
    }
  }
    
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCAL_Pi0TOF::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}


//--------------------------------------------
// Get TOF position from bar
//--------------------------------------------
double JEventProcessor_FCAL_Pi0TOF::bar2x(int bar) {
  
  int ic = 2*bar - 45; 
  
  double pos;
  if( ic ==  1  || ic == -1 ) pos = 3.0*(double)ic;
  else if( ic ==  3  || ic ==  5 ) pos = 1.5*(double)(ic+2);
  else if( ic == -3  || ic == -5 ) pos = 1.5*(double)(ic-2);
  else if( ic >  5 ) pos = 3.*(ic-2);
  else pos = 3.*(ic+2);
  
  double x = 1.1*pos;
  
  return x;
}
/*
int JEventProcessor_FCAL_Pi0TOF::TOF_Match(double kinVertexX, double kinVertexY, double kinVertexZ, double x, double y, double z) {
  
  //-----   Check for match between TOF and FCAL   -----//
  int tof_match = 0;
  for (vector< const DTOFPoint* >::const_iterator tof_p = tof_points.begin(); tof_p != tof_points.end(); tof_p++ ) {
    
    double xtof = (*tof_p)->pos.X() - kinVertexX;
    double ytof = (*tof_p)->pos.Y() - kinVertexY;
    double ztof = (*tof_p)->pos.Z() - kinVertexZ;
    
    //double rtof = sqrt(xtof*xtof + ytof*ytof + ztof*ztof );
    //double ttof = (*tof_p)->t - (rtof/TMath::C());
    
    xtof = xtof * (z / ztof);
    ytof = ytof * (z / ztof);
    
    int hbar  = (*tof_p)->dHorizontalBar;			
    int hstat = (*tof_p)->dHorizontalBarStatus;
    int vbar  = (*tof_p)->dVerticalBar;
    int vstat = (*tof_p)->dVerticalBarStatus;
    
    double dx, dy;
    if( hstat==3 && vstat==3 ) {
      dx = x - xtof;
      dy = y - ytof;
    } else if( vstat==3 ) {
      dx = x - bar2x(vbar)*(z / ztof);
      dy = y - ytof;
    } else if( hstat==3 ) {
      dx = x - xtof;
      dy = y - bar2x(hbar)*(z / ztof);
    } else {
      dx = x - bar2x(vbar)*(z / ztof);
      dy = y - bar2x(hbar)*(z / ztof);
    }
    
    if( fabs(dx) < 6. && fabs(dy) < 6. ) { 
      //if( dt > -1. && dt < 3. ) tof_match = 1;
      tof_match = 1;
    }
  } // end DTOFPoint loop
  
  return tof_match;
}
*/



