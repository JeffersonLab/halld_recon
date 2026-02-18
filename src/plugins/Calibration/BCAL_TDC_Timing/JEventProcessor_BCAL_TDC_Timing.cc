// $Id$
//
//    File: JEventProcessor_BCAL_TDC_Timing.cc
// Created: Tue Jul 28 10:55:56 EDT 2015
// Creator: mstaib (on Linux egbert 2.6.32-504.30.3.el6.x86_64 x86_64)
//

/*************************************
  This plugin is designed to calibrate the TDC times for the BCAL.
  These calibrations include the time-walk for each TDC channel, effective velocities and relative offsets between ends of the detector.
  Creation of constants from histograms is controlled by ROOT scripts in the FitScripts directory.
 *************************************/

#include "JEventProcessor_BCAL_TDC_Timing.h"
#include "BCAL/DBCALHit.h"
#include "BCAL/DBCALTDCHit.h"
#include "BCAL/DBCALCluster.h"
#include "BCAL/DBCALDigiHit.h"
#include "BCAL/DBCALPoint.h"
#include "BCAL/DBCALUnifiedHit.h"
#include "BCAL/DBCALGeometry.h"
#include "DANA/DStatusBits.h"
#include "PID/DChargedTrack.h"
#include "PID/DEventRFBunch.h"
#include "PID/DDetectorMatches.h"
#include "PID/DNeutralShower.h"
#include "PID/DVertex.h"
#include "TRACKING/DTrackTimeBased.h"
#include "TRIGGER/DL1Trigger.h"

#include <TDirectory.h>


#include "DANA/DEvent.h"
// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_BCAL_TDC_Timing());
}
} // "C"


//------------------
// JEventProcessor_BCAL_TDC_Timing (Constructor)
//------------------
JEventProcessor_BCAL_TDC_Timing::JEventProcessor_BCAL_TDC_Timing()
{
}

//------------------
// ~JEventProcessor_BCAL_TDC_Timing (Destructor)
//------------------
JEventProcessor_BCAL_TDC_Timing::~JEventProcessor_BCAL_TDC_Timing()
{
}

//------------------
// Init
//------------------
void JEventProcessor_BCAL_TDC_Timing::Init()
{
	// This is called once at program startup on a single thread.
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	VERBOSE = 0;
	VERBOSEHISTOGRAMS = 0;
    DONT_USE_SC = false;

	if(app){
		app->SetDefaultParameter("BCAL_TDC_Timing:VERBOSE", VERBOSE, "Verbosity level");
		app->SetDefaultParameter("BCAL_TDC_Timing:VERBOSEHISTOGRAMS", VERBOSEHISTOGRAMS, "Create more histograms (default 0 for monitoring)");
		app->SetDefaultParameter("BCAL_TDC_Timing:DONT_USE_SC", DONT_USE_SC, "Don't require tracks to match to the start counter (default 0 to require this matching)");
	}
	
	
   double MIN_TDIFF = -10.0, MAX_TDIFF = 10.0, MAX_TDIFF_WIDE = 30.0;
   const int ndtbins = 100;
   // double dtbins[ndtbins+1];
   // for (int i=0; i<=ndtbins; i++) {
   //     dtbins[i] = MIN_TDIFF + (MAX_TDIFF-MIN_TDIFF)/ndtbins*i;
   // }
   const int ndtbinswide = 200;
   double dtbinswide[ndtbinswide+1];
   for (int i=0; i<=ndtbinswide; i++) {
       dtbinswide[i] = MIN_TDIFF + (MAX_TDIFF_WIDE-MIN_TDIFF)/ndtbinswide*i;
   }
   const int npeakbins = 100;
   double peakbins[npeakbins+1] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,20,
                                   21,23,24,26,28,30,32,34,36,39,42,44,48,51,54,58,62,66,71,76,
                                   81,86,92,98,105,112,120,128,137,146,156,167,178,190,203,217,
                                   231,247,264,281,300,321,343,366,390,417,445,475,507,541,578,
                                   617,659,703,750,801,855,913,974,1040,1110,1185,1265,1351,1442,
                                   1539,1643,1754,1872,1998,2133,2277,2430,2594,2769,2956,3155,
                                   3368,3595,3837,4096};

	float zminhall = 0;
	float zmaxhall = 450;
	float zminlocal = -250;
	float zmaxlocal = 250;


    TDirectory *main = gDirectory;
    gDirectory->mkdir("BCAL_TDC_Timing")->cd();
    gDirectory->mkdir("Timewalk_All")->cd();

	hUpstream_Channel_Deltat_All = new TH2I("Upstream_Channel_Deltat",
											"BCAL Upstream t_{TDC}-t_{ADC}; cellID; t_{TDC} - t_{ADC} [ns] ",
                             				576, 0.5, 576.5, ndtbins, MIN_TDIFF, MAX_TDIFF);	
	hDownstream_Channel_Deltat_All = new TH2I("Downstream_Channel_Deltat",
											"BCAL Downstream t_{TDC}-t_{ADC}; cellID; t_{TDC} - t_{ADC} [ns] ",
                             				576, 0.5, 576.5, ndtbins, MIN_TDIFF, MAX_TDIFF);	
	hUpstream_Channel_Deltat_All_Corrected = new TH2I("Upstream_Channel_Deltat_TimewalkCorrected",
											"BCAL Upstream t_{TDC}-t_{ADC} corrected; cellID; t_{TDC} - t_{ADC} [ns] ",
                             				576, 0.5, 576.5, ndtbins, MIN_TDIFF, MAX_TDIFF);	
	hDownstream_Channel_Deltat_All_Corrected = new TH2I("Downstream_Channel_Deltat_TimewalkCorrected",
											"BCAL Downstream t_{TDC}-t_{ADC} corrected; cellID; t_{TDC} - t_{ADC} [ns] ",
                             				576, 0.5, 576.5, ndtbins, MIN_TDIFF, MAX_TDIFF);	

    gDirectory->cd("..");
    gDirectory->mkdir("Upstream_TimewalkVsPeak")->cd();

	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200];
				sprintf(name, "M%02iL%iS%i", module, layer, sector);

				int the_tdc_cell = module * 12 + layer * 4 + sector + 1;
				hUpstream_TimewalkVsPeak[the_tdc_cell] = new TH2I(name,  "Timewalk; Pulse Peak [ADC Counts]; t_{TDC} - t_{ADC} [ns]",
                             npeakbins, peakbins, ndtbinswide, dtbinswide);
			}
		}
	}
	
    gDirectory->cd("..");
    gDirectory->mkdir("Downstream_TimewalkVsPeak")->cd();

	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200];
				sprintf(name, "M%02iL%iS%i", module, layer, sector);

				int the_tdc_cell = module * 12 + layer * 4 + sector + 1;
				hDownstream_TimewalkVsPeak[the_tdc_cell] = new TH2I(name,  "Timewalk; Pulse Peak [ADC Counts]; t_{TDC} - t_{ADC} [ns]",
                             npeakbins, peakbins, ndtbinswide, dtbinswide);
			}
		}
	}
	
    gDirectory->cd("..");
    gDirectory->mkdir("Upstream_TimewalkVsPeak_Corrected")->cd();

	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200];
				sprintf(name, "M%02iL%iS%i", module, layer, sector);

				int the_tdc_cell = module * 12 + layer * 4 + sector + 1;
				hUpstream_TimewalkVsPeak_Corrected[the_tdc_cell] = new TH2I(name,  "Timewalk; Pulse Peak [ADC Counts]; t_{TDC} - t_{ADC} [ns]",
                             npeakbins, peakbins, ndtbinswide, dtbinswide);
			}
		}
	}
	
    gDirectory->cd("..");
    gDirectory->mkdir("Downstream_TimewalkVsPeak_Corrected")->cd();

	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200];
				sprintf(name, "M%02iL%iS%i", module, layer, sector);

				int the_tdc_cell = module * 12 + layer * 4 + sector + 1;
				hDownstream_TimewalkVsPeak_Corrected[the_tdc_cell] = new TH2I(name,  "Timewalk; Pulse Peak [ADC Counts]; t_{TDC} - t_{ADC} [ns]",
                             npeakbins, peakbins, ndtbinswide, dtbinswide);
			}
		}
	}
	

	
	
	main->cd();
	gDirectory->mkdir("BCAL_Global_Offsets")->cd();
    gDirectory->mkdir("Debug")->cd();

	hDebug = new TH1I("Success", "Success profile;Step", 16, -0.5, 15.5);

    gDirectory->cd("..");
    gDirectory->mkdir("Showers")->cd();

	hBCALMatch = new TH2I("BCAL Match",  "BCAL Match;#Delta Z [cm]; #Delta#phi [rad]",
                      200, -25, 25, 200, -0.1, 0.1);
    hPosShowers_Evst = new TH2I("AllShowers_q+",  "Charged shower; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);
    hNegShowers_Evst = new TH2I("AllShowers_q-",  "Charged shower; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);

    hPosPionShowers_Evst = new TH2I("PionShowers_q+",  "Pion showers; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);
    hPosPionShowers_Pvst = new TH2I("PionShowersVsP_q+",  "Pion showers; P [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);
    hPosPionShowers_zvst = new TH2I("PionShowersVsZ_q+",  "Pion showers; Z [cm]; t_{Target} - t_{RF} [ns]", 880, 0.0, 440.0, 200, -10, 10);
    hNegPionShowers_Evst = new TH2I("PionShowers_q-",  "Pion showers; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);
    hNegPionShowers_Pvst = new TH2I("PionShowersVsP_q-",  "Pion showers; P [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);
    hNegPionShowers_zvst = new TH2I("PionShowersVsZ_q-",  "Pion showers; Z [cm]; t_{Target} - t_{RF} [ns]", 880, 0.0, 440.0, 200, -10, 10);

    hNeutShowers_Evst = new TH2I("AllShowers_q0",  "Neutral Showers; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 5.0, 200, -10, 10);
    hNeutShowers_zvst = new TH2I("AllShowers_q0",  "Neutral Showers; Z [cm]; t_{Target} - t_{RF} [ns]", 880, 0.0, 440.0, 200, -10, 10);

    gDirectory->cd("..");
    gDirectory->mkdir("Showers_PID")->cd();

    hPosShowers_PvsdEdx = new TH2I("dEdxVsP_q+",  "CDC dE/dx vs P; P [GeV]; dE/dx [keV/cm]", 200, 0.0, 5.0, 200, 0.0, 5.0);
    hPosShowers_dEdxvst = new TH2I("deltaTVsdEdx_q+",  "PID; dE/dx [keV/cm]; t_{Target} - t_{RF} [ns]", 200, 0.0, 5.0, 200, -10, 10);
    hPosShowers_PvsE = new TH2I("EVsP_q+", "PID; P  [GeV]; E/P", 200, 0.0, 5.0, 200, 0, 5.0);
    hPosShowers_PvsEP = new TH2I("EoverPVsP_q+", "PID; P  [GeV]; E  [GeV]", 200, 0.0, 5.0, 200, 0, 2);
    hNegShowers_PvsdEdx = new TH2I("dEdxVsP_q-",  "CDC dE/dx vs P; P [GeV]; dE/dx [keV/cm]", 200, 0.0, 5.0, 200, 0.0, 5.0);
    hNegShowers_dEdxvst = new TH2I("deltaTVsdEdx_q-",  "PID; dE/dx [keV/cm]; t_{Target} - t_{RF} [ns]", 200, 0.0, 5.0, 200, -10, 10);
    hNegShowers_PvsE = new TH2I("EVsP_q-", "PID; P  [GeV]; E/P", 200, 0.0, 5.0, 200, 0, 5.0);
    hNegShowers_PvsEP = new TH2I("EoverPVsP_q-", "PID; P  [GeV]; E  [GeV]", 200, 0.0, 5.0, 200, 0, 2);

    hMatching_dZvsdPhi = new TH2I("Matching",  "Shower-Track position difference;dZ [cm]; d#phi [degrees]", 200, -60.0, 60.0, 200, -30, 30); 

    gDirectory->cd("..");
    gDirectory->mkdir("Points")->cd();

	hNpointVsEshower_qpos = new TH2I("NpointVsEshower_q+",  "PID; E_{shower} [GeV]; N_{point}", 500, 0.0, 5.0, 50, 0, 50);
	hNpointVsEshower_qneg = new TH2I("NpointVsEshower_q-",  "PID; E_{shower} [GeV]; N_{point}", 500, 0.0, 5.0, 50, 0, 50);
	hEpointVsEshower_qpos = new TH2I("EpointVsEshower_q+",  "PID; E_{shower} [GeV]; E_{point} [GeV]", 1000, 0.0, 5.0, 1000, 0, 2);
	hEpointVsEshower_qneg = new TH2I("EpointVsEshower_q-",  "PID; E_{shower} [GeV]; E_{point} [GeV]", 1000, 0.0, 5.0, 1000, 0, 2);
	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "EpointVsEshower_Layer%i_q+", layer+1);
		hEpointVsEshower_qpos_layer[layer] = new TH2I(layername, "PID; E_{shower} [GeV]; E_{point} [GeV]", 1000, 0.0, 5.0, 1000, 0, 2);  
		sprintf(layername, "EpointVsEshower_Layer%i_q-", layer+1);
		hEpointVsEshower_qneg_layer[layer] = new TH2I(layername, "PID; E_{shower} [GeV]; E_{point} [GeV]", 1000, 0.0, 5.0, 1000, 0, 2);  
	}

	hNpointVsEshower_q0 = new TH2I("NpointVsEshower_q0",  "PID; E_{shower} [GeV]; N_{point}", 500, 0.0, 5.0, 50, 0, 50);


    gDirectory->cd("..");
    gDirectory->mkdir("Z Position")->cd();

	hAllPointsVsShower_zpos = new TH2I("AllPointsVsShower", "Z_{Point} vs Z_{Shower};Z_{Shower}  [cm];Z_{Point} [cm]", 225, zminhall, zmaxhall, 225, zminhall, zmaxhall);

    gDirectory->cd("..");
    gDirectory->mkdir("Deltat")->cd();

    hDeltat_corr_all = new TH1I("AllPoints",  "#Delta t (Hit) corrected for Z;#Delta t - Z_{Track}/v_{eff}", 70, -10, 14);  
    hDeltatvscell_corr = new TH2I("VsCell",  "#Delta t (Hit) corrected for Z;#Delta t - Z_{Track}/v_{eff}", 768, 0.5, 768.5, 70, -10, 14);  
	
    gDirectory->cd("..");
    gDirectory->mkdir("Deltat_raw")->cd();
    
    hDeltat_raw_all = new TH1I("AllPoints", "#Delta t (Hit) corrected for Z;#Delta t_{raw} - Z_{Track}/v_{eff}",  70, -10, 14);
    hDeltat_raw_chan = new TH2I("VsCell", "#Delta t (Hit) corrected for Z;#Delta t_{raw} - Z_{Track}/v_{eff}", 768, 0.5, 768.5, 70, -10, 14);

    gDirectory->cd("..");
    gDirectory->mkdir("Target Time")->cd();

    hCCDB_raw_channel_global_offset = new TH1I("CCDB_raw_channel_global_offset",  "Offsets at time of running;CCDB Index;CCDB timing offset [ns]", 769, 0.5, 769.5); 

    hdeltaTVsCell_all = new TH2I("deltaTVsCell",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10); 
    hdeltaTVsCell_qpos = new TH2I("deltaTVsCell_q+",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_qneg = new TH2I("deltaTVsCell_q-",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_q0 = new TH2I("deltaTVsCell_q-0",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_qpos_Eweight = new TH2I("deltaTVsCell_q+_Eweight",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_qneg_Eweight = new TH2I("deltaTVsCell_q-_Eweight",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_q0_Eweight = new TH2I("deltaTVsCell_q0_Eweight",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_qpos_E2weight = new TH2I("deltaTVsCell_q+_E2weight",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_qneg_E2weight = new TH2I("deltaTVsCell_q-_E2weight",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsCell_q0_E2weight = new TH2I("deltaTVsCell_q0_E2weight",  "Charged shower points; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);  
    hdeltaTVsLayer_qpos = new TH2I("deltaTVsLayer_q+",  "Charged shower points; Layer; t_{Target} - t_{RF} [ns]", 4, 0.5, 4.5, 200, -10, 10);  
    hdeltaTVsLayer_qneg = new TH2I("deltaTVsLayer_q-",  "Charged shower points; Layer; t_{Target} - t_{RF} [ns]", 4, 0.5, 4.5, 200, -10, 10);  

	hHitDeltaTVsChannel = new TH2I("hitDeltaTVsChannel",  "Charged shower hit; CCDB Index; t_{Target} - t_{RF} [ns]", 1536, 0.5, 1536.5, 200, -10, 10);
	hHittimediff = new TH2I("hittimediff",  "Charged shower hit; CCDB Index; t_{Target} - t_{RF} [ns]", 768, 0.5, 768.5, 200, -10, 10);


    gDirectory->cd("..");
    gDirectory->mkdir("Hits_deltaTVsE")->cd();
    
    hHits_deltaTVsE_ADC_qpos = new TH2I("AllHits_ADC_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hHits_deltaTVsE_TDC_qpos = new TH2I("AllHits_TDC_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hHits_deltaTVsE_Mixed_qpos = new TH2I("AllHits_Mixed_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hHits_deltaTVsE_ADC_qneg = new TH2I("AllHits_ADC_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hHits_deltaTVsE_TDC_qneg = new TH2I("AllHits_TDC_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hHits_deltaTVsE_Mixed_qneg = new TH2I("AllHits_Mixed_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  

    gDirectory->cd("..");
    gDirectory->mkdir("Hits_deltaTVsPPmax")->cd();

    hHits_deltaTVsPPmax_ADC_qpos = new TH2I("AllHits_ADC_q+",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmax_TDC_qpos = new TH2I("AllHits_TDC_q+",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmax_Mixed_qpos = new TH2I("AllHits_Mixed_q+",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmax_ADC_qneg = new TH2I("AllHits_ADC_q-",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmax_TDC_qneg = new TH2I("AllHits_TDC_q-",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmax_Mixed_qneg = new TH2I("AllHits_Mixed_q-",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    
    gDirectory->cd("..");
    gDirectory->mkdir("Hits_deltaTVsPPmin")->cd();

    hHits_deltaTVsPPmin_ADC_qpos = new TH2I("AllHits_ADC_q+",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmin_TDC_qpos = new TH2I("AllHits_TDC_q+",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmin_Mixed_qpos = new TH2I("AllHits_Mixed_q+",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmin_ADC_qneg = new TH2I("AllHits_ADC_q-",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmin_TDC_qneg = new TH2I("AllHits_TDC_q-",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    hHits_deltaTVsPPmin_Mixed_qneg = new TH2I("AllHits_Mixed_q-",  "Charged shower points; peak [counts]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 4000, 200, -10, 10);  
    

    gDirectory->cd("..");
    gDirectory->mkdir("Points_deltaTVsEnergy")->cd();

    hPoints_deltaTVsEnergy_qpos = new TH2I("AllPoints_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_qneg = new TH2I("AllPoints_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_q0 = new TH2I("AllPoints_q0",  "Neutral shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_ADC_qpos = new TH2I("AllPoints_ADC_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_ADC_qneg = new TH2I("AllPoints_ADC_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_TDC_qpos = new TH2I("AllPoints_TDC_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_TDC_qneg = new TH2I("AllPoints_TDC_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_Mixed_qpos = new TH2I("AllPoints_Mixed_q+",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsEnergy_Mixed_qneg = new TH2I("AllPoints_Mixed_q-",  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  

	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "Layer%i_q+", layer+1);
		hPoints_deltaTVsEnergy_qpos_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_q-", layer+1);
		hPoints_deltaTVsEnergy_qneg_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_q0", layer+1);
		hPoints_deltaTVsEnergy_q0_layer[layer] = new TH2I(layername,  "Neutral shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  

		sprintf(layername, "Layer%i_ADC_q+", layer+1);
		hPoints_deltaTVsEnergy_ADC_qpos_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_ADC_q-", layer+1);
		hPoints_deltaTVsEnergy_ADC_qneg_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_TDC_q+", layer+1);
		hPoints_deltaTVsEnergy_TDC_qpos_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_TDC_q-", layer+1);
		hPoints_deltaTVsEnergy_TDC_qneg_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_Mixed_q+", layer+1);
		hPoints_deltaTVsEnergy_Mixed_qpos_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_Mixed_q-", layer+1);
		hPoints_deltaTVsEnergy_Mixed_qneg_layer[layer] = new TH2I(layername,  "Charged shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
	}

    gDirectory->cd("..");
    gDirectory->mkdir("Points_altDeltaTVsEnergy")->cd();

    hPoints_altdeltaTVsEnergy_q0 = new TH2I("AllPoints_q0",  "Neutral shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "Layer%i_q0", layer);
		hPoints_altdeltaTVsEnergy_q0_layer[layer] = new TH2I(layername,  "Neutral shower points; E_{point} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
	}

    gDirectory->cd("..");
    gDirectory->mkdir("Points_deltaTVsShowerEnergy")->cd();

    hPoints_deltaTVsShowerEnergy_qpos = new TH2I("AllPoints_q+",  "Charged shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsShowerEnergy_qneg = new TH2I("AllPoints_q-",  "Charged shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
    hPoints_deltaTVsShowerEnergy_q0 = new TH2I("AllPoints_q0",  "Neutral shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  

	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "Layer%i_q+", layer+1);
		hPoints_deltaTVsShowerEnergy_qpos_layer[layer] = new TH2I(layername,  "Charged shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_q-", layer+1);
		hPoints_deltaTVsShowerEnergy_qneg_layer[layer] = new TH2I(layername,  "Charged shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
		sprintf(layername, "Layer%i_q0", layer+1);
		hPoints_deltaTVsShowerEnergy_q0_layer[layer] = new TH2I(layername,  "Neutral shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
	}

    gDirectory->cd("..");
    gDirectory->mkdir("Points_altDeltaTVsShowerEnergy")->cd();

    hPoints_altdeltaTVsShowerEnergy_q0 = new TH2I("AllPoints_q0",  "Neutral shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "Layer%i_q0", layer);
		hPoints_altdeltaTVsShowerEnergy_q0_layer[layer] = new TH2I(layername,  "Neutral shower points; E_{shower} [GeV]; t_{Target} - t_{RF} [ns]", 1000, 0.0, 2.0, 200, -10, 10);  
	}


    gDirectory->cd("../..");
    gDirectory->mkdir("BCAL_atten_gain")->cd();
    gDirectory->mkdir("logintratiovsZtrack")->cd();

    hlogintratiovsZtrack_all = new TH2I("AllPoints", "Attenuation;Z_{Track}  (cm);log of integral ratio US/DS", 250, zminlocal, zmaxlocal, 250, -3, 3);
	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200], title[200];
				sprintf(name, "M%02iL%iS%i", module+1, layer+1, sector+1);
                sprintf(title,"Attenuation (M%i,L%i,S%i);Z_{Track}  (cm);log of integral ratio US/DS", module+1, layer+1, sector+1);

				int the_cell = module * 12 + layer * 4 + sector + 1;
				hlogintratiovsZtrack_chan[the_cell] = new TH2I(name, title, 250, zminlocal, zmaxlocal, 250, -3, 3);
			}
		}
	}

	main->cd();

    gDirectory->mkdir("BCAL_TDC_Offsets")->cd();
    gDirectory->mkdir("ZvsDeltat")->cd();
    
    hZvsDeltat_all = new TH2I("AllPoints",  "Z_{Track} vs #Delta t;#Delta t = t_{US}-t_{DS};Z_{Track} [cm]", 480, -30, 30, 250, zminhall, zmaxhall);  // simulation has 16 values in each Deltat=1
    hZvsDeltat_qpos = new TH2I("All_q+",  "Z_{Track} vs #Delta t;#Delta t = t_{US}-t_{DS};Z_{Track} [cm]", 480, -30, 30, 250, zminhall, zmaxhall);  
    hZvsDeltat_qneg = new TH2I("All_q-",  "Z_{Track} vs #Delta t;#Delta t = t_{US}-t_{DS};Z_{Track} [cm]", 480, -30, 30, 250, zminhall, zmaxhall);  

	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "AllLayer%i", layer+1);
		hZvsDeltat_layer[layer] = new TH2I(layername, "Z_{Track} vs #Delta t;#Delta t = t_{US}-t_{DS};Z_{Track} [cm]", 480, -30, 30, 250, zminhall, zmaxhall);  
	}
	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200], title[400];
				sprintf(name, "M%02iL%iS%i", module+1, layer+1, sector+1);
            	sprintf(title, "%s  Z_{Track} vs #Delta t;#Delta t = t_{US}-t_{DS};Z_{Track} [cm]", name);

				int the_cell = module * 12 + layer * 4 + sector + 1;
				hZvsDeltat_chan[the_cell] = new TH2I(name, title, 480, -30, 30, 250, zminhall, zmaxhall);
			}
		}
	}
	
    gDirectory->cd("..");
    gDirectory->mkdir("DeltatvsTheta")->cd();

	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "AllLayer%i", layer+1);
		hThetavsDeltat_layer[layer] = new TH2I(layername, "#Delta t vs #theta_{Track};#theta_{Track}  (deg);#Delta t = t_{US}-t_{DS}  (ns)",
                             360,0,180, 480, -30, 30);  
	}

    gDirectory->cd("..");
    gDirectory->mkdir("Delta Z")->cd();

    hZvsDeltaz_all = new TH2I("AllPoints",  "#Delta Z vs Z_{Track};Z_{Track} [cm];#Delta Z = Z_{Track} - Z_{Point}", 250, zminhall, zmaxhall, 100, -50, 50); 
    hZvsDeltaz_qpos = new TH2I("All_q+",  "#Delta Z vs Z_{Track};Z_{Track} [cm];#Delta Z = Z_{Track} - Z_{Point}", 250, zminhall, zmaxhall, 100, -50, 50);  
    hZvsDeltaz_qneg = new TH2I("All_q-",  "#Delta Z vs Z_{Track};Z_{Track} [cm];#Delta Z = Z_{Track} - Z_{Point}", 250, zminhall, zmaxhall, 100, -50, 50);  

	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "AllLayer%i", layer+1);
		hZvsDeltaz_layer[layer] = new TH2I(layername, "#Delta Z vs Z_{Track};Z_{Track} [cm];#Delta Z = Z_{Track} - Z_{Point}", 250, zminhall, zmaxhall, 100, -50, 50);  
	}
	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200], title[400];
				sprintf(name, "M%02iL%iS%i", module+1, layer+1, sector+1);
            	sprintf(title, "%s  #Delta Z vs Z_{Track};Z_{Track} [cm];#Delta Z = Z_{Track} - Z_{Point}", name);

				int the_cell = module * 12 + layer * 4 + sector + 1;
				hZvsDeltaz_chan[the_cell] = new TH2I(name, title, 250, zminhall, zmaxhall, 100, -50, 50);
			}
		}
	}

    gDirectory->cd("..");
    gDirectory->mkdir("Z Position")->cd();

    htrackZvsBCALZ_all = new TH2I("AllPoints",  "Z_{point} Vs. Z_{Track}; Z_{Track} [cm]; Z_{Point} [cm]", 500, zminhall, zmaxhall, 500, zminhall, zmaxhall); 
    htrackZvsBCALZ_qpos = new TH2I("All_q+",  "Z_{point} Vs. Z_{Track}; Z_{Track} [cm]; Z_{Point} [cm]", 500, zminhall, zmaxhall, 500, zminhall, zmaxhall);  
    htrackZvsBCALZ_qneg = new TH2I("All_q-",  "Z_{point} Vs. Z_{Track}; Z_{Track} [cm]; Z_{Point} [cm]", 500, zminhall, zmaxhall, 500, zminhall, zmaxhall);  

	for(int layer = 0; layer < NBCALLAYERS; layer++) {
		char layername[255];
		sprintf(layername, "AllLayer%i", layer+1);
		htrackZvsBCALZ_layer[layer] = new TH2I(layername,   "Z_{point} Vs. Z_{Track}; Z_{Track} [cm]; Z_{Point} [cm]", 500, zminhall, zmaxhall, 500, zminhall, zmaxhall);  
	}
	for(int module = 0; module < NBCALMODS; module++) {
		for(int layer = 0; layer < NBCALLAYERS; layer++) {
			for(int sector = 0; sector < NBCALSECTORS; sector++) {
				char name[200], title[400];
				sprintf(name, "M%02iL%iS%i", module+1, layer+1, sector+1);
            	sprintf(title, "%s  Z_{point} Vs. Z_{Track}; Z_{Track} [cm]; Z_{Point} [cm]]", name);

				int the_cell = module * 12 + layer * 4 + sector + 1;
				htrackZvsBCALZ_chan[the_cell] = new TH2I(name, title, 500, zminhall, zmaxhall, 500, zminhall, zmaxhall);
			}
		}
	}

	main->cd();


}

//------------------
// BeginRun
//------------------
void JEventProcessor_BCAL_TDC_Timing::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
    DGeometry* geom = DEvent::GetDGeometry(event);
    geom->GetTargetZ(Z_TARGET);

	// load BCAL geometry
  	vector<const DBCALGeometry *> BCALGeomVec;
  	event->Get(BCALGeomVec);
  	if(BCALGeomVec.size() == 0)
		throw JException("Could not load DBCALGeometry object!");
	auto locBCALGeom = BCALGeomVec[0];

    printf("locBCALGeom->GetBCAL_center()=%f\nZ_TARGET=%f\n",
           locBCALGeom->GetBCAL_center(), Z_TARGET);

    // //////
    // // THIS NEEDS TO CHANGE IF THERE IS A NEW TABLE
    // //////
    // //get timewalk corrections from CCDB
    // JCalibration *jcalib = event->GetJCalibration();
    // //these tables hold: module layer sector end c0 c1 c2 c3
    // vector<vector<float> > tdc_timewalk_table;
    // jcalib->Get("BCAL/timewalk_tdc",tdc_timewalk_table);

    // for (vector<vector<float> >::const_iterator iter = tdc_timewalk_table.begin();
    //       iter != tdc_timewalk_table.end();
    //       ++iter) {
    //    if (iter->size() != 8) {
    //       cout << "DBCALUnifiedHit_factory: Wrong number of values in timewalk_tdc table (should be 8)" << endl;
    //       continue;
    //    }
    //    //be really careful about float->int conversions
    //    int module = (int)((*iter)[0]+0.5);
    //    int layer  = (int)((*iter)[1]+0.5);
    //    int sector = (int)((*iter)[2]+0.5);
    //    int endi   = (int)((*iter)[3]+0.5);
    //    DBCALGeometry::End end = (endi==0) ? DBCALGeometry::kUpstream : DBCALGeometry::kDownstream;
    //    float c0 = (*iter)[4];
    //    float c1 = (*iter)[5];
    //    float c2 = (*iter)[6];
    //    float a_thresh = (*iter)[7];
    //    int cellId = locBCALGeom->cellId(module, layer, sector);
    //    readout_channel channel(cellId,end);
    //    tdc_timewalk_map[channel] = timewalk_coefficients(c0,c1,c2,a_thresh);
    // }

    /// Read in initial calibration constants and write to root file for use in later calibration
    vector<double> raw_channel_global_offset;
    //if(print_messages) jout << "In BCAL_TDC_Timing, loading constants..." << endl;
    if(DEvent::GetCalib(event, "/BCAL/channel_global_offset", raw_channel_global_offset))
        jout << "Error loading /BCAL/channel_global_offset !" << endl;

    lockService->RootFillLock(this);
    //TH1D *CCDB_raw_channel_global_offset = new TH1D("CCDB_raw_channel_global_offset","Offsets at time of running;channel;offset",768,0.5,768.5);
    int counter = 1;
    hCCDB_raw_channel_global_offset->Fill(769, 1);
    for (vector<double>::iterator iter = raw_channel_global_offset.begin(); iter != raw_channel_global_offset.end(); ++iter) {
        hCCDB_raw_channel_global_offset->Fill(counter, *iter);
        counter++;
    }
    lockService->RootFillUnLock(this);
}

//------------------
// Process
//------------------
void JEventProcessor_BCAL_TDC_Timing::Process(const std::shared_ptr<const JEvent>& event)
{
   auto eventnumber = event->GetEventNumber();

   // First check that this is not a font panel trigger or no trigger
   bool goodtrigger=1;

   const DL1Trigger *trig = NULL;
   try {
       event->GetSingle(trig);
   } catch (...) {}
   if (trig) {
       if (trig->fp_trig_mask){
           goodtrigger=0;
       }
   } else {
       // HDDM files are from simulation, so keep them even though they have no trigger
       bool locIsHDDMEvent = DEvent::GetStatusBit(event, kSTATUS_HDDM);
       if (!locIsHDDMEvent) goodtrigger=0;
   }
   
	// load BCAL geometry
  	vector<const DBCALGeometry *> BCALGeomVec;
  	event->Get(BCALGeomVec);
  	if(BCALGeomVec.size() == 0)
		throw JException("Could not load DBCALGeometry object!");
	auto locBCALGeom = BCALGeomVec[0];

    vector<const DTrackFitter *> fitters;
    event->Get(fitters);
    
    if(fitters.size()<1){
      _DBG_<<"Unable to get a DTrackFinder object!"<<endl;
      throw JException("Unable to get a DTrackFinder object!");
    }
    
    auto fitter = fitters[0];



   // calculate total BCAL energy in order to catch BCAL LED events
   vector<const DBCALHit *> bcal_hits;
   event->Get(bcal_hits);
   double total_bcal_energy = 0.;
   for(unsigned int i=0; i<bcal_hits.size(); i++) {
       total_bcal_energy += bcal_hits[i]->E;
   }
   if (total_bcal_energy > 12.) goodtrigger=0;

   if (!goodtrigger) {
       return;
   }

   vector<const DBCALUnifiedHit *> bcalUnifiedHitVector;
   event->Get(bcalUnifiedHitVector);

   /**********************************************
      _____ _                   __        __    _ _    
     |_   _(_)_ __ ___   ___    \ \      / /_ _| | | __
       | | | | '_ ` _ \ / _ \____\ \ /\ / / _` | | |/ /
       | | | | | | | | |  __/_____\ V  V / (_| | |   < 
       |_| |_|_| |_| |_|\___|      \_/\_/ \__,_|_|_|\_\
    ********************************************/
   // The very first thing to do is correct for the timewalk. If this calibration is screwed up, the calibrations that follow will be wrong...
   // The following plots can be used for the calibration or to check the calibration if it has already been done

//    double MIN_TDIFF = -10.0, MAX_TDIFF = 10.0, MAX_TDIFF_WIDE = 30.0;
//    const int ndtbins = 100;
//    double dtbins[ndtbins+1];
//    for (int i=0; i<=ndtbins; i++) {
//        dtbins[i] = MIN_TDIFF + (MAX_TDIFF-MIN_TDIFF)/ndtbins*i;
//    }
//    const int ndtbinswide = 200;
//    double dtbinswide[ndtbinswide+1];
//    for (int i=0; i<=ndtbinswide; i++) {
//        dtbinswide[i] = MIN_TDIFF + (MAX_TDIFF_WIDE-MIN_TDIFF)/ndtbinswide*i;
//    }
//    const int npeakbins = 100;
//    double peakbins[npeakbins+1] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,20,
//                                    21,23,24,26,28,30,32,34,36,39,42,44,48,51,54,58,62,66,71,76,
//                                    81,86,92,98,105,112,120,128,137,146,156,167,178,190,203,217,
//                                    231,247,264,281,300,321,343,366,390,417,445,475,507,541,578,
//                                    617,659,703,750,801,855,913,974,1040,1110,1185,1265,1351,1442,
//                                    1539,1643,1754,1872,1998,2133,2277,2430,2594,2769,2956,3155,
//                                    3368,3595,3837,4096};

   lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

   for (unsigned int i = 0; i < bcalUnifiedHitVector.size(); i++){
      //int the_cell = (bcalUnifiedHitVector[i]->module - 1) * 16 + (bcalUnifiedHitVector[i]->layer - 1) * 4 + bcalUnifiedHitVector[i]->sector;
      // There is one less layer of TDCs so the numbering relects this
      int the_tdc_cell = (bcalUnifiedHitVector[i]->module - 1) * 12 + (bcalUnifiedHitVector[i]->layer - 1) * 4 + bcalUnifiedHitVector[i]->sector;
      //int cellId = locBCALGeom->cellId(bcalUnifiedHitVector[i]->module, bcalUnifiedHitVector[i]->layer, bcalUnifiedHitVector[i]->sector);
      // Get the underlying associated objects
      const DBCALHit * thisADCHit;
      const DBCALTDCHit * thisTDCHit;
      bcalUnifiedHitVector[i]->GetSingle(thisADCHit);
      bcalUnifiedHitVector[i]->GetSingle(thisTDCHit);

      // From the ADC hit we can extract the pulse peak information...
      int pulse_peak = thisADCHit->pulse_peak;

      // The raw information from the DBCALHit and DBCALTDCHit is not corrected for timewalk yet, so we can always plot the before and after.
      if (thisADCHit != NULL && thisTDCHit != NULL){
//          char name[200];
//          sprintf(name, "M%02iL%iS%i", bcalUnifiedHitVector[i]->module, bcalUnifiedHitVector[i]->layer, bcalUnifiedHitVector[i]->sector);
         if (bcalUnifiedHitVector[i]->end == 0){
			 hUpstream_TimewalkVsPeak[the_tdc_cell]->Fill(pulse_peak, thisTDCHit->t - thisADCHit->t);
			 hUpstream_Channel_Deltat_All->Fill(the_tdc_cell, thisTDCHit->t - thisADCHit->t);
         }
         else{
			 hDownstream_TimewalkVsPeak[the_tdc_cell]->Fill(pulse_peak, thisTDCHit->t - thisADCHit->t);
			 hDownstream_Channel_Deltat_All->Fill(the_tdc_cell, thisTDCHit->t - thisADCHit->t);
         }
      }
      
      // Next look directly at the DBCALUnifiedHit to get the corrected times and plot those seperately
      if (bcalUnifiedHitVector[i]->has_TDC_hit){
         double correctedTDCTime = bcalUnifiedHitVector[i]->t_TDC;
         char name[200];
         sprintf(name, "M%02iL%iS%i", bcalUnifiedHitVector[i]->module, bcalUnifiedHitVector[i]->layer, bcalUnifiedHitVector[i]->sector);
         if (bcalUnifiedHitVector[i]->end == 0){
			 hUpstream_TimewalkVsPeak_Corrected[the_tdc_cell]->Fill(pulse_peak, correctedTDCTime - thisADCHit->t);
			 hUpstream_Channel_Deltat_All_Corrected->Fill(the_tdc_cell, correctedTDCTime - thisADCHit->t);
         }
         else{
			 hDownstream_TimewalkVsPeak_Corrected[the_tdc_cell]->Fill(pulse_peak, thisTDCHit->t - thisADCHit->t);
			 hDownstream_Channel_Deltat_All_Corrected->Fill(the_tdc_cell, thisTDCHit->t - thisADCHit->t);
         }
      }
   }

   lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

   //
   // Attenuation Length
   //
   // vector<const DBCALPoint *> BCALPointVector;
   // event->Get(BCALPointVector);

   // for (unsigned int i = 0; i < BCALPointVector.size(); i++){
   //     const DBCALPoint *thisPoint = BCALPointVector[i];
   //     vector<const DBCALDigiHit*> digihits;
   //     thisPoint->Get(digihits);
   //     if (digihits.size()!=2) {
   //         printf("Warning: BCAL_attenlength_gainratio: event %llu: wrong number of BCALDigiHit objects found %i\n",
   //                (long long unsigned int)eventnumber,(int)digihits.size());
   //         continue;
   //     }
   //     if (digihits[0]->end==digihits[1]->end) {
   //         printf("Warning: BCAL_attenlength_gainratio: event %llu: two hits in same end of point\n",(long long unsigned int)eventnumber);
   //         continue;
   //     }
   //     float integralUS, integralDS;
   //     // end 0=upstream, 1=downstream
   //     if (digihits[0]->end==0) {
   //         integralUS = digihits[0]->pulse_integral - ((float)digihits[0]->nsamples_integral*(float)digihits[0]->pedestal)/
   //             (float)digihits[0]->nsamples_pedestal;
   //         integralDS = digihits[1]->pulse_integral - ((float)digihits[1]->nsamples_integral*(float)digihits[1]->pedestal)/
   //             (float)digihits[1]->nsamples_pedestal;
   //     } else { 
   //         integralDS = digihits[0]->pulse_integral - ((float)digihits[0]->nsamples_integral*(float)digihits[0]->pedestal)/
   //             (float)digihits[0]->nsamples_pedestal;
   //         integralUS = digihits[1]->pulse_integral - ((float)digihits[1]->nsamples_integral*(float)digihits[1]->pedestal)/
   //             (float)digihits[1]->nsamples_pedestal;
   //     }
   //     float intratio = (float)integralUS/(float)integralDS;
   //     float logintratio = log(intratio);

   //     float zminlocal = -250;
   //     float zmaxlocal = 250;
   //     char name[200], title[200];
   //     sprintf(title,"Attenuation;Z_{Track}  (cm);log of integral ratio US/DS");
   //     Fill2DHistogram ("BCAL_atten_gain", "logintratiovsZtrack", "AllPoints",
   //                      thisPoint->z, logintratio, title,
   //                      250, zminlocal, zmaxlocal, 250, -3, 3);
   //     sprintf(title,"Attenuation (M%i,L%i,S%i);Z_{Track}  (cm);log of integral ratio US/DS", 
   //             thisPoint->module(), thisPoint->layer(), thisPoint->sector());
   //     Fill2DHistogram ("BCAL_atten_gain", "logintratiovsZtrack", channame,
   //                      thisPoint->z, logintratio, title,
   //                      250, zminlocal, zmaxlocal, 250, -3, 3);
   // }
    
   


     /*************************************************
        _________  _____  _______       _
       /_  __/ _ \/ ___/ /_  __(_)_ _  (_)__  ___ _
        / / / // / /__    / / / /  ' \/ / _ \/ _ `/
       /_/ /____/\___/   /_/ /_/_/_/_/_/_//_/\_, /
                                            /___/
    **************************************************/

   // Now we will use the track matching in order to work out the time offsets and effective velocities
   // We just need to grab the charged particles in order to get their detector matches
   // Note that this method is only really applicable for runs with the solenoid on...

   // We need the RF bunch for the event in order to check the global alignemtn of the timing
   const DEventRFBunch *thisRFBunch = NULL;
   event->GetSingle(thisRFBunch);

   vector <const DChargedTrack *> chargedTrackVector;
   event->Get(chargedTrackVector);

   lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
   
   hDebug->Fill(1);
   for (unsigned int iTrack = 0; iTrack < chargedTrackVector.size(); iTrack++){
      // Pick out the best charged track hypothesis for this charged track based only on the Tracking FOM
      // const DChargedTrackHypothesis* bestHypothesis = chargedTrackVector[iTrack]->Get_BestTrackingFOM();
      hDebug->Fill(2);
      // get charge and choose pion hypothesis as most likely
      int charge = chargedTrackVector[iTrack]->Get_Charge();
      char q[2];
      const DChargedTrackHypothesis* bestHypothesis;
      if (charge>0) {
          bestHypothesis = chargedTrackVector[iTrack]->Get_Hypothesis(PiPlus);
          sprintf(q,"+");
      } else {
          bestHypothesis = chargedTrackVector[iTrack]->Get_Hypothesis(PiMinus);
          sprintf(q,"-");
      }
      if (bestHypothesis == NULL) continue;
      hDebug->Fill(3);

      // Now from this hypothesis we can get the detector matches to the BCAL
      auto bcalMatch = bestHypothesis->Get_BCALShowerMatchParams();
      auto scMatch = bestHypothesis->Get_SCHitMatchParams(); // Needed for quality cut later
      DVector3 position = bestHypothesis->position();
      //DVector3 momentum = bestHypothesis->momentum();
      //float Z_track = position.z();
      //float_track = momentum.Mag();

      if (bcalMatch == NULL) continue; 
      hDebug->Fill(4);
      if (!DONT_USE_SC && (scMatch == NULL)) continue;
      hDebug->Fill(5);

      double dDeltaZToShower = bcalMatch->dDeltaZToShower;
      double dDeltaPhiToShower = bcalMatch->dDeltaPhiToShower;
      
      hBCALMatch->Fill(dDeltaZToShower, dDeltaPhiToShower);

      const DTrackTimeBased *timeBasedTrack = bestHypothesis->Get_TrackTimeBased();

      if(DONT_USE_SC) {
          if (timeBasedTrack->FOM < 1e-10) continue; // if ther's no SC, the tracking FOM is not as good
      } else {
          if (timeBasedTrack->FOM < 0.0027) continue; // 3-sigma cut on tracking FOM
      }
      hDebug->Fill(6);
      if (timeBasedTrack->Ndof < 10) continue; // CDC: 5 params in fit, 10 dof => [15 hits]; FDC [10 hits]

      // Use CDC dEdx to help reject protons
      double dEdx=1e6*timeBasedTrack->ddEdx_CDC_amp;
      double P_track=timeBasedTrack->momentum().Mag();
      bool dEdx_pion = 0;
      if (dEdx<2.5) dEdx_pion = 1;

      // Get the shower from the match
      const DBCALShower *thisShower = bcalMatch->dBCALShower;

      // Fill histograms based on the shower
      // char name[200], title[200];
      DVector3 proj_pos;
      double flightTime=0.;
      //double innerpathLength, innerflightTime;
      double shower_x = thisShower->x;
      double shower_y = thisShower->y;

      double t_shower = thisShower->t;
      double E_shower = thisShower->E;
      double Z_shower = thisShower->z;
      // UNPROJECTION CODE
      // double shower_deltaz = thisShower->z-Z_TARGET;
      // double straightPathLength = sqrt(r_shower*r_shower + shower_deltaz*shower_deltaz);
      // double modulepath = (r_shower-locBCALGeom->GetBCAL_inner_rad())/r_shower*straightPathLength; // project time back to shower location
      // double t_module = modulepath/SPEED_OF_LIGHT; // project time back to shower location
      // t_shower += t_module;

      // printf("shower (%5.1f,%5.1f,%5.1f) r=%5.1f t=%5.1f E=%5.3f rho=%5.1f module rho=%5.1f t=%5.1f new t=%5.1f\n",
      //         shower_x,shower_y,shower_deltaz,r_shower,thisShower->t,E_shower,straightPathLength,modulepath,t_module,t_shower);
      //int res1 = rt->GetIntersectionWithRadius(r_shower,proj_pos, &pathLength, &flightTime);
      //int res2 = rt->GetIntersectionWithRadius(locBCALGeom->GetBCAL_inner_rad(),proj_pos, &innerpathLength, &innerflightTime);
      //if (res1==NOERROR && res2==NOERROR) {
      DVector3 bcalpos(shower_x,shower_y,Z_shower);
      double R=bcalpos.Perp();
      double pathLength=0.;
      DVector3 proj_mom;
      vector<DTrackFitter::Extrapolation_t>extrapolations=timeBasedTrack->extrapolations.at(SYS_BCAL);
      if (fitter->ExtrapolateToRadius(R,extrapolations,proj_pos,proj_mom,
				      flightTime,pathLength)){	

          hDebug->Fill(7);
          if (thisRFBunch->dNumParticleVotes >= 2){ // Require good RF bunch and this track match the SC
              hDebug->Fill(8);
              // We have the flight time to our BCAL point, so we can get the target time
              double targetCenterTime = t_shower - flightTime - ((timeBasedTrack->position()).Z() - Z_TARGET) / SPEED_OF_LIGHT;
			  if (charge>0) {
				  hPosShowers_Evst->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
				  hPosShowers_PvsdEdx->Fill(P_track, dEdx);
				  hPosShowers_dEdxvst->Fill(dEdx, targetCenterTime - thisRFBunch->dTime);
				  hPosShowers_PvsE->Fill(P_track, E_shower);
				  hPosShowers_PvsEP->Fill(P_track, E_shower/P_track);
				  if (dEdx_pion) {
					  hDebug->Fill(9);
					  hPosPionShowers_Evst->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
					  hPosPionShowers_Pvst->Fill(P_track, targetCenterTime - thisRFBunch->dTime);
					  hPosPionShowers_zvst->Fill(Z_shower, targetCenterTime - thisRFBunch->dTime);
				  }
			  } else {
				  hNegShowers_Evst->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
				  hNegShowers_PvsdEdx->Fill(P_track, dEdx);
				  hNegShowers_dEdxvst->Fill(dEdx, targetCenterTime - thisRFBunch->dTime);
				  hNegShowers_PvsE->Fill(P_track, E_shower);
				  hNegShowers_PvsEP->Fill(P_track, E_shower/P_track);
				  if (dEdx_pion) {
					  hDebug->Fill(9);
					  hNegPionShowers_Evst->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
					  hNegPionShowers_Pvst->Fill(P_track, targetCenterTime - thisRFBunch->dTime);
					  hNegPionShowers_zvst->Fill(Z_shower, targetCenterTime - thisRFBunch->dTime);
				  }
			  }

          }
      }


      // Get the points from the shower
      vector <const DBCALPoint*> pointVector;
      thisShower->Get(pointVector);

      int N_points = pointVector.size();
      // N_points vs E_shower
      if (charge>0) {
      	hNpointVsEshower_qpos->Fill(E_shower, N_points);
      } else {
      	hNpointVsEshower_qneg->Fill(E_shower, N_points);
      }

      // Loop over the points within the cluster
      for (unsigned int iPoint = 0; iPoint < pointVector.size(); iPoint++){
         const DBCALPoint *thisPoint = pointVector[iPoint];
         //if (thisPoint->E() < 0.05) continue; // The timing is known not to be great for very low energy, so only use our best info 
         double rpoint = thisPoint->r();
         float E_point = thisPoint->E();
         double Z_point = thisPoint->z();
         
         hAllPointsVsShower_zpos->Fill(Z_shower, Z_point + Z_TARGET);

		 if (fitter->ExtrapolateToRadius(rpoint,extrapolations,proj_pos,
						 proj_mom,
						 flightTime,pathLength)){	

            // Now proj_pos contains the projected position of the track at this particular point within the BCAL
            // We can plot the difference of the projected position and the BCAL position as a function of the channel
            // These results are in slightly different coordinate systems. We want one where the center of the BCAL is z=0
            double trackHitZ = proj_pos.z();
            double localTrackHitZ = proj_pos.z() - locBCALGeom->GetBCAL_center();
            double BCALHitZ = thisPoint->z() + Z_TARGET;
            //double localBCALHitZ = thisPoint->z() + Z_TARGET - locBCALGeom->GetBCAL_center();
            double deltaZ = trackHitZ-BCALHitZ;
            double Deltat = thisPoint->t_US() - thisPoint->t_DS();

			int the_cell = (thisPoint->module() - 1) * 12 + (thisPoint->layer() - 1) * 4 + thisPoint->sector();

            
            hZvsDeltat_all->Fill(Deltat, trackHitZ); 
            if(charge>0) {
	            hZvsDeltat_qpos->Fill(Deltat, trackHitZ);
			} else {
	            hZvsDeltat_qneg->Fill(Deltat, trackHitZ);
			}
			hZvsDeltat_layer[thisPoint->layer() - 1]->Fill(Deltat, trackHitZ);
			hZvsDeltat_chan[the_cell]->Fill(Deltat, trackHitZ);
			
            double trackTheta = 180/3.14159265358*atan2(timeBasedTrack->pperp(),timeBasedTrack->pz());
			hThetavsDeltat_layer[thisPoint->layer() - 1]->Fill(trackTheta, Deltat);

            //int the_cell = (thisPoint->module() - 1) * 16 + (thisPoint->layer() - 1) * 4 + thisPoint->sector();
            float Deltat_Zcorr = Deltat - (trackHitZ-212)/8.1;

			hDeltat_corr_all->Fill(Deltat_Zcorr);
			hDeltatvscell_corr->Fill(the_cell, Deltat_Zcorr);

            hZvsDeltaz_all->Fill(trackHitZ, deltaZ); 
            if(charge>0) {
	            hZvsDeltaz_qpos->Fill(trackHitZ, deltaZ);
			} else {
	            hZvsDeltaz_qneg->Fill(trackHitZ, deltaZ);
			}
			hZvsDeltaz_layer[thisPoint->layer() - 1]->Fill(trackHitZ, deltaZ);
			hZvsDeltaz_chan[the_cell]->Fill(trackHitZ, deltaZ);
			
            htrackZvsBCALZ_all->Fill(trackHitZ, BCALHitZ); 
            if(charge>0) {
	            htrackZvsBCALZ_qpos->Fill(trackHitZ, BCALHitZ);
			} else {
	            htrackZvsBCALZ_qneg->Fill(trackHitZ, BCALHitZ);
			}
			htrackZvsBCALZ_layer[thisPoint->layer() - 1]->Fill(trackHitZ, BCALHitZ);
			htrackZvsBCALZ_chan[the_cell]->Fill(trackHitZ, BCALHitZ);
			
			
            // Get the unifiedhits
            vector <const DBCALUnifiedHit*> unifiedhitVector;
            thisPoint->Get(unifiedhitVector);
            int up   = unifiedhitVector[0]->end; // up=1   if unifiedhitVector[0]->end = 1 (is downstream)
            int down = unifiedhitVector[1]->end; // down=1 if unifiedhitVector[1]->end = 1 (is downstream)
            const DBCALUnifiedHit *thisUnifiedhitup   = unifiedhitVector[up];
            const DBCALUnifiedHit *thisUnifiedhitdown = unifiedhitVector[down];
            float t_up = thisUnifiedhitup->t;
            float t_ADC_up = thisUnifiedhitup->t_ADC;
            float t_TDC_up = thisUnifiedhitup->t_TDC;
            float t_down = thisUnifiedhitdown->t;
            float t_ADC_down = thisUnifiedhitdown->t_ADC;
            float t_TDC_down = thisUnifiedhitdown->t_TDC;
            char type[10];
            sprintf(type,"Mixed");
            if (t_up == t_ADC_up && t_down == t_ADC_down) sprintf(type,"ADC");
            if (t_up == t_TDC_up && t_down == t_TDC_down) sprintf(type,"TDC");

            const DBCALHit * thisADCHit_up;
            thisUnifiedhitup->GetSingle(thisADCHit_up);
            const DBCALHit * thisADCHit_down;
            thisUnifiedhitdown->GetSingle(thisADCHit_down);

            // Get raw times
            float Deltat_raw = thisADCHit_up->t_raw - thisADCHit_down->t_raw;
            float Deltat_raw_Zcorr = Deltat_raw - (trackHitZ-212)/8.1;
            
            hDeltat_raw_all->Fill(Deltat_raw_Zcorr);
            hDeltat_raw_chan->Fill(the_cell, Deltat_raw_Zcorr);
            
            
            // Attenuation Length
            vector<const DBCALDigiHit*> digihits;
            thisPoint->Get(digihits);
      		if (digihits.size()!=2) {
                printf("Warning: BCAL_attenlength_gainratio: event %llu: wrong number of BCALDigiHit objects found %i\n",
                       (long long unsigned int)eventnumber,(int)digihits.size());
                continue;
            }
            if (digihits[0]->end==digihits[1]->end) {
                printf("Warning: BCAL_attenlength_gainratio: event %llu: two hits in same end of point\n",(long long unsigned int)eventnumber);
                continue;
            }
            float integralUS, integralDS;
            // end 0=upstream, 1=downstream
            if (digihits[0]->end==0) {
                integralUS = digihits[0]->pulse_integral - ((float)digihits[0]->nsamples_integral*(float)digihits[0]->pedestal)/
                    (float)digihits[0]->nsamples_pedestal;
                integralDS = digihits[1]->pulse_integral - ((float)digihits[1]->nsamples_integral*(float)digihits[1]->pedestal)/
                    (float)digihits[1]->nsamples_pedestal;
            } else { 
                integralDS = digihits[0]->pulse_integral - ((float)digihits[0]->nsamples_integral*(float)digihits[0]->pedestal)/
                    (float)digihits[0]->nsamples_pedestal;
                integralUS = digihits[1]->pulse_integral - ((float)digihits[1]->nsamples_integral*(float)digihits[1]->pedestal)/
                    (float)digihits[1]->nsamples_pedestal;
            }
            float intratio = (float)integralUS/(float)integralDS;
            float logintratio = log(intratio);

            if (VERBOSEHISTOGRAMS) {
            	hlogintratiovsZtrack_all->Fill(localTrackHitZ, logintratio);
            	hlogintratiovsZtrack_chan[the_cell]->Fill(localTrackHitZ, logintratio);
            }

            // Now fill some histograms that are useful for aligning the BCAL with the rest of the detector systems
            if (thisRFBunch->dNumParticleVotes >= 2 && (DONT_USE_SC || (scMatch != NULL)) && dEdx_pion){ // Require good RF bunch and this track match the SC
               // Get the time of the BCAL point
               double pointTime = thisPoint->t();
               // We have the flight time to our BCAL point, so we can get the target time
               double vertexTime = (timeBasedTrack->position().Z() - Z_TARGET) / SPEED_OF_LIGHT;; // time for beam to go from center of target to vertex
               double targetCenterTime = pointTime - flightTime - vertexTime;

               // Now we just plot the difference in from the RF Time to get out the correction
               if (E_point > 0.05) { // The timing is known not to be great for very low energy, so only use our best info 

					hdeltaTVsCell_all->Fill(the_cell, targetCenterTime - thisRFBunch->dTime);
					if(charge > 0) {
						hdeltaTVsCell_qpos->Fill(the_cell, targetCenterTime - thisRFBunch->dTime);
						hdeltaTVsCell_qpos_Eweight->Fill(the_cell, targetCenterTime - thisRFBunch->dTime, E_point);
						hdeltaTVsCell_qpos_E2weight->Fill(the_cell, targetCenterTime - thisRFBunch->dTime, E_point*E_point);
						hdeltaTVsLayer_qpos->Fill(thisPoint->layer(), targetCenterTime - thisRFBunch->dTime);
					} else {
						hdeltaTVsCell_qneg->Fill(the_cell, targetCenterTime - thisRFBunch->dTime);
						hdeltaTVsCell_qneg_Eweight->Fill(the_cell, targetCenterTime - thisRFBunch->dTime, E_point);
						hdeltaTVsCell_qneg_E2weight->Fill(the_cell, targetCenterTime - thisRFBunch->dTime, E_point*E_point);
						hdeltaTVsLayer_qneg->Fill(thisPoint->layer(), targetCenterTime - thisRFBunch->dTime);
					}
               }

               float pulse_peak_max = max(thisADCHit_up->pulse_peak,thisADCHit_down->pulse_peak);
               float pulse_peak_min = min(thisADCHit_up->pulse_peak,thisADCHit_down->pulse_peak);

               double fibLen = locBCALGeom->GetBCAL_length();
               double c_effective = locBCALGeom->GetBCAL_c_effective();
               double BCALtrackHitZ = trackHitZ - (locBCALGeom->GetBCAL_center() - fibLen/2); // position wrt BCAL front edge
               double barproptime_up   = BCALtrackHitZ / c_effective;
               double barproptime_down = (fibLen-BCALtrackHitZ) / c_effective;
               double hitup_TargetCenterTime   = t_up   - barproptime_up   - flightTime - vertexTime;
               double hitdown_TargetCenterTime = t_down - barproptime_down - flightTime - vertexTime;
               double hittimediff = t_down - t_up - 2*localTrackHitZ/c_effective;
	    
               int the_cell = (thisPoint->module() - 1) * 16 + (thisPoint->layer() - 1) * 4 + thisPoint->sector();
               //int channel = end*768 + the_cell;
               hHitDeltaTVsChannel->Fill(the_cell, hitup_TargetCenterTime - thisRFBunch->dTime);
               hHitDeltaTVsChannel->Fill(the_cell+768, hitdown_TargetCenterTime - thisRFBunch->dTime);
               hHittimediff->Fill(the_cell, hittimediff);
               
//             sprintf(type,"Mixed");
//             if (t_up == t_ADC_up && t_down == t_ADC_down) sprintf(type,"ADC");
//             if (t_up == t_TDC_up && t_down == t_TDC_down) sprintf(type,"TDC");

			   if (t_up == t_ADC_up && t_down == t_ADC_down) {   // "ADC"
			   	if(charge > 0) {
			   		hHits_deltaTVsE_ADC_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmax_ADC_qpos->Fill(pulse_peak_max, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmin_ADC_qpos->Fill(pulse_peak_min, targetCenterTime - thisRFBunch->dTime);
			   	} else {
			   		hHits_deltaTVsE_ADC_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmax_ADC_qneg->Fill(pulse_peak_max, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmin_ADC_qneg->Fill(pulse_peak_min, targetCenterTime - thisRFBunch->dTime);
			   	}
			   		
			   } else if (t_up == t_TDC_up && t_down == t_TDC_down) {   // "TDC"
			   	if(charge > 0) {
			   		hHits_deltaTVsE_TDC_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmax_TDC_qpos->Fill(pulse_peak_max, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmin_TDC_qpos->Fill(pulse_peak_min, targetCenterTime - thisRFBunch->dTime);
			   	} else {
			   		hHits_deltaTVsE_TDC_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmax_TDC_qneg->Fill(pulse_peak_max, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmin_TDC_qneg->Fill(pulse_peak_min, targetCenterTime - thisRFBunch->dTime);
			   	}
			   		
			   } else {    // "Mixed"
			   	if(charge > 0) {
			   		hHits_deltaTVsE_Mixed_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmax_Mixed_qpos->Fill(pulse_peak_max, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmin_Mixed_qpos->Fill(pulse_peak_min, targetCenterTime - thisRFBunch->dTime);
			   	} else {
			   		hHits_deltaTVsE_Mixed_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmax_Mixed_qneg->Fill(pulse_peak_max, targetCenterTime - thisRFBunch->dTime);
			   		hHits_deltaTVsPPmin_Mixed_qneg->Fill(pulse_peak_min, targetCenterTime - thisRFBunch->dTime);
			   	}
			   		
			   }


               int layer = thisPoint->layer();
               // E_point vs E_shower
               if(charge > 0) {
               		hEpointVsEshower_qpos->Fill(E_shower, E_point);
               		hEpointVsEshower_qpos_layer[layer-1]->Fill(E_shower, E_point);
               } else {
               		hEpointVsEshower_qneg->Fill(E_shower, E_point);
               		hEpointVsEshower_qneg_layer[layer-1]->Fill(E_shower, E_point);
               }
               
               // deltaT vs E_point and deltaT vs E_shower
               if(charge > 0) {
               		hPoints_deltaTVsEnergy_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
               		hPoints_deltaTVsEnergy_qpos_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
               		
               		hPoints_deltaTVsShowerEnergy_qpos->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
               		hPoints_deltaTVsShowerEnergy_qpos_layer[layer-1]->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
               		
					if (t_up == t_ADC_up && t_down == t_ADC_down) {   // "ADC"
						hPoints_deltaTVsEnergy_ADC_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
						hPoints_deltaTVsEnergy_ADC_qpos_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
					} else if (t_up == t_TDC_up && t_down == t_TDC_down) {   // "TDC"
						hPoints_deltaTVsEnergy_TDC_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
						hPoints_deltaTVsEnergy_TDC_qpos_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
					} else {    // "Mixed"
						hPoints_deltaTVsEnergy_Mixed_qpos->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
						hPoints_deltaTVsEnergy_Mixed_qpos_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
					}
               } else {
               		hPoints_deltaTVsEnergy_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
               		hPoints_deltaTVsEnergy_qneg_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
               		
               		hPoints_deltaTVsShowerEnergy_qneg->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
               		hPoints_deltaTVsShowerEnergy_qneg_layer[layer-1]->Fill(E_shower, targetCenterTime - thisRFBunch->dTime);
               		
					if (t_up == t_ADC_up && t_down == t_ADC_down) {   // "ADC"
						hPoints_deltaTVsEnergy_ADC_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
						hPoints_deltaTVsEnergy_ADC_qneg_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
					} else if (t_up == t_TDC_up && t_down == t_TDC_down) {   // "TDC"
						hPoints_deltaTVsEnergy_TDC_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
						hPoints_deltaTVsEnergy_TDC_qneg_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
					} else {    // "Mixed"
						hPoints_deltaTVsEnergy_Mixed_qneg->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
						hPoints_deltaTVsEnergy_Mixed_qneg_layer[layer-1]->Fill(E_point, targetCenterTime - thisRFBunch->dTime);
					}
               }
                                              
            }
         }
      }
   }

   /*************************************************
                             ___             _ 
     |\| _    _|_ __ _  |     |  o __  o __ (_|
     | |(/_|_| |_ | (_| |     |  | ||| | | |__|

   **************************************************/


   // double pathLength, flightTime;
   // //double r_shower = sqrt(shower_x*shower_x+shower_y*shower_y);
   // double t_shower = shower_t;
   // double E_shower = shower_E;
   // char name[200], title[200];
   hDebug->Fill(10);
   if (thisRFBunch->dNumParticleVotes >= 2){ // Require good RF bunch
       hDebug->Fill(11);
       vector<const DVertex*> locVertex;
       event->Get(locVertex);       
       // *** get unmatched BCAL showers from neutral showers
       // *** not restrictive enough so do the matching locally
       //vector <const DNeutralShower *> neutralShower;
       // event->Get(neutralShower);
       //       for (unsigned int ishower = 0; ishower < neutralShower.size(); ishower++){
       // vector <const DBCALShower*> bcalshowervector;
       // if (neutralShower[ishower]->dDetectorSystem == SYS_BCAL) {
       //        neutralShower[ishower]->Get(bcalshowervector);
       // }
       vector <const DBCALShower *> locBCALShowers;
       event->Get(locBCALShowers);
       vector<const DTrackTimeBased*> locTrackTimeBased;
       event->Get(locTrackTimeBased);
       for (unsigned int ibcalshower = 0; ibcalshower < locBCALShowers.size(); ibcalshower++){
           hDebug->Fill(12);
           const DBCALShower *bcalshower = locBCALShowers[ibcalshower];
           double x = bcalshower->x;
           double y = bcalshower->y;
           double z = bcalshower->z;
           DVector3 showerpos(x,y,z);
           double R_shower = showerpos.Perp();
           // *** Remove matched BCAL showers
           bool matched=0;
           for (unsigned int i=0; i < locTrackTimeBased.size() ; ++i) {
               DVector3 trackpos(0.0,0.0,0.0); 
	       vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased[i]->extrapolations.at(SYS_BCAL);
	       if (fitter->ExtrapolateToRadius(R_shower,extrapolations,trackpos)){	

		 double dPhi = 180./3.14159265358*(trackpos.Phi()-showerpos.Phi());
		 double dZ = (trackpos.Z() - z);
		 hMatching_dZvsdPhi->Fill(dZ, dPhi);

		 // analysis shows 40 and 15 are better
		 if (TMath::Abs(dZ < 40.0) && TMath::Abs(dPhi) < 15) matched=1;
	       }
	   }
           if (matched) continue;
           hDebug->Fill(13);

           float vertexX = locVertex[0]->dSpacetimeVertex.X();
           float vertexY = locVertex[0]->dSpacetimeVertex.Y();
           float vertexZ = locVertex[0]->dSpacetimeVertex.Z();
           float xdiff = bcalshower->x - vertexX;
           float ydiff = bcalshower->y - vertexY;
           float zdiff = bcalshower->z - vertexZ;
           float pathLength = sqrt(xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
            // UNPROJECTION CODE
           // since time is projected to BCAL inner radius, shorten pathlength to match
           //pathLength *= locBCALGeom->GetBCAL_inner_rad()/R_shower;

           float flightTime = pathLength/SPEED_OF_LIGHT;
           float vertexTime = (vertexZ - Z_TARGET) / SPEED_OF_LIGHT; // time for beam to go from center of target to vertex
           double targetCenterTime = bcalshower->t - flightTime - vertexTime;
           double deltaTime = targetCenterTime - thisRFBunch->dTime;
           double E_shower = bcalshower->E;
           //printf("shower (%5.1f,%5.1f,%5.1f) vertex (%5.1f,%5.1f,%5.1f) length (%5.1f,%5.1f,%5.1f) time sh %5.1f flight %5.1f %5.1f target %5.1f rf %5.1f\n",
           // bcalshower->x,bcalshower->y,bcalshower->z,vertexX,vertexY,vertexZ,
           // xdiff,ydiff,zdiff, bcalshower->t,flightTime,vertexTime,targetCenterTime,thisRFBunch->dTime);
           hNeutShowers_Evst->Fill(E_shower, deltaTime);
           hNeutShowers_zvst->Fill(bcalshower->z, deltaTime);
           
           // Get the points from the shower
           vector <const DBCALPoint*> pointVector;
           bcalshower->Get(pointVector);
           int N_points = pointVector.size();
           // N_points vs E_shower
           hNpointVsEshower_q0->Fill(E_shower, N_points);

           // Loop over the points within the cluster
           for (unsigned int iPoint = 0; iPoint < pointVector.size(); iPoint++){
               const DBCALPoint *thisPoint = pointVector[iPoint];
               float E_point = thisPoint->E();
               // Now we just plot the difference in from the RF Time to get out the correction
               pathLength = thisPoint->rho();  // This is an approximation: the photon doesn't come from the target center
               flightTime = pathLength/SPEED_OF_LIGHT;
               vertexTime = 0; // no vertex time since we are using target center from point
               targetCenterTime =  thisPoint->t() - flightTime - vertexTime;
               deltaTime = targetCenterTime - thisRFBunch->dTime;

               if (E_point > 0.05) { // The timing is known not to be great for very low energy, so only use our best info 
                   int the_cell = (thisPoint->module() - 1) * 16 + (thisPoint->layer() - 1) * 4 + thisPoint->sector();
                   hdeltaTVsCell_q0->Fill(the_cell, deltaTime);
                   hdeltaTVsCell_q0_Eweight->Fill(the_cell, deltaTime, E_point);
                   hdeltaTVsCell_q0_E2weight->Fill(the_cell, deltaTime, E_point*E_point);
                                  }
               int layer = thisPoint->layer();
               // deltaT vs E_point
               hPoints_deltaTVsEnergy_q0->Fill(E_point, deltaTime);
               hPoints_deltaTVsEnergy_q0_layer[layer-1]->Fill(E_point, deltaTime);
               
               // deltaT vs E_shower
               hPoints_deltaTVsShowerEnergy_q0->Fill(E_shower, deltaTime);
               hPoints_deltaTVsShowerEnergy_q0_layer[layer-1]->Fill(E_shower, deltaTime);

               // Now simulate the projection to the inner radius

               pathLength = thisPoint->rho()*locBCALGeom->GetBCAL_inner_rad()/R_shower;
               flightTime = pathLength/SPEED_OF_LIGHT;
               vertexTime = 0; // no vertex time since we are using target center from point
               targetCenterTime =  thisPoint->tInnerRadius() - flightTime - vertexTime;
               float altDeltaTime = targetCenterTime - thisRFBunch->dTime;

               // altDeltaT vs E_point
               hPoints_altdeltaTVsEnergy_q0->Fill(E_point, altDeltaTime);
               hPoints_altdeltaTVsEnergy_q0_layer[layer-1]->Fill(E_point, altDeltaTime);

               // altDeltaT vs E_shower
               hPoints_altdeltaTVsShowerEnergy_q0->Fill(E_shower, altDeltaTime);
               hPoints_altdeltaTVsShowerEnergy_q0_layer[layer-1]->Fill(E_shower, altDeltaTime);

           }
       }
   }   

   lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

   return;
}

//------------------
// EndRun
//------------------
void JEventProcessor_BCAL_TDC_Timing::EndRun()
{
   // This is called whenever the run number changes, before it is
   // changed to give you a chance to clean up before processing
   // events from the next run number.
   return;
}

//------------------
// Finish
//------------------
void JEventProcessor_BCAL_TDC_Timing::Finish()
{
   // Called before program exit after event processing is finished.
   return;
}

