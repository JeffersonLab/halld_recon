#include "JEventProcessor_FCAL_TimingOffsets.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"
#include "FCAL/DFCALCluster.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DVertex.h"
#include "DVector3.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include <TTree.h>
#include "DVector3.h"
#include "PID/DParticleID.h"
#include "GlueX.h"
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <TDirectory.h>
#include <TH1I.h>
#include <TH2F.h>
#include <TProfile.h>
#include "ANALYSIS/DTreeInterface.h"
#include "TRIGGER/DTrigger.h"

#include <thread>


// Routine used to create our JEventProcessor
#include "DANA/DEvent.h"

const int nChan = 2800;

// Define Histograms
static TH1I* Offsets[nChan];

extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_FCAL_TimingOffsets());
  }
} // "C"

//------------------
// JEventProcessor_FCAL_TimingOffsets (Constructor)
//------------------
JEventProcessor_FCAL_TimingOffsets::JEventProcessor_FCAL_TimingOffsets()
{
	SetTypeName("JEventProcessor_FCAL_TimingOffsets");
}

//------------------
// ~JEventProcessor_FCAL_TimingOffsets (Destructor)
//------------------
JEventProcessor_FCAL_TimingOffsets::~JEventProcessor_FCAL_TimingOffsets()
{

}

//------------------
// Init
//------------------
void JEventProcessor_FCAL_TimingOffsets::Init()
{
  // This is called once at program startup. If you are creating
  // and filling historgrams in this plugin, you should lock the
  // ROOT mutex like this:
  //
	TDirectory *main = gDirectory;
  gDirectory->mkdir("FCAL_TimingOffsets")->cd();
  
  
  for (int i = 0; i < nChan; ++i) {
    Offsets[i] = new TH1I(Form("Offset_%i",i),Form("Timing Offset for Channel %i",i),800,-50,50);
  }

  main->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL_TimingOffsets::BeginRun(const std::shared_ptr<const JEvent> &event)
{

  // get the FCAL z position from the global geometry interface
  const DGeometry *geom = GetDGeometry(event);
  if( geom ) {

    geom->GetFCALZ( m_FCALfront );
  }
  else{
      
    cerr << "No geometry accessbile." << endl;
    throw JException("No geometry accessbile.");
  }
}

//------------------
// Process
//------------------
void JEventProcessor_FCAL_TimingOffsets::Process(const std::shared_ptr<const JEvent> &event)
{

  // select events with physics events, i.e., not LED and other front panel triggers
  const DTrigger* locTrigger = NULL; 
  event->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) 
    return;


  // we need an FCAL Geometry object
  vector< const DFCALGeometry* > geomVec;
  event->Get( geomVec );

  if( geomVec.size() != 1 ){

    cerr << "No geometry accessbile." << endl;
    throw JException("No geometry accessbile.");
  }

  // next line commented out to suppress warning, variable unused
  //  auto fcalGeom = geomVec[0];


  double FCAL_C_EFFECTIVE = 15.0;
  DGeometry *locGeometry = GetDGeometry(event);
  double locTargetZCenter = 0.0;
  locTargetZCenter = locGeometry->GetTargetZ(locTargetZCenter);
  dTargetCenter.SetXYZ(0.0, 0.0, locTargetZCenter);
  
  vector< const DFCALDigiHit*  > digiHits;
  vector< const DFCALHit*      > hits;
  vector<const DEventRFBunch*> locEventRFBunches;
  vector< const DVertex* > kinfitVertex;
  vector< const DTrackTimeBased* > locTrackTimeBased;
  vector < const DFCALShower * > matchedShowers;
  

  event->Get( hits );
  
  vector< const DFCALShower* > locFCALShowers;
 
   if( hits.size() < 500 ){  // only form clusters and showers if there aren't too many hits
    event->Get(locFCALShowers);    
    event->Get(locEventRFBunches);
    event->Get(locTrackTimeBased);
   }

  double locStartTime = locEventRFBunches.empty() ? 0.0 : locEventRFBunches[0]->dTime;

 DVector3 norm(0.0,0.0,-1);
  DVector3 pos,mom;
  
 
  for (unsigned int i=0; i < locTrackTimeBased.size() ; ++i){ 
    vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_FCAL);
    if (extrapolations.size()>0){
      for (unsigned int j=0; j< locFCALShowers.size(); ++j){
	
	Double_t x = locFCALShowers[j]->getPosition().X();
	Double_t y = locFCALShowers[j]->getPosition().Y();
	pos=extrapolations[0].position;
	Double_t trkmass = locTrackTimeBased[i]->mass();
	Double_t FOM = TMath::Prob(locTrackTimeBased[i]->chisq, locTrackTimeBased[i]->Ndof);
	Double_t dRho = sqrt(((pos.X() - x)*(pos.X() - x)) + ((pos.Y() - y)* (pos.Y() - y)));

	if(trkmass < 0.15 && dRho < 5 && FOM > 0.0001 ) {  
	  matchedShowers.push_back(locFCALShowers[j]);
	}
      }
      
    }
  }


   
  

    for(unsigned int k=0; k<locFCALShowers.size(); k++)
      {
     if (find(matchedShowers.begin(), matchedShowers.end(),locFCALShowers[k]) != matchedShowers.end()) continue;
	
const DFCALShower *s1 = locFCALShowers[k];

     
	double pos_corrected_Z = locFCALShowers[k]->getPosition().Z();

        // Get the clusters from the showers
        vector <const DFCALCluster *> clusterVector;
        s1->Get(clusterVector);
        
        // Loop over clusters within the shower
        for (unsigned int iCluster = 0; iCluster < clusterVector.size(); iCluster++){
            // Get the hits
            const vector<DFCALCluster::DFCALClusterHit_t> hitVector = clusterVector[iCluster]->GetHits();

            //Loop over hits
            for (unsigned int iHit = 0; iHit < 1; iHit++){ 
		double hitEnergy = hitVector[iHit].E;
		if (hitEnergy <= 0.4) continue;                
		double hitTime = hitVector[iHit].t;
		double tCorr = ( m_FCALfront + DFCALGeometry::blockLength() - pos_corrected_Z )/FCAL_C_EFFECTIVE;
                hitTime -= tCorr; // Apply the t corection used for the cluster/shower conversion 
                
		int chanx =  hitVector[iHit].x;
		int chany =  hitVector[iHit].y;
		int ChannelNumber = hitVector[iHit].ch;
		
		dFCALblockcenter.SetXYZ(chanx, chany,  pos_corrected_Z);

		double locPathLength = (dFCALblockcenter - dTargetCenter).Mag();
		double locDeltaT = hitTime - locPathLength/29.9792458 - locStartTime;

		Offsets[ChannelNumber]->Fill(locDeltaT);
  
            }
        }
    }
	




}

//------------------
// EndRun
//------------------
void JEventProcessor_FCAL_TimingOffsets::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_FCAL_TimingOffsets::Finish()
{
  // Called before program exit after event processing is finished.
}


