// $Id$
//
//    File: JEventProcessor_occupancy_online.cc
// Created: Tue Apr 12 09:43:54 EDT 2016
// Creator: zihlmann (on Linux gluon47.jlab.org 2.6.32-358.23.2.el6.x86_64 x86_64)
//

#include <TMath.h>

#include "JEventProcessor_occupancy_online.h"

#include <BCAL/DBCALHit.h>
#include <BCAL/DBCALDigiHit.h>
#include <BCAL/DBCALTDCDigiHit.h>
#include <CCAL/DCCALDigiHit.h>
#include <CDC/DCDCDigiHit.h>
#include <FDC/DFDCCathodeDigiHit.h>
#include <FDC/DFDCWireDigiHit.h>
#include <FMWPC/DFMWPCDigiHit.h>
#include <FCAL/DFCALDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSCTDCDigiHit.h>
#include <PAIR_SPECTROMETER/DPSDigiHit.h>
#include <PAIR_SPECTROMETER/DPSGeometry.h>
#include <TAGGER/DTAGHTDCDigiHit.h>
#include <TAGGER/DTAGHDigiHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHGeometry.h>
#include <TOF/DTOFDigiHit.h>
#include <TOF/DTOFTDCDigiHit.h>
#include <TPOL/DTPOLSectorDigiHit.h>
#include <TPOL/DTPOLHit_factory.h>
#include <RF/DRFTDCDigiTime.h>
#include <START_COUNTER/DSCDigiHit.h>
#include <START_COUNTER/DSCTDCDigiHit.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <DIRC/DDIRCTDCDigiHit.h>

// Script-fu for handling digihit type enmasse
#define DigiHitTypes(X) \
	X(DBCALDigiHit) \
	X(DBCALTDCDigiHit) \
	X(DCCALDigiHit) \
	X(DCDCDigiHit) \
	X(DFDCCathodeDigiHit) \
	X(DFDCWireDigiHit) \
	X(DFMWPCDigiHit) \
	X(DFCALDigiHit) \
	X(DPSCDigiHit) \
	X(DPSCTDCDigiHit) \
	X(DPSDigiHit) \
	X(DTOFDigiHit) \
	X(DTOFTDCDigiHit) \
	X(DSCDigiHit) \
	X(DSCTDCDigiHit) \
	X(DRFTDCDigiTime) \
	X(DTAGMDigiHit) \
	X(DTAGMTDCDigiHit) \
	X(DTAGHDigiHit) \
	X(DTAGHTDCDigiHit) \
	X(DTPOLSectorDigiHit)\
	X(DDIRCTDCDigiHit) \

// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_occupancy_online());
  }
} // "C"


//------------------
// JEventProcessor_occupancy_online (Constructor)
//------------------
JEventProcessor_occupancy_online::JEventProcessor_occupancy_online()
{
	SetTypeName("JEventProcessor_occupancy_online");
}

//------------------
// ~JEventProcessor_occupancy_online (Destructor)
//------------------
JEventProcessor_occupancy_online::~JEventProcessor_occupancy_online()
{
  
}

//------------------
// Init
//------------------
void JEventProcessor_occupancy_online::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();
	lockService->RootWriteLock();

	// All histograms go in the "occupancy" directory
	TDirectory *main = gDirectory;
	gDirectory->mkdir("occupancy")->cd();
//	TDirectory *occ_dir = gDirectory;



	//------------------------ BCAL -----------------------
	//FADC
	bcal_adc_occ = new TH2I("bcal_adc_occ","ADC occupancy (DBCALDigiHit);Module", 48, 0.5, 48.5, 33, 0.5, 33.5);
	// Set y-axis labels for occupancy plots
	for(int ibin = 1; ibin <= 16; ibin++)
	{
		int idy = ibin-1; // convenient to use index that starts from zero!
		int layer  = 1 + (idy%4);
		int sector = 1 + idy/4;
		
		ostringstream ss;
		ss << "D  S" << sector << "  L" << layer;
		bcal_adc_occ->GetYaxis()->SetBinLabel(ibin, ss.str().c_str());

		ss.str("");
		ss << "U  S" << sector << "  L" << layer;
		bcal_adc_occ->GetYaxis()->SetBinLabel(ibin + 17, ss.str().c_str());
	}

	// TDC
	bcal_tdc_occ = new TH2I("bcal_tdc_occ","TDC occupancy (DBCALDigiTDCHit);Module", 48, 0.5, 48.5, 25, 0.5, 25.5);
	// Set y-axis labels for occupancy plots (without layer 4)
	for(int ibin = 1; ibin <= 12; ibin++)
	{
		int idy = ibin-1; // convenient to use index that starts from zero!
		
		int layer  = 1 + (idy%3);
		int sector = 1 + idy/3;
		
		ostringstream ss;
		ss << "D  S" << sector << "  L" << layer;
		bcal_tdc_occ->GetYaxis()->SetBinLabel(ibin, ss.str().c_str());

		ss.str("");
		ss << "U  S" << sector << "  L" << layer;
		bcal_tdc_occ->GetYaxis()->SetBinLabel(ibin + 13, ss.str().c_str());
	}
	bcal_num_events = new TH1I("bcal_num_events", "BCAL number of events", 1, 0.0, 1.0);

	//------------------------ CCAL -----------------------
	ccal_occ = new TH2F("ccal_occ", "CCAL Occupancy; column; row", 14, -1.5, 12.5, 14, -1.5, 12.5);
	ccal_num_events = new TH1I("ccal_num_events", "CCAL number of events", 1, 0.0, 1.0);

	//------------------------ CDC ------------------------
	int Nstraws[28] = {42, 42, 54, 54, 66, 66, 80, 80, 93, 93, 106, 106, 123, 123, 
			 135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209};
	double radius[28] = {10.72134, 12.08024, 13.7795, 15.14602, 18.71726, 20.2438, 22.01672, 
			   23.50008, 25.15616, 26.61158, 28.33624, 29.77388, 31.3817, 32.75838, 
			   34.43478, 35.81146, 38.28542, 39.7002, 41.31564, 42.73042, 44.34078, 
			   45.75302, 47.36084, 48.77054, 50.37582, 51.76012, 53.36286, 54.74716};
	double phi[28] = {0, 0.074707844, 0.038166294, 0.096247609, 0.05966371, 0.012001551, 0.040721951, 
			0.001334527, 0.014963808, 0.048683644, 0.002092645, 0.031681749, 0.040719354, 
			0.015197341, 0.006786058, 0.030005892, 0.019704045, -0.001782064, -0.001306618, 
			0.018592421, 0.003686784, 0.022132975, 0.019600866, 0.002343723, 0.021301449, 
			0.005348855, 0.005997358, 0.021018761}; 

	// Define a different 2D histogram for each ring. 
	// X-axis is phi, Y-axis is radius (to plot correctly with "pol" option)
	for(int iring=0; iring<28; iring++){
		double r_start = radius[iring] - 0.8;
		double r_end = radius[iring] + 0.8;
		double phi_start = phi[iring]; // this is for center of straw. Need additional calculation for phi at end plate
		double phi_end = phi_start + TMath::TwoPi();

		char hname[256];
		sprintf(hname, "cdc_occ_ring_%02d", iring+1);
		cdc_occ_ring[iring] = new TH2F(hname, "", Nstraws[iring], phi_start, phi_end, 1, r_start, r_end);
	}
	cdc_num_events = new TH1I("cdc_num_events", "CDC number of events", 1, 0.0, 1.0);
	new TH2D("cdc_axes", "CDC Occupancy", 100, -57.0*4.0/3.0, 57.0*4.0/3.0, 100, -57.0, 57.0);
    
	//------------------------ FCAL -----------------------
	fcal_occ = new TH2F("fcal_occ", "FCAL Occupancy; column; row", 61, -1.5, 59.5, 61, -1.5, 59.5);
	fcal_num_events = new TH1I("fcal_num_events", "FCAL number of events", 1, 0.0, 1.0);

	//------------------------ FDC ------------------------
    fdc_cathode_occ = new TH2F("fdc_cathode_occ","FDC Cathode Occupancy; Channel;", 48, 0.5, 48.5, 216, 0.5, 216.5); 
    fdc_wire_occ = new TH2F("fdc_wire_occ", "FDC Wire Occupancy; Channel;", 24, 0.5, 24.5, 96, 0.5, 96.5); 
    fdc_num_events = new TH1I("fdc_num_events", "FDC number of events", 1, 0.0, 1.0);

	//------------------------ FMWPC ----------------------
    fmwpc_occ = new TH2F("fmwpc_occ","FMWPC Occupancy; Layer; Channel;", 6, 0.5, 6.5, 144, 0.5, 144.5);
    fmwpc_num_events = new TH1I("fmwpc_num_events", "FMWPC number of events", 1, 0.0, 1.0); 

	//------------------------ PS/PSC ---------------------
	psc_adc_left_occ   = new TH1I("psc_adc_left_occ",  "PSC fADC hit occupancy Left", 8, 0.5, 8.5);
	psc_adc_right_occ  = new TH1I("psc_adc_right_occ", "PSC fADC hit occupancy Right", 8, 0.5, 8.5);
	psc_tdc_left_occ   = new TH1I("psc_tdc_left_occ",  "PSC TDC hit occupancy Left",  8, 0.5, 8.5);
	psc_tdc_right_occ  = new TH1I("psc_tdc_right_occ", "PSC TDC hit occupancy Right",  8, 0.5, 8.5);
	ps_left_occ        = new TH1I("ps_left_occ",       "PS fADC hit occupancy Left", 145, 0.5, 145.5);
	ps_right_occ       = new TH1I("ps_right_occ",      "PS fADC hit occupancy Right", 145, 0.5, 145.5);
	ps_num_events = new TH1I("ps_num_events", "PS number of events", 1, 0.0, 1.0);

	//------------------------ RF -------------------------
	rf_occ = new TH1D("rf_occ", "RF TDC Occupancy", 4, 0.5, 4.5);
	rf_occ->GetXaxis()->SetBinLabel(1, "FDC");
	rf_occ->GetXaxis()->SetBinLabel(2, "PSC");
	rf_occ->GetXaxis()->SetBinLabel(3, "TAGH");
	rf_occ->GetXaxis()->SetBinLabel(4, "TOF");
	rf_num_events = new TH1I("rf_num_events", "RF number of events", 1, 0.0, 1.0);
	dRFBinValueMap[SYS_FDC]  = 1.0;
	dRFBinValueMap[SYS_PSC]  = 2.0;
	dRFBinValueMap[SYS_TAGH] = 3.0;
	dRFBinValueMap[SYS_TOF]  = 4.0;
	
	//------------------------ Trigger -------------------------
	L1GTPRate = new TH2F("L1GTPRate","L1 GTP Rate by bit;Trigger Bit;L1 GTP Rate (kHz)", 8, 0.5, 8.5, 1000, 0.0, 100.0);
	L1livetime = new TH1F("L1livetime","L1 instantaneous livetime from TS scalers", 200, 0.0, 100.0);

	//------------------------ ST -------------------------
	st_adc_occ    = new TH1I("st_adc_occ", "ST fADC250 DigiHit Occupancy; Channel Number; fADC250 Counts", 30, 0.5, 30 + 0.5);
	st_tdc_occ    = new TH1I("st_tdc_occ", "ST TDC DigiHit Occupancy; Channel Number; TDC Counts", 30, 0.5, 30 + 0.5);
	st_num_events = new TH1I("st_num_events", "Start Counter number of events", 1, 0.0, 1.0);

	//------------------------ TAGH -----------------------
	const int Nslots = DTAGHGeometry::kCounterCount;
	tagh_adc_occ = new TH1I("tagh_adc_occ","TAGH fADC hit occupancy;counter (slot) ID;raw hits / counter",Nslots,0.5,0.5+Nslots);
	tagh_tdc_occ = new TH1I("tagh_tdc_occ","TAGH TDC hit occupancy;counter (slot) ID;raw hits / counter",Nslots,0.5,0.5+Nslots);
	tag_num_events = new TH1I("tag_num_events", "TAGGER number of events", 1, 0.0, 1.0);

	//------------------------ TAGM -----------------------
	const uint32_t NCOLUMNS = 102;
	tagm_adc_occ = new TH1I("tagm_adc_occ", "TAGM FADC250 column occupancy", NCOLUMNS, 0., NCOLUMNS + 1.);
	tagm_tdc_occ = new TH1I("tagm_tdc_occ", "TAGM F1TDC column occupancy",  NCOLUMNS, 0., NCOLUMNS + 1.);

	//------------------------ TPOL -----------------------
	const int Nsectors = DTPOLHit_factory::NSECTORS;
	tpol_occ = new TH1I("tpol_occ","TPOL fADC hit occupancy;sector;raw hits / counter",Nsectors,0.5,0.5+Nsectors);
	tpol_occ2 = new TH1I("tpol_occ2","TPOL fADC hit occupancy waveform[0] < 150.0;sector;raw hits / counter",Nsectors,0.5,0.5+Nsectors);
	tpol_occ3 = new TH1I("tpol_occ3","TPOL fADC hit occupancy waveform[0] < 150.0 & amp > 50.0;sector;raw hits / counter",Nsectors,0.5,0.5+Nsectors);

	//------------------------ TOF ------------------------
	tof_num_events = new TH1I("tof_num_events", "TOF number of events", 1, 0.0, 1.0);
	tof_tdc_S_occ = new TH1I("tof_tdc_S_occ","TOF, TDC Occupancy",88,1,46);
	tof_tdc_N_occ = new TH1I("tof_tdc_N_occ","TOF, TDC Occupancy",88,1,46);
	tof_tdc_U_occ = new TH1I("tof_tdc_U_occ","TOF, TDC Occupancy",88,1,46);
	tof_tdc_D_occ = new TH1I("tof_tdc_D_occ","TOF, TDC Occupancy",88,1,46);

	tof_adc_S_occ = new TH1I("tof_adc_S_occ","TOF, fADC Occupancy",88,1,46);
	tof_adc_N_occ = new TH1I("tof_adc_N_occ","TOF, fADC Occupancy",88,1,46);
	tof_adc_U_occ = new TH1I("tof_adc_U_occ","TOF, fADC Occupancy",88,1,46);
	tof_adc_D_occ = new TH1I("tof_adc_D_occ","TOF, fADC Occupancy",88,1,46);

	//------------------------ DIRC ------------------------
	dirc_num_events = new TH1I("dirc_num_events", "DIRC number of events", 1, 0.0, 1.0);
	dirc_tdc_pixel_N_occ_led = new TH2I("dirc_tdc_pixel_N_occ_led","DIRC, TDC North (Upper) Pixel Occupancy: LED trigger; pixel rows; pixel columns",144,-0.5,143.5,48,-0.5,47.5);
	dirc_tdc_pixel_S_occ_led = new TH2I("dirc_tdc_pixel_S_occ_led","DIRC, TDC South (Lower) Pixel Occupancy: LED trigger; pixel rows; pixel columns",144,-0.5,143.5,48,-0.5,47.5);
	dirc_tdc_pixel_N_occ = new TH2I("dirc_tdc_pixel_N_occ","DIRC, TDC North (Upper) Pixel Occupancy: Non-LED triggers; pixel rows; pixel columns",144,-0.5,143.5,48,-0.5,47.5);
        dirc_tdc_pixel_S_occ = new TH2I("dirc_tdc_pixel_S_occ","DIRC, TDC South (Lower) Pixel Occupancy; Non-LED triggers; pixel rows; pixel columns",144,-0.5,143.5,48,-0.5,47.5);

	//------------------------ DigiHits ------------------------
	#define FillDigiHitMap(A) digihitbinmap[#A]=(double)(digihitbinmap.size());
	DigiHitTypes(FillDigiHitMap) // Initialize digihitbinmap with values from DigiHitTypes

	digihits_trig1 = new TH2I("digihits_trig1", "Digihits", digihitbinmap.size(), 0.5, 0.5+(double)digihitbinmap.size(), 151, 0.0,  151.0);
	digihits_trig1->SetYTitle("Nhits/event");
	digihits_scale_factors = new TH1F("digihits_scale_factors", "Digihits scale factors", digihitbinmap.size()+1, 0.5, 0.5+(double)digihitbinmap.size()+1.0);
	for(auto p : digihitbinmap){
		digihits_trig1->GetXaxis()->SetBinLabel(p.second, p.first.c_str());
		digihits_scale_factors->GetXaxis()->SetBinLabel(p.second, p.first.c_str());
	}
	digihits_trig3 = (TH2I*)digihits_trig1->Clone("digihits_trig3");
	digihits_trig4 = (TH2I*)digihits_trig1->Clone("digihits_trig4");
	
	// Some detectors have a much higher or lower rates so we scale the number of hits to
	// fit them on the same histogram. We pass this to the RootSpy GUI in the form of a
	// histogram with one extra bin. The extra (last) bin will have its content set to
	// 1 so when RootSpy adds up N histograms, the last bin will keep track of how many
	// histos where added. This can then be divided out of the other values to get back
	// the original scale factors.
	digihitsclmap["DFDCCathodeDigiHit"] = 3000.0/150.0;
	digihitsclmap["DFDCWireDigiHit"]    = 1500.0/150.0;
	digihitsclmap["DCDCDigiHit"]        =  900.0/150.0;
	digihitsclmap["DBCALDigiHit"]       =  300.0/150.0;
	digihitsclmap["DBCALTDCDigiHit"]    =  300.0/150.0;
	digihitsclmap["DPSCDigiHit"]        =  1.0/10.0;
	digihitsclmap["DPSCTDCDigiHit"]     =  1.0/10.0;
	for(auto p : digihitsclmap) digihits_scale_factors->Fill(digihitbinmap[p.first], p.second);
	digihits_scale_factors->Fill((double)digihitbinmap.size()+1.0, 1.0); // set last bin content to 1.0

	// back to main dir
	main->cd();
  
	lockService->RootUnLock();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_occupancy_online::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes

  vector<const DDIRCGeometry*> locDIRCGeometry;
  event->Get(locDIRCGeometry);
  dDIRCGeometry = locDIRCGeometry[0];
}

//------------------
// Process
//------------------
void JEventProcessor_occupancy_online::Process(const std::shared_ptr<const JEvent>& event)
{
	// The following 2 lines will do the following for each DigiHitTypes type:
	//   vector<const A*> vDBCALDigiHit;
	//   event->Get(vDBCALDigiHit);
	#define GetDigihits(A) vector<const A*> v##A; event->Get(v##A);
	DigiHitTypes(GetDigihits)
	
	// L1 triggers
	vector<const DL1Trigger*>          l1triggers;
	event->Get(l1triggers);
	const DL1Trigger* l1trigger = l1triggers.empty() ? NULL : l1triggers[0];
	
	// trig[idx[  where idx=0-31 corresponds to "trig bit 1-32"
	vector<bool> trig(32, 0);
	vector<bool> fp_trig(32, 0);
	if(l1trigger){
		for(int itrig=0; itrig<32; itrig++) trig[itrig] = (l1trigger->trig_mask >> itrig)&0x01;
		for(int itrig=0; itrig<32; itrig++) fp_trig[itrig] = (l1trigger->fp_trig_mask >> itrig)&0x01;
	}
	else // not a triggered event
	  return;

	// calculate total BCAL energy in order to catch BCAL LED events
	vector<const DBCALHit *> bcal_hits;
	event->Get(bcal_hits);
	double total_bcal_energy = 0.;
	for(unsigned int i=0; i<bcal_hits.size(); i++) {
            total_bcal_energy += bcal_hits[i]->E;
        }
	
	vector<const DTPOLHit *> tpol_hits;
	event->Get(tpol_hits);

	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
	
	//------------------------ BCAL -----------------------
	
	// don't fill occupancy plots for BCAL LED events
	if(l1trigger){
	if( !((l1trigger->fp_trig_mask & 0x100) || (l1trigger->fp_trig_mask & 0x200)) 
               && (total_bcal_energy < 20.) ) {
  
            bcal_num_events->Fill(0.5);
	    //ADC
	    for(unsigned int i = 0; i < vDBCALDigiHit.size(); i++){
		const DBCALDigiHit *hit = vDBCALDigiHit[i];

		int ix = hit->module;
		int iy = (hit->sector-1)*4 + hit->layer;

		if(hit->end == DBCALGeometry::kUpstream)
			bcal_adc_occ->Fill(ix, iy+17);
		else if(hit->end == DBCALGeometry::kDownstream)
			bcal_adc_occ->Fill(ix, iy);
	    }

	    //TDC
	    for(unsigned int i = 0; i < vDBCALTDCDigiHit.size(); i++){
		const DBCALTDCDigiHit *hit = vDBCALTDCDigiHit[i];

		int ix = hit->module;
		int iy = (hit->sector-1)*3 + hit->layer; // TDC has 3 layers per sector

		if(hit->end == DBCALGeometry::kUpstream)
			bcal_tdc_occ->Fill(ix, iy+13);
		else if(hit->end == DBCALGeometry::kDownstream)
			bcal_tdc_occ->Fill(ix, iy);
	    }
        }
	}

	//------------------------ CCAL -----------------------
	ccal_num_events->Fill(0.5);
	for(size_t loc_i = 0; loc_i < vDCCALDigiHit.size(); ++loc_i){
		ccal_occ->Fill(vDCCALDigiHit[loc_i]->column, vDCCALDigiHit[loc_i]->row);
	}

	//------------------------ CDC ------------------------
	cdc_num_events->Fill(0.5);
	for(uint32_t i=0; i<vDCDCDigiHit.size(); i++) {

		const DCDCDigiHit *digihit = vDCDCDigiHit[i];  
		int ring     = digihit->ring-1; // start counting from zero! 
		int straw    = digihit->straw;  // first bin is one

		Double_t w = cdc_occ_ring[ring]->GetBinContent(straw, 1) + 1.0;
		cdc_occ_ring[ring]->SetBinContent(straw, 1, w);
	}

	//------------------------ FCAL -----------------------
	fcal_num_events->Fill(0.5);
	for(size_t loc_i = 0; loc_i < vDFCALDigiHit.size(); ++loc_i){
		fcal_occ->Fill(vDFCALDigiHit[loc_i]->column, vDFCALDigiHit[loc_i]->row);
	}
	
	//------------------------ FDC ------------------------
	fdc_num_events->Fill(0.5);
	for(unsigned int i = 0; i < vDFDCCathodeDigiHit.size(); i++){
		uint32_t strip = vDFDCCathodeDigiHit[i]->strip;
		if(vDFDCCathodeDigiHit[i]->strip_type == 3) strip += 9*12;
		int ud = -1;
		if (vDFDCCathodeDigiHit[i]->view == 3) ud = 0;
		fdc_cathode_occ->Fill((vDFDCCathodeDigiHit[i]->package - 1)*12 + vDFDCCathodeDigiHit[i]->chamber*2 + ud, strip);
	}
	for(unsigned int i = 0; i < vDFDCWireDigiHit.size(); i++){
		fdc_wire_occ->Fill((vDFDCWireDigiHit[i]->package - 1)*6 + vDFDCWireDigiHit[i]->chamber, vDFDCWireDigiHit[i]->wire);
	}

	//------------------------ FMWPC ----------------------
	fmwpc_num_events->Fill(0.5);
	for(unsigned int i = 0; i < vDFMWPCDigiHit.size(); i++){
		fmwpc_occ->Fill(vDFMWPCDigiHit[i]->layer, vDFMWPCDigiHit[i]->wire);
	}

	//------------------------ PS/PSC ---------------------
	ps_num_events->Fill(0.5);
	const int Nmods = 8; 
	for(unsigned int i=0; i < vDPSCDigiHit.size(); i++) {
		const DPSCDigiHit *hit = vDPSCDigiHit[i];
		if( hit->counter_id <= Nmods )
			psc_adc_left_occ->Fill(hit->counter_id);
		else
			psc_adc_right_occ->Fill(hit->counter_id - Nmods);
	}
	for(unsigned int i=0; i < vDPSCTDCDigiHit.size(); i++) {
		const DPSCTDCDigiHit *hit = vDPSCTDCDigiHit[i];
		if( hit->counter_id <= Nmods )
			psc_tdc_left_occ->Fill(hit->counter_id);
		else
			psc_tdc_right_occ->Fill(hit->counter_id - Nmods);
	}
	for(unsigned int i=0; i < vDPSDigiHit.size(); i++) {
		const DPSDigiHit *hit = vDPSDigiHit[i];
		if( hit->arm == 0 )
			ps_left_occ->Fill(hit->column);
		else
			ps_right_occ->Fill(hit->column);
	}

	//------------------------ RF -------------------------
	rf_num_events->Fill(0.5);
	for(size_t loc_i = 0; loc_i < vDRFTDCDigiTime.size(); ++loc_i){
		DetectorSystem_t locSystem = vDRFTDCDigiTime[loc_i]->dSystem;
		rf_occ->Fill(dRFBinValueMap[locSystem]);
	}

	//------------------------ Trigger -------------------------
	if(l1trigger){
		if(!l1trigger->gtp_rate.empty())
		{
			// Sync Events
			for(unsigned int ii = 0; ii < 8; ++ii) L1GTPRate->Fill(ii + 1, Float_t(l1trigger->gtp_rate[ii])/1000.0);
			L1livetime->Fill((double)l1trigger->live_inst/10.0);
		}
	}

	//------------------------ ST -------------------------
	st_num_events->Fill(0.5);
	for(uint32_t i = 0; i < vDSCDigiHit.size();    i++) st_adc_occ->Fill(vDSCDigiHit[i]->sector);
	for(uint32_t i = 0; i < vDSCTDCDigiHit.size(); i++) st_tdc_occ->Fill(vDSCTDCDigiHit[i]->sector);

	//------------------------ TAGH -----------------------
 	tag_num_events->Fill(0.5);
   for(unsigned int i=0; i < vDTAGHDigiHit.size();    i++) tagh_adc_occ->Fill(vDTAGHDigiHit[i]->counter_id);
   for(unsigned int i=0; i < vDTAGHTDCDigiHit.size(); i++) tagh_tdc_occ->Fill(vDTAGHTDCDigiHit[i]->counter_id);

	//------------------------ TAGM -----------------------
	for(uint32_t i=0; i< vDTAGMDigiHit.size(); i++) {
		const DTAGMDigiHit *hit = vDTAGMDigiHit[i];
		if (hit->row == 0) tagm_adc_occ->Fill(hit->column);
	}
	for(uint32_t i=0; i< vDTAGMTDCDigiHit.size(); i++) {
		const DTAGMTDCDigiHit *hit = vDTAGMTDCDigiHit[i];
		if (hit->row == 0) tagm_tdc_occ->Fill(hit->column);
	}

	//------------------------ TPOL -----------------------
	for(unsigned int i=0; i < tpol_hits.size(); i++) 
	{
		tpol_occ->Fill(tpol_hits[i]->sector);
		if (tpol_hits[i]->w_samp1 > 160.0) continue;
		tpol_occ2->Fill(tpol_hits[i]->sector);		
		if (tpol_hits[i]->pulse_peak > 60.0) continue;
		tpol_occ3->Fill(tpol_hits[i]->sector);
	}

	//------------------------ TOF ------------------------
	tof_num_events->Fill(0.5);
	// fADC Hits
	for(uint32_t i=0; i<vDTOFDigiHit.size(); i++){

		const DTOFDigiHit *hit = vDTOFDigiHit[i];
		int plane = hit->plane;
		int bar   = hit->bar;
		int end   = hit->end;

		if( plane==0 && end==0 ) tof_adc_U_occ->Fill(bar);
		if( plane==0 && end==1 ) tof_adc_D_occ->Fill(bar);
		if( plane==1 && end==0 ) tof_adc_N_occ->Fill(bar);
		if( plane==1 && end==1 ) tof_adc_S_occ->Fill(bar);
	}

	// TDC Hits
	for(uint32_t i=0; i<vDTOFTDCDigiHit.size(); i++){

		const DTOFTDCDigiHit *hit = vDTOFTDCDigiHit[i];
		int plane = hit->plane;
		int bar   = hit->bar;
		int end   = hit->end;

		if( plane==0 && end==0 ) tof_tdc_U_occ->Fill(bar);
		if( plane==0 && end==1 ) tof_tdc_D_occ->Fill(bar);
		if( plane==1 && end==0 ) tof_tdc_N_occ->Fill(bar);
		if( plane==1 && end==1 ) tof_tdc_S_occ->Fill(bar);
	}

	//------------------------ DIRC ------------------------
	dirc_num_events->Fill(0.5);
	for(uint32_t i=0; i<vDDIRCTDCDigiHit.size(); i++){

		const DDIRCTDCDigiHit *hit = vDDIRCTDCDigiHit[i];
		int ch = hit->channel;

		if(ch >= 108*64) {
			if(fp_trig[14]) dirc_tdc_pixel_N_occ_led->Fill(dDIRCGeometry->GetPixelRow(ch), dDIRCGeometry->GetPixelColumn(ch));
			else dirc_tdc_pixel_N_occ->Fill(dDIRCGeometry->GetPixelRow(ch), dDIRCGeometry->GetPixelColumn(ch));
		}
		else { 
			if(fp_trig[14]) dirc_tdc_pixel_S_occ_led->Fill(dDIRCGeometry->GetPixelRow(ch), dDIRCGeometry->GetPixelColumn(ch));
			else dirc_tdc_pixel_S_occ->Fill(dDIRCGeometry->GetPixelRow(ch), dDIRCGeometry->GetPixelColumn(ch));
		}
	}

	//------------------------ DigiHits ------------------------
	#define FillDigiHitHist(A) \
		scale = digihitsclmap[#A]; \
		if(scale==0.0) scale = digihitsclmap[#A] = 1.0; \
		digihits_trigX->Fill(digihitbinmap[#A], v##A.size()/scale);
		//digihits_hi_occ_trigX->Fill(digihitbinmap[#A], v##A.size());
	double scale;
	if(trig[0]){  // FCAL+BCAL
		TH2I *digihits_trigX = digihits_trig1;
		DigiHitTypes(FillDigiHitHist)
	}
	if(trig[2]){  // BCAL 
		TH2I *digihits_trigX = digihits_trig3;
		DigiHitTypes(FillDigiHitHist)
	}
	if(trig[3]){  // PS
		TH2I *digihits_trigX = digihits_trig4;
		DigiHitTypes(FillDigiHitHist)
	}

	lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_occupancy_online::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_occupancy_online::Finish()
{
  // Called before program exit after event processing is finished.
}

