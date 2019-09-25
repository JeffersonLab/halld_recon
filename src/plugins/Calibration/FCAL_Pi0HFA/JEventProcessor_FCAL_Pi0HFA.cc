// $Id$
//
//    File: JEventProcessor_FCAL_Pi0HFA.cc
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCAL_Pi0HFA.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_FCAL_Pi0HFA());
}
} // "C"


//------------------
// JEventProcessor_FCAL_Pi0HFA (Constructor)
//------------------
JEventProcessor_FCAL_Pi0HFA::JEventProcessor_FCAL_Pi0HFA()
{

}

//------------------
// ~JEventProcessor_FCAL_Pi0HFA (Destructor)
//------------------
JEventProcessor_FCAL_Pi0HFA::~JEventProcessor_FCAL_Pi0HFA()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCAL_Pi0HFA::init(void)
{
	// This is called once at program startup. 

   gDirectory->mkdir("FCAL_Pi0HFA");
   gDirectory->cd("FCAL_Pi0HFA");
   hCurrentGainConstants = new TProfile("CurrentGainConstants", "Current Gain Constants", 2800, -0.5, 2799.5);
   gDirectory->cd("..");

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCAL_Pi0HFA::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes

   // Put the current gain constants into the output file
   vector< double > raw_gains;
   // This is called whenever the run number changes
   eventLoop->GetCalib("/FCAL/gains", raw_gains);
   for (unsigned int i=0; i<raw_gains.size(); i++){
      hCurrentGainConstants->Fill(i,raw_gains[i]);
   }


   return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCAL_Pi0HFA::evnt(JEventLoop *loop, uint64_t eventnumber)
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
    vector< const DNeutralShower* > neutralShowers;
    loop->Get( neutralShowers );
    vector< const DFCALShower* > locFCALShowers;
    loop->Get(locFCALShowers);
    vector< const DFCALCluster* > locFCALClusters;
    loop->Get(locFCALClusters);

    // Cut at most 6 neutral particles
    if (neutralParticleVector.size() > 6 || neutralParticleVector.size() < 2) return NOERROR;
    
    for (unsigned int i=0; i< neutralParticleVector.size() - 1; i++){
        //const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
        // Go into the FCAL shower and find the largest energy deposition
        const DNeutralParticleHypothesis *photon1 = neutralParticleVector[i]->Get_Hypothesis(Gamma);
        
        const DNeutralShower *shower1 =neutralShowers[i];

        //const DNeutralShower *shower1 = NULL;
        //photon1->GetSingle(shower1);
        //if( shower1 == NULL ) continue;

        //shower needs to come from FCAL
        if(shower1->dDetectorSystem != SYS_FCAL) continue;
        
        //DFCALShower *fcalShower1 = (DFCALShower *) shower1->dBCALFCALShower;
        const DFCALShower *fcalShower1=locFCALShowers[i];
        
        //const DFCALCluster* fcalCluster1 = locFCALClusters[i];
        
        /* OLD METHOD
        const DFCALCluster* fcalCluster1 = NULL;
        fcalShower1->GetSingle( fcalCluster1 );
        if( fcalCluster1 == NULL ) continue;
        
        int hits1 = fcalCluster1->getHits();

        int ch1=fcalCluster1->getChannelEmax();

        double cluster1E=fcalCluster1->getEnergy();
        int nHits1=fcalCluster1->getHits();
         
         double xShower1=fcalShower1->getPosition().X();
         double yShower1=fcalShower1->getPosition().Y();
         double radiusShower1=sqrt(pow(xShower1,2)+pow(yShower1,2));
         
         double frac1 = fcalCluster1->getEmax()/fcalCluster1->getEnergy();
         if(fcalCluster1->getEnergy() < 0.8) continue;
        */
         
        for (unsigned int j=i+1; j< neutralParticleVector.size(); j++){
            const DNeutralParticleHypothesis *photon2 = neutralParticleVector[j]->Get_Hypothesis(Gamma);
            
            const DNeutralShower *shower2 =neutralShowers[j];

            //const DNeutralShower *shower2 = NULL;
            //photon2->GetSingle(shower1);
            //if( shower2 == NULL ) continue;
            
            //shower needs to come from FCAL
            if(shower2->dDetectorSystem != SYS_FCAL) continue;
            
            //DFCALShower *fcalShower2 = (DFCALShower *) shower2->dBCALFCALShower;
            const DFCALShower *fcalShower2=locFCALShowers[j];

             /*
            const DFCALCluster* fcalCluster2 = NULL;
            fcalShower2->GetSingle( fcalCluster2 );
            if( fcalCluster2 == NULL ) continue;
            */
            
            vector<const DFCALCluster*> associated_clusters1;
            fcalShower2->Get(associated_clusters1);
            vector<const DFCALCluster*> associated_clusters2;
            fcalShower2->Get(associated_clusters2);
            
            double xShower2=fcalShower2->getPosition().X();
            double yShower2=fcalShower2->getPosition().Y();
            double radiusShower2=sqrt(pow(xShower2,2)+pow(yShower2,2));
            
            double xShower1=fcalShower1->getPosition().X();
            double yShower1=fcalShower1->getPosition().Y();
            double radiusShower1=sqrt(pow(xShower1,2)+pow(yShower1,2));
            
            double pi0Mass = (photon1->lorentzMomentum() + photon2->lorentzMomentum()).M();

            for(unsigned int loc_j = 0; loc_j < associated_clusters1.size(); loc_j++)
            {
                for(unsigned int loc_jj = 0; loc_jj < associated_clusters2.size(); loc_jj++)
                {
                    
                    vector< DFCALCluster::DFCALClusterHit_t > hits1 = associated_clusters1[loc_j]->GetHits();
                    vector< DFCALCluster::DFCALClusterHit_t > hits2 = associated_clusters2[loc_jj]->GetHits();
                    
                    Int_t numhits_per_cluster1 = associated_clusters1[loc_j]->GetNHits();
                    Int_t numhits_per_cluster2 = associated_clusters2[loc_jj]->GetNHits();
		    // next two lines commented out to supress warnings, variables unused
		    //                    double hitEnergyMax1=associated_clusters1[loc_j]->getEmax();
		    //                    double hitEnergyMax2=associated_clusters1[loc_j]->getEmax();

                    double frac1 = associated_clusters1[loc_j]->getEmax()/associated_clusters1[loc_j]->getEnergy();
                    if(associated_clusters1[loc_j]->getEnergy() < 0.8) continue;
                    
                    double frac2 = associated_clusters2[loc_jj]->getEmax()/associated_clusters2[loc_jj]->getEnergy();
                    if(associated_clusters2[loc_jj]->getEnergy() < 0.8) continue;
                    
                    if (numhits_per_cluster1<1) continue;
		    // next line commented out to supress warning, variable unused                    
		    //                    double energry1[numhits_per_cluster1];
                    double fEmax1=0, eMax1=0;
		    //                    int chMax1=0, fChannelEmax1=0;
                    int chMax1=0; // supress warning
                    for(  int i = 0; i < numhits_per_cluster1; ++i ){
		      // next line commented out to supress warning, variable unused                    
		      //                        energry1[i]=hits1[i].E;
                        eMax1=hits1[i].E;
                        chMax1=hits1[i].ch;
                        if (fabs(eMax1-fEmax1) > 0.001) {
                            fEmax1 = eMax1;
			    //supress warning, variable unused
			    //                            fChannelEmax1 = chMax1;
                        }
                        Fill1DHistogram("FCAL_Pi0HFA","","eMax",
                                        fEmax1,
                                        "#pi^{0} Mass; #pi^{0} Mass;",
                                        500, 0.05, 04);
                    }
                    
		    // next line commented out to supress warning, variable unused                    
		    //                    double energry2[numhits_per_cluster2];
                    double fEmax2=0, eMax2=0;
		    //                    int chMax2=0, fChannelEmax2=0;
                    int chMax2=0; // supress warning
                    for(  int i = 0; i < numhits_per_cluster2; ++i ){
		      // next line commented out to supress warning, variable unused                    
		      //                        energry2[i]=hits2[i].E;
                        eMax2=hits2[i].E;
                        chMax2=hits2[i].ch;
                        if (fabs(eMax2-fEmax2) > 0.001) {
                            fEmax2 = eMax2;
			    // supress warning, variable unused
			    //                            fChannelEmax2 = chMax2;
                        }
                        Fill1DHistogram("FCAL_Pi0HFA","","eMax",
                                        fEmax2,
                                        "#pi^{0} Mass; #pi^{0} Mass;",
                                        500, 0.05, 04);
                    }
                    
                    int ch1 = chMax1;
                    int ch2 = chMax2;
                    double avgE = 0.5*associated_clusters1[loc_j]->getEnergy() + 0.5*associated_clusters2[loc_jj]->getEnergy();

                    if(radiusShower1>20.785 || radiusShower2>20.785){
                        Fill1DHistogram("FCAL_Pi0HFA","","Pi0Mass",
                                        pi0Mass,
                                        "#pi^{0} Mass; #pi^{0} Mass;",
                                        500, 0.05, 0.7);
                        
                    }
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
                    for(auto hit : associated_clusters1[loc_j]->GetHits()){
                        Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted",
                                                hit.ch, pi0Mass, hit.E / associated_clusters1[loc_j]->getEnergy(),
                                                "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                2800, -0.5, 2799.5, 200, 0.00, 0.5);
                        Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared",
                                                hit.ch, pi0Mass, (hit.E / associated_clusters1[loc_j]->getEnergy())*(hit.E / associated_clusters1[loc_j]->getEnergy()),
                                                "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                2800, -0.5, 2799.5, 200, 0.00, 0.5);
                        if(radiusShower2<108.4239  && radiusShower2>20.785){
                            Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_Fid",
                                                    hit.ch, pi0Mass, hit.E / associated_clusters1[loc_j]->getEnergy(),
                                                    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                    2800, -0.5, 2799.5, 200, 0.00, 0.5);
                            Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_Fid",
                                                    hit.ch, pi0Mass, (hit.E / associated_clusters1[loc_j]->getEnergy())*(hit.E / associated_clusters1[loc_j]->getEnergy()),
                                                    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                    2800, -0.5, 2799.5, 200, 0.00, 0.5);
                            if(radiusShower1<108.4239 && radiusShower1>20.785){
                                Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_FidBoth",
                                                        hit.ch, pi0Mass, hit.E / associated_clusters1[loc_j]->getEnergy(),
                                                        "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                        2800, -0.5, 2799.5, 200, 0.00, 0.5);
                                Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_FidBoth",
                                                        hit.ch, pi0Mass, (hit.E / associated_clusters1[loc_j]->getEnergy())*(hit.E / associated_clusters1[loc_j]->getEnergy()),
                                                        "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                        2800, -0.5, 2799.5, 200, 0.00, 0.5);
                            }
                        }
                        
                    }
                    
                    for(auto hit : associated_clusters2[loc_jj]->GetHits()){
                        Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted",
                                                hit.ch, pi0Mass, hit.E / associated_clusters2[loc_jj]->getEnergy(),
                                                "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                2800, -0.5, 2799.5, 200, 0.00, 0.5);
                        Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared",
                                                hit.ch, pi0Mass, (hit.E / associated_clusters2[loc_jj]->getEnergy())*(hit.E / associated_clusters2[loc_jj]->getEnergy()),
                                                "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                2800, -0.5, 2799.5, 200, 0.00, 0.5);
                        if(radiusShower1<108.4239 && radiusShower1>20.785){
                            Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_Fid",
                                                    hit.ch, pi0Mass, hit.E / associated_clusters2[loc_jj]->getEnergy(),
                                                    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                    2800, -0.5, 2799.5, 200, 0.00, 0.5);
                            Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_Fid",
                                                    hit.ch, pi0Mass, (hit.E / associated_clusters2[loc_jj]->getEnergy())*(hit.E / associated_clusters2[loc_jj]->getEnergy()),
                                                    "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                    2800, -0.5, 2799.5, 200, 0.00, 0.5);
                            if(radiusShower2<108.4239 && radiusShower2>20.785){
                                Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeighted_FidBoth",
                                                        hit.ch, pi0Mass, hit.E / associated_clusters2[loc_jj]->getEnergy(),
                                                        "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                        2800, -0.5, 2799.5, 200, 0.00, 0.5);
                                Fill2DWeightedHistogram("FCAL_Pi0HFA","","Pi0MassVsChNumWeightedSquared_FidBoth",
                                                        hit.ch, pi0Mass, (hit.E / associated_clusters2[loc_jj]->getEnergy())*(hit.E / associated_clusters2[loc_jj]->getEnergy()),
                                                        "#pi^{0} Mass Vs. Channel Number Weighted; CCDB Index; #pi^{0} Mass",
                                                        2800, -0.5, 2799.5, 200, 0.00, 0.5);
                            }
                        }
                        
                    }
                    
                    if (fabs(associated_clusters1[loc_j]->getEnergy() - associated_clusters2[loc_jj]->getEnergy()) < 0.25){
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
                    if (fabs(associated_clusters1[loc_j]->getEnergy() - associated_clusters2[loc_jj]->getEnergy()) < 0.5){
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
                    if (fabs(associated_clusters1[loc_j]->getEnergy() - associated_clusters2[loc_jj]->getEnergy()) < 0.1){
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
                    if (fabs(associated_clusters1[loc_j]->getEnergy() - associated_clusters2[loc_jj]->getEnergy()) < 0.05){
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
    }
    
    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCAL_Pi0HFA::erun(void)
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCAL_Pi0HFA::fini(void)
{
    // Called before program exit after event processing is finished.
    return NOERROR;
}

