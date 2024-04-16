// $Id$
//
//    File: JEventProcessor_CDC_PerStrawReco.cc
// Created: Mon Jul  6 13:00:51 EDT 2015
// Creator: mstaib (on Linux egbert 2.6.32-504.16.2.el6.x86_64 x86_64)
//

#include "JEventProcessor_CDC_PerStrawReco.h"
#include "PID/DChargedTrack.h"
#include "TRACKING/DTrackTimeBased.h"
#include "HistogramTools.h"

using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_CDC_PerStrawReco());
}
} // "C"

static const double binwidth = 0.005;

//------------------
// JEventProcessor_CDC_PerStrawReco (Constructor)
//------------------
JEventProcessor_CDC_PerStrawReco::JEventProcessor_CDC_PerStrawReco()
{

}

//------------------
// ~JEventProcessor_CDC_PerStrawReco (Destructor)
//------------------
JEventProcessor_CDC_PerStrawReco::~JEventProcessor_CDC_PerStrawReco()
{

}

enum {
	kMIDDLE = 0,
	kDOWNSTREAM
};

//------------------
// init
//------------------
jerror_t JEventProcessor_CDC_PerStrawReco::init(void)
{

    EXCLUDERING=0;
    if (gPARMS){
        gPARMS->SetDefaultParameter("CDCCOSMIC:EXCLUDERING", EXCLUDERING, "Ring Excluded from the fit");
        gPARMS->SetDefaultParameter("KALMAN:RING_TO_SKIP", EXCLUDERING);
    }
    if(EXCLUDERING == 0 ){
        jout << "Did not set CDCCOSMIC:EXCLUDERING on the command line -- Using Biased fits" << endl;
    }


    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_CDC_PerStrawReco::brun(JEventLoop *eventLoop, int32_t runnumber)
{

	int numrings = 28;
    char folder[100];
    char name[100];
    char title[256];
// 	char strawname[100];
// 	char strawtitle[256];
// 	char residualname[100];
// 	char residualtitle[256];
// 	char binname[150];
// 	char bintitle[150];

//     unsigned int numstraws[28]={42,42,54,54,66,66,80,80,93,93,106,106,123,123,
//         135,135,146,146,158,158,170,170,182,182,197,197,
//         209,209};

    DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
    JCalibration *jcalib = dapp->GetJCalibration(runnumber);
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

 	// make histograms here, since we need the sag information
    japp->RootWriteLock();
    if(hResiduals.size() == 0) {
    
		vector<TH1F*> empty_h1d;
		vector<TH2F*> empty_h2d;
		vector<vector<TH1F*>> empty_h1d_vec;
		vector<vector<TH2F*>> empty_h2d_vec;
	
		TDirectory *main = gDirectory;
		gDirectory->mkdir("CDCReco_Middle")->cd();
	
		hResiduals.push_back(empty_h1d);
		hResidualsVsMomentum.push_back(empty_h2d);
		hResidualsVsTheta.push_back(empty_h2d);
		hResidualsVsZ.push_back(empty_h2d); 
		hResidualsVsTrackingFOM.push_back(empty_h2d);
	
		hDriftTime.push_back(empty_h1d);
		hDriftDistance.push_back(empty_h1d);
		hPredictedDriftDistance.push_back(empty_h1d);
	
		hResidualsVsDriftTime.push_back(empty_h2d);
		hResidualsVsDriftDistance.push_back(empty_h2d);
		hResidualsVsPredictedDriftDistance.push_back(empty_h2d);
		hPredictedDriftDistanceVsDriftTime.push_back(empty_h2d);
	
		for(int ring=1; ring<=numrings; ring++) {
		
			sprintf(folder, "Ring %.2i", ring);
			gDirectory->mkdir(folder)->cd();
		
			hResiduals.at(kMIDDLE).push_back( new TH1F("Residuals", "Residuals; Residual [cm]; Entries", 200, -0.05, 0.05) );
			hResidualsVsMomentum.at(kMIDDLE).push_back( new TH2F("Residual Vs. Momentum", 
					"Residual Vs. Momentum; Momentum [GeV/c]; Residual [cm]", 50, 0.0, 12.0, 100, -0.05, 0.05) );
			hResidualsVsTheta.at(kMIDDLE).push_back( new TH2F("Residual Vs. Theta", 
					"Residual Vs. Theta; Theta [deg]; Residual [cm]", 60, 0.0, 180.0, 100, -0.05, 0.05) );
			hResidualsVsZ.at(kMIDDLE).push_back( new TH2F("Residual Vs. Z", 
					"Residual Vs. Z; Z (Measured from CDC center) [cm]; Residual [cm]", 100, -75.0, 75.0, 100, -0.05, 0.05) );
			hResidualsVsTrackingFOM.at(kMIDDLE).push_back( new TH2F("Residual Vs. Tracking FOM", 
					"Residual Vs. Tracking FOM; Tracking FOM; Residual [cm]", 100, 0.0, 1.0, 100, -0.05, 0.05) );
		
			hDriftTime.at(kMIDDLE).push_back( new TH1F("Drift Time", "Drift Time; Drift Time [ns]; Entries", 500, -10, 1500) );
			hDriftDistance.at(kMIDDLE).push_back( new TH1F("Drift Distance", "Drift Distance; Drift Distance [cm]; Entries", 250, 0.0, 1.2) );
			hPredictedDriftDistance.at(kMIDDLE).push_back( new TH1F("Predicted Drift Distance", 
					"Predicted Drift Distance; Drift Distance [cm]; Entries", 250, 0.0, 1.2) );
	
			hResidualsVsDriftTime.at(kMIDDLE).push_back( new TH2F("Residual Vs. Drift Time", 
					"Residual Vs. Drift Time; Drift Time [ns]; Residual [cm]", 500, -10, 1500, 100, -0.05, 0.05) );
			hResidualsVsDriftDistance.at(kMIDDLE).push_back( new TH2F("Residual Vs. Drift Distance", 
					"Residual Vs. Drift Distance; Drift Distance [cm]; Residual [cm]", 50, 0.0, 1.0, 100, -0.05, 0.05) );
			hResidualsVsPredictedDriftDistance.at(kMIDDLE).push_back( new TH2F("Residual Vs. Predicted Drift Distance", 
					"Residual Vs. Predicted Drift Distance; Predicted Drift Distance [cm]; Residual [cm]", 50, 0.0, 1.0, 100, -0.05, 0.05) );
			hPredictedDriftDistanceVsDriftTime.at(kMIDDLE).push_back( new TH2F("Predicted Drift Distance Vs. Drift Time", 
					"Predicted Drift Distance Vs. Drift Time; Drift Time [ns]; Predicted Drift Distance [cm]", 500, -10, 1500, 100, 0.0, 1.0) );
					
			gDirectory->cd("..");
		}
	
		
		main->cd();
		gDirectory->mkdir("CDCPerStrawReco_Middle")->cd();
		
		hStrawDriftTimeVsPhiDOCA.push_back( empty_h2d_vec );
		hStrawPredictedDistanceVsPhiDOCA.push_back( empty_h2d_vec );
		hStrawResidual.push_back( empty_h1d_vec );
		hStrawResidualVsZ.push_back( empty_h2d_vec );
		
		hStrawResidualVsDelta.push_back( empty_h2d_vec );
		hStrawPredictedDriftDistanceVsDriftTime.push_back( empty_h2d_vec );
		hStrawPredictedDriftDistanceVsDelta.push_back( empty_h2d_vec );
	
		hPredictedDriftDistanceVsDriftTime_PosDelta.push_back( empty_h2d_vec );
		hResidualVsDriftTime_PosDelta.push_back( empty_h2d_vec );
		hResidual_PosDelta.push_back( empty_h1d_vec );
	
		hPredictedDriftDistanceVsDriftTime_NegDelta.push_back( empty_h2d_vec );
		hResidualVsDriftTime_NegDelta.push_back( empty_h2d_vec );
		hResidual_NegDelta.push_back( empty_h1d_vec );
	
		for(int ring=1; ring<=numrings; ring++) {
		
			sprintf(folder, "Ring %.2i", ring);
			gDirectory->mkdir(folder)->cd();
			
			hStrawDriftTimeVsPhiDOCA.at(kMIDDLE).push_back( empty_h2d );
			hStrawPredictedDistanceVsPhiDOCA.at(kMIDDLE).push_back( empty_h2d );
			hStrawResidual.at(kMIDDLE).push_back( empty_h1d );
			hStrawResidualVsZ.at(kMIDDLE).push_back( empty_h2d );
	
			hStrawResidualVsDelta.at(kMIDDLE).push_back( empty_h2d );
			hStrawPredictedDriftDistanceVsDriftTime.at(kMIDDLE).push_back( empty_h2d );
			hStrawPredictedDriftDistanceVsDelta.at(kMIDDLE).push_back( empty_h2d );
	
			hPredictedDriftDistanceVsDriftTime_PosDelta.at(kMIDDLE).push_back( empty_h2d );
			hResidualVsDriftTime_PosDelta.at(kMIDDLE).push_back( empty_h2d );
			hResidual_PosDelta.at(kMIDDLE).push_back( empty_h1d );
	
			hPredictedDriftDistanceVsDriftTime_NegDelta.at(kMIDDLE).push_back( empty_h2d );
			hResidualVsDriftTime_NegDelta.at(kMIDDLE).push_back( empty_h2d );
			hResidual_NegDelta.at(kMIDDLE).push_back( empty_h1d );
	
			
			for(unsigned int straw=1; straw<=numstraws[ring-1]; straw++) {
				sprintf(name,"Straw %.3i Drift time Vs phi_DOCA", straw);
				sprintf(title,"Ring %.2i Straw %.3i Drift time Vs phi_DOCA;#phi_{DOCA};Drift Time [ns]", ring, straw);
				hStrawDriftTimeVsPhiDOCA.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title,  8, -3.14, 3.14,  500, -10, 1500) );
				
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs phi_DOCA", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs phi_DOCA; #phi_{DOCA};Predicted Distance [cm]", ring, straw);
				hStrawPredictedDistanceVsPhiDOCA.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title,  16, -3.14, 3.14,  400, 0.0, 1.2) );
				sprintf(name,"Straw %.3i Residual", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual;Residual [cm]", ring, straw);
				hStrawResidual.at(kMIDDLE).at(ring-1).push_back( new TH1F(name, title, 200, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Residual Vs. Z", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual;Z [cm]; Residual [cm]", ring, straw);
				hStrawResidualVsZ.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title,  30, -75.0,75.0,200, -0.05, 0.05) );

				sprintf(name,"Straw %.3i residual Vs delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual Vs #delta; #delta [cm]; Residual [cm]",ring,  straw);
				hStrawResidualVsDelta.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title,  Int_t(2 * max_sag[ring - 1][straw - 1] / binwidth), -1 * max_sag[ring - 1][straw - 1], max_sag[ring - 1][straw - 1], 100, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. Drift Time", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. Drift Time", ring, straw);
				hStrawPredictedDriftDistanceVsDriftTime.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title, 250, -50, 200, 250, 0.0, 0.4) );
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. #delta;#delta [cm]; Predicted Drift Distance - Nominal Radius [cm]", ring, straw);
				hStrawPredictedDriftDistanceVsDelta.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title, 20, -0.25, 0.25, 250, -0.25, 0.25) );
	
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. Drift Time Positive Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. Drift Time (Positive Delta)", ring, straw);
				hPredictedDriftDistanceVsDriftTime_PosDelta.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title, 250, -10, 1500, 50, 0.0, 1.2) );
				sprintf(name,"Straw %.3i Residual Vs. Drift Time Positive Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual Vs. Drift Time (Positive Delta)", ring, straw);
				hResidualVsDriftTime_PosDelta.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title, 100, -10, 1500, 100, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Residual Positive Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual (Positive Delta); Residual [cm]; Entries", ring, straw);
				hResidual_PosDelta.at(kMIDDLE).at(ring-1).push_back( new TH1F(name, title, 200, -0.05, 0.05) );
	
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. Drift Time Negative Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. Drift Time (Negative Delta)", ring, straw);
				hPredictedDriftDistanceVsDriftTime_NegDelta.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title, 250, -10, 1500, 50, 0.0, 1.2) );
				sprintf(name,"Straw %.3i Residual Vs. Drift Time Negative Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual Vs. Drift Time (Negative Delta)", ring, straw);
				hResidualVsDriftTime_NegDelta.at(kMIDDLE).at(ring-1).push_back( new TH2F(name, title, 100, -10, 1500, 100, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Residual Negative Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual (Negative Delta); Residual [cm]; Entries", ring, straw);
				hResidual_NegDelta.at(kMIDDLE).at(ring-1).push_back( new TH1F(name, title, 200, -0.05, 0.05) );
				
			}
			
			
			gDirectory->cd("..");
		}
		
	
		main->cd();
		gDirectory->mkdir("CDCReco_Downstream")->cd();
	
		hResiduals.push_back(empty_h1d);
		hResidualsVsMomentum.push_back(empty_h2d);
		hResidualsVsTheta.push_back(empty_h2d);
		hResidualsVsZ.push_back(empty_h2d); 
		hResidualsVsTrackingFOM.push_back(empty_h2d);
	
		hDriftTime.push_back(empty_h1d);
		hDriftDistance.push_back(empty_h1d);
		hPredictedDriftDistance.push_back(empty_h1d);
	
		hResidualsVsDriftTime.push_back(empty_h2d);
		hResidualsVsDriftDistance.push_back(empty_h2d);
		hResidualsVsPredictedDriftDistance.push_back(empty_h2d);
		hPredictedDriftDistanceVsDriftTime.push_back(empty_h2d);
	
		for(int ring=1; ring<=numrings; ring++) {
		
			sprintf(folder, "Ring %.2i", ring);
			gDirectory->mkdir(folder)->cd();
		
			hResiduals.at(kDOWNSTREAM).push_back( new TH1F("Residuals", "Residuals; Residual [cm]; Entries", 200, -0.05, 0.05) );
			hResidualsVsMomentum.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Momentum", 
					"Residual Vs. Momentum; Momentum [GeV/c]; Residual [cm]", 50, 0.0, 12.0, 100, -0.05, 0.05) );
			hResidualsVsTheta.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Momentum", 
					"Residual Vs. Theta; Theta [deg]; Residual [cm]", 60, 0.0, 180.0, 100, -0.05, 0.05) );
			hResidualsVsZ.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Momentum", 
					"Residual Vs. Z; Z (Measured from CDC center) [cm]; Residual [cm]", 100, -75.0, 75.0, 100, -0.05, 0.05) );
			hResidualsVsTrackingFOM.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Momentum", 
					"Residual Vs. Tracking FOM; Tracking FOM; Residual [cm]", 100, 0.0, 1.0, 100, -0.05, 0.05) );
		
			hDriftTime.at(kDOWNSTREAM).push_back( new TH1F("Drift Time", "Drift Time; Drift Time [ns]; Entries", 500, -10, 1500) );
			hDriftDistance.at(kDOWNSTREAM).push_back( new TH1F("Drift Distance", "Drift Distance; Drift Distance [cm]; Entries", 250, 0.0, 1.2) );
			hPredictedDriftDistance.at(kDOWNSTREAM).push_back( new TH1F("Predicted Drift Distance", 
					"Predicted Drift Distance; Drift Distance [cm]; Entries", 250, 0.0, 1.2) );
	
			hResidualsVsDriftTime.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Drift Time", 
					"Residual Vs. Drift Time; Drift Time [ns]; Residual [cm]", 500, -10, 1500, 100, -0.05, 0.05) );
			hResidualsVsDriftDistance.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Drift Distance", 
					"Residual Vs. Drift Distance; Drift Distance [cm]; Residual [cm]", 50, 0.0, 1.0, 100, -0.05, 0.05) );
			hResidualsVsPredictedDriftDistance.at(kDOWNSTREAM).push_back( new TH2F("Residual Vs. Predicted Drift Distance", 
					"Residual Vs. Predicted Drift Distance; Predicted Drift Distance [cm]; Residual [cm]", 50, 0.0, 1.0, 100, -0.05, 0.05) );
			hPredictedDriftDistanceVsDriftTime.at(kDOWNSTREAM).push_back( new TH2F("Predicted Drift Distance Vs. Drift Time", 
					"Predicted Drift Distance Vs. Drift Time; Drift Time [ns]; Predicted Drift Distance [cm]", 500, -10, 1500, 100, 0.0, 1.0) );
					
			gDirectory->cd("..");
		}
	
		
		main->cd();
		gDirectory->mkdir("CDCPerStrawReco_Downstream")->cd();
		
		hStrawDriftTimeVsPhiDOCA.push_back( empty_h2d_vec );
		hStrawPredictedDistanceVsPhiDOCA.push_back( empty_h2d_vec );
		hStrawResidual.push_back( empty_h1d_vec );
		hStrawResidualVsZ.push_back( empty_h2d_vec );
		
		hStrawResidualVsDelta.push_back( empty_h2d_vec );
		hStrawPredictedDriftDistanceVsDriftTime.push_back( empty_h2d_vec );
		hStrawPredictedDriftDistanceVsDelta.push_back( empty_h2d_vec );
	
		hPredictedDriftDistanceVsDriftTime_PosDelta.push_back( empty_h2d_vec );
		hResidualVsDriftTime_PosDelta.push_back( empty_h2d_vec );
		hResidual_PosDelta.push_back( empty_h1d_vec );
	
		hPredictedDriftDistanceVsDriftTime_NegDelta.push_back( empty_h2d_vec );
		hResidualVsDriftTime_NegDelta.push_back( empty_h2d_vec );
		hResidual_NegDelta.push_back( empty_h1d_vec );
	
		for(int ring=1; ring<=numrings; ring++) {
		
			sprintf(folder, "Ring %.2i", ring);
			gDirectory->mkdir(folder)->cd();
			
			hStrawDriftTimeVsPhiDOCA.at(kDOWNSTREAM).push_back( empty_h2d );
			hStrawPredictedDistanceVsPhiDOCA.at(kDOWNSTREAM).push_back( empty_h2d );
			hStrawResidual.at(kDOWNSTREAM).push_back( empty_h1d );
			hStrawResidualVsZ.at(kDOWNSTREAM).push_back( empty_h2d );
			
			hStrawResidualVsDelta.at(kDOWNSTREAM).push_back( empty_h2d );
			hStrawPredictedDriftDistanceVsDriftTime.at(kDOWNSTREAM).push_back( empty_h2d );
			hStrawPredictedDriftDistanceVsDelta.at(kDOWNSTREAM).push_back( empty_h2d );
	
			hPredictedDriftDistanceVsDriftTime_PosDelta.at(kDOWNSTREAM).push_back( empty_h2d );
			hResidualVsDriftTime_PosDelta.at(kDOWNSTREAM).push_back( empty_h2d );
			hResidual_PosDelta.at(kDOWNSTREAM).push_back( empty_h1d );
	
			hPredictedDriftDistanceVsDriftTime_NegDelta.at(kDOWNSTREAM).push_back( empty_h2d );
			hResidualVsDriftTime_NegDelta.at(kDOWNSTREAM).push_back( empty_h2d );
			hResidual_NegDelta.at(kDOWNSTREAM).push_back( empty_h1d );
		 
		 
			for(unsigned int straw=1; straw<=numstraws[ring-1]; straw++) {
				
				sprintf(name,"Straw %.3i Drift time Vs phi_DOCA", straw);
				sprintf(title,"Ring %.2i Straw %.3i Drift time Vs phi_DOCA;#phi_{DOCA};Drift Time [ns]", ring, straw);
				hStrawDriftTimeVsPhiDOCA.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title,  8, -3.14, 3.14,  500, -10, 1500) );
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs phi_DOCA", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs phi_DOCA; #phi_{DOCA};Predicted Distance [cm]", ring, straw);
				hStrawPredictedDistanceVsPhiDOCA.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title,  16, -3.14, 3.14,  400, 0.0, 1.2) );
				sprintf(name,"Straw %.3i Residual", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual;Residual [cm]", ring, straw);
				hStrawResidual.at(kDOWNSTREAM)[ring-1].push_back( new TH1F(name, title, 200, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Residual Vs. Z", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual;Z [cm]; Residual [cm]", ring, straw);
				hStrawResidualVsZ.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title,  30, -75.0,75.0,200, -0.05, 0.05) );
				
				sprintf(name,"Straw %.3i residual Vs delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual Vs #delta; #delta [cm]; Residual [cm]",ring,  straw);
				hStrawResidualVsDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title,  Int_t(2 * max_sag[ring - 1][straw - 1] / binwidth), -1 * max_sag[ring - 1][straw - 1], max_sag[ring - 1][straw - 1], 100, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. Drift Time", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. Drift Time", ring, straw);
				hStrawPredictedDriftDistanceVsDriftTime.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title, 250, -50, 200, 250, 0.0, 0.4) );
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. #delta;#delta [cm]; Predicted Drift Distance - Nominal Radius [cm]", ring, straw);
				hStrawPredictedDriftDistanceVsDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title, 20, -0.25, 0.25, 250, -0.25, 0.25) );
	
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. Drift Time Positive Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. Drift Time (Positive Delta)", ring, straw);
				hPredictedDriftDistanceVsDriftTime_PosDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title, 250, -10, 1500, 50, 0.0, 1.2) );
				sprintf(name,"Straw %.3i Residual Vs. Drift Time Positive Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual Vs. Drift Time (Positive Delta)", ring, straw);
				hResidualVsDriftTime_PosDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title, 100, -10, 1500, 100, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Residual Positive Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual (Positive Delta); Residual [cm]; Entries", ring, straw);
				hResidual_PosDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH1F(name, title, 200, -0.05, 0.05) );
	
				sprintf(name,"Straw %.3i Predicted Drift Distance Vs. Drift Time Negative Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Predicted Drift Distance Vs. Drift Time (Negative Delta)", ring, straw);
				hPredictedDriftDistanceVsDriftTime_NegDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title, 250, -10, 1500, 50, 0.0, 1.2) );
				sprintf(name,"Straw %.3i Residual Vs. Drift Time Negative Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual Vs. Drift Time (Negative Delta)", ring, straw);
				hResidualVsDriftTime_NegDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH2F(name, title, 100, -10, 1500, 100, -0.05, 0.05) );
				sprintf(name,"Straw %.3i Residual Negative Delta", straw);
				sprintf(title,"Ring %.2i Straw %.3i Residual (Negative Delta); Residual [cm]; Entries", ring, straw);
				hResidual_NegDelta.at(kDOWNSTREAM)[ring-1].push_back( new TH1F(name, title, 200, -0.05, 0.05) );
				
			}
	
			gDirectory->cd("..");
		}
		
		
		main->cd();
		
    }
    japp->RootUnLock();


    return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_CDC_PerStrawReco::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    // Getting the charged tracks will allow us to use the field on data
    vector <const DChargedTrack *> chargedTrackVector;
    loop->Get(chargedTrackVector);

    for (unsigned int iTrack = 0; iTrack < chargedTrackVector.size(); iTrack++){

        const DChargedTrackHypothesis* bestHypothesis = chargedTrackVector[iTrack]->Get_BestTrackingFOM();

        // Require Single track events
        //if (trackCandidateVector.size() != 1) return NOERROR;
        //const DTrackCandidate* thisTrackCandidate = trackCandidateVector[0];
        // Cut very loosely on the track quality
        auto thisTimeBasedTrack = bestHypothesis->Get_TrackTimeBased();
        if (thisTimeBasedTrack->FOM < 1E-20) continue;
        if (!thisTimeBasedTrack->IsSmoothed) continue;
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
            double dz = docaz - 92.0;
            bool isMiddle = false, isDownstream = false;
            if (docaz > 70.0 && docaz < 110.0) isMiddle = true; 
            if (docaz > 140.0) isDownstream = true;
            if (!isMiddle && !isDownstream) continue;
            double predictedDistance = thisPull.d; // This is the DOCA from the track
            double distance = residual + predictedDistance; // This is the distance from the T-D lookup
            const DCDCTrackHit* thisCDCHit = thisPull.cdc_hit;

            if (thisCDCHit == NULL) continue;

            int ring = thisCDCHit->wire->ring;
            int straw = thisCDCHit->wire->straw;
            // Allow for unbiased fits
            if ( EXCLUDERING != 0 && ring != EXCLUDERING) continue;
            // Now we have just the unbiased information for the ring we have chosen
            // Now just make a bunch of histograms to display all of the information
            char folder[100];
            sprintf(folder, "Ring %.2i", ring);

	    int region = 0; // safe, because either isMiddle or isDownstream are true
            if (isMiddle) {
            	region = kMIDDLE;
            }
            else if (isDownstream){
            	region = kDOWNSTREAM;
            }
            
            // fill per-ring histograms
            hResiduals[region][ring-1]->Fill(residual);
            hResidualsVsMomentum[region][ring-1]->Fill(thisTimeBasedTrack->pmag(), residual);
            hResidualsVsTheta[region][ring-1]->Fill(thisTimeBasedTrack->momentum().Theta()*TMath::RadToDeg(), residual);
            hResidualsVsZ[region][ring-1]->Fill(dz, residual);
            hResidualsVsTrackingFOM[region][ring-1]->Fill(thisTimeBasedTrack->FOM, residual);
            
            hDriftTime[region][ring-1]->Fill(time);
            hDriftDistance[region][ring-1]->Fill(distance);
            hPredictedDriftDistance[region][ring-1]->Fill(predictedDistance);
            
			hResidualsVsDriftTime[region][ring-1]->Fill(time, residual);
			hResidualsVsDriftDistance[region][ring-1]->Fill(distance, residual);
			hResidualsVsPredictedDriftDistance[region][ring-1]->Fill(predictedDistance, residual);
			hPredictedDriftDistanceVsDriftTime[region][ring-1]->Fill(time, predictedDistance);


			// fill per-straw histograms
			hStrawDriftTimeVsPhiDOCA[region][ring-1][straw-1]->Fill(docaphi, time);	
			hStrawPredictedDistanceVsPhiDOCA[region][ring-1][straw-1]->Fill(docaphi, predictedDistance);	
			hStrawResidual[region][ring-1][straw-1]->Fill(residual);	
			hStrawResidualVsZ[region][ring-1][straw-1]->Fill(dz,residual);	

            //Time to distance relation in bins
            // Calcuate delta
            double delta = max_sag[ring - 1][straw - 1]*(1.-dz*dz/5625.)
               *cos(docaphi + sag_phi_offset[ring - 1][straw - 1]);
            
            if ( 2 * max_sag[ring - 1][straw - 1] > binwidth){
            	hStrawResidualVsDelta[region][ring-1][straw-1]->Fill(delta, residual);
            }

			hStrawPredictedDriftDistanceVsDriftTime[region][ring-1][straw-1]->Fill(time, predictedDistance);
			hStrawPredictedDriftDistanceVsDelta[region][ring-1][straw-1]->Fill(delta, predictedDistance - 0.78);

            if (delta > 0){ // Long side of straw
				hPredictedDriftDistanceVsDriftTime_PosDelta[region][ring-1][straw-1]->Fill(time, predictedDistance);
				hResidualVsDriftTime_PosDelta[region][ring-1][straw-1]->Fill(time, residual);
				hResidual_PosDelta[region][ring-1][straw-1]->Fill(residual);
            }
            else { // Short side of straw
				hPredictedDriftDistanceVsDriftTime_NegDelta[region][ring-1][straw-1]->Fill(time, predictedDistance);
				hResidualVsDriftTime_NegDelta[region][ring-1][straw-1]->Fill(time, residual);
				hResidual_NegDelta[region][ring-1][straw-1]->Fill(residual);
            }


        } 
    }
    return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_CDC_PerStrawReco::erun(void)
{
   // This is called whenever the run number changes, before it is
   // changed to give you a chance to clean up before processing
   // events from the next run number.
   return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_CDC_PerStrawReco::fini(void)
{
   // Called before program exit after event processing is finished.
   //SortDirectories();
   return NOERROR;
}

