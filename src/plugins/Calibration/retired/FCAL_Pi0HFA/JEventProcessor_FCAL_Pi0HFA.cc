// $Id$
//
//    File: JEventProcessor_FCAL_Pi0HFA.cc
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_Pi0HFA.h"

#include <DANA/DEvent.h>

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_FCAL_Pi0HFA());
  }
} // "C"


//------------------
// JEventProcessor_FCAL_Pi0HFA (Constructor)
//------------------
JEventProcessor_FCAL_Pi0HFA::JEventProcessor_FCAL_Pi0HFA()
{
	SetTypeName("JEventProcessor_FCAL_Pi0HFA");
}

//------------------
// ~JEventProcessor_FCAL_Pi0HFA (Destructor)
//------------------
JEventProcessor_FCAL_Pi0HFA::~JEventProcessor_FCAL_Pi0HFA()
{

}

//------------------
// Init
//------------------
void JEventProcessor_FCAL_Pi0HFA::Init()
{
  // This is called once at program startup. 

  gDirectory->mkdir("FCAL_Pi0HFA");
  gDirectory->cd("FCAL_Pi0HFA");
  hCurrentGainConstants = new TProfile("CurrentGainConstants", "Current Gain Constants", 2800, -0.5, 2799.5);
  gDirectory->cd("..");
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL_Pi0HFA::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes

  // Put the current gain constants into the output file
  vector< double > raw_gains;
  // This is called whenever the run number changes
  GetCalib(event, "/FCAL/gains", raw_gains);
  for (unsigned int i=0; i<raw_gains.size(); i++){
    hCurrentGainConstants->Fill(i,raw_gains[i]);
  }
}

//------------------
// Process
//------------------
void JEventProcessor_FCAL_Pi0HFA::Process(const std::shared_ptr<const JEvent>& event)
{
  // This is called for every event. Use of common resources like writing
  // to a file or filling a histogram should be mutex protected. Using
  // event->Get(...) to get reconstructed objects (and thereby activating the
  // reconstruction algorithm) should be done outside of any mutex lock
  // since multiple threads may call this method at the same time.
  // Here's an example:
  //
  // vector<const MyDataClass*> mydataclasses;
  // event->Get(mydataclasses);
  //
  // japp->RootFillLock(this);
  //  ... fill historgrams or trees ...
  // japp->RootFillUnLock(this);

  vector<const DFCALGeometry*> fcalGeomVect;
  event->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1) throw JException("FCAL geometry object not available!");

  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
    
  vector<const DNeutralParticle *> neutralParticleVector;
  event->Get(neutralParticleVector);
    
    
  // Cut at most 6 neutral particles
  if (neutralParticleVector.size() > 6 || neutralParticleVector.size() < 2) return;
    
  for (unsigned int i=0; i< neutralParticleVector.size() - 1; i++){
    const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
    if(photon1 == nullptr) continue;
    // Go into the FCAL shower and find the largest energy deposition
    const DNeutralShower *shower1 = photon1->Get_NeutralShower();
    //photon1->GetSingle(shower1);
    //if(shower1 == nullptr) continue;
    if(shower1->dDetectorSystem != SYS_FCAL) continue;
    DFCALShower *fcalShower1 = (DFCALShower *) shower1->dBCALFCALShower;
    const DFCALCluster *fcalCluster1;
    fcalShower1->GetSingle(fcalCluster1);
    int ch1 = fcalCluster1->getChannelEmax();
    double xShower1=fcalShower1->getPosition().X();
    double yShower1=fcalShower1->getPosition().Y();
    double radiusShower1=sqrt(pow(xShower1,2)+pow(yShower1,2));
    double radius1 = fcalGeom.positionOnFace(ch1).Mod();
    int ring1 = (int)(radius1/(5*k_cm));
    double avgE1 = fcalCluster1->getEnergy();

    double frac1 = fcalCluster1->getEmax()/fcalCluster1->getEnergy();
    if(fcalCluster1->getEnergy() < 0.8) continue;
    for (unsigned int j=i+1; j< neutralParticleVector.size(); j++){
      const DNeutralParticleHypothesis *photon2 = neutralParticleVector[j]->Get_Hypothesis(Gamma);
      if(photon2 == nullptr) continue;
      const DNeutralShower *shower2 = photon2->Get_NeutralShower();
      //const DNeutralShower *shower2;
      //if(shower2 == nullptr) continue;
      //photon2->GetSingle(shower2);
      if(shower2->dDetectorSystem != SYS_FCAL) continue;
      DFCALShower *fcalShower2 = (DFCALShower *) shower2->dBCALFCALShower;
      const DFCALCluster *fcalCluster2;
      fcalShower2->GetSingle(fcalCluster2);
      int ch2 = fcalCluster2->getChannelEmax();
      double xShower2=fcalShower2->getPosition().X();
      double yShower2=fcalShower2->getPosition().Y();
      double radiusShower2=sqrt(pow(xShower2,2)+pow(yShower2,2));
      double frac2 = fcalCluster2->getEmax()/fcalCluster2->getEnergy();
      double radius2 = fcalGeom.positionOnFace(ch2).Mod();
      int ring2 = (int)(radius2/(5*k_cm));
      if(fcalCluster2->getEnergy() < 0.8) continue;
            
      double pi0Mass = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).M();
      double avgE = 0.5*fcalCluster1->getEnergy() + 0.5*fcalCluster2->getEnergy();
      double avgE2 = fcalCluster2->getEnergy();
      
      Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass",
		      pi0Mass,
		      "#pi^{0} Mass; #pi^{0} Mass;",
		      500, 0.05, 0.7);
            
      if(frac1 > 0.5){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsChNum",
			ch1, pi0Mass,
			"#pi^{0} Mass Vs. Channel Number; CCDB Index; #pi^{0} Mass",
			2800, -0.5, 2799.5, 200, 0.00, 0.5);
                
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

      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_ring_%d", ring1),
		      avgE1, pi0Mass,
		      "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
		      100, 0.0, 10.0, 100, 0.05, 0.25);
      
      Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_ring_%d", ring2),
		      avgE2, pi0Mass,
		      "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
		      100, 0.0, 10.0, 100, 0.05, 0.25);
      
      if (neutralParticleVector.size() == 2) {
	Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE2g_ring_%d", ring1),
			avgE1, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
	
	Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE2g_ring_%d", ring2),
			avgE2, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
      }


      if (fabs(fcalCluster1->getEnergy() - fcalCluster2->getEnergy()) < 0.1){
	Fill2DHistogram("FCAL_Pi0HFA","","Pi0MassVsE_100",
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);

	Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_100_ring_%d", ring1),
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
	
	Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE_100_ring_%d", ring2),
			avgE, pi0Mass,
			"#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			100, 0.0, 10.0, 100, 0.05, 0.25);
	
	if (neutralParticleVector.size() == 2) {
	  Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE2g_100_ring_%d", ring1),
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	  
	  Fill2DHistogram("FCAL_Pi0HFA","",Form("Pi0MassVsE2g_100_ring_%d", ring2),
			  avgE, pi0Mass,
			  "#pi^{0} Mass Vs. Average Shower Energy; Cluster Energy; #pi^{0} Mass",
			  100, 0.0, 10.0, 100, 0.05, 0.25);
	}
	
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
            
    }
  }
}

//------------------
// EndRun
//------------------
void JEventProcessor_FCAL_Pi0HFA::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FCAL_Pi0HFA::Finish()
{
  // Called before program exit after event processing is finished.
}

