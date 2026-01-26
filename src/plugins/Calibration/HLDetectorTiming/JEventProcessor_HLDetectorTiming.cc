// $Id$
//
//    File: JEventProcessor_HLDetectorTiming.cc
// Created: Mon Jan 12 14:37:56 EST 2015
// Creator: mstaib (on Linux egbert 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#include "JEventProcessor_HLDetectorTiming.h"

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
#include <DANA/DEvent.h>


#include "PID/DChargedTrack.h"
#include "PID/DEventRFBunch.h"
#include "TTAB/DTTabUtilities.h"
#include "TTAB/DTranslationTable.h"
#include "ECAL/DECALGeometry.h"
#include "FCAL/DFCALGeometry.h"
#include "BCAL/DBCALGeometry.h"
#include "CCAL/DCCALGeometry.h"
#include "TRIGGER/DTrigger.h"
#include "RF/DRFTime.h"

#include "PAIR_SPECTROMETER/DPSCHit.h"
#include "PAIR_SPECTROMETER/DPSHit.h"
#include "CCAL/DCCALHit.h"
#include "CCAL/DCCALShower.h"
#include "DIRC/DDIRCPmtHit.h"
#include "DIRC/DDIRCPmtHit_factory.h"
#include "FMWPC/DFMWPCHit.h"
#include "FMWPC/DCTOFHit.h"


extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_HLDetectorTiming());
    app->Add(new DFactoryGenerator_p2pi()); //register the factory generator
}
} // "C"

static int Get_FDCTDC_crate_slot(int mod, string &act_crate, int &act_slot){ //expected mod range from 1 to 48
  int LH_module=(mod-1)%2; //low (1-48) or high (49-96) wire number (0/1)
  int package=(mod-1)/12; //package number (0-3)
  int cell=(mod-1-package*12)/2;  //cell number (0-5)

  int rotation = -45 + 90*LH_module -60*cell;
  if(rotation<-180)rotation+=360;

  int crate=0; // (0-3) actual crates are ROCFDC1,4,13,14
  if(package<2){
    crate=0;
    if(rotation>0)crate=3;
  } else {
    crate=1;
    if(rotation>0)crate=2;
  }

  int slot=0; //(0-11) actual slots are 3-10,13-16
  if(rotation<0){
    if(cell==0){
      slot=0;
    } else if (cell==1) {
      slot=1+LH_module;
    } else if (cell==2) {
      slot=3+LH_module;
    } else {
      slot=5;
    }
  } else {
    if(cell==0){
      slot=0;
    } else if (cell==3) {
      slot=1;
    } else if (cell==4) {
      slot=2+LH_module;
    } else {
      slot=4+LH_module;
    }
  } 
  slot+=(package%2)*6;

  //string act_crate="ROCFDC1";
  act_crate="ROCFDC1";
  if(crate==1)act_crate="ROCFDC4";
  if(crate==2)act_crate="ROCFDC13";
  if(crate==3)act_crate="ROCFDC14";
  //int act_slot=slot+3;
  act_slot=slot+3;
  if(act_slot>10)act_slot+=2;

  //cout<<"        "<<act_crate<<endl;
  //cout<<" actual slot="<<act_slot<<endl;

  return crate*12+slot+1; //returns modules in crate/slot sequence (1-48)

}


//------------------
// JEventProcessor_HLDetectorTiming (Constructor)
//------------------
JEventProcessor_HLDetectorTiming::JEventProcessor_HLDetectorTiming()
{
	SetTypeName("JEventProcessor_HLDetectorTiming");
}

//------------------
// ~JEventProcessor_HLDetectorTiming (Destructor)
//------------------
JEventProcessor_HLDetectorTiming::~JEventProcessor_HLDetectorTiming()
{

}


//------------------
// CreateHistograms
//------------------
void JEventProcessor_HLDetectorTiming::CreateHistograms(string dirname)
{
    int nBins = 2000;
    float xMin = -500, xMax = 1500;
	int nStraws = 3522;
	int nTAGMColumns = 122; // 102 + 20 including 4 fully read out columns
	int nTAGHCounters = 274;
    int nPSCCounters = 16;
	int nTOFCounters = 176;
	int nSCCounters = 30;
	int nTPOLSectors = 32;
	unsigned int nTPOLsamples = 100; // CHECK ????

	char name [200];
	char title[500];

    // We want to plot the delta t at the target between the SC hit and the tagger hits
    // Some limits for these
    float nBinsE = 160, EMin = 3.0, EMax = 12.0;

	//TDirectory *curDir = gDirectory;
    TDirectory *newDir = gDirectory->mkdir(dirname.c_str());

    newDir->cd();
	newDir->mkdir("CDC")->cd();

	dCDCHitTimes[dirname] = new TH1F( "CDCHit time", "CDCHit time;t [ns];", nBins, xMin, xMax );
	dCDCHitTimesPerStraw[dirname] = new TH2F( "CDCHit time per Straw", "Hit time for each CDC wire; t [ns]; CCDB Index",
	                     					  750, -500, 1000, nStraws, 0.5, nStraws + 0.5);
	                     					  
    newDir->cd();
	newDir->mkdir("FDC")->cd();

	dFDCWireHitTimes[dirname] = new TH1F( "FDCHit Wire time", "FDCHit Wire time;t [ns];", nBins, xMin, xMax );
	dFDCWireModuleHitTimes[dirname] = new TH2F( "FDCHit Wire time vs. module", "FDCHit Wire time; module/slot; t [ns];",
	                     					    48, 0.5, 48.5, 400, -200, 600);
	dFDCCathodeHitTimes[dirname] = new TH1F( "FDCHit Cathode time", "FDCHit Cathode time;t [ns];", nBins, xMin, xMax );
			 
    newDir->cd();
	newDir->mkdir("SC")->cd();

	dSCHitTimes[dirname] = new TH1F( "SCHit time", "SCHit time;t [ns];", nBins, xMin, xMax );
	dSCADCHitTimes[dirname] = new TH1F( "SCHit ADC time", "SCHit ADC only time;t [ns];", nBins, xMin, xMax );
	dSCTDCHitTimes[dirname] = new TH1F( "SCHit TDC time", "SCHit TDC only time;t [ns];", nBins, xMin, xMax );
	dSCMatchedHitTimes[dirname] = new TH1F( "SCHit Matched time", "SCHit Matched ADC/TDC time;t [ns];", nBins, xMin, xMax );
	dSCADCTDCHitTimes[dirname] = new TH2F( "SCHit TDC_ADC Difference", "SC #Deltat TDC-ADC; Sector ;t_{TDC} - t_{ADC} [ns]",
	                     					    nSCCounters, 0.5, nSCCounters + 0.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF);
	dSCMatchedHitTimesPerSector[dirname] = new TH2F( "SCHit Matched time per Counter", "SCHit Matched ADC/TDC time; Sector ;t [ns]",
	                     					    nSCCounters, 0.5, nSCCounters + 0.5, 50, -50, 50);

    newDir->cd();
	newDir->mkdir("TOF")->cd();

	dTOFHitTimes[dirname] = new TH1F( "TOFHit time", "TOFHit time;t [ns];", nBins, xMin, xMax );
	dTOFADCHitTimes[dirname] = new TH1F( "TOFHit ADC time", "TOFHit ADC only time;t [ns];", nBins, xMin, xMax );
	dTOFTDCHitTimes[dirname] = new TH1F( "TOFHit TDC time", "TOFHit TDC only time;t [ns];", nBins, xMin, xMax );
	dTOFMatchedHitTimes[dirname] = new TH1F( "TOFHit Matched time", "TOFHit Matched ADC/TDC time;t [ns];", nBins, xMin, xMax );
	dTOFADCTDCHitTimes[dirname] = new TH2F( "TOFHit TDC_ADC Difference", "TOF #Deltat TDC-ADC; Sector ;t_{TDC} - t_{ADC} [ns]",
	                     					    nTOFCounters, 0.5, nTOFCounters + 0.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF);

    newDir->cd();
	newDir->mkdir("BCAL")->cd();

	dBCALADCHitTimes[dirname] = new TH1F( "BCALHit ADC time", "BCALHit ADC time;t [ns];", nBins, xMin, xMax );
	dBCALADCHitTimesUpstream[dirname] = new TH2F( "BCALHit Upstream Per Channel ADC Hit Time", 
							"BCALHit Upstream Per Channel Hit Time; cellID; t_{ADC} [ns]", 768, 0.5, 768.5, 250, -50, 50 );
	dBCALADCHitTimesDownstream[dirname] = new TH2F( "BCALHit Downstream Per Channel ADC Hit Time", 
							"BCALHit Downstream Per Channel Hit Time; cellID; t_{ADC} [ns]", 768, 0.5, 768.5, 250, -50, 50 );
	dBCALTDCHitTimes[dirname] = new TH1F( "BCALHit TDC time", "BCALHit TDC time;t [ns];", nBins, xMin, xMax );
	dBCALTDCHitTimesUpstream[dirname] = new TH2F( "BCALHit Upstream Per Channel TDC Hit Time", 
					"BCALHit Upstream Per Channel TDC Hit Time; cellID; t_{TDC} [ns]", 576, 0.5, 576.5, 350, -50, 300 );
	dBCALTDCHitTimesDownstream[dirname] = new TH2F( "BCALHit Downstream Per Channel TDC Hit Time", 
					"BCALHit Downstream Per Channel TDC Hit Time; cellID; t_{TDC} [ns]", 576, 0.5, 576.5, 350, -50, 300 );
	dBCALADCTDCHitTimesUpstream[dirname] = new TH2F( "BCALHit Upstream Per Channel TDC-ADC Hit Time", 
					"BCALHit Upstream Per Channel TDC-ADC Hit Time; cellID; t_{TDC} - t_{ADC} [ns]", 576, 0.5, 576.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF );
	dBCALADCTDCHitTimesDownstream[dirname] = new TH2F( "BCALHit Downstream Per Channel TDC-ADC Hit Time", 
					"BCALHit Downstream Per Channel TDC-ADC Hit Time; cellID; t_{TDC} - t_{ADC} [ns]", 576, 0.5, 576.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF );


    newDir->cd();
	newDir->mkdir("FCAL")->cd();

	dFCALTotalEnergy[dirname] = new TH1F( "FCAL total energy", "FCAL total energy;FCAL energy [GeV]", 400, 0, 8000 );
	dFCALHitTimes[dirname] = new TH1F( "FCALHit time", "FCALHit time;t [ns];", nBins, xMin, xMax );
	dFCALHitOccupancy[dirname] = new TH2F( "FCALHit Occupancy", "FCAL Hit Occupancy; column; row",
	                     				   61, -1.5, 59.5, 61, -1.5, 59.5);
	dFCALHitLocalTimes[dirname] = new TH2F( "FCALHit Local Time", "FCAL Hit Local Time [ns]; column; row",
	                     					61, -1.5, 59.5, 61, -1.5, 59.5);
	dFCALHitTimesPerChannel[dirname] = new TH2F( "FCALHit Per Channel Time", "FCALHit time;t [ns]; CCDB Index",
	                     					    2800, 0.5, 2800.+0.5, 250, -50, 50);

    newDir->cd();
	newDir->mkdir("ECAL")->cd();

	dECALTotalEnergy[dirname] = new TH1F( "ECAL total energy", "ECAL total energy;ECAL energy [GeV]", 400, 0, 8000 );
	dECALHitTimes[dirname] = new TH1F( "ECALHit time", "ECALHit time;t [ns];", nBins, xMin, xMax );
	dECALHitOccupancy[dirname] = new TH2F( "ECALHit Occupancy", "ECAL Hit Occupancy; column; row",
	                     				   42, -1.5, 40.5, 42, -1.5, 40.5);
	dECALHitLocalTimes[dirname] = new TH2F( "ECALHit Local Time", "ECAL Hit Local Time [ns]; column; row",
	                     					42, -1.5, 40.5, 42, -1.5, 40.5);
	dECALHitTimesPerChannel[dirname] = new TH2F( "ECALHit Per Channel Time", "ECALHit time;t [ns]; CCDB Index",
	                     					    1600, 0.5, 1600+0.5, 250, -50, 50);

    newDir->cd();
	newDir->mkdir("DIRC")->cd();

	dDIRCHitTimes[dirname] = new TH1F( "DIRCHit time", "DIRCHit time;t [ns];", nBins, xMin, xMax );
	dDIRCHitTimesPerChannelNorth[dirname] = new TH2F( "DIRCHit North Per Channel TDC Hit Time", 
													  "DIRCHit North Per Channel TDC Hit Time; channel ID; t_{TDC} [ns]",
	                     					    	  DDIRCPmtHit_factory::DIRC_MAX_CHANNELS, 0.5, 
								                      (double)DDIRCPmtHit_factory::DIRC_MAX_CHANNELS+0.5, 500, -50, 150);
	dDIRCHitTimesPerChannelSouth[dirname] = new TH2F( "DIRCHit South Per Channel TDC Hit Time", 
													  "DIRCHit South Per Channel TDC Hit Time; channel ID; t_{TDC} [ns]",
	                     					    	  DDIRCPmtHit_factory::DIRC_MAX_CHANNELS, 0.5, 
								                      (double)DDIRCPmtHit_factory::DIRC_MAX_CHANNELS+0.5, 500, -50, 150);

	dDIRCDeltaTimePerChannelNorth[dirname] = new TH2F( "DIRCHit North Per Channel t_{DIRC} - t_{track}", 
										"DIRCHit North Per Channel t_{DIRC} - t_{track}; channel ID; t_{DIRC} - t_{track} [ns]", 
										DDIRCPmtHit_factory::DIRC_MAX_CHANNELS, 0.5, 
													(double)DDIRCPmtHit_factory::DIRC_MAX_CHANNELS+0.5, 400, -50, 50 );
	dDIRCDeltaTimePerChannelSouth[dirname] = new TH2F( "DIRCHit South Per Channel t_{DIRC} - t_{track}", 
										"DIRCHit South Per Channel t_{DIRC} - t_{track}; channel ID; t_{DIRC} - t_{track} [ns]", 
										DDIRCPmtHit_factory::DIRC_MAX_CHANNELS, 0.5, 
													(double)DDIRCPmtHit_factory::DIRC_MAX_CHANNELS+0.5, 400, -50, 50);



    newDir->cd();
	newDir->mkdir("TPOL")->cd();

	dTPOLHitTimes[dirname] = new TH1F( "TPOLHit time", "TPOLHit time;t [ns];", nTPOLsamples+25,0.0,4.0*nTPOLsamples+100 );
	dTPOLHitTimesPerSector[dirname] = new TH2F( "TPOLHit time per Layer", "TPOLHit time;t [ns]; CCDB Index",
	                     					   nTPOLSectors,0.5,nTPOLSectors+0.5,nTPOLsamples+25,0.0,4.0*nTPOLsamples+100);

    newDir->cd();
	newDir->mkdir("FMWPC")->cd();

	dFMWPCHitTimes[dirname] = new TH1F( "FMWPCHit time", "FMWPCHit time;t [ns];", nBins, xMin, xMax );
	dFMWPCHitTimesPerLayer[dirname] = new TH2F( "FMWPCHit time per Layer", "FMWPCHit time;t [ns]; CCDB Index",
	                     					   nBins, xMin, xMax, 1152, 0.5, 1152.5);

    newDir->cd();
	newDir->mkdir("CTOF")->cd();

	dCTOFHitTimes[dirname] = new TH1F( "CTOFHit time", "CDCHit time;t [ns];", nBins, xMin, xMax );
	dCTOFHitTimesPerLayer[dirname] = new TH2F( "CTOFHit time per Layer", "CTOFHit time;t [ns]; CCDB Index",
	                     					  nBins, xMin, xMax, 16, 0.5, 16.5);

    newDir->cd();
	newDir->mkdir("PS")->cd();

	dPSHitTimes[dirname] = new TH1F( "PSHit time", "PSHit time;t [ns];", nBins, xMin, xMax );
	dPSHitTimesPerColumn[dirname] = new TH2F( "PSHit time per Column", "Hit time for each PS column; t [ns]; CCDB Index",
	                     					  nBins, xMin, xMax, 145*2, 0.5, 145.*2.+0.5);

    newDir->cd();
	newDir->mkdir("TAGH")->cd();

	dTAGHHitTimes[dirname] = new TH1F( "TAGHHit time", "TAGHHit time;t [ns];", nBins, xMin, xMax );
	dTAGHADCHitTimes[dirname] = new TH1F( "TAGHHit ADC time", "TAGHHit ADC only time;t [ns];", nBins, xMin, xMax );
	dTAGHTDCHitTimes[dirname] = new TH1F( "TAGHHit TDC time", "TAGHHit TDC only time;t [ns];", nBins, xMin, xMax );
	dTAGHMatchedHitTimes[dirname] = new TH1F( "TAGHHit Matched time", "TAGHHit Matched ADC/TDC time;t [ns];", nBins, xMin, xMax );
	dTAGHADCTDCHitTimes[dirname] = new TH2F( "TAGHHit TDC_ADC Difference", "TAGH #Deltat TDC-ADC; Sector ;t_{TDC} - t_{ADC} [ns]",
	                     					    nTAGHCounters, 0.5, nTAGHCounters + 0.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF);
	dTAGHMatchedHitTimesPerSector[dirname] = new TH2F( "TAGHHit Matched time per Counter", "TAGHHit Matched ADC/TDC time; Sector ;t [ns]",
	                     					    nTAGHCounters, 0.5, nTAGHCounters + 0.5, 50, -50, 50);

    newDir->cd();
	newDir->mkdir("TAGM")->cd();

	dTAGMHitTimes[dirname] = new TH1F( "TAGMHit time", "TAGMHit time;t [ns];", nBins, xMin, xMax );
	dTAGMADCHitTimes[dirname] = new TH1F( "TAGMHit ADC time", "TAGMHit ADC only time;t [ns];", nBins, xMin, xMax );
	dTAGMTDCHitTimes[dirname] = new TH1F( "TAGMHit TDC time", "TAGMHit TDC only time;t [ns];", nBins, xMin, xMax );
	dTAGMMatchedHitTimes[dirname] = new TH1F( "TAGMHit Matched time", "TAGMHit Matched ADC/TDC time;t [ns];", nBins, xMin, xMax );
	dTAGMADCTDCHitTimes[dirname] = new TH2F( "TAGMHit TDC_ADC Difference", "TAGM #Deltat TDC-ADC; Sector ;t_{TDC} - t_{ADC} [ns]",
	                     					    nTAGMColumns, 0.5, nTAGMColumns + 0.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF);
	dTAGMMatchedHitTimesPerSector[dirname] = new TH2F( "TAGMHit Matched time per Counter", "TAGMHit Matched ADC/TDC time; Sector ;t [ns]",
	                     					    nTAGMColumns, 0.5, nTAGMColumns + 0.5, 50, -50, 50);

    newDir->cd();
	newDir->mkdir("PSC")->cd();

	dPSCHitTimes[dirname] = new TH1F( "PSCHit time", "PSCHit time;t [ns];", nBins, xMin, xMax );
	dPSCADCHitTimes[dirname] = new TH1F( "PSCHit ADC time", "PSCHit ADC only time;t [ns];", nBins, xMin, xMax );
	dPSCTDCHitTimes[dirname] = new TH1F( "PSCHit TDC time", "PSCHit TDC only time;t [ns];", nBins, xMin, xMax );
	dPSCMatchedHitTimes[dirname] = new TH1F( "PSCHit Matched time", "PSCHit Matched ADC/TDC time;t [ns];", nBins, xMin, xMax );
	dPSCADCTDCHitTimes[dirname] = new TH2F( "PSCHit TDC_ADC Difference", "PSC #Deltat TDC-ADC; Sector ;t_{TDC} - t_{ADC} [ns]",
	                     					    nPSCCounters, 0.5, nPSCCounters + 0.5, NBINS_TDIFF, MIN_TDIFF, MAX_TDIFF);
	dPSCMatchedHitTimesPerSector[dirname] = new TH2F( "PSCHit Matched time per Counter", "PSCHit Matched ADC/TDC time; Sector ;t [ns]",
	                     					    nPSCCounters, 0.5, nPSCCounters + 0.5, 50, -50, 50);


    newDir->cd();
	newDir->mkdir("CCAL")->cd();

	dCCALHitTimes[dirname] = new TH1F( "CCALHit time", "CCALHit time;t [ns];", nBins, xMin, xMax );
	dCCALHitOccupancy[dirname] = new TH2F( "CCALHit Occupancy", "CCAL Hit Occupancy; column; row",
	                     				   13, -1.5, 11.5, 13, -1.5, 11.5);
	dCCALHitLocalTimes[dirname] = new TH2F( "CCALHit Local Time", "CCAL Hit Local Time [ns]; column; row",
	                     					13, -1.5, 11.5, 13, -1.5, 11.5);
	dCCALHitTimesPerChannel[dirname] = new TH2F( "CCALHit Per Channel Time", "CCALHit time;t [ns]; CCDB Index",
	                     					    150, 0.5, 150+0.5, 13, -1.5, 11.5); 


	if(DO_OPTIONAL) {
		newDir->cd();
		newDir->mkdir("TAGH_ADC_RF_Compare")->cd();
	
		for(int i=0; i<nTAGHCounters; i++) {
            sprintf(name, "TAGH Counter ID %.3i", i);
            sprintf(title, "TAGH Counter ID %i t_{ADC} - t_{RF}; t_{ADC} - t_{RF} [ns]; Entries", i);
			dTAGHADCRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );			
		}
	}
	
    newDir->cd();
	newDir->mkdir("TAGH_TDC_RF_Compare")->cd();

	for(int i=0; i<nTAGHCounters; i++) {
		sprintf(name, "TAGH Counter ID %.3i", i);
		sprintf(title, "TAGH Counter ID %i t_{TDC} - t_{RF}; t_{TDC} - t_{RF} [ns]; Entries", i);
		dTAGHTDCRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );			
	}

	if(DO_OPTIONAL) {
		newDir->cd();
		newDir->mkdir("TAGM_ADC_RF_Compare")->cd();
	
		for(int column=0,row=0; column<102; column++) {
            sprintf(name, "TAGM Column %.3i Row %.1i", column, row);
            sprintf(title, "TAGM Column %.3i Row %.1i t_{ADC} - t_{RF}; t_{ADC} - t_{RF} [ns]; Entries", column, row);
			dTAGMADCRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );
			
			if( (column==9) || (column==27) || (column==81) || (column==99) ) {
				for(column=1; column<5; column++) {
					sprintf(name, "TAGM Column %.3i Row %.1i", column, row);
					sprintf(title, "TAGM Column %.3i Row %.1i t_{ADC} - t_{RF}; t_{ADC} - t_{RF} [ns]; Entries", column, row);
					dTAGMADCRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );
				}
				column = 0;
			}
		}
	}
	
    newDir->cd();
	newDir->mkdir("TAGM_TDC_RF_Compare")->cd();

	for(int column=0,row=0; column<102; column++) {
		sprintf(name, "TAGM Column %.3i Row %.1i", column, row);
		sprintf(title, "TAGM Column %.3i Row %.1i t_{TDC} - t_{RF}; t_{TDC} - t_{RF} [ns]; Entries", column, row);
		dTAGMTDCRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );
	
		if( (column==9) || (column==27) || (column==81) || (column==99) ) {
			for(row=1; row<=5; row++) {
				sprintf(name, "TAGM Column %.3i Row %.1i", column, row);
				sprintf(title, "TAGM Column %.3i Row %.1i t_{TDC} - t_{RF}; t_{TDC} - t_{RF} [ns]; Entries", column, row);
				dTAGMTDCRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );
			}
			row = 0;
		}
	}

    newDir->cd();
	newDir->mkdir("TRACKING")->cd();

	dTaggerRFTime[dirname] = new TH1F( "Tagger - RFBunch 1D Time", "Tagger - RFBunch Time; #Deltat_{Tagger - RFBunch} [ns]; Entries", 480, -30, 30 );
	dTAGHRFTime[dirname] = new TH1F( "TAGH - RFBunch 1D Time", "TAGH - RFBunch Time; #Deltat_{TAGH - RFBunch} [ns]; Entries", 480, -30, 30 );
	dTAGMRFTime[dirname] = new TH1F( "TAGM - RFBunch 1D Time", "TAGM - RFBunch Time; #Deltat_{TAGM - RFBunch} [ns]; Entries", 480, -30, 30 );
	dTaggerRFEnergyTime[dirname] = new TH2F( "Tagger - RFBunch Time", "Tagger - RFBunch Time; #Deltat_{Tagger - RFBunch} [ns]; Energy [GeV]",
										NBINS_TAGGER_TIME,MIN_TAGGER_TIME,MAX_TAGGER_TIME, nBinsE, EMin, EMax );
	dTAGHRFCounterTime[dirname] = new TH2F( "TAGH - RFBunch Time", "#Deltat TAGH-RFBunch; Counter ID ;t_{TAGH} - t_{RFBunch} [ns]", 
										nTAGHCounters, 0.5, nTAGHCounters + 0.5, NBINS_TAGGER_TIME,MIN_TAGGER_TIME,MAX_TAGGER_TIME );
	dTAGMRFCounterTime[dirname] = new TH2F( "TAGM - RFBunch Time", "#Deltat TAGM-RFBunch; Counter ID ;t_{TAGH} - t_{RFBunch} [ns]", 
										nTAGMColumns, 0.5, nTAGMColumns + 0.5, NBINS_TAGGER_TIME,MIN_TAGGER_TIME,MAX_TAGGER_TIME );
	dTaggerSCTime[dirname] = new TH1F( "Tagger - SC 1D Target Time", "Tagger - SC Time at Target; #Deltat_{Tagger - SC} [ns]; Entries", 480, -30, 30 );
	dTaggerSCEnergyTime[dirname] = new TH2F( "Tagger - SC Target Time", "Tagger - SC Target Time; #Deltat_{Tagger - SC} [ns]; Energy [GeV]",
										NBINS_TAGGER_TIME,MIN_TAGGER_TIME,MAX_TAGGER_TIME, nBinsE, EMin, EMax );
	dTAGHSCCounterTime[dirname] = new TH2F( "TAGH - SC Target Time", "#Deltat TAGH-SC; Column ;t_{TAGM} - t_{SC @ target} [ns]", 
										nTAGHCounters, 0.5, nTAGHCounters + 0.5, NBINS_TAGGER_TIME,MIN_TAGGER_TIME,MAX_TAGGER_TIME );
	dTAGMSCCounterTime[dirname] = new TH2F( "TAGM - SC Target Time", "#Deltat TAGM-SC; Column ;t_{TAGM} - t_{SC @ target} [ns]", 
										nTAGMColumns, 0.5, nTAGMColumns + 0.5, NBINS_TAGGER_TIME,MIN_TAGGER_TIME,MAX_TAGGER_TIME );
	

	dBCALShowerRFTime_NoTracks[dirname] = new TH1F( "BCAL - RF Time  (Neutral)", 
										"t_{BCAL} - t_{RF} at Target (Neutral); t_{BCAL} - t_{RF} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dBCALShowerRFTimeVsEnergy_NoTracks[dirname] = new TH2F( "BCAL - RF Time vs. Energy (Neutral)", 
										"Shower Energy [GeV]; t_{BCAL} - t_{RF} at Target (Neutral); t_{BCAL} - t_{RF} [ns]; Entries", 
										100, 0., 10., NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dFCALShowerRFTime_NoTracks[dirname] = new TH1F( "FCAL - RF Time (Neutral)", 
										"t_{FCAL} - t_{RF} at Target (Neutral); t_{FCAL} - t_{RF} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dFCALShowerRFTimeVsEnergy_NoTracks[dirname] = new TH2F( "FCAL - RF Time vs. Energy (Neutral)", 
										"Shower Energy [GeV]; t_{FCAL} - t_{RF} at Target (Neutral); t_{FCAL} - t_{RF} [ns]; Entries", 
										100, 0., 10., NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dECALShowerRFTime_NoTracks[dirname] = new TH1F( "ECAL - RF Time (Neutral)", 
										"t_{ECAL} - t_{RF} at Target (Neutral); t_{ECAL} - t_{RF} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dECALShowerRFTimeVsEnergy_NoTracks[dirname] = new TH2F( "ECAL - RF Time vs. Energy (Neutral)", 
										"Shower Energy [GeV]; t_{ECAL} - t_{RF} at Target (Neutral); t_{ECAL} - t_{RF} [ns]; Entries", 
										100, 0., 10., NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dCCALShowerRFTime_NoTracks[dirname] = new TH1F( "CCAL - RF Time vs. Energy (Neutral)", 
										"t_{CCAL} - t_{RF} at Target (Neutral); t_{CCAL} - t_{RF} [ns]; Entries", 
										2000, -50, 50 );
	dCCALShowerRFTimeVsEnergy_NoTracks[dirname] = new TH2F( "CCAL - RF Time (Neutral)", 
										"Shower Energy [GeV]; t_{CCAL} - t_{RF} at Target (Neutral); t_{CCAL} - t_{RF} [ns]; Entries", 
										100, 0., 10., 500, -20, 20 );
	dTOFShowerRFTime_NoTracks[dirname] = new TH1F( "TOF - RF Time (No Tracks)", 
										"t_{TOF} - t_{RF} at Target (No Tracks); t_{TOF} - t_{RF} at Target [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );

	dSCRFTime_AllHits[dirname] = new TH1F( "SC - RF Time (all)", "t_{SC} - t_{RF} at Target; t_{SC} - t_{RF} at Target [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dSCRFTime[dirname] = new TH1F( "SC - RF Time", "t_{SC} - t_{RF} at Target; t_{SC} - t_{RF} at Target [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dSCRFTimeVsSector[dirname] = new TH2F( "SC - RF Time vs. Sector", "t_{SC} - t_{RF} at Target; Sector; t_{SC} - t_{RF} at Target [ns];", 
										30, 0.5, 30.5, 800, -20., 20. );
// 	dBCALShowerRFTime_NoTracks[dirname] = new TH1F( "BCAL - RF Time vs. Energy (Neutral)", 
// 										"t_{BCAL} - t_{RF} at Target (Neutral); t_{BCAL} - t_{RF} [ns]; Entries", 
// 										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dCDCSCTime[dirname] = new TH1F( "Earliest CDC Time Minus Matched SC Time", "Earliest CDC Time Minus Matched SC Time; t_{CDC} - t_{SC} [ns];", 
										400, -50, 150 );
	dCDCBCALTime[dirname] = new TH1F( "Earliest CDC Time Minus Matched BCAL Time", "Earliest CDC Time Minus Matched BCAL Time; t_{CDC} - t_{BCAL} [ns];", 
										400, -50, 150 );
	dTOFRFTime[dirname] = new TH1F( "TOF - RF Time", 
										"t_{TOF} - t_{RF} at Target; t_{TOF} - t_{RF} at Target [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );

	//
	dTOFRFTimeVSBCALRFTime[dirname] = new TH2F( "TOF - RF Time VS BCAL - RF time", 
					"t_{TOF} - t_{RF} VS t_{BCAL} - t_{RF} at Target; t_{BCAL} - t_{RF} at Target [ns]; t_{TOF} - t_{RF} at Target [ns];", 
						    NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T, NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dTOFRFTimeVSFCALRFTime[dirname] = new TH2F( "TOF - RF Time VS FCAL - RF time", 
					"t_{TOF} - t_{RF} VS t_{FCAL} - t_{RF} at Target; t_{FCAL} - t_{RF} at Target [ns]; t_{TOF} - t_{RF} at Target [ns];", 
						    NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T, NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dTOFRFTimeVSECALRFTime[dirname] = new TH2F( "TOF - RF Time VS ECAL - RF time", 
					"t_{TOF} - t_{RF} VS t_{ECAL} - t_{RF} at Target; t_{ECAL} - t_{RF} at Target [ns]; t_{TOF} - t_{RF} at Target [ns];", 
						    NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T, NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	//
	



	dTOFSCTime[dirname] = new TH1F( "TOF - SC Target Time", 
										"t_{TOF} - t_{SC} at Target; t_{TOF} - t_{SC} at Target [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dEarliestCDCTime[dirname] = new TH1F( "Earliest Flight-time Corrected CDC Time", 
										"Earliest Flight-time corrected CDC Time; t_{CDC} [ns];", 
										200, -50, 150 );
	dEarliestFDCTime[dirname] = new TH1F( "Earliest Flight-time Corrected FDC Time", 
										"Earliest Flight-time corrected FDC Time; t_{FDC} [ns];", 
										200, -50, 150 );
	dBCALShowerRFTime[dirname] = new TH1F( "BCAL - RF Time", "t_{BCAL} - t_{RF} at Target; t_{BCAL} - t_{RF} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dBCALShowerSCTime[dirname] = new TH1F( "BCAL - SC Target Time", "t_{BCAL} - t_{SC} at Target; t_{BCAL} - t_{SC} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dBCALShowerSCTimeVsCorrection[dirname] = new TH2F( "BCAL - SC Target Time Vs Correction", 
										"t_{BCAL} - t_{SC} at Target; Flight time [ns]; t_{BCAL} - t_{SC} [ns]", 
										100, 0, 20, 50, -10, 10 );
	dFCALShowerRFTime[dirname] = new TH1F( "FCAL - RF Time", "t_{FCAL} - t_{RF} at Target; t_{FCAL} - t_{RF} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dFCALShowerSCTime[dirname] = new TH1F( "FCAL - SC Target Time", "t_{FCAL} - t_{SC} at Target; t_{FCAL} - t_{SC} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dECALShowerRFTime[dirname] = new TH1F( "ECAL - RF Time", "t_{ECAL} - t_{RF} at Target; t_{ECAL} - t_{RF} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );
	dECALShowerSCTime[dirname] = new TH1F( "ECAL - SC Target Time", "t_{ECAL} - t_{SC} at Target; t_{ECAL} - t_{SC} [ns]; Entries", 
										NBINS_MATCHING, MIN_MATCHING_T, MAX_MATCHING_T );


    newDir->cd();
	newDir->mkdir("SC_Target_RF_Compare")->cd();

	for(int i=0; i<nSCCounters; i++) {
		sprintf(name, "Sector %.2i", i);
		sprintf(title, "SC Sector %i t_{Target} - t_{RF}; t_{Target} - t_{RF} [ns]; Entries", i);
		dSCTargetRFCompareTimes[dirname].push_back( new TH1F(name, title, NBINS_RF_COMPARE, MIN_RF_COMPARE, MAX_RF_COMPARE) );			
	}




}


//------------------
// Init
//------------------
void JEventProcessor_HLDetectorTiming::Init()
{
    auto app = GetApplication();
    lockService = GetLockService(app);

    BEAM_CURRENT = 50; // Assume that there is beam until first EPICs event. Set from EPICS evio data, can override on command line

    fBeamEventCounter = 0;
    dMaxDIRCChannels = 108*64;

    REQUIRE_BEAM = 0;
    BEAM_EVENTS_TO_KEEP = 1000000000; // Set enormously high
    DO_ROUGH_TIMING = 0;
    DO_CDC_TIMING = 0;
    DO_TDC_ADC_ALIGN = 0;
    DO_TRACK_BASED = 0;
    DO_VERIFY = 1;
    DO_OPTIONAL = 0;
    DO_REACTION = 0;
    DO_HIGH_RESOLUTION = 0;

    USE_RF_BUNCH = 1;
    TRIGGER_MASK = 0;
    NO_START_COUNTER = 0;

	INCLUDE_ALL_TRIGGERS = false;
	INCLUDE_PS_TRIGGERS = false;
	PRIMEX_TRIGGERS = false;
	CPP_TRIGGERS = false;

    NO_TRACKS = false;
    NO_FIELD = true;
    CCAL_CALIB = false;
    STRAIGHT_TRACK = false;

    if(app){
        app->SetDefaultParameter("HLDETECTORTIMING:DO_ROUGH_TIMING", DO_ROUGH_TIMING, "Set to > 0 to do rough timing of all detectors");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_CDC_TIMING", DO_CDC_TIMING, "Set to > 0 to do CDC Per channel Alignment");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_TDC_ADC_ALIGN", DO_TDC_ADC_ALIGN, "Set to > 0 to do TDC/ADC alignment of SC,TOF,TAGM,TAGH");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_TRACK_BASED", DO_TRACK_BASED, "Set to > 0 to do Track Based timing corrections");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_HIGH_RESOLUTION", DO_HIGH_RESOLUTION, "Set to > 0 to increase the resolution of the track Based timing corrections");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_VERIFY", DO_VERIFY, "Set to > 0 to verify timing with current constants");
        app->SetDefaultParameter("HLDETECTORTIMING:REQUIRE_BEAM", REQUIRE_BEAM, "Set to 0 to skip beam current check");
        app->SetDefaultParameter("HLDETECTORTIMING:BEAM_EVENTS_TO_KEEP", BEAM_EVENTS_TO_KEEP, "Set to the number of beam on events to use");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_OPTIONAL", DO_OPTIONAL, "Set to >0 to enable optional histograms ");
        app->SetDefaultParameter("HLDETECTORTIMING:DO_REACTION", DO_REACTION, "Set to >0 to run DReaction");
        app->SetDefaultParameter("HLDETECTORTIMING:USE_RF_BUNCH", USE_RF_BUNCH, "Set to 0 to disable use of 2 vote RF Bunch");
        app->SetDefaultParameter("HLDETECTORTIMING:NO_TRACKS", NO_TRACKS, "Don't use tracking information for timing calibrations");
        app->SetDefaultParameter("HLDETECTORTIMING:CCAL_CALIB", CCAL_CALIB, "Perform CCAL calibrations");
        app->SetDefaultParameter("HLDETECTORTIMING:TRIGGER_MASK", TRIGGER_MASK, "Set to >0 to override use of standard physics trigger");
        app->SetDefaultParameter("HLDETECTORTIMING:STRAIGHT_TRACK", STRAIGHT_TRACK, "Set to >0 to change better for straight track data (field-off, drift chambers-on)");
        app->SetDefaultParameter("HLDETECTORTIMING:NO_START_COUNTER", NO_START_COUNTER, "Set to >0 to disable the use of the start counter (e.g. for the CPP experiment)");

        app->SetDefaultParameter("HLDETECTORTIMING:INCLUDE_ALL_TRIGGERS", INCLUDE_ALL_TRIGGERS, "Set to >0 to disable the use of the start counter (e.g. for the CPP experiment)");
        app->SetDefaultParameter("HLDETECTORTIMING:INCLUDE_PS_TRIGGERS", INCLUDE_PS_TRIGGERS, "Set to >0 to disable the use of the start counter (e.g. for the CPP experiment)");
        app->SetDefaultParameter("HLDETECTORTIMING:PRIMEX_TRIGGERS", PRIMEX_TRIGGERS, "Set to >0 to disable the use of the start counter (e.g. for the CPP experiment)");
        app->SetDefaultParameter("HLDETECTORTIMING:CPP_TRIGGERS", CPP_TRIGGERS, "Set to >0 to disable the use of the start counter (e.g. for the CPP experiment)");
    }

    // Would like the code with no arguments to simply verify the current status of the calibration
    if (DO_ROUGH_TIMING > 0 || DO_CDC_TIMING > 0 || DO_TDC_ADC_ALIGN > 0 || DO_TRACK_BASED > 0) DO_VERIFY = 0;

    if(NO_FIELD)  CCAL_CALIB = true;  // HM

    // Increase range for initial search
    if(DO_TDC_ADC_ALIGN){
        NBINS_TDIFF = 2800; MIN_TDIFF = -150.0; MAX_TDIFF = 550.0;
    }
    else{
        NBINS_TDIFF = 200; MIN_TDIFF = -40.0; MAX_TDIFF = 40.0;
    }

    // DEBUG
    //NBINS_TDIFF = 2800; MIN_TDIFF = -200.0; MAX_TDIFF = 500.0;


    if (DO_TRACK_BASED){
        if (DO_HIGH_RESOLUTION) {
	    NBINS_TAGGER_TIME = 400; MIN_TAGGER_TIME = -20; MAX_TAGGER_TIME = 20;
	    NBINS_MATCHING = 1000; MIN_MATCHING_T = -10; MAX_MATCHING_T = 10;
	} else {
	    NBINS_TAGGER_TIME = 1600; MIN_TAGGER_TIME = -200; MAX_TAGGER_TIME = 400;
	    //NBINS_MATCHING = 1000; MIN_MATCHING_T = -100; MAX_MATCHING_T = 400;
	    NBINS_MATCHING = 800; MIN_MATCHING_T = -100; MAX_MATCHING_T = 100;
	}
    } else if (DO_VERIFY){
        NBINS_TAGGER_TIME = 200; MIN_TAGGER_TIME = -20; MAX_TAGGER_TIME = 20;
        NBINS_MATCHING = 1000; MIN_MATCHING_T = -10; MAX_MATCHING_T = 10;
    } else{
        NBINS_TAGGER_TIME = 100; MIN_TAGGER_TIME = -50; MAX_TAGGER_TIME = 50;
        NBINS_MATCHING = 100; MIN_MATCHING_T = -10; MAX_MATCHING_T = 10;
    }

    NBINS_RF_COMPARE = 200; MIN_RF_COMPARE = -2.2; MAX_RF_COMPARE = 2.2;


	// we define here different cut functions which select out different classes of events
	// all histograms will be created for each different class of events
	// this was originally written to select out events from different
	// trigger types, but the sky is the limit!
	if(INCLUDE_ALL_TRIGGERS) {
		dCutFunctions["All Events"] = [](const std::shared_ptr<const JEvent>& event) { return true; };
	}
	dCutFunctions["Physics Triggers"] = [](const std::shared_ptr<const JEvent>& event) { 
			const DTrigger* locTrigger = NULL; 
    		event->GetSingle(locTrigger); 
			if(!locTrigger->Get_IsPhysicsEvent()) return false; else return true; 
	};
	if(INCLUDE_PS_TRIGGERS) {
		dCutFunctions["PS Triggers"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (1 << 3)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
	}
	if(PRIMEX_TRIGGERS) {
		// currently based on 2022-08 run period
		// see, e.g. https://logbooks.jlab.org/entry/4039521
		// note that the "physics trigger" in this case is the CCAL + FCAL trigger
		dCutFunctions["CCAL+FCAL"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (0x1)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
		dCutFunctions["FCAL (E>3.5 GeV)"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (1 << 1)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
		dCutFunctions["FCAL (E>0.5 GeV)"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (1 << 2)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
		dCutFunctions["CCAL"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (1 << 10)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};

	}
	if(CPP_TRIGGERS) {
		// based on ???
		dCutFunctions["FCAL"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (0x1)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
		dCutFunctions["FCAL+BCAL"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (1 << 1)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
		dCutFunctions["TOF"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = (locTrigger->Get_L1TriggerBits() & (1 << 2)) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};
		dCutFunctions["FCAL+CTOF"] = [](const std::shared_ptr<const JEvent>& event) { 
				const DTrigger* locTrigger = NULL; 
				event->GetSingle(locTrigger); 
				int trig_bit = ((locTrigger->Get_L1TriggerBits() & (0x1)) && (locTrigger->Get_L1FrontPanelTriggerBits() & (1 << 5))) ? 1 : 0;
				if(locTrigger->Get_L1FrontPanelTriggerBits() == 0 && trig_bit) return true; else return false; 
		};

	}
	
	// Fill histograms in different directories
	TDirectory *mainDir = gDirectory;
    TDirectory *timingDir = gDirectory->mkdir("HLDetectorTiming");
    timingDir->cd();

	// book event-wide histograms for monitoring
	
	dHistBeamCurrent = new TH1F("Beam Current", "Beam Current; Beam Current [nA]; Entries", 100, 0, 200);
	dHistBeamEvents = new TH1F("Beam Events", "Beam On Events (0 = no beam, 1 = beam > 10nA)", 2, -0.5, 1.5);

	// book the histograms for each class of events
	/** C++17
    for (const auto& [key, ignore] : dCutFunctions) {
		CreateHistograms(key);
		timingDir->cd();
	}
	**/
	
	for (auto const& cut : dCutFunctions) {
		const string &key = cut.first;
		CreateHistograms(key);
		timingDir->cd();
	}


    // back to main dir
    mainDir->cd();


    return;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_HLDetectorTiming::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
    DGeometry* geom = GetDGeometry(event);
    geom->GetTargetZ(Z_TARGET);

// 	if(dCutFunctions.size() == 0) {
// 	}

    return;
}

//------------------
// Process
//------------------
void JEventProcessor_HLDetectorTiming::Process(const std::shared_ptr<const JEvent>& event)
{

   //DApplication* app = dynamic_cast<DApplication*>(loop->GetJApplication());
   //   DGeometry* geom = app->GetDGeometry(loop->GetJEvent().GetRunNumber());
   
   // Check for magnetic field
   const DMagneticFieldMap *bfield = GetBfield(event);
   bool locIsNoFieldFlag = (dynamic_cast<const DMagneticFieldMapNoField*>(bfield) != NULL);

    const DTrigger* locTrigger = NULL; 
    event->GetSingle(locTrigger);
    
    // make sure no "special" front-panel trigger events are used (e.g. LED, random pulser...)
    if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) 
      return;

	/*    
	// allow the user to select which trigger select events to use for calibrations
	if( TRIGGER_MASK > 0) {
	    if( !((locTrigger->Get_L1TriggerBits())&TRIGGER_MASK) )
        	return; // NOERROR;
	} else {
		// but default to the main physics trigger
    	if(!locTrigger->Get_IsPhysicsEvent())
	    	return; // NOERROR;
	}
	*/
	
	// COMMENT
	map<string, bool> passed_cuts;
	for (auto const& cut : dCutFunctions) {
		const string &key = cut.first;
		passed_cuts[key] = cut.second(event);
	}
	
    // Get the particleID object for each run
    vector<const DParticleID *> locParticleID_algos;
    event->Get(locParticleID_algos);
    if(locParticleID_algos.size()<1){
        _DBG_<<"Unable to get a DParticleID object! NO PID will be done!"<<endl;
        return;
    }
    auto locParticleID = locParticleID_algos[0];

    // We want to be use some of the tools available in the RFTime factory 
    // Specifically steping the RF back to a chosen time
    vector<const DRFTime *> locRFTimes;
    event->Get(locRFTimes);      // make sure brun() gets called for this factory!

//     vector<const DDIRCGeometry*> locDIRCGeometryVec;
//     loop->Get(locDIRCGeometryVec);
    // next line commented out to supress warning
    //    const DDIRCGeometry* locDIRCGeometry = locDIRCGeometryVec[0];

    // Initialize DIRC LUT
	const DDIRCLut* dDIRCLut = nullptr;
    event->GetSingle(dDIRCLut);

    // Get the EPICs events and update beam current. Skip event if current too low (<10 nA).
    vector<const DEPICSvalue *> epicsValues;
    event->Get(epicsValues);
    
	DEvent::GetLockService(event)->RootFillLock(this); //ACQUIRE ROOT LOCK!!
	
    for(unsigned int j = 0; j < epicsValues.size(); j++){
        const DEPICSvalue *thisValue = epicsValues[j];
        if (strcmp((thisValue->name).c_str(), "IBCAD00CRCUR6") == 0){
            BEAM_CURRENT = thisValue->fval;
            dHistBeamCurrent->Fill(BEAM_CURRENT);
        }
    }

    // There is a caveat here when running multithreaded
    // Another thread might be the one to catch the EPICS event
    // and there is no way to reject events that may have come from earilier
    // Expect number of entries in histograms to vary slightly over the same file with many threads
    if (BEAM_CURRENT < 10.0) {
    	dHistBeamEvents->Fill(0);
        if (REQUIRE_BEAM){
			DEvent::GetLockService(event)->RootFillUnLock(this); //RELEASE ROOT LOCK!!
            return; // Skip events where we can't verify the beam current
        }
    }
    else{
    	dHistBeamEvents->Fill(1);
        fBeamEventCounter++;
    }
	DEvent::GetLockService(event)->RootFillUnLock(this); //RELEASE ROOT LOCK!!

    if (fBeamEventCounter >= BEAM_EVENTS_TO_KEEP) { // Able to specify beam ON events instead of just events
        cout<< "Maximum number of Beam Events reached" << endl;
        japp->Quit();
        return;
    }

    
    // Get the objects from the eveevent
    vector<const DCDCHit *> cdcHitVector;
    vector<const DFDCHit *> fdcHitVector;
    vector<const DSCHit *> scHitVector;
    vector<const DBCALUnifiedHit *> bcalUnifiedHitVector;
    vector<const DTOFHit *> tofHitVector;
    vector<const DTOFPoint *> tofPointVector;
    vector<const DFCALHit *> fcalHitVector;
    vector<const DECALHit *> ecalHitVector;
    vector<const DCCALHit *> ccalHitVector;
    vector<const DDIRCPmtHit *> dircPmtHitVector;
    vector<const DTAGMHit *> tagmHitVector;
    vector<const DTAGHHit *> taghHitVector;
    vector<const DPSHit *> psHitVector;
    vector<const DPSCHit *> pscHitVector;
    vector<const DFMWPCHit *> fmwpcHitVector;
    vector<const DCTOFHit *> ctofHitVector;
    vector<const DTPOLHit *> tpolHitVector;

    event->Get(cdcHitVector);
    event->Get(fdcHitVector);
    event->Get(scHitVector);
    event->Get(bcalUnifiedHitVector);
    event->Get(tofHitVector);
    event->Get(tofPointVector);
    event->Get(fcalHitVector);
    event->Get(ecalHitVector);
    if(CCAL_CALIB) {
      event->Get(ccalHitVector);
    }
    event->Get(dircPmtHitVector);
    event->Get(psHitVector);
    event->Get(pscHitVector);
    event->Get(tagmHitVector, "Calib");
    event->Get(taghHitVector, "Calib");
    event->Get(fmwpcHitVector);
    event->Get(ctofHitVector);
    event->Get(tpolHitVector);

    vector<const DNeutralShower *> neutralShowerVector;
    event->Get(neutralShowerVector);
    vector<const DCCALShower *> ccalShowerVector;
    event->Get(ccalShowerVector);
    vector<const DChargedTrack *> chargedTrackVector;
    event->Get(chargedTrackVector);

    
   bool ECAL_EXISTS = true;

	// extract the FCAL Geometry
	vector<const DFCALGeometry*> fcalGeomVect;
	event->Get( fcalGeomVect );
	if (fcalGeomVect.size() < 1){
        cout << "FCAL Geometry not available?" << endl;
        return; //OBJECT_NOT_AVAILABLE;
	}
	const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);

	// extract the ECAL Geometry
	vector<const DECALGeometry*> ecalGeomVect;
	event->Get( ecalGeomVect );
	static bool print_messages = true;
	if (ecalGeomVect.size() < 1){
		if(print_messages)
        	jerr << "HLDetectorTiming: ECAL Geometry not available?" << endl;
        print_messages = false;
        
        ECAL_EXISTS = false;
        //return; //OBJECT_NOT_AVAILABLE;
	}

    // TTabUtilities object used for RF time conversion
    const DTTabUtilities* locTTabUtilities = NULL;
    event->GetSingle(locTTabUtilities);

    unsigned int i = 0;

	// retrieve RF bunch information
	const DEventRFBunch *thisRFBunch = NULL;
    
    if(NO_TRACKS) {
	    // If the drift chambers are turned off, we'll need to use the neutral showers to choose the RF
	    event->GetSingle(thisRFBunch, "CalorimeterOnly");
    } else {
        if(NO_START_COUNTER) {
		    event->GetSingle(thisRFBunch);   // if there's no start counter, then use the normal RF times 
		} else {
	    	event->GetSingle(thisRFBunch, "Calibrations"); // SC only hits
	    }
    }


	DEvent::GetLockService(event)->RootFillLock(this); //ACQUIRE ROOT LOCK!!
	// Start by filling individual hit times
	// 
	// The detectors with both TDCs and ADCs need these two to be aligned
    // These detectors are the SC,TAGM,TAGH,TOF,PSC

    for (i = 0; i < cdcHitVector.size(); i++) {
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
		
			dCDCHitTimes[key]->Fill(cdcHitVector[i]->t);
			if(DO_VERIFY || DO_CDC_TIMING){
				dCDCHitTimesPerStraw[key]->Fill(cdcHitVector[i]->t, GetCCDBIndexCDC(cdcHitVector[i]));
			}
		}
	}
    
    for (i = 0; i < fdcHitVector.size(); i++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			if(fdcHitVector[i]->type == 0 ) {
				dFDCWireHitTimes[key]->Fill(fdcHitVector[i]->t);

				// Keep track of module/crate level shifts
				// two F1TDC modules per wire layer
				int module = 2 * fdcHitVector[i]->gLayer - 1;  // layers start counting at 1
				if(fdcHitVector[i]->element > 48)
					module++;
					
				dFDCWireModuleHitTimes[key]->Fill(module, fdcHitVector[i]->t);
			}
			else{
				dFDCCathodeHitTimes[key]->Fill(fdcHitVector[i]->t);
			}
		}
	}
	    
    for (unsigned int j = 0; j < tpolHitVector.size(); j++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			if (tpolHitVector[j]->w_samp1 > 160.0 || tpolHitVector[j]->pulse_peak < 60.0) continue;  // pick good events
			//unsigned int nsamples = tpolHitVector[j]->nsamples;
			
			dTPOLHitTimes[key]->Fill(tpolHitVector[j]->t_proxy);
			dTPOLHitTimesPerSector[key]->Fill(tpolHitVector[j]->sector,tpolHitVector[j]->t_proxy);
		}
	}

	if(!NO_START_COUNTER) {
		for (i = 0; i < scHitVector.size(); i++) {
			for (auto const& cut : passed_cuts) {
				const string &key = cut.first;
				bool passed = cut.second;
				if(!passed) continue;

				//if(!scHitVector[i]->has_fADC || !scHitVector[i]->has_TDC) continue;
				dSCHitTimes[key]->Fill(scHitVector[i]->t);
			
				// Break these histograms up into hits coming from the TDC and hits coming from the ADC
				const DSCHit *thisSCHit = scHitVector[i];
				if (thisSCHit->has_fADC && !thisSCHit->has_TDC){
					dSCADCHitTimes[key]->Fill(scHitVector[i]->t_fADC);
							
					// Manual loop over hits to match out of time
					for (auto hit = scHitVector.begin(); hit != scHitVector.end(); hit++){
						if ((*hit)->has_TDC && !(*hit)->has_fADC){
							if (scHitVector[i]->sector == (*hit)->sector){
								dSCADCTDCHitTimes[key]->Fill(scHitVector[i]->sector, 
															 scHitVector[i]->t_TDC - scHitVector[i]->t_fADC);
							}
						}
					}
				}
				else if (!thisSCHit->has_fADC && thisSCHit->has_TDC){
					dSCTDCHitTimes[key]->Fill(scHitVector[i]->t_TDC);
				}
				else{
					dSCMatchedHitTimes[key]->Fill(scHitVector[i]->t);
					dSCADCHitTimes[key]->Fill(scHitVector[i]->t_fADC);
					dSCTDCHitTimes[key]->Fill(scHitVector[i]->t_TDC);
					
					dSCADCTDCHitTimes[key]->Fill(scHitVector[i]->sector, scHitVector[i]->t_TDC - scHitVector[i]->t_fADC);
					dSCMatchedHitTimesPerSector[key]->Fill(scHitVector[i]->sector, scHitVector[i]->t);
				}
			}
		}
	}
	

    for (i = 0; i < tofHitVector.size(); i++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			const DTOFHit *thisTOFHit = tofHitVector[i];

			dTOFHitTimes[key]->Fill(tofHitVector[i]->t);
				
			if(thisTOFHit->has_fADC && !thisTOFHit->has_TDC){
				dTOFADCHitTimes[key]->Fill(tofHitVector[i]->t);
				
				// Manual loop over hits to match out of time
				for (auto hit = tofHitVector.begin(); hit != tofHitVector.end(); hit++){
					if ((*hit)->has_TDC && !(*hit)->has_fADC){
						if (GetCCDBIndexTOF(tofHitVector[i]) == GetCCDBIndexTOF(*hit)){
							dTOFADCTDCHitTimes[key]->Fill(GetCCDBIndexTOF(tofHitVector[i]), tofHitVector[i]->t_TDC - tofHitVector[i]->t_fADC);
						}
					}
				}
			}
			else if (!thisTOFHit->has_fADC && thisTOFHit->has_TDC){
				dTOFTDCHitTimes[key]->Fill(tofHitVector[i]->t_TDC);
			}
			else{
				dTOFMatchedHitTimes[key]->Fill(tofHitVector[i]->t);
				dTOFADCHitTimes[key]->Fill(tofHitVector[i]->t_fADC);
				dTOFTDCHitTimes[key]->Fill(tofHitVector[i]->t_TDC);

				dTOFADCTDCHitTimes[key]->Fill(GetCCDBIndexTOF(tofHitVector[i]), tofHitVector[i]->t_TDC - tofHitVector[i]->t_fADC);
			}
		}
    }
    
	
    for (i = 0; i < dircPmtHitVector.size(); i++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
			dDIRCHitTimes[key]->Fill(dircPmtHitVector[i]->t);
			
			if(dircPmtHitVector[i]->ch < DDIRCPmtHit_factory::DIRC_MAX_CHANNELS) {
				dDIRCHitTimesPerChannelNorth[key]->Fill(dircPmtHitVector[i]->ch, dircPmtHitVector[i]->t);
			} else {
				dDIRCHitTimesPerChannelNorth[key]->Fill(dircPmtHitVector[i]->ch-DDIRCPmtHit_factory::DIRC_MAX_CHANNELS, 
														dircPmtHitVector[i]->t);
			}
		}
	}
	    
    for (i = 0; i < bcalUnifiedHitVector.size(); i++){
 		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
		    int the_cell = (bcalUnifiedHitVector[i]->module - 1) * 16 + (bcalUnifiedHitVector[i]->layer - 1) * 4 + bcalUnifiedHitVector[i]->sector;
			// There is one fewer layer of TDCs so the numbering relects this
			int the_tdc_cell = (bcalUnifiedHitVector[i]->module - 1) * 12 + (bcalUnifiedHitVector[i]->layer - 1) * 4 + bcalUnifiedHitVector[i]->sector;

			// Get the underlying associated objects
			const DBCALHit * thisADCHit;
			const DBCALTDCHit * thisTDCHit;
			bcalUnifiedHitVector[i]->GetSingle(thisADCHit);
			bcalUnifiedHitVector[i]->GetSingle(thisTDCHit);

			if (thisADCHit != NULL) { //This should never be NULL but might as well check
				dBCALADCHitTimes[key]->Fill(thisADCHit->t);

				if (bcalUnifiedHitVector[i]->end == 0) {
					dBCALADCHitTimesUpstream[key]->Fill(the_cell, thisADCHit->t);
				} else {
					dBCALADCHitTimesDownstream[key]->Fill(the_cell, thisADCHit->t);
				}
			}

			if (thisTDCHit != NULL){
				dBCALTDCHitTimes[key]->Fill(thisTDCHit->t);

				if (DO_OPTIONAL){
					if (bcalUnifiedHitVector[i]->end == 0) {
						dBCALTDCHitTimesUpstream[key]->Fill(the_cell, thisTDCHit->t);
					} else {
						dBCALTDCHitTimesDownstream[key]->Fill(the_cell, thisTDCHit->t);
					}
				}
			}

			if (thisADCHit != NULL && thisTDCHit != NULL){
				if (bcalUnifiedHitVector[i]->end == 0) {
					dBCALADCTDCHitTimesUpstream[key]->Fill(the_tdc_cell, thisTDCHit->t - thisADCHit->t);
				} else {
					dBCALADCTDCHitTimesDownstream[key]->Fill(the_tdc_cell, thisTDCHit->t - thisADCHit->t);
				}
			}
    	}
    }


    for (i = 0; i < fmwpcHitVector.size(); i++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
			dFMWPCHitTimes[key]->Fill(fmwpcHitVector[i]->t);
			dFMWPCHitTimesPerLayer[key]->Fill(fmwpcHitVector[i]->t, 144*(fmwpcHitVector[i]->layer-1) + fmwpcHitVector[i]->wire);		
    	}
    }
    
    for (i = 0; i < ctofHitVector.size(); i++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
			dCTOFHitTimes[key]->Fill(ctofHitVector[i]->t);
			dCTOFHitTimesPerLayer[key]->Fill(ctofHitVector[i]->t, 2*(ctofHitVector[i]->bar-1) + ctofHitVector[i]->end+1);		
    	}
    }

    // from FCAL_online:  find energy weighted average time for FCAL hits, useful as a t0
    double fcalHitETot = 0;
    double fcalHitEwtT = 0;
    for (i = 0; i < fcalHitVector.size(); i++){
        fcalHitETot += fcalHitVector[i]->E;
        fcalHitEwtT += fcalHitVector[i]->E * fcalHitVector[i]->t;
    }
    fcalHitEwtT /= fcalHitETot;
    
    // FCAL energy
	for (auto const& cut : passed_cuts) {
		const string &key = cut.first;
		bool passed = cut.second;
		if(!passed) continue;

		dFCALTotalEnergy[key]->Fill(fcalHitETot);
	}
	    
	// FCAL timing
    for (i = 0; i < fcalHitVector.size(); i++){
		double locTime = ( fcalHitVector[i]->t - fcalHitEwtT )*k_to_nsec; // CHECK

		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
			dFCALHitTimes[key]->Fill(fcalHitVector[i]->t);
			dFCALHitOccupancy[key]->Fill(fcalHitVector[i]->column, fcalHitVector[i]->row);
			dFCALHitLocalTimes[key]->Fill(fcalHitVector[i]->column, fcalHitVector[i]->row, locTime);
			if(DO_OPTIONAL) {
				dFCALHitTimesPerChannel[key]->Fill(fcalGeom.channel(fcalHitVector[i]->row, fcalHitVector[i]->column), fcalHitVector[i]->t);
			}
		}
    }

	if(ECAL_EXISTS) {
		// similar plots for the ECAL as for the FCAL
		double ecalHitETot = 0;
		double ecalHitEwtT = 0;
		for (i = 0; i < ecalHitVector.size(); i++){
			ecalHitETot += ecalHitVector[i]->E;
			ecalHitEwtT += ecalHitVector[i]->E * ecalHitVector[i]->t;
		}
		ecalHitEwtT /= ecalHitETot;
		
		// FCAL energy
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
	
			dECALTotalEnergy[key]->Fill(ecalHitETot);
		}
			
		// ECAL timing
		for (i = 0; i < ecalHitVector.size(); i++){
			double locTime = ( ecalHitVector[i]->t - ecalHitEwtT )*k_to_nsec; // CHECK
	
			for (auto const& cut : passed_cuts) {
				const string &key = cut.first;
				bool passed = cut.second;
				if(!passed) continue;
				
				dECALHitTimes[key]->Fill(ecalHitVector[i]->t);
				dECALHitOccupancy[key]->Fill(ecalHitVector[i]->column, ecalHitVector[i]->row);
				dECALHitLocalTimes[key]->Fill(ecalHitVector[i]->column, ecalHitVector[i]->row, locTime);
				if(DO_OPTIONAL) {
					dECALHitTimesPerChannel[key]->Fill(ecalGeomVect[0]->channel(ecalHitVector[i]->row, ecalHitVector[i]->column), ecalHitVector[i]->t);
				}
			}
		}
	}

    if(CCAL_CALIB) {
		// Do the same thing for the CCAL as a start
		double ccalHitETot = 0;
		double ccalHitEwtT = 0;
		for (i = 0; i < ccalHitVector.size(); i++){
			ccalHitETot += ccalHitVector[i]->E;
			ccalHitEwtT += ccalHitVector[i]->E * ccalHitVector[i]->t;
		}
		ccalHitEwtT /= ccalHitETot;

		for (i = 0; i < ccalHitVector.size(); i++){
			double locTime = ( ccalHitVector[i]->t - ccalHitEwtT )*k_to_nsec;

			for (auto const& cut : passed_cuts) {
				const string &key = cut.first;
				bool passed = cut.second;
				if(!passed) continue;
	
			
				dCCALHitTimes[key]->Fill(ccalHitVector[i]->t);
				dCCALHitOccupancy[key]->Fill(ccalHitVector[i]->row, ccalHitVector[i]->column);
				dCCALHitLocalTimes[key]->Fill(ccalHitVector[i]->row, ccalHitVector[i]->column, locTime);
				if(DO_OPTIONAL) {
					dCCALHitTimesPerChannel[key]->Fill(12.*ccalHitVector[i]->row+ccalHitVector[i]->column, ccalHitVector[i]->t);
				}
			}
		}
    }

    for (i = 0; i < psHitVector.size(); i++){
		int nColumns = 145*2;
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
			dPSHitTimes[key]->Fill(psHitVector[i]->t);
			dPSHitTimesPerColumn[key]->Fill(psHitVector[i]->t, psHitVector[i]->column+psHitVector[i]->arm*nColumns/2);		
    	}
    }

	for (i = 0; i < tagmHitVector.size(); i++) {
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			//if(!tagmHitVector[i]->has_fADC || !tagmHitVector[i]->has_TDC) continue;
			dTAGMHitTimes[key]->Fill(tagmHitVector[i]->t);
		
			// Break these histograms up into hits coming from the TDC and hits coming from the ADC
			const DTAGMHit *thisTAGMHit = tagmHitVector[i];
			if (thisTAGMHit->has_fADC && !thisTAGMHit->has_TDC){
				dTAGMADCHitTimes[key]->Fill(tagmHitVector[i]->time_fadc);
						
				// Manual loop over hits to match out of time
				for (auto hit = tagmHitVector.begin(); hit != tagmHitVector.end(); hit++){
					if ((*hit)->has_TDC && !(*hit)->has_fADC){
						if (GetCCDBIndexTAGM(tagmHitVector[i]) == GetCCDBIndexTAGM(*hit)){
							dTAGMADCTDCHitTimes[key]->Fill(GetCCDBIndexTAGM(tagmHitVector[i]), 
														 tagmHitVector[i]->t - tagmHitVector[i]->time_fadc);
						}
					}
				}
			}
			else if (!thisTAGMHit->has_fADC && thisTAGMHit->has_TDC){
				dTAGMTDCHitTimes[key]->Fill(tagmHitVector[i]->t);
			}
			else{
				dTAGMMatchedHitTimes[key]->Fill(tagmHitVector[i]->t);
				dTAGMADCHitTimes[key]->Fill(tagmHitVector[i]->time_fadc);
				dTAGMTDCHitTimes[key]->Fill(tagmHitVector[i]->t);
				
				dTAGMADCTDCHitTimes[key]->Fill(GetCCDBIndexTAGM(tagmHitVector[i]), tagmHitVector[i]->t - tagmHitVector[i]->time_fadc);
				dTAGMMatchedHitTimesPerSector[key]->Fill(GetCCDBIndexTAGM(tagmHitVector[i]), tagmHitVector[i]->t);
			}
		}
	}

	for (i = 0; i < taghHitVector.size(); i++) {
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			//if(!taghHitVector[i]->has_fADC || !taghHitVector[i]->has_TDC) continue;
			dTAGHHitTimes[key]->Fill(taghHitVector[i]->t);
		
			// Break these histograms up into hits coming from the TDC and hits coming from the ADC
			const DTAGHHit *thisTAGHHit = taghHitVector[i];
			if (thisTAGHHit->has_fADC && !thisTAGHHit->has_TDC){
				dTAGHADCHitTimes[key]->Fill(taghHitVector[i]->time_fadc);
						
				// Manual loop over hits to match out of time
				for (auto hit = taghHitVector.begin(); hit != taghHitVector.end(); hit++){
					if ((*hit)->has_TDC && !(*hit)->has_fADC){
						if (taghHitVector[i]->counter_id == (*hit)->counter_id){
							dTAGHADCTDCHitTimes[key]->Fill(taghHitVector[i]->counter_id, 
														 taghHitVector[i]->time_tdc - taghHitVector[i]->time_fadc);
						}
					}
				}
			}
			else if (!thisTAGHHit->has_fADC && thisTAGHHit->has_TDC){
				dTAGHTDCHitTimes[key]->Fill(taghHitVector[i]->t);
			}
			else{
				dTAGHMatchedHitTimes[key]->Fill(taghHitVector[i]->t);
				dTAGHADCHitTimes[key]->Fill(taghHitVector[i]->time_fadc);
				dTAGHTDCHitTimes[key]->Fill(taghHitVector[i]->time_tdc);
				
				dTAGHADCTDCHitTimes[key]->Fill(taghHitVector[i]->counter_id, taghHitVector[i]->time_tdc - taghHitVector[i]->time_fadc);
				dTAGHMatchedHitTimesPerSector[key]->Fill(taghHitVector[i]->counter_id, taghHitVector[i]->t);
			}
		}
	}

	for (i = 0; i < pscHitVector.size(); i++) {
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			//if(!pscHitVector[i]->has_fADC || !pscHitVector[i]->has_TDC) continue;
			dPSCHitTimes[key]->Fill(pscHitVector[i]->t);
		
			// Break these histograms up into hits coming from the TDC and hits coming from the ADC
			const DPSCHit *thisPSCHit = pscHitVector[i];
			if (thisPSCHit->has_fADC && !thisPSCHit->has_TDC){
				dPSCADCHitTimes[key]->Fill(pscHitVector[i]->time_fadc);
						
				// Manual loop over hits to match out of time
				for (auto hit = pscHitVector.begin(); hit != pscHitVector.end(); hit++){
					if ((*hit)->has_TDC && !(*hit)->has_fADC){
						if ( (pscHitVector[i]->arm == (*hit)->arm) && (pscHitVector[i]->module == (*hit)->module) ){
							dPSCADCTDCHitTimes[key]->Fill( pscHitVector[i]->module+pscHitVector[i]->arm*16/2, 
														   pscHitVector[i]->time_tdc - pscHitVector[i]->time_fadc);
						}
					}
				}
			}
			else if (!thisPSCHit->has_fADC && thisPSCHit->has_TDC){
				dPSCTDCHitTimes[key]->Fill(pscHitVector[i]->t);
			}
			else{
				dPSCMatchedHitTimes[key]->Fill(pscHitVector[i]->t);
				dPSCADCHitTimes[key]->Fill(pscHitVector[i]->time_fadc);
				dPSCTDCHitTimes[key]->Fill(pscHitVector[i]->time_tdc);
				
				dPSCADCTDCHitTimes[key]->Fill( pscHitVector[i]->module+pscHitVector[i]->arm*16/2, 
											   pscHitVector[i]->time_tdc - pscHitVector[i]->time_fadc);
				dPSCMatchedHitTimesPerSector[key]->Fill(pscHitVector[i]->module+pscHitVector[i]->arm*16/2, pscHitVector[i]->t);
			}
		}
	}




    // Next the relative times between detectors using tracking
    // By the time we get to this point, our first guess at the timing should be fairly good. 
    // Certainly good enough to take a pass at the time based tracking
    // This will be the final alignment step for now

    if (thisRFBunch->dNumParticleVotes < 2) { DEvent::GetLockService(event)->RootFillUnLock(this); return; }
    auto dRFTimeFactory = static_cast<DRFTime_factory*>(event->GetFactory("DRFTime", ""));

    // Loop over TAGM hits
    for (unsigned int j = 0 ; j < tagmHitVector.size(); j++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
        
        	if(DO_OPTIONAL && tagmHitVector[j]->has_fADC) {
            	double locShiftedADCTime = dRFTimeFactory->Step_TimeToNearInputTime(thisRFBunch->dTime, tagmHitVector[j]->time_fadc);
            	dTAGMADCRFCompareTimes[key][GetCCDBIndexTAGM(tagmHitVector[j])-1]->Fill(tagmHitVector[j]->time_fadc - locShiftedADCTime);
            }
            if(tagmHitVector[j]->has_TDC) {
				double locShiftedTDCTime = dRFTimeFactory->Step_TimeToNearInputTime(thisRFBunch->dTime, tagmHitVector[j]->t);
				//cerr << GetCCDBIndexTAGM(tagmHitVector[j]) << "  " << dTAGMTDCRFCompareTimes[key].size() << endl;
				dTAGMTDCRFCompareTimes[key][GetCCDBIndexTAGM(tagmHitVector[j])-1]->Fill(tagmHitVector[j]->t - locShiftedTDCTime);
			}
        
        	if(tagmHitVector[j]->row == 0) {
				dTaggerRFTime[key]->Fill(tagmHitVector[j]->t - thisRFBunch->dTime);
				dTAGMRFTime[key]->Fill(tagmHitVector[j]->t - thisRFBunch->dTime);

				dTaggerRFEnergyTime[key]->Fill(tagmHitVector[j]->t - thisRFBunch->dTime, tagmHitVector[j]->E);
				dTAGMRFCounterTime[key]->Fill(GetCCDBIndexTAGM(tagmHitVector[j]), tagmHitVector[j]->t - thisRFBunch->dTime);        
        	}
        }
    }

    // Loop over TAGH hits
    for (unsigned int j = 0 ; j < taghHitVector.size(); j++){
		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			if(DO_OPTIONAL && taghHitVector[j]->has_fADC) {
            	double locShiftedADCTime = dRFTimeFactory->Step_TimeToNearInputTime(thisRFBunch->dTime, taghHitVector[j]->time_fadc);
            	dTAGHADCRFCompareTimes[key][taghHitVector[j]->counter_id-1]->Fill(taghHitVector[j]->time_fadc - locShiftedADCTime);
            }
            if(taghHitVector[j]->has_TDC) {
				double locShiftedTDCTime = dRFTimeFactory->Step_TimeToNearInputTime(thisRFBunch->dTime, taghHitVector[j]->time_tdc);
				//cerr << (taghHitVector[j]->counter_id-1) << " out of " << dTAGHADCRFCompareTimes[key].size() << endl;
				dTAGHTDCRFCompareTimes[key][taghHitVector[j]->counter_id-1]->Fill(taghHitVector[j]->time_tdc - locShiftedTDCTime);
			}

			dTaggerRFTime[key]->Fill(taghHitVector[j]->t - thisRFBunch->dTime);
			dTAGHRFTime[key]->Fill(taghHitVector[j]->t - thisRFBunch->dTime);

			dTaggerRFEnergyTime[key]->Fill(taghHitVector[j]->t - thisRFBunch->dTime, taghHitVector[j]->E);
			dTAGHRFCounterTime[key]->Fill(taghHitVector[j]->counter_id, taghHitVector[j]->t - thisRFBunch->dTime);
		}
    }

    // now loop over neutral showers to align calorimeters
    DVector3 locTargetCenter(0.,0.,Z_TARGET);

    for (i = 0; i <  neutralShowerVector.size(); i++) {
	    double locPathLength = (neutralShowerVector[i]->dSpacetimeVertex.Vect() - locTargetCenter).Mag();
	    double locDeltaT = neutralShowerVector[i]->dSpacetimeVertex.T() - locPathLength/29.9792458 - thisRFBunch->dTime;

		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;

			// to eliminate low-energy tails and other reconstruction problems, require minimum energies
			//   E(FCAL) > 200 MeV,  E(BCAL) > 100 MeV
			if(neutralShowerVector[i]->dDetectorSystem == SYS_FCAL) {
				dFCALShowerRFTimeVsEnergy_NoTracks[key]->Fill(neutralShowerVector[i]->dEnergy, locDeltaT);
				if(neutralShowerVector[i]->dEnergy > 0.2) {
					dFCALShowerRFTime_NoTracks[key]->Fill(locDeltaT);
				}
			
				// if we're not using tracking, then align the TOF using hits matched between the TOF and FCAL
				if(NO_TRACKS) {
					for( vector< const DTOFPoint* >::const_iterator tof = tofPointVector.begin(); 
						tof != tofPointVector.end(); tof++ ) {
					
						const DTOFPoint* tof_hit = *tof;
					
						// select double-ended hits
						if( tof_hit->dHorizontalBarStatus != 3 || tof_hit->dVerticalBarStatus != 3 )
							continue;
						
						double dx = tof_hit->pos.X() - neutralShowerVector[i]->dSpacetimeVertex.X();
						double dy = tof_hit->pos.Y() - neutralShowerVector[i]->dSpacetimeVertex.Y();
					
						double locTOFPathLength = (tof_hit->pos - locTargetCenter).Mag();
						double locTOFDeltaT = tof_hit->t - locTOFPathLength/29.9792458 - thisRFBunch->dTime;
					
						// match the hits
						if( ( fabs(dx - TOF_X_MEAN) < 2.*TOF_X_SIG ) && 
							( fabs(dy - TOF_Y_MEAN) < 2.*TOF_Y_SIG ) ) {
							dTOFShowerRFTime_NoTracks[key]->Fill(locTOFDeltaT);
						}

					}
				}
			} else if(neutralShowerVector[i]->dDetectorSystem == SYS_ECAL) {
				dECALShowerRFTimeVsEnergy_NoTracks[key]->Fill(neutralShowerVector[i]->dEnergy, locDeltaT);
				if(neutralShowerVector[i]->dEnergy > 0.2) {
					dECALShowerRFTime_NoTracks[key]->Fill(locDeltaT);
				}
			
				//  should we do no-tracking alignment for the TOF with the ECAL as well?				
			} else {  // neutralShowerVector[i]->dDetectorSystem == SYS_BCAL
				dBCALShowerRFTimeVsEnergy_NoTracks[key]->Fill(neutralShowerVector[i]->dEnergy, locDeltaT);
				if(neutralShowerVector[i]->dEnergy > 0.1) {
					dBCALShowerRFTime_NoTracks[key]->Fill(locDeltaT);
				}
			}
	    }
    } // End of loop over neutral showers

    for (i = 0; i <  ccalShowerVector.size(); i++){
	    DVector3 locShowerPos(ccalShowerVector[i]->x, ccalShowerVector[i]->y, ccalShowerVector[i]->z);
	    double locPathLength = (locShowerPos - locTargetCenter).Mag();
	    double locDeltaT = ccalShowerVector[i]->time - locPathLength/29.9792458 - thisRFBunch->dTime;

		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
			
			dCCALShowerRFTimeVsEnergy_NoTracks[key]->Fill(ccalShowerVector[i]->E, locDeltaT);
			if(neutralShowerVector[i]->dEnergy > 0.1) {
				dCCALShowerRFTime_NoTracks[key]->Fill(locDeltaT);
			}
		}
    }
    
    // we went this far just to align the tagger with the RF time, nothing else to do without tracks
    if(NO_TRACKS) 
	    { DEvent::GetLockService(event)->RootFillUnLock(this);  return; }

    if (!DO_TRACK_BASED && !DO_VERIFY ) { DEvent::GetLockService(event)->RootFillUnLock(this);  return; }   // Before this stage we aren't really ready yet, so just return

    // Try using the detector matches
    // Loop over the charged tracks

    vector <double> TOFmRF;
    vector <double> BCALmRF;
    vector <double> FCALmRF;
    vector <double> ECALmRF;
    
    for (i = 0; i < chargedTrackVector.size(); i++) {
        const DChargedTrackHypothesis *pionHypothesis;

        // We only want negative particles to kick out protons
		if(!locIsNoFieldFlag) {
	        if (chargedTrackVector[i]->Get_Charge() > 0) continue;
	        pionHypothesis = chargedTrackVector[i]->Get_Hypothesis(PiMinus);
		} else {
	        pionHypothesis = chargedTrackVector[i]->Get_Hypothesis(PiPlus);
		}

        if (pionHypothesis == NULL) continue;

		auto locTrackTimeBased = pionHypothesis->Get_TrackTimeBased();
        double trackingFOM = TMath::Prob(locTrackTimeBased->chisq, locTrackTimeBased->Ndof);
        // Some quality cuts for the tracks we will use
        // Keep this minimal for now and investigate later
        //float trackingFOMCut = 0.01;
        //float trackingFOMCut =0.0027;
		float trackingFOMCut = 2.87E-7;
		float trackingNDFCut = 5;
		if(STRAIGHT_TRACK) {
        	trackingFOMCut = 1.E-10;
        	trackingNDFCut = 5;
		}
		
        if( trackingFOM < trackingFOMCut ) continue;
        if( locTrackTimeBased->Ndof < trackingNDFCut) continue;

        //////////////////////////////////////////
        // get best matches to SC/TOF/FCAL/BCAL //
        //////////////////////////////////////////
        auto locSCHitMatchParams       = pionHypothesis->Get_SCHitMatchParams();
        auto locTOFHitMatchParams      = pionHypothesis->Get_TOFHitMatchParams();
        auto locFCALShowerMatchParams  = pionHypothesis->Get_FCALShowerMatchParams();
        auto locBCALShowerMatchParams  = pionHypothesis->Get_BCALShowerMatchParams();
        auto locECALShowerMatchParams  = pionHypothesis->Get_ECALShowerMatchParams();

        // We will only use tracks matched to the start counter for our calibration since this will be our reference for t0
        if(!NO_START_COUNTER) {
        	if (locSCHitMatchParams == NULL) continue;
		}
		
        // the idea will be to fix the SC time and reference the other PID detectors off of this

        // These "flightTime" corrected time are essentially that detector's estimate of the target time
        float targetCenterCorrection = ((pionHypothesis->position()).Z() - Z_TARGET) / SPEED_OF_LIGHT;
        float flightTimeCorrectedSCTime = 0.; 
        double locSCzIntersection = 0.;
 		bool sc_match_pid = false;
 		
		if(!NO_START_COUNTER) {
			// need to get the projected hit position at the SC in order to cut on it
			DVector3 IntersectionPoint, IntersectionMomentum;	
			vector<DTrackFitter::Extrapolation_t> extrapolations = locTrackTimeBased->extrapolations.at(SYS_START);
			shared_ptr<DSCHitMatchParams> locSCHitMatchParams2;
			sc_match_pid = locParticleID->Cut_MatchDistance(extrapolations, locSCHitMatchParams->dSCHit, locSCHitMatchParams->dSCHit->t, locSCHitMatchParams2, 
											   true, &IntersectionPoint, &IntersectionMomentum);
			if(sc_match_pid) {
				locSCzIntersection = IntersectionPoint.z();
				flightTimeCorrectedSCTime = locSCHitMatchParams->dHitTime - locSCHitMatchParams->dFlightTime - targetCenterCorrection; 
			}
		}
		double locShiftedTime = dRFTimeFactory->Step_TimeToNearInputTime(thisRFBunch->dTime, flightTimeCorrectedSCTime);
		double locSCDeltaT = flightTimeCorrectedSCTime - thisRFBunch->dTime;

		// Get the pulls vector from the track
		auto thisTimeBasedTrack = pionHypothesis->Get_TrackTimeBased();

		vector<DTrackFitter::pull_t> pulls = thisTimeBasedTrack->pulls;
		double earliestCDCTime = 10000.;
		double earliestFDCTime = 10000.;
		for (size_t iPull = 0; iPull < pulls.size(); iPull++){
			if ( pulls[iPull].cdc_hit != nullptr && pulls[iPull].tdrift < earliestCDCTime) earliestCDCTime = pulls[iPull].tdrift;
			if ( pulls[iPull].fdc_hit != nullptr && pulls[iPull].tdrift < earliestFDCTime) earliestFDCTime = pulls[iPull].tdrift;
		 }

		// get CDC hit info for timing
		vector < const DCDCTrackHit *> cdcTrackHitVector;
		pionHypothesis->Get_TrackTimeBased()->Get(cdcTrackHitVector);

 		for (auto const& cut : passed_cuts) {
			const string &key = cut.first;
			bool passed = cut.second;
			if(!passed) continue;
 
			if(!NO_START_COUNTER && sc_match_pid) {
				dSCRFTime_AllHits[key]->Fill(flightTimeCorrectedSCTime - thisRFBunch->dTime);
				
				// Stay away from the nose section, since the propagation time corrections are not stable there.
				// cut corresponds to ~50 cm path length through the SC - not too far into the nose section
				// but enough to get some statistics
				if( sc_match_pid && locSCzIntersection < 83. ) {
					dSCTargetRFCompareTimes[key][locSCHitMatchParams->dSCHit->sector-1]->Fill(flightTimeCorrectedSCTime - locShiftedTime);
					dSCRFTime[key]->Fill(flightTimeCorrectedSCTime - thisRFBunch->dTime);
					dSCRFTimeVsSector[key]->Fill(locSCHitMatchParams->dSCHit->sector, locSCDeltaT);
				}

				// Do this the old way for the CDC
				if (cdcTrackHitVector.size() != 0) {
					float earliestTime = 10000; // Initialize high
					for (unsigned int iCDC = 0; iCDC < cdcTrackHitVector.size(); iCDC++){
						if (cdcTrackHitVector[iCDC]->tdrift < earliestTime) earliestTime = cdcTrackHitVector[iCDC]->tdrift;
					}

					dCDCSCTime[key]->Fill(earliestTime - locSCHitMatchParams->dHitTime);
				}

				// Loop over TAGM hits
				for (unsigned int j = 0 ; j < tagmHitVector.size(); j++){
					dTaggerSCTime[key]->Fill(tagmHitVector[j]->t - flightTimeCorrectedSCTime);
					dTAGMSCCounterTime[key]->Fill(GetCCDBIndexTAGM(tagmHitVector[j]), tagmHitVector[j]->t - flightTimeCorrectedSCTime);
					dTaggerSCEnergyTime[key]->Fill(tagmHitVector[j]->t - flightTimeCorrectedSCTime, tagmHitVector[j]->E);
				}
				// Loop over TAGH hits
				for (unsigned int j = 0 ; j < taghHitVector.size(); j++){
					dTaggerSCTime[key]->Fill(taghHitVector[j]->t - flightTimeCorrectedSCTime);
					dTAGHSCCounterTime[key]->Fill(taghHitVector[j]->counter_id, taghHitVector[j]->t - flightTimeCorrectedSCTime);
					dTaggerSCEnergyTime[key]->Fill(taghHitVector[j]->t - flightTimeCorrectedSCTime, taghHitVector[j]->E);
				}				
			} else {
				// TODO: need to come up with some way to at least time in the CDC if there's no start counter??
			}

			if (locTOFHitMatchParams != NULL) {
			   	// Now check the TOF matching. Do this on a full detector level.
			   	float flightTimeCorrectedTOFTime = locTOFHitMatchParams->dHitTime - locTOFHitMatchParams->dFlightTime - targetCenterCorrection;

			   	if(!NO_START_COUNTER) {
					dTOFSCTime[key]->Fill(flightTimeCorrectedTOFTime - flightTimeCorrectedSCTime);
			   	}
				dTOFRFTime[key]->Fill(flightTimeCorrectedTOFTime - thisRFBunch->dTime);
				//cout<<"key = "<<key<<endl;
				TOFmRF.push_back(flightTimeCorrectedTOFTime - thisRFBunch->dTime);
				dEarliestFDCTime[key]->Fill(earliestFDCTime);

				 
				 // TODO: OPTIMIZE THIS
				// get DIRC match parameters (contains LUT information)
				const DDetectorMatches* locDetectorMatches = NULL;
				event->GetSingle(locDetectorMatches);
				DDetectorMatches &locDetectorMatch = (DDetectorMatches&)locDetectorMatches[0];
				shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
				bool foundDIRC = locParticleID->Get_DIRCMatchParams(locTrackTimeBased, locDetectorMatches, locDIRCMatchParams);

				// For DIRC calibrations, select tracks which have a good TOF match
				if(foundDIRC && locTOFHitMatchParams->dDeltaXToHit < 10.0 && locTOFHitMatchParams->dDeltaYToHit < 10.0) {

					// Get match parameters
					DVector3 posInBar = locDIRCMatchParams->dExtrapolatedPos; 
					DVector3 momInBar = locDIRCMatchParams->dExtrapolatedMom;
					// next line commented out to suppress warning
					//				double locExpectedThetaC = locDIRCMatchParams->dExpectedThetaC;
					double locExtrapolatedTime = locDIRCMatchParams->dExtrapolatedTime;
					// next line commented out to suppress warning
					//				int locBar = locDIRCGeometry->GetBar(posInBar.Y());

					Particle_t locPID = locTrackTimeBased->PID();
					double locMass = ParticleMass(locPID);
					double locAngle = dDIRCLut->CalcAngle(momInBar.Mag(), locMass);
					map<Particle_t, double> locExpectedAngle = dDIRCLut->CalcExpectedAngles(momInBar.Mag());

					// get map of DIRCMatches to PMT hits
					map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > locDIRCTrackMatchParamsMap;
					locDetectorMatch.Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParamsMap);
					map<Particle_t, double> logLikelihoodSum;

					// loop over associated hits for LUT diagnostic plots
					for(uint loc_i=0; loc_i<dircPmtHitVector.size(); loc_i++) {
						bool locIsReflected = false;
						vector<pair<double, double>> locDIRCPhotons = dDIRCLut->CalcPhoton(dircPmtHitVector[loc_i], locExtrapolatedTime, posInBar, momInBar, locExpectedAngle, locAngle, locPID, locIsReflected, logLikelihoodSum);
						double locHitTime = dircPmtHitVector[loc_i]->t - locExtrapolatedTime;
						int locChannel = dircPmtHitVector[loc_i]->ch%dMaxDIRCChannels;

						if(locDIRCPhotons.size() > 0) {
							// loop over candidate photons
							for(uint loc_j = 0; loc_j<locDIRCPhotons.size(); loc_j++) {
								double locDeltaT = locDIRCPhotons[loc_j].first - locHitTime;
		
								if(locChannel < DDIRCPmtHit_factory::DIRC_MAX_CHANNELS) {
									dDIRCDeltaTimePerChannelNorth[key]->Fill(locChannel, locDeltaT);
								} else {
									dDIRCDeltaTimePerChannelSouth[key]->Fill(locChannel-DDIRCPmtHit_factory::DIRC_MAX_CHANNELS, locDeltaT);
								}
							}
						}
					}
				}
			}
			
			if (locBCALShowerMatchParams != NULL) {
			   	float flightTimeCorrectedBCALTime = locBCALShowerMatchParams->dBCALShower->t - locBCALShowerMatchParams->dFlightTime - targetCenterCorrection;
				dBCALShowerRFTime[key]->Fill(flightTimeCorrectedBCALTime - thisRFBunch->dTime);
				BCALmRF.push_back(flightTimeCorrectedBCALTime - thisRFBunch->dTime);
				
			   	if(!NO_START_COUNTER) {
			   		dBCALShowerSCTime[key]->Fill(flightTimeCorrectedBCALTime - flightTimeCorrectedSCTime);
			   		dBCALShowerSCTimeVsCorrection[key]->Fill(locBCALShowerMatchParams->dFlightTime, flightTimeCorrectedBCALTime - flightTimeCorrectedSCTime);
			   	}

			   	// Fill the following when there is a SC/BCAL match.
			   	dEarliestCDCTime[key]->Fill(earliestCDCTime);
			   	
				// Try to correlate the latest CDC track hit times with the BCAL shower time
				if (cdcTrackHitVector.size() != 0) {
					float earliestTime = 10000; // Initialize high
					//float latestTime = -10000; // Initialize low
					//float lastTime = -10000; // Initialize low
					for (unsigned int iCDC = 0; iCDC < cdcTrackHitVector.size(); iCDC++){
						//if (cdcTrackHitVector[iCDC]->tdrift > latestTime) latestTime = cdcTrackHitVector[iCDC]->tdrift;
						if (cdcTrackHitVector[iCDC]->tdrift < earliestTime) earliestTime = cdcTrackHitVector[iCDC]->tdrift;
						//lastTime = cdcTrackHitVector[iCDC]->tdrift;
					}

					dCDCBCALTime[key]->Fill(earliestTime - flightTimeCorrectedBCALTime);
				}

			}
			
			if (locFCALShowerMatchParams != NULL) {
			   	float flightTimeCorrectedFCALTime = locFCALShowerMatchParams->dFCALShower->getTime() - locFCALShowerMatchParams->dFlightTime - targetCenterCorrection;
				dFCALShowerRFTime[key]->Fill(flightTimeCorrectedFCALTime - thisRFBunch->dTime);
				FCALmRF.push_back(flightTimeCorrectedFCALTime - thisRFBunch->dTime);

			   	if(!NO_START_COUNTER) {
					dFCALShowerSCTime[key]->Fill(flightTimeCorrectedFCALTime - flightTimeCorrectedSCTime);
			   	}
			}
			if (locECALShowerMatchParams != NULL) {
			   	float flightTimeCorrectedECALTime = locECALShowerMatchParams->dECALShower->t - locECALShowerMatchParams->dFlightTime - targetCenterCorrection;
				dECALShowerRFTime[key]->Fill(flightTimeCorrectedECALTime - thisRFBunch->dTime);
				ECALmRF.push_back(flightTimeCorrectedECALTime - thisRFBunch->dTime);

			   	if(!NO_START_COUNTER) {
					dECALShowerSCTime[key]->Fill(flightTimeCorrectedECALTime - flightTimeCorrectedSCTime);
			   	}
			}
		}
    } // End of loop over time based tracks

    if ((TOFmRF.size()>0) && (BCALmRF.size()>0)) {
      for (unsigned int m=0; m<TOFmRF.size(); m++) {
	for (unsigned int j=0; j<BCALmRF.size(); j++) {
	  dTOFRFTimeVSBCALRFTime["Physics Triggers"]->Fill(BCALmRF[j], TOFmRF[m]);
	}
      }
    }
    if ((TOFmRF.size()>0) && (FCALmRF.size()>0)) {
      for (unsigned int m=0; m<TOFmRF.size(); m++) {
	for (unsigned int j=0; j<FCALmRF.size(); j++) {
	  dTOFRFTimeVSFCALRFTime["Physics Triggers"]->Fill(FCALmRF[j], TOFmRF[m]);
	}
      }
    }
    if ((TOFmRF.size()>0) && (ECALmRF.size()>0)) {
      for (unsigned int m=0; m<TOFmRF.size(); m++) {
	for (unsigned int j=0; j<ECALmRF.size(); j++) {
	  dTOFRFTimeVSECALRFTime["Physics Triggers"]->Fill(ECALmRF[j], TOFmRF[m]);
	}
      }
    }


    
   DEvent::GetLockService(event)->RootFillUnLock(this);


    return;
}


//------------------
// EndRun
//------------------
void JEventProcessor_HLDetectorTiming::EndRun()
{
   // This is called whenever the run number changes, before it is
   // changed to give you a chance to clean up before processing
   // events from the next run number.

  // set some histogram properties


  TH2I *fdc_time_module_hist = (TH2I*)gDirectory->Get("HLDetectorTiming/FDC/FDCHit Wire time vs. module");
  if(fdc_time_module_hist != NULL) {
    string act_crate;
    int act_slot;
    for(int ibin=1; ibin<=48; ibin++){
      int mod = Get_FDCTDC_crate_slot(ibin, act_crate, act_slot);
      if (mod) {} // gratuitious check of return value to suppress warning
      stringstream ss;
      ss << act_crate << "/" << act_slot;
      fdc_time_module_hist->GetXaxis()->SetBinLabel(ibin, ss.str().c_str());
    }
    fdc_time_module_hist->LabelsOption("v");
  }
  

   return;
}

//------------------
// Finish
//------------------
void JEventProcessor_HLDetectorTiming::Finish()
{
   // Called before program exit after event processing is finished.
   //Here is where we do the fits to the data to see if we have a reasonable alignment

   return;
}

int JEventProcessor_HLDetectorTiming::GetCCDBIndexTOF(const DTOFHit *thisHit){
   // Returns the CCDB index of a particular hit
   // This 
   int plane = thisHit->plane;
   int bar = thisHit->bar;
   int end = thisHit->end;
   // 44 bars per plane
   int CCDBIndex = plane * 88 + end * 44 + bar; 
   return CCDBIndex;
}

int JEventProcessor_HLDetectorTiming::GetCCDBIndexBCAL(const DBCALHit *thisHit){
   return 0;
}

int JEventProcessor_HLDetectorTiming::GetCCDBIndexTAGM(const DTAGMHit *thisHit){
   // Since there are a few counters where each row is read out seperately this is a bit of a mess
   int row = thisHit->row;
   int column = thisHit->column;

   int CCDBIndex = column + row;
   if (column > 9) CCDBIndex += 5;
   if (column > 27) CCDBIndex += 5;
   if (column > 81) CCDBIndex += 5;
   if (column > 99) CCDBIndex += 5;

   return CCDBIndex;
}

int JEventProcessor_HLDetectorTiming::GetCCDBIndexCDC(const DCDCHit *thisHit){

   int ring = thisHit->ring;
   int straw = thisHit->straw;

   int CCDBIndex = GetCCDBIndexCDC(ring, straw);
   return CCDBIndex;
}

int JEventProcessor_HLDetectorTiming::GetCCDBIndexCDC(int ring, int straw){

   //int Nstraws[28] = {42, 42, 54, 54, 66, 66, 80, 80, 93, 93, 106, 106, 123, 123, 135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209};
   int StartIndex[28] = {0, 42, 84, 138, 192, 258, 324, 404, 484, 577, 670, 776, 882, 1005, 1128, 1263, 1398, 1544, 1690, 1848, 2006, 2176, 2346, 2528, 2710, 2907, 3104, 3313};

   int CCDBIndex = StartIndex[ring - 1] + straw;
   return CCDBIndex;
}

