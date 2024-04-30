// $Id$
//
//    File: JEventProcessor_CDC_TimeToDistance.cc
// Created: Mon Nov  9 12:37:01 EST 2015
// Creator: mstaib (on Linux ifarm1102 2.6.32-431.el6.x86_64 x86_64)
//

#include "JEventProcessor_CDC_TimeToDistance.h"


// Routine used to create our JEventProcessor
#include "HistogramTools.h"
#include "PID/DVertex.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"
#include "TRIGGER/DTrigger.h"
#include "HDGEOMETRY/DMagneticFieldMapNoField.h"
#include "DANA/DEvent.h"


extern "C"{
void InitPlugin(JApplication *app){
   InitJANAPlugin(app);
   app->Add(new JEventProcessor_CDC_TimeToDistance());
}
} // "C"


//------------------
// JEventProcessor_CDC_TimeToDistance (Constructor)
//------------------
JEventProcessor_CDC_TimeToDistance::JEventProcessor_CDC_TimeToDistance()
{
	SetTypeName("JEventProcessor_CDC_TimeToDistance");
}

//------------------
// ~JEventProcessor_CDC_TimeToDistance (Destructor)
//------------------
JEventProcessor_CDC_TimeToDistance::~JEventProcessor_CDC_TimeToDistance()
{

}

//------------------
// Init
//------------------
void JEventProcessor_CDC_TimeToDistance::Init()
{
    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

   int prof_td_max = 1000;

   gDirectory->mkdir("CDC_TimeToDistance");
   gDirectory->cd("CDC_TimeToDistance");
   // We need the constants used for this iteration
   // Use a TProfile to avoid problems adding together multiple root files...
   HistCurrentConstants = new TProfile("CDC_TD_Constants", "CDC T/D constants", 125 ,0.5, 125.5);

   dHistZ2tracks0001 = new TH1F("Z_2tracks_0_001", "Vertex z from 2 tracks with fom 0.001+; z (cm)",700,30,100);
   dHistZ2tracks001 = new TH1F("Z_2tracks_0_01", "Vertex z from 2 tracks with fom 0.01+; z (cm)",700,30,100);
   dHistZ2tracks01 = new TH1F("Z_2tracks_0_1", "Vertex z from 2 tracks with fom 0.1+; z (cm)",700,30,100);

   dHistResidualVslogFOM = new TH2F("Residual vs logFOM", "Residual vs log10(FOM); log10(FOM); Residual(cm)",
									 50,-10,0,100, -0.05,0.05);
   dHistResidualVsFOM = new TH2F("Residual vs FOM", "Residual vs FOM; FOM; Residual(cm)",
									 50,0,1,100, -0.05,0.05);
   dHistEarlyDriftTimesPerChannel = new TH2F("Early Drift Times", "Per straw drift times; Drift time [ns];CCDB Index",
             					     200,-50,50,3522,0.5,3522.5);
   dHistResidualVsDriftTime = new TH2F("Residual Vs. Drift Time", "Residual Vs. Drift Time; Drift time [ns];Residual [cm]",
               						 250,0.,1000,100, -0.05,0.05);
   dHistResidualVsDriftTimeFOM09 = new TH2F("Residual Vs. Drift Time, FOM 0.9+", "Residual Vs. Drift Time, FOM 0.9+; Drift time [ns];Residual [cm]", 
   									250,0.,1000,100, -0.05,0.05);
   dHistResidualVsDriftTimeFOM06 = new TH2F("Residual Vs. Drift Time, FOM 0.6+", "Residual Vs. Drift Time, FOM 0.6+; Drift time [ns];Residual [cm]", 
   									250,0.,1000,100, -0.05,0.05);
   dHistResidualVsDriftTimeFOM01 = new TH2F("Residual Vs. Drift Time, FOM 0.1+", "Residual Vs. Drift Time, FOM 0.1+; Drift time [ns];Residual [cm]", 
   									250,0.,1000,100, -0.05,0.05);
   dHistResidualVsDriftTimeFOM001 = new TH2F("Residual Vs. Drift Time, FOM 0.01+", "Residual Vs. Drift Time, FOM 0.01+; Drift time [ns];Residual [cm]", 
   									250,0.,1000,100, -0.05,0.05);
   dHistResidualVsDriftTimeStraightStraws = new TH2F("Residual Vs. Drift Time, max sag < 0.2mm", "Residual Vs. Drift Time (straight straws); Drift time [ns];Residual [cm]",
               						 250,0.,1000,100, -0.05,0.05);

   dHistBz = new TH1F("Bz", "B_{z};B_{z} [T]", 100, 0.0, 2.5);
   
   dHistPredictedDistanceVsDeltaVsDrift = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDriftFOM09 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift, FOM 0.9+",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}, FOM 0.9+; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDriftFOM06 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift, FOM 0.6+",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}, FOM 0.9+; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDriftFOM01 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift, FOM 0.1+",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}, FOM 0.9+; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDriftFOM001 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift, FOM 0.01+",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}, FOM 0.9+; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDrift05 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift < 0.05",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDrift10 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift < 0.10",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDrift15 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift < 0.15",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDrift20 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift < 0.20",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDrift25 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift < 0.25",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
   dHistPredictedDistanceVsDeltaVsDriftBz18 = new TProfile2D("Predicted Drift Distance Vs Delta Vs t_drift, Bz 1.8T",
   										"Predicted Drift Distance Vs. #delta Vs. t_{drift}; t_{drift} [ns]; #delta [cm]",
               							500, 0, prof_td_max, 200, -0.3, 0.3);
	

   gDirectory->cd("..");

   UNBIASED_RING=0;
   MIN_FOM = 1e-8;
   app->SetDefaultParameter("KALMAN:RING_TO_SKIP",UNBIASED_RING);
   app->SetDefaultParameter("CDCCOSMIC:EXCLUDERING", UNBIASED_RING);
   app->SetDefaultParameter("CDC_TTOD:MIN_FOM", MIN_FOM);
}

//------------------
// BeginRun
//------------------
void JEventProcessor_CDC_TimeToDistance::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    auto runnumber = event->GetRunNumber();
   // This is called whenever the run number changes
   dMagneticField = GetBfield(event);
   JCalibration *jcalib = GetJCalibration(event);
   // This is called whenever the run number changes
   // Get the straw sag parameters from the database
   unsigned int numstraws[28]={42,42,54,54,66,66,80,80,93,93,106,106,123,123,
      135,135,146,146,158,158,170,170,182,182,197,197,
      209,209};
   max_sag.clear();
   sag_phi_offset.clear();
   vector< map<string, double> > tvals;
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

   bool dIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField*>(GetBfield(event)) != NULL);

   char ccdbTable[128];
   sprintf(ccdbTable,"CDC/cdc_drift_table%s",dIsNoFieldFlag?"::NoBField":"");

   if (jcalib->Get(ccdbTable, tvals)==false){
      lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!
      for(unsigned int i=0; i<tvals.size(); i++){
         map<string, double> &row = tvals[i];
         HistCurrentConstants->Fill(i+1,1000.*row["t"]);
      }
      lockService->RootUnLock(); //RELEASE ROOT LOCK
   }
   else{
      jerr << " CDC time-to-distance table not available... bailing..." << endl;
      exit(0);
   }

   sprintf(ccdbTable,"CDC/drift_parameters%s",dIsNoFieldFlag?"::NoBField":"");
   if (jcalib->Get(ccdbTable, tvals)==false){
      lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!
      map<string, double> &row = tvals[0]; // long drift side
      HistCurrentConstants->Fill(101,row["a1"]);
      HistCurrentConstants->Fill(102,row["a2"]);
      HistCurrentConstants->Fill(103,row["a3"]);  
      HistCurrentConstants->Fill(104,row["b1"]);
      HistCurrentConstants->Fill(105,row["b2"]);
      HistCurrentConstants->Fill(106,row["b3"]);
      HistCurrentConstants->Fill(107,row["c1"]);
      HistCurrentConstants->Fill(108,row["c2"]);
      HistCurrentConstants->Fill(109,row["c3"]);
      HistCurrentConstants->Fill(110,row["B1"]);
      HistCurrentConstants->Fill(111,row["B2"]);

      row = tvals[1]; // short drift side
      HistCurrentConstants->Fill(112,row["a1"]);
      HistCurrentConstants->Fill(113,row["a2"]);
      HistCurrentConstants->Fill(114,row["a3"]);  
      HistCurrentConstants->Fill(115,row["b1"]);
      HistCurrentConstants->Fill(116,row["b2"]);
      HistCurrentConstants->Fill(117,row["b3"]);
      HistCurrentConstants->Fill(118,row["c1"]);
      HistCurrentConstants->Fill(119,row["c2"]);
      HistCurrentConstants->Fill(120,row["c3"]);
      HistCurrentConstants->Fill(121,row["B1"]);
      HistCurrentConstants->Fill(122,row["B2"]);
      lockService->RootUnLock(); //RELEASE ROOT LOCK
   }

   // Save run number
   HistCurrentConstants->Fill(125,runnumber);
}

//------------------
// Process
//------------------
void JEventProcessor_CDC_TimeToDistance::Process(const std::shared_ptr<const JEvent>& event)
{
   int straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};

   // select events with physics events, i.e., not LED and other front panel triggers
   const DTrigger* locTrigger = NULL; 
   event->GetSingle(locTrigger); 
   if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) return;

   // Getting the charged tracks will allow us to use the field on data
   vector <const DChargedTrack *> chargedTrackVector;
   event->Get(chargedTrackVector);

   
   // 2 track events 

    if ((int)chargedTrackVector.size() == 2) {

        const DVertex* locVertex  = NULL;
        event->GetSingle(locVertex);
        double z = locVertex->dSpacetimeVertex.Z();
        double x = locVertex->dSpacetimeVertex.X();
        double y = locVertex->dSpacetimeVertex.Y();

        double least_fom=1;

        if (sqrt(x*x + y*y) > 1.0) {

            least_fom = 0;  // don't use this event for vertex z plots

        } else {

            for (unsigned int iTrack = 0; iTrack < chargedTrackVector.size(); iTrack++){

                const DChargedTrackHypothesis* bestHypothesis = chargedTrackVector[iTrack]->Get_BestTrackingFOM();

                auto thisTimeBasedTrack = bestHypothesis->Get_TrackTimeBased();

                if (thisTimeBasedTrack->FOM < least_fom) least_fom = thisTimeBasedTrack->FOM;
                if(!thisTimeBasedTrack->IsSmoothed) least_fom = 0; // don't use this event
            }
        }
        
        lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!
        if (least_fom >= 0.001) dHistZ2tracks0001->Fill(z);
        if (least_fom >= 0.01) dHistZ2tracks001->Fill(z);    
        if (least_fom >= 0.1) dHistZ2tracks01->Fill(z);
        lockService->RootUnLock(); //RELEASE ROOT LOCK
        
   }   // end if 2 tracks


   for (unsigned int iTrack = 0; iTrack < chargedTrackVector.size(); iTrack++){

      const DChargedTrackHypothesis* bestHypothesis = chargedTrackVector[iTrack]->Get_BestTrackingFOM();

      // Require Single track events
      //if (trackCandidateVector.size() != 1) return;
      //const DTrackCandidate* thisTrackCandidate = trackCandidateVector[0];
      // Cut very loosely on the track quality
      auto thisTimeBasedTrack = bestHypothesis->Get_TrackTimeBased();
      if(!thisTimeBasedTrack->IsSmoothed) continue;

      if (thisTimeBasedTrack->FOM < MIN_FOM) continue;  // was 1e-10
      vector<DTrackFitter::pull_t> pulls = thisTimeBasedTrack->pulls;
      // Loop over the pulls to get the appropriate information for our ring
      for (unsigned int i = 0; i < pulls.size(); i++){
         DTrackFitter::pull_t thisPull = pulls[i];
         double residual = thisPull.resi;
         //double error = thisPull.err;
         double time = thisPull.tcorr;
         double docaphi = thisPull.docaphi;
         if (docaphi > TMath::Pi()) docaphi -= 2 * TMath::Pi();
         double docaz = thisPull.z;
         //if (docaz < 70.0 || docaz > 110.0) continue; // Only focus on the center of the chamber
         //if (docaz < 140.0) continue; // Only focus on downstream end of chamber
         double predictedDistance = thisPull.d; // This is the DOCA of the track
         //double distance = residual + predictedDistance; // This is the distance from the T-D lookup

         const DCDCTrackHit* thisCDCHit = thisPull.cdc_hit;

         if (thisCDCHit == NULL) continue;
         if (predictedDistance > 1.5 || predictedDistance < 0.0) continue; // Some strange behavior in field on data?
         int ring = thisCDCHit->wire->ring;
         int straw = thisCDCHit->wire->straw;

		 lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!
		 
		 dHistResidualVsFOM->Fill(thisTimeBasedTrack->FOM, thisPull.resi);
		 dHistResidualVslogFOM->Fill(log10(thisTimeBasedTrack->FOM), thisPull.resi);

         // Fill Histogram with the drift times near t0 (+-50 ns)
         dHistEarlyDriftTimesPerChannel->Fill(time,straw_offset[ring]+straw);
         dHistResidualVsDriftTime->Fill(time,residual);

         if (thisTimeBasedTrack->FOM >= 0.9) dHistResidualVsDriftTimeFOM09->Fill(time,residual);
         if (thisTimeBasedTrack->FOM >= 0.6) dHistResidualVsDriftTimeFOM06->Fill(time,residual);
         if (thisTimeBasedTrack->FOM >= 0.1) dHistResidualVsDriftTimeFOM01->Fill(time,residual);
         if (thisTimeBasedTrack->FOM >= 0.01) dHistResidualVsDriftTimeFOM001->Fill(time,residual);
         
         if (max_sag[ring - 1][straw - 1] < 0.02) dHistResidualVsDriftTimeStraightStraws->Fill(time,residual);

         if(UNBIASED_RING != 0 && (ring != UNBIASED_RING) ) continue;

         // Now just make a bunch of histograms to display all of the information
         //Time to distance relation in bins
         // Calculate delta
         double dz = docaz - 92.0;
         // We need the BField to make a cut for the field on data
         DVector3 udir = thisCDCHit->wire->udir;
         DVector3 thisHitLocation = thisCDCHit->wire->origin + udir * (dz / udir.CosTheta());
         double Bz = dMagneticField->GetBz(thisHitLocation.X(), thisHitLocation.Y(), thisHitLocation.Z());
         if ( Bz != 0.0 ) {
         	dHistBz->Fill(Bz);
         }
         double delta = max_sag[ring - 1][straw - 1]*(1.-dz*dz/5625.)
            *cos(docaphi + sag_phi_offset[ring - 1][straw - 1]);


         // We only really need one histogram here
         dHistPredictedDistanceVsDeltaVsDrift->Fill(time, delta, predictedDistance);

		 if (thisTimeBasedTrack->FOM >= 0.9) dHistPredictedDistanceVsDeltaVsDriftFOM09->Fill(time, delta, predictedDistance);
		 if (thisTimeBasedTrack->FOM >= 0.6) dHistPredictedDistanceVsDeltaVsDriftFOM06->Fill(time, delta, predictedDistance);
		 if (thisTimeBasedTrack->FOM >= 0.1) dHistPredictedDistanceVsDeltaVsDriftFOM01->Fill(time, delta, predictedDistance);
		 if (thisTimeBasedTrack->FOM >= 0.01) dHistPredictedDistanceVsDeltaVsDriftFOM001->Fill(time, delta, predictedDistance);


         // To investigate some features, also do this in bins of Max sag
         if (max_sag[ring - 1][straw - 1] < 0.05){
         	dHistPredictedDistanceVsDeltaVsDrift05->Fill(time, delta, predictedDistance);
         } 
         else if (max_sag[ring - 1][straw - 1] < 0.10){
         	dHistPredictedDistanceVsDeltaVsDrift10->Fill(time, delta, predictedDistance);
         }
         else if (max_sag[ring - 1][straw - 1] < 0.15){
         	dHistPredictedDistanceVsDeltaVsDrift15->Fill(time, delta, predictedDistance);
         }
         else if (max_sag[ring - 1][straw - 1] < 0.20){
         	dHistPredictedDistanceVsDeltaVsDrift20->Fill(time, delta, predictedDistance);
         }
         else if (max_sag[ring - 1][straw - 1] < 0.25){
         	dHistPredictedDistanceVsDeltaVsDrift25->Fill(time, delta, predictedDistance);
         }

         // histo for hits in 1.8T region
         if (Bz > 1.75 && Bz < 1.85) dHistPredictedDistanceVsDeltaVsDriftBz18->Fill(time, delta, predictedDistance);
         
		 lockService->RootUnLock(); //RELEASE ROOT LOCK
      }   
   }
}

//------------------
// EndRun
//------------------
void JEventProcessor_CDC_TimeToDistance::EndRun()
{
   // This is called whenever the run number changes, before it is
   // changed to give you a chance to clean up before processing
   // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_CDC_TimeToDistance::Finish()
{
   // Called before program exit after event processing is finished.
}

