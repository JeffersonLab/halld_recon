// $Id$
//
//    File: JEventProcessor_BCAL_LED.cc
//

#include "JEventProcessor_BCAL_LED.h"



//----------------------------------------------------------------------------------

// 	string make_filename( const string& basename, int index, const string& ext )
// 	  {
// 	  ostringstream result;
// 	  result << basename << index << ext;
// 	  return result.str();
// 	  }


// Routine used to create our JEventProcessor
extern "C"{
	void InitPlugin(JApplication *app){
		InitJANAPlugin(app);
		app->Add(new JEventProcessor_BCAL_LED());
	}

}

	//define static local variable //declared in header file
	thread_local DTreeFillData JEventProcessor_BCAL_LED::dTreeFillData;

//----------------------------------------------------------------------------------


JEventProcessor_BCAL_LED::JEventProcessor_BCAL_LED() {
	SetTypeName("JEventProcessor_BCAL_LED");
}


//----------------------------------------------------------------------------------


JEventProcessor_BCAL_LED::~JEventProcessor_BCAL_LED() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_BCAL_LED::Init() {

	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	// First thread to get here makes all histograms. If one pointer is
	// already not NULL, assume all histograms are defined and return now
// 	if(bcal_peak != NULL){
// 		lockService->RootUnLock();
// 		return;
// 	}
	
        //int up_trigger_count = 0;
        //int down_trigger_count = 0;
	//NOtrig=0; FPtrig=0; GTPtrig=0; FPGTPtrig=0; trigUS=0; trigDS=0; trigCosmic=0;
	//low_down_1_counter=0; low_down_2_counter=0; low_down_3_counter=0; low_down_4_counter=0; low_up_1_counter=0; low_up_2_counter=0; low_up_3_counter=0; 		low_up_4_counter=0; high_down_1_counter=0; high_down_2_counter=0; high_down_3_counter=0; high_down_4_counter=0; high_up_1_counter=0;
	//high_up_2_counter=0; high_up_3_counter=0; high_up_4_counter=0;
	//unidentified = 0; ledcounter = 0;
	//overflow=0; underflow=0; negatives=0; zeros=0;

	// create root folder for bcal and cd to it, store main dir
	TDirectory *main = gDirectory;
	gDirectory->mkdir("bcalLED")->cd();
	//gStyle->SetOptStat(111110);

    //TTREE INTERFACE
    //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
    dTreeInterface = DTreeInterface::Create_DTreeInterface("BCAL_LED", "BCAL_LED_diagnostic.root");

    //TTREE BRANCHES
    DTreeBranchRegister locTreeBranchRegister;
    
    locTreeBranchRegister.Register_Single<ULong64_t>("EventNumber");//0
    locTreeBranchRegister.Register_Single<Double_t>("bcal_peak");//1
    
    locTreeBranchRegister.Register_Single<Double_t>("up_peak");//2
    locTreeBranchRegister.Register_Single<Double_t>("down_peak");//3
    
    locTreeBranchRegister.Register_Single<Double_t>("sector1_peak");//4
    locTreeBranchRegister.Register_Single<Double_t>("sector2_peak");//5
    locTreeBranchRegister.Register_Single<Double_t>("sector3_peak");//;6
    locTreeBranchRegister.Register_Single<Double_t>("sector4_peak");//7

    locTreeBranchRegister.Register_Single<Double_t>("sector1_up_peak");//8
    locTreeBranchRegister.Register_Single<Double_t>("sector2_up_peak");//9
    locTreeBranchRegister.Register_Single<Double_t>("sector3_up_peak");//10
    locTreeBranchRegister.Register_Single<Double_t>("sector4_up_peak");//11

    locTreeBranchRegister.Register_Single<Double_t>("sector1_down_peak");//12
    locTreeBranchRegister.Register_Single<Double_t>("sector2_down_peak");//13
    locTreeBranchRegister.Register_Single<Double_t>("sector3_down_peak");//14
    locTreeBranchRegister.Register_Single<Double_t>("sector4_down_peak");//15
    
    locTreeBranchRegister.Register_Single<Double_t>("sector1_up_peak_lowup");//16
    locTreeBranchRegister.Register_Single<Double_t>("sector2_up_peak_lowup");//17
    locTreeBranchRegister.Register_Single<Double_t>("sector3_up_peak_lowup");//18
    locTreeBranchRegister.Register_Single<Double_t>("sector4_up_peak_lowup");//19

    locTreeBranchRegister.Register_Single<Double_t>("sector1_down_peak_lowup");//20
    locTreeBranchRegister.Register_Single<Double_t>("sector2_down_peak_lowup");//21
    locTreeBranchRegister.Register_Single<Double_t>("sector3_down_peak_lowup");//22
    locTreeBranchRegister.Register_Single<Double_t>("sector4_down_peak_lowup");//23
    
    locTreeBranchRegister.Register_Single<Double_t>("sector1_up_peak_lowdown");//24
    locTreeBranchRegister.Register_Single<Double_t>("sector2_up_peak_lowdown");//25
    locTreeBranchRegister.Register_Single<Double_t>("sector3_up_peak_lowdown");//26
    locTreeBranchRegister.Register_Single<Double_t>("sector4_up_peak_lowdown");//27

    locTreeBranchRegister.Register_Single<Double_t>("sector1_down_peak_lowdown");//28
    locTreeBranchRegister.Register_Single<Double_t>("sector2_down_peak_lowdown");//29
    locTreeBranchRegister.Register_Single<Double_t>("sector3_down_peak_lowdown");//30
    locTreeBranchRegister.Register_Single<Double_t>("sector4_down_peak_lowdown");//31
    
    locTreeBranchRegister.Register_Single<Double_t>("sector1_up_peak_highup");//32
    locTreeBranchRegister.Register_Single<Double_t>("sector2_up_peak_highup");//33
    locTreeBranchRegister.Register_Single<Double_t>("sector3_up_peak_highup");//34
    locTreeBranchRegister.Register_Single<Double_t>("sector4_up_peak_highup");//35

    locTreeBranchRegister.Register_Single<Double_t>("sector1_down_peak_highup");//36
    locTreeBranchRegister.Register_Single<Double_t>("sector2_down_peak_highup");//37
    locTreeBranchRegister.Register_Single<Double_t>("sector3_down_peak_highup");//38
    locTreeBranchRegister.Register_Single<Double_t>("sector4_down_peak_highup");//39
    
    locTreeBranchRegister.Register_Single<Double_t>("sector1_up_peak_highdown");//40
    locTreeBranchRegister.Register_Single<Double_t>("sector2_up_peak_highdown");//41
    locTreeBranchRegister.Register_Single<Double_t>("sector3_up_peak_highdown");//42
    locTreeBranchRegister.Register_Single<Double_t>("sector4_up_peak_highdown");//43

    locTreeBranchRegister.Register_Single<Double_t>("sector1_down_peak_highdown");//44
    locTreeBranchRegister.Register_Single<Double_t>("sector2_down_peak_highdown");//45
    locTreeBranchRegister.Register_Single<Double_t>("sector3_down_peak_highdown");//46
    locTreeBranchRegister.Register_Single<Double_t>("sector4_down_peak_highdown");//47
    
    //REGISTER BRANCHES
     dTreeInterface->Create_Branches(locTreeBranchRegister);
    
    	// lock all root operations
   	lockService->RootWriteLock();
 
 	bcal_vevent = new TProfile("bcal_vevent","Avg BCAL peak vs event;event num;peak",48,0.0,48.0);
	
	low_up_1 = new TProfile("low_bias_up_sector_1_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	low_up_2 = new TProfile("low_bias_up_sector_2_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	low_up_3 = new TProfile("low_bias_up_sector_3_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	low_up_4 = new TProfile("low_bias_up_sector_4_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	
	low_down_1 = new TProfile("low_bias_down_sector_1_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	low_down_2 = new TProfile("low_bias_down_sector_2_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	low_down_3 = new TProfile("low_bias_down_sector_3_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	low_down_4 = new TProfile("low_bias_down_sector_4_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	
	high_up_1 = new TProfile("high_bias_up_sector_1_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	high_up_2 = new TProfile("high_bias_up_sector_2_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	high_up_3 = new TProfile("high_bias_up_sector_3_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	high_up_4 = new TProfile("high_bias_up_sector_4_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	
	high_down_1 = new TProfile("high_bias_down_sector_1_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	high_down_2 = new TProfile("high_bias_down_sector_2_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	high_down_3 = new TProfile("high_bias_down_sector_3_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);
	high_down_4 = new TProfile("high_bias_down_sector_4_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);	

	down = new TProfile("down_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);	
	up = new TProfile("up_peak_vchannel","Avg BCAL peak vs channel;channel ID;peak",1536,0,1536);	


	h2_ledboth_Aall_vs_event = new TProfile("h2_ledboth_Aall_vs_event", "LED uboth - Aup and Adown vs event", 20000,0,200000000);
	h2_ledboth_sector_vs_event = new TProfile("h2_ledboth_sector_vs_event", "LED both - sector vs event", 20000,0,200000000);

	h1_ledup_sector = new TH1I("h1_ledup_sector", "LED up - sector", 50,0,5);
	h2_ledup_z_vs_cellid = new TH2I("h2_ledup_z_vs_cellid", "LED up - z vs Chan ID", 800,0,800,500,-100,400);
	h1_ledup_sector_config = new TH1I("h1_ledup_sector_config", "LED up - sector -config", 5,0,5);
	h1_ledup_z_all = new TH1I("h1_ledup_z_all", "LED up - z all channels", 500,-100,400);
	h1_ledup_Tdiff_all = new TH1I("h1_ledup_Tdiff_all", "LED up - Tdiff all channels", 400,-50,50);
	h1_ledup_Tup_all = new TH1I("h1_ledup_Tup_all", "LED up - Tup all channels", 410,0,410);
	h1_ledup_Tdown_all = new TH1I("h1_ledup_Tdown_all", "LED up - Tdown all channels", 410,0,410);
	h1_ledup_Aup_all = new TH1I("h1_ledup_Aup_all", "LED up - Aup all channels", 410,0,4100);
	h1_ledup_Adown_all = new TH1I("h1_ledup_Adown_all", "LED up - Adown all channels", 410,0,4100);
	h2_ledup_Aup_vs_z = new TH2I("h2_ledup_Aup_vs_z", "LED up - Aup vs z", 100,-100,400,410,0,4100);
	h2_ledup_Adown_vs_z = new TH2I("h2_ledup_Adown_vs_z", "LED up - Adown vs z", 100,-100,400,410,0,4100);
	h2_ledup_Aup_vs_event = new TProfile("h2_ledup_Aup_vs_event", "LED up - Aup vs event", 20000,0,200000000);
	h2_ledup_Adown_vs_event = new TProfile("h2_ledup_Adown_vs_event", "LED up - Adown vs event", 20000,0,200000000);
	h2_ledup_Aall_vs_event = new TProfile("h2_ledup_Aall_vs_event", "LED up - Aup and Adown vs event", 20000,0,200000000);
	h2_ledup_sector_vs_event = new TProfile("h2_ledup_sector_vs_event", "LED up - sector vs event", 20000,0,200000000);

	h1_leddown_sector = new TH1I("h1_leddown_sector", "LED down - sector", 50,0,5);
	h2_leddown_z_vs_cellid = new TH2I("h2_leddown_z_vs_cellid", "LED down - z vs Chan ID", 800,0,800,500,-100,400);
	h1_leddown_sector_config = new TH1I("h1_leddown_sector_config", "LED down - sector -config", 5,0,5);
	h1_leddown_z_all = new TH1I("h1_leddown_z_all", "LED down - z all channels", 500,-100,400);
	h1_leddown_Tdiff_all = new TH1I("h1_leddown_Tdiff_all", "LED down - Tdiff all channels", 400,-50,50);
	h1_leddown_Tup_all = new TH1I("h1_leddown_Tup_all", "LED down - Tup all channels", 410,0,410);
	h1_leddown_Tdown_all = new TH1I("h1_leddown_Tdown_all", "LED down - Tdown all channels", 410,0,410);
	h1_leddown_Aup_all = new TH1I("h1_leddown_Aup_all", "LED down - Aup all channels", 410,0,4100);
	h1_leddown_Adown_all = new TH1I("h1_leddown_Adown_all", "LED down - Adown all channels", 410,0,4100);
	h2_leddown_Aup_vs_z = new TH2I("h2_leddown_Aup_vs_z", "LED down - Aup vs z", 100,-100,400,410,0,4100);
	h2_leddown_Adown_vs_z = new TH2I("h2_leddown_Adown_vs_z", "LED down - Adown vs z", 100,-100,400,410,0,4100);
	h2_leddown_Aup_vs_event = new TProfile("h2_leddown_Aup_vs_event", "LED down - Aup vs event", 20000,0,200000000);
	h2_leddown_Adown_vs_event = new TProfile("h2_leddown_Adown_vs_event", "LED down - Adown vs event", 20000,0,200000000);
	h2_leddown_Aall_vs_event = new TProfile("h2_leddown_Aall_vs_event", "LED down - Aup and Adown vs event", 20000,0,200000000);
	h2_leddown_sector_vs_event = new TProfile("h2_leddown_sector_vs_event", "LED down - sector vs event", 20000,0,200000000);

	// back to main dir
	main->cd();
	
	gDirectory->mkdir("BCAL_LED_highlevel")->cd();
	
	dHist_L1bits_fp  = new TH1I("L1bits_fp", "L1 trig bits from FP;Trig. bit (9-10)", 4, 7.5, 11.5);

	dHist_L1bits_fp_twelvehundhits  = new TH1I("L1bits_fp_twelvehundhits", "Pseudo-trig bits (FP or >1200 hits);Trig. bit (1-32)", 4, 7.5, 11.5);

	dHist_quad_count_up  = new TH1I("dHist_quad_count_up", "Quadrants Count - Upstream LED; Quadrant (1-4)", 8, 0.5, 1535.5);
	dHist_quad_count_down  = new TH1I("dHist_quad_count_down", "Quadrants Count - Downstream LED; Quadrant (1-4)", 8, 0.5, 1535.5);

// 	dHist_quad_occ_up  = new TH2I("dHist_quad_occ_up", "Quadrants Occupancy - Upstream Pulser", 2, 0.5, 2.5, 2, 0.5, 2.5);
// 	dHist_quad_occ_down  = new TH2I("dHist_quad_occ_down", "Quadrants triggered -  Downstream Pulser", 2, 0.5, 2.5, 2, 0.5, 2.5);

	dHist_quad_occ_up  = new TH1F("dHist_quad_occ_up", "Quadrants Occupancy - Upstream Pulser", 4, 0.5, 4.5);
	dHist_quad_occ_down  = new TH1F("dHist_quad_occ_down", "Quadrants triggered -  Downstream Pulser", 4, 0.5, 4.5);
	// back to main dir
	main->cd();
	// unlock
	lockService->RootUnLock();
	
}


//----------------------------------------------------------------------------------


void JEventProcessor_BCAL_LED::BeginRun(const std::shared_ptr<const JEvent>& event) {
  	// This is called whenever the run number changes

  	auto runnumber = event->GetRunNumber();
	adccount = 1700; //default threshold
	
	if (runnumber > 50742)
	{//fall2018 all sectors pulsed simultaneously with single bias
	
	simultaneous = 1;
	}
}


//----------------------------------------------------------------------------------


void JEventProcessor_BCAL_LED::Process(const std::shared_ptr<const JEvent>& event) {
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop-Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.

	auto eventnumber = event->GetEventNumber();
	
	int chcounter[1536] = { 0 } ;
	  
	
	vector<const DBCALDigiHit*> bcaldigihits;
	vector<const DBCALHit*> dbcalhits;
	vector<const DBCALPoint*> dbcalpoints;
	vector <const DBCALHit*> hitVector;
	
	
	bool LED_US=0, LED_DS=0;
	
	const DL1Trigger *trig = NULL;
	try {
		event->GetSingle(trig);
	} catch (...) {}
	if (trig) {

		if (trig->trig_mask){
			// GTP tigger
			//GTPtrig++;
		}
		if (trig->fp_trig_mask){
			// Front panel trigger
			//FPtrig++;
		}
		if (trig->trig_mask && trig->fp_trig_mask){
			// Both GTP and front panel trigger
			//FPGTPtrig++;
		}
		if (trig->trig_mask & 0x1){
			// Cosmic trigger fired
			//trigCosmic++;
		}
		if (trig->fp_trig_mask & 0x100){//bit=9
			// Upstream LED trigger fired
			//trigUS++;
			LED_US=1;
			
		}
		if (trig->fp_trig_mask & 0x200){//bit=10
			// Downstream LED trigger fired
			//trigDS++;
			LED_DS=1;
		
		}
	} else {
		//NOtrig++;
	}	
	// Lock ROOT
	lockService->RootWriteLock();

	float ledup_sector = 0;
	//int ledup_sector_int = 0;
	float ledup_mean = 0;
	int ledup_events = 0;
	float leddown_sector = 0;
	//int leddown_sector_int = 0;
	float leddown_mean = 0;
	int leddown_events = 0;
	int pseudo_triggerbit = 0;
//	int up_trigger_count = 0;
//	int down_trigger_count = 0;
		
		dTreeFillData.Fill_Single<ULong64_t>("EventNumber", eventnumber); 

		event->Get(dbcalhits);
		event->Get(bcaldigihits);
		event->Get(dbcalpoints);
		
      if (LED_US || LED_DS || dbcalhits.size() >= 1200.) {
	        // float apedsubtime[1536] = { 0. };
	        int apedsubpeak[1536] = { 0 };
		int cellsector[1536] =  { 0 };
		int cellend[1536] =  { 0 };
		     
		for( unsigned int i=0; i<dbcalpoints.size(); i++) {

			int module = dbcalpoints[i]->module();
			int layer = dbcalpoints[i]->layer();
			int sector = dbcalpoints[i]->sector();
			int cell_id = (module-1)*16 + (layer-1)*4 + sector-1;
			float z =  dbcalpoints[i]->z();
			dbcalpoints[i]->Get(hitVector);
			const DBCALHit *Hit1 = hitVector[0];
			const DBCALHit *Hit2 = hitVector[1];
			// float Aup =  dbcalpoints[i]->E_US();
			// float Adown =  dbcalpoints[i]->E_DS();
			float Aup = 0;
			float Adown = 0;
			float Tup = 0;
			float Tdown = 0;
			float Tdiff = 0;
			if (Hit1->end == DBCALGeometry::kUpstream && Hit2->end == DBCALGeometry::kDownstream) {	  
			  Aup = Hit1->pulse_peak;
			  Adown = Hit2->pulse_peak;	  
			  Tup = Hit1->t_raw;
			  Tdown = Hit2->t_raw;
			  Tdiff = Tdown - Tup;
  			  cellend[cell_id] = Hit2->end;
			  cellend[cell_id+768] = Hit1->end;
			}
			else if (Hit2->end == DBCALGeometry::kUpstream && Hit1->end == DBCALGeometry::kDownstream){
			  Aup = Hit2->pulse_peak;
			  Adown = Hit1->pulse_peak;
			  Tup = Hit2->t_raw;
			  Tdown = Hit1->t_raw;
			  Tdiff = Tdown - Tup;
  			  cellend[cell_id] = Hit1->end;
			  cellend[cell_id+768] = Hit2->end;
			}

			// fill histograms for all channels
			if (LED_US) {
			  h1_ledup_sector->Fill(sector);
			  h1_ledup_z_all->Fill(z);;
			  h2_ledup_z_vs_cellid->Fill(cell_id,z);
			  h1_ledup_Tdiff_all->Fill(Tdiff);;
			  h1_ledup_Tup_all->Fill(Tup);;
			  h1_ledup_Tdown_all->Fill(Tdown);;
			  h1_ledup_Aup_all->Fill(Aup);;
			  h1_ledup_Adown_all->Fill(Adown);;
			  h2_ledup_Aup_vs_z->Fill(z,Aup);
			  h2_ledup_Adown_vs_z->Fill(z,Adown);
			  h2_ledup_Aup_vs_event->Fill(eventnumber,Aup);
			  h2_ledup_Adown_vs_event->Fill(eventnumber,Adown);
			  // cout << "US - eventnumber=" << eventnumber << " Aup=" << Aup << " Aup=" << Aup << " Adown=" << Adown << " Adown=" << Adown << endl;
			}
			else if (LED_DS) {
			  h1_leddown_sector->Fill(sector);
			  h1_leddown_z_all->Fill(z);
			  h2_leddown_z_vs_cellid->Fill(cell_id,z);
			  h1_leddown_Tdiff_all->Fill(Tdiff);;
			  h1_leddown_Tup_all->Fill(Tup);;
			  h1_leddown_Tdown_all->Fill(Tdown);;
			  h1_leddown_Aup_all->Fill(Aup);;
			  h1_leddown_Adown_all->Fill(Adown);;
			  h2_leddown_Aup_vs_z->Fill(z,Aup);
			  h2_leddown_Adown_vs_z->Fill(z,Adown);
			  h2_leddown_Aup_vs_event->Fill(eventnumber,Aup);
			  h2_leddown_Adown_vs_event->Fill(eventnumber,Adown);
			  // cout << "DS - eventnumber=" << eventnumber << " Aup=" << Aup << " Aup=" << Aup << " Adown=" << Adown << " Adown=" << Adown << endl;
			}

			// make cuts on z for all hits
			// if ( LED_DS && (z>280 && z<400)) {
			// cut on Tdiff instead
			if ( LED_DS && (Tdiff>-30 && Tdiff<-15)) {
				apedsubpeak[cell_id] = Adown;
				chcounter[cell_id]++;
				apedsubpeak[cell_id+768] = Aup;
				chcounter[cell_id+768]++;
				h2_ledboth_Aall_vs_event->Fill(eventnumber,Aup);
				h2_ledboth_Aall_vs_event->Fill(eventnumber,Adown);
				h2_leddown_Aall_vs_event->Fill(eventnumber,Aup);
				h2_leddown_Aall_vs_event->Fill(eventnumber,Adown);
				h2_leddown_sector_vs_event->Fill(eventnumber,sector);
				h2_ledboth_sector_vs_event->Fill(eventnumber,sector);
				
				// compute sums
				leddown_sector += sector;
				leddown_mean += Aup + Adown;
				leddown_events++;

				cellsector[cell_id] = sector;
				cellsector[cell_id+768] = sector;
				
			} // if condition on Tdiff

			// if (LED_US && (z>-60 && z<0)) {
			if (LED_US && (Tdiff>15 && Tdiff<30)) {
				apedsubpeak[cell_id] = Adown;
				chcounter[cell_id]++;
				apedsubpeak[cell_id+768] = Aup;
				chcounter[cell_id+768]++;
				h2_ledboth_Aall_vs_event->Fill(eventnumber,Aup);
				h2_ledboth_Aall_vs_event->Fill(eventnumber,Adown);
				h2_ledup_Aall_vs_event->Fill(eventnumber,Aup);
				h2_ledup_Aall_vs_event->Fill(eventnumber,Adown);
				h2_ledup_sector_vs_event->Fill(eventnumber,sector);
				h2_ledboth_sector_vs_event->Fill(eventnumber,sector);
				
				// compute sums
				ledup_sector += sector;
				ledup_mean += Aup + Adown;
				ledup_events++;

				cellsector[cell_id] = sector;
				cellsector[cell_id+768] = sector;
				
			} // if condition on z

		}//loop over bcalhits

	for (int chid = 0; chid < 1536; chid++)
	    {
	      if (chcounter[chid] != 1) continue;
	      bcal_vevent->Fill(1,apedsubpeak[chid]); 
				if (cellend[chid] == DBCALGeometry::kUpstream)
				   {bcal_vevent->Fill(2,apedsubpeak[chid]);
				    if (chid%4 == 0) {bcal_vevent->Fill(8,apedsubpeak[chid]);}
				    else if (chid%4 == 1) {bcal_vevent->Fill(9,apedsubpeak[chid]);}
				    else if (chid%4 == 2) {bcal_vevent->Fill(10,apedsubpeak[chid]);}
				    else if (chid%4 == 3) {bcal_vevent->Fill(11,apedsubpeak[chid]);}
				   }
				    
				else if (cellend[chid] == DBCALGeometry::kDownstream) 
				    {bcal_vevent->Fill(3,apedsubpeak[chid]);
				    if (chid%4 == 0) {bcal_vevent->Fill(12,apedsubpeak[chid]);}
				    else if (chid%4 == 1) {bcal_vevent->Fill(13,apedsubpeak[chid]);}
				    else if (chid%4 == 2) {bcal_vevent->Fill(14,apedsubpeak[chid]);}
				    else if (chid%4 == 3) {bcal_vevent->Fill(15,apedsubpeak[chid]);}
				   }
				    
				if (cellsector[chid] == 1) {bcal_vevent->Fill(4,apedsubpeak[chid]);}
				else if (cellsector[chid] == 2) {bcal_vevent->Fill(5,apedsubpeak[chid]);}
				else if (cellsector[chid] == 3) {bcal_vevent->Fill(6,apedsubpeak[chid]);}
				else if (cellsector[chid] == 4) {bcal_vevent->Fill(7,apedsubpeak[chid]);}
		}//loop over bcalhits
    
		 //Deduce LED pulsing configuration based on average pulse peak in BCAL, each side & each sector then fill correponding profile.
		 double sector1up = 0;
 		 double sector2up = 0;
		 double sector3up = 0;
		 double sector4up = 0;

		 double sector1down = 0;
		 double sector2down = 0;
 		 double sector3down = 0;
		 double sector4down = 0;
		 
		 sector1up = bcal_vevent->GetBinContent(bcal_vevent->FindBin(8));
 		 sector2up = bcal_vevent->GetBinContent(bcal_vevent->FindBin(9));
		 sector3up = bcal_vevent->GetBinContent(bcal_vevent->FindBin(10));
		 sector4up = bcal_vevent->GetBinContent(bcal_vevent->FindBin(11));

		 sector1down = bcal_vevent->GetBinContent(bcal_vevent->FindBin(12));
		 sector2down = bcal_vevent->GetBinContent(bcal_vevent->FindBin(13));
 		 sector3down = bcal_vevent->GetBinContent(bcal_vevent->FindBin(14));
		 sector4down = bcal_vevent->GetBinContent(bcal_vevent->FindBin(15));

if (simultaneous == 0){//Pulsing sectors separately for runs before 50743

		 if (LED_US || sector1down+sector2down+sector3down+sector4down > sector1up+sector2up+sector3up+sector4up){
			//set psedo-trigger bit=9 for upstream pulser.
		 	pseudo_triggerbit = 9;
			up_trigger_count++;
			if (adccount > sector1down && sector1down > sector1up && sector1up > sector2up && sector1down > sector2down && sector1up > sector3up && sector1down > sector3down && sector1up > sector4up && sector1down > sector4down)
			    {//sector = 1;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 0 && apedsubpeak[k] > 0) {low_up_1->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
					    if (k < 768) {bcal_vevent->Fill(20,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(16,apedsubpeak[k]);
								}
					    }
			    }
			    //low_up_1_counter++;			    
			    }
			    
			else if (adccount > sector2down && sector2down > sector2up && sector2up > sector1up && sector2down > sector1down && sector2up > sector3up && sector2down > sector3down && sector2up > sector4up && sector2down > sector4down)
			    {//sector = 2;
                            for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 1 && apedsubpeak[k] > 0) {low_up_2->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
					    if (k < 768) {bcal_vevent->Fill(21,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(17,apedsubpeak[k]);
								}
					    }
			    }
			    //low_up_2_counter++;
			    }
			else if (adccount > sector3down && sector3down > sector3up && sector3up > sector1up && sector3down > sector1down && sector3up > sector2up && sector3down > sector2down && sector3up > sector4up && sector3down > sector4down)
			    {//sector = 3;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 2 && apedsubpeak[k] > 0) {low_up_3->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
					    if (k < 768) {bcal_vevent->Fill(22,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(18,apedsubpeak[k]);
								}
					    }
			    }
			    //low_up_3_counter++;
			    }
			else if (adccount > sector4down && sector4down > sector4up && sector4up > sector1up && sector4down > sector1down && sector4up > sector2up && sector4down > sector2down && sector4up > sector3up && sector4down > sector3down)
			    {//sector = 4;
                            for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 3 && apedsubpeak[k] > 0) {low_up_4->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
					    if (k < 768) {bcal_vevent->Fill(23,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(19,apedsubpeak[k]);
								}
					    }
			    }
			    //low_up_4_counter++;
			    }
			  

			else if (sector1down > adccount && sector1down > sector1up && sector1up > sector2up && sector1down > sector2down && sector1up > sector3up && sector1down > sector3down && sector1up > sector4up && sector1down > sector4down)
			    {//sector = 1;
                            for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 0 && apedsubpeak[k] > 0) {high_up_1->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(36,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(32,apedsubpeak[k]);
								}
					   }
			    }
			    //high_up_1_counter++;
			    }
			else if (sector2down > adccount && sector2down > sector2up && sector2up > sector1up && sector2down > sector1down && sector2up > sector3up && sector2down > sector3down && sector2up > sector4up && sector2down > sector4down)
			    {//sector = 2;
                            for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 1 && apedsubpeak[k] > 0) {high_up_2->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
     					    if (k < 768) {bcal_vevent->Fill(37,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(33,apedsubpeak[k]);
								}
					   }
			    }
			    //high_up_2_counter++;
			    }
			else if (sector3down > adccount && sector3down > sector3up && sector3up > sector1up && sector3down > sector1down && sector3up > sector2up && sector3down > sector2down && sector3up > sector4up && sector3down > sector4down)
			    {//sector = 3;
                            for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 2 && apedsubpeak[k] > 0) {high_up_3->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(38,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(34,apedsubpeak[k]);
								}
					   }
			    }
			    //high_up_3_counter++;
			    }
			else if (sector4down > adccount && sector4down > sector4up && sector4up > sector1up && sector4down > sector1down && sector4up > sector2up && sector4down > sector2down && sector4up > sector3up && sector4down > sector3down)
			    {//sector = 4;
                            for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 3 && apedsubpeak[k] > 0) {high_up_4->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_up->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(39,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(35,apedsubpeak[k]);
								}
					   }
			    }
			    //high_up_4_counter++;
			    }
			}//if LED_US
			
			if (LED_DS || sector1down+sector2down+sector3down+sector4down < sector1up+sector2up+sector3up+sector4up){
			//set psedo-trigger bit=10 for Downstream pulser.
		 	pseudo_triggerbit = 10;
                        down_trigger_count++;

			    if (adccount > sector1up && sector1up > sector1down && sector1up > sector2up && sector1down > sector2down && sector1up > sector3up && sector1down > sector3down && sector1up > sector4up && sector1down > sector4down)
			    {
			      //sector = 1;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 0 && apedsubpeak[k] > 0) {low_down_1->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(28,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(24,apedsubpeak[k]);
								}
					    }
			    }
			    //low_down_1_counter++;
			    }
			else if (adccount > sector2up && sector2up > sector2down && sector2up > sector1up && sector2down > sector1down && sector2up > sector3up && sector2down > sector3down && sector2up > sector4up && sector2down > sector4down)
			    {//sector = 2;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 1 && apedsubpeak[k] > 0) {low_down_2->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(29,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(25,apedsubpeak[k]);
								}
					    }
			    }
			    //low_down_2_counter++;
			    }
			else if (adccount > sector3up && sector3up > sector3down && sector3up > sector1up && sector3down > sector1down && sector3up > sector2up && sector3down > sector2down && sector3up > sector4up && sector3down > sector4down)
			    {//sector = 3;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 2 && apedsubpeak[k] > 0) {low_down_3->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(30,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(26,apedsubpeak[k]);
								}
					    }
			    }
			    //low_down_3_counter++;
			    }
			else if (adccount > sector4up && sector4up > sector4down && sector4up > sector1up && sector4down > sector1down && sector4up > sector2up && sector4down > sector2down && sector4up > sector3up && sector4down > sector3down)
			    {//sector = 4;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 3 && apedsubpeak[k] > 0) {low_down_4->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(31,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(27,apedsubpeak[k]);
								}
					    }
			    }
			    //low_down_4_counter++;
			    }
			else if (sector1up > adccount && sector1up > sector1down && sector1up > sector2up && sector1down > sector2down && sector1up > sector3up && sector1down > sector3down && sector1up > sector4up && sector1down > sector4down)
			    {//sector = 1;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 0 && apedsubpeak[k] > 0) {high_down_1->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(44,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(40,apedsubpeak[k]);
								}
					    }
			    }
			    //high_down_1_counter++;
			    }
			else if (sector2up > adccount && sector2up > sector2down && sector2up > sector1up && sector2down > sector1down && sector2up > sector3up && sector2down > sector3down && sector2up > sector4up && sector2down > sector4down)
			    {//sector = 2;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 1 && apedsubpeak[k] > 0) {high_down_2->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(45,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(41,apedsubpeak[k]);
								}
					    }
			    }
			    //high_down_2_counter++;
			    }
			else if (sector3up > adccount && sector3up > sector3down && sector3up > sector1up && sector3down > sector1down && sector3up > sector2up && sector3down > sector2down && sector3up > sector4up && sector3down > sector4down)
			    {//sector = 3;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 2 && apedsubpeak[k] > 0) {high_down_3->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(46,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(42,apedsubpeak[k]);
								}
					    }
			    }
			    //high_down_3_counter++;
			    }
			else if (sector4up > adccount && sector4up > sector4down && sector4up > sector1up && sector4down > sector1down && sector4up > sector2up && sector4down > sector2down && sector4up > sector3up && sector4down > sector3down)
			    {//sector = 4;
			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (k%4 == 3 && apedsubpeak[k] > 0) {high_down_4->Fill(k, apedsubpeak[k]);
								   dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(47,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(43,apedsubpeak[k]);
								}
					    }
			    }
			    //high_down_4_counter++;
			    }
		  
			}//if LED_DS
}//Pulsing sectors separately for runs before 50743

else if (simultaneous == 1){//Pulsing all sectors simultaneously starting run 50743
    
		 
		 if (LED_US || sector1down+sector2down+sector3down+sector4down > sector1up+sector2up+sector3up+sector4up){
			//set psedo-trigger bit=9 for upstream pulser.
		 	pseudo_triggerbit = 9;
                        up_trigger_count++;

			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (apedsubpeak[k] > 0) {up->Fill(k, apedsubpeak[k]);
						      dHist_quad_count_up->Fill(k);
					    if (k < 768) {bcal_vevent->Fill(20,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(16,apedsubpeak[k]);
								}
					    }
			    }
			    //up_counter++;			    
			    
			}//if LED_US
			
        if (LED_DS || sector1down+sector2down+sector3down+sector4down < sector1up+sector2up+sector3up+sector4up){
			//set psedo-trigger bit=10 for Downstream pulser.
		 	pseudo_triggerbit = 10;
	                down_trigger_count++;

			    for(int k=0 ;k < 1536;k++) 
			    {if (chcounter[k] != 1) continue;
			      if (apedsubpeak[k] > 0) {down->Fill(k, apedsubpeak[k]);
						      dHist_quad_count_down->Fill(k);
    					    if (k < 768) {bcal_vevent->Fill(28,apedsubpeak[k]);
							  }
					    else if  (k > 767) {bcal_vevent->Fill(24,apedsubpeak[k]);
								}
					    }
			    }
			    //down_counter++;
		  
			}//if LED_DS
}//Pulsing all sectors simultaneously starting run 50743

		//Fill diagnostic tree branches
		dTreeFillData.Fill_Single<Double_t>("bcal_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(1)));//1
    
		dTreeFillData.Fill_Single<Double_t>("up_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(2)));//2
		dTreeFillData.Fill_Single<Double_t>("down_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(3)));//3
		
		dTreeFillData.Fill_Single<Double_t>("sector1_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(4)));//4
		dTreeFillData.Fill_Single<Double_t>("sector2_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(5)));//5
		dTreeFillData.Fill_Single<Double_t>("sector3_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(6)));//;6
		dTreeFillData.Fill_Single<Double_t>("sector4_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(7)));//7

		dTreeFillData.Fill_Single<Double_t>("sector1_up_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(8)));//8
		dTreeFillData.Fill_Single<Double_t>("sector2_up_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(9)));//9
		dTreeFillData.Fill_Single<Double_t>("sector3_up_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(10)));//10
		dTreeFillData.Fill_Single<Double_t>("sector4_up_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(11)));//11

		dTreeFillData.Fill_Single<Double_t>("sector1_down_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(12)));//12
		dTreeFillData.Fill_Single<Double_t>("sector2_down_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(13)));//13
		dTreeFillData.Fill_Single<Double_t>("sector3_down_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(14)));//14
		dTreeFillData.Fill_Single<Double_t>("sector4_down_peak", bcal_vevent->GetBinContent(bcal_vevent->FindBin(15)));//15
		
		dTreeFillData.Fill_Single<Double_t>("sector1_up_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(16)));//16
		dTreeFillData.Fill_Single<Double_t>("sector2_up_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(17)));//17
		dTreeFillData.Fill_Single<Double_t>("sector3_up_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(18)));//18
		dTreeFillData.Fill_Single<Double_t>("sector4_up_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(19)));//19

		dTreeFillData.Fill_Single<Double_t>("sector1_down_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(20)));//20
		dTreeFillData.Fill_Single<Double_t>("sector2_down_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(21)));//21
		dTreeFillData.Fill_Single<Double_t>("sector3_down_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(22)));//22
		dTreeFillData.Fill_Single<Double_t>("sector4_down_peak_lowup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(23)));//23
		
		dTreeFillData.Fill_Single<Double_t>("sector1_up_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(24)));//24
		dTreeFillData.Fill_Single<Double_t>("sector2_up_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(25)));//25
		dTreeFillData.Fill_Single<Double_t>("sector3_up_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(26)));//26
		dTreeFillData.Fill_Single<Double_t>("sector4_up_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(27)));//27

		dTreeFillData.Fill_Single<Double_t>("sector1_down_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(28)));//28
		dTreeFillData.Fill_Single<Double_t>("sector2_down_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(29)));//29
		dTreeFillData.Fill_Single<Double_t>("sector3_down_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(30)));//30
		dTreeFillData.Fill_Single<Double_t>("sector4_down_peak_lowdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(31)));//31
		
		dTreeFillData.Fill_Single<Double_t>("sector1_up_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(32)));//32
		dTreeFillData.Fill_Single<Double_t>("sector2_up_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(33)));//33
		dTreeFillData.Fill_Single<Double_t>("sector3_up_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(34)));//34
		dTreeFillData.Fill_Single<Double_t>("sector4_up_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(35)));//35

		dTreeFillData.Fill_Single<Double_t>("sector1_down_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(36)));//36
		dTreeFillData.Fill_Single<Double_t>("sector2_down_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(37)));//37
		dTreeFillData.Fill_Single<Double_t>("sector3_down_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(38)));//38
		dTreeFillData.Fill_Single<Double_t>("sector4_down_peak_highup", bcal_vevent->GetBinContent(bcal_vevent->FindBin(39)));//39
		
		dTreeFillData.Fill_Single<Double_t>("sector1_up_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(40)));//40
		dTreeFillData.Fill_Single<Double_t>("sector2_up_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(41)));//41
		dTreeFillData.Fill_Single<Double_t>("sector3_up_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(42)));//42
		dTreeFillData.Fill_Single<Double_t>("sector4_up_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(43)));//43

		dTreeFillData.Fill_Single<Double_t>("sector1_down_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(44)));//44
		dTreeFillData.Fill_Single<Double_t>("sector2_down_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(45)));//45
		dTreeFillData.Fill_Single<Double_t>("sector3_down_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(46)));//46
		dTreeFillData.Fill_Single<Double_t>("sector4_down_peak_highdown", bcal_vevent->GetBinContent(bcal_vevent->FindBin(47)));//47

		dTreeInterface->Fill(dTreeFillData);
		
		for (int quad = 1; quad < 5; quad++)
		{
		  int quad_occ_up, quad_occ_down = 0;
		  
		  quad_occ_up = dHist_quad_count_up->GetBinContent(dHist_quad_count_up->FindBin(quad))+dHist_quad_count_up->GetBinContent(dHist_quad_count_up->FindBin(quad+4));
//		 cout << "quad " << quad <<" occ_up =" << quad_occ_up << ", down =" << quad_occ_down << endl;
		 if (quad_occ_up > 191) dHist_quad_occ_up->Fill(quad);
		  
		 quad_occ_down = dHist_quad_count_down->GetBinContent(dHist_quad_count_down->FindBin(quad))+dHist_quad_count_down->GetBinContent(dHist_quad_count_up->FindBin(quad+4));
		  if (quad_occ_down > 191) dHist_quad_occ_down->Fill(quad);
		 //cout << "quad " << quad <<" occ_up =" << quad_occ_up << ", down =" << quad_occ_down << endl;
		}


		bcal_vevent->Reset();
		dHist_quad_count_up->Reset();
		dHist_quad_count_down->Reset();

	/*************************************************************** TRIGGER **************************************************************/

	//Fill histogram if fp LED trigger exists
	if(LED_US || LED_DS)
	{
	dHist_L1bits_fp->Fill(pseudo_triggerbit);		
    	}

	//Fill histogram if fp LED trigger exists or number of BCAL hits is > 1200
	dHist_L1bits_fp_twelvehundhits->Fill(pseudo_triggerbit);	
		 
	}//if LEDUP || LEDDOWN || 1200 hits   
	// Unlock ROOT
	lockService->RootUnLock();
	

    return;
}


//----------------------------------------------------------------------------------


void JEventProcessor_BCAL_LED::EndRun() {
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.


/*	printf("\nTrigger statistics");
	printf("------------------------\n");
	printf("%20s: %10i\n","no triggers",NOtrig);
	printf("%20s: %10i\n","Front Panel",FPtrig);
	printf("%20s: %10i\n","GTP",GTPtrig);
	printf("%20s: %10i\n","FP && GTP",FPGTPtrig);
	printf("%20s: %10i\n","US LED",trigUS);
	printf("%20s: %10i\n","DS LED",trigDS);
	printf("%20s: %10i\n","BCAL",trigCosmic);
	ledcounter = low_down_1_counter + low_down_2_counter + low_down_3_counter + low_down_4_counter + low_up_1_counter + low_up_2_counter + low_up_3_counter + low_up_4_counter + high_down_1_counter + high_down_2_counter + high_down_3_counter + high_down_4_counter + high_up_1_counter + high_up_2_counter + high_up_3_counter + high_up_4_counter + unidentified;
	printf("%20s: %10i\n","low_down_1_counter",low_down_1_counter);
	printf("%20s: %10i\n","low_down_2_counter",low_down_2_counter);
	printf("%20s: %10i\n","low_down_3_counter",low_down_3_counter);
	printf("%20s: %10i\n","low_down_4_counter",low_down_4_counter);

	printf("%20s: %10i\n","low_up_1_counter",low_up_1_counter);
	printf("%20s: %10i\n","low_up_2_counter",low_up_2_counter);
	printf("%20s: %10i\n","low_up_3_counter",low_up_3_counter);
	printf("%20s: %10i\n","low_up_4_counter",low_up_4_counter);

	printf("%20s: %10i\n","high_down_1_counter",high_down_1_counter);
	printf("%20s: %10i\n","high_down_2_counter",high_down_2_counter);
	printf("%20s: %10i\n","high_down_3_counter",high_down_3_counter);
	printf("%20s: %10i\n","high_down_4_counter",high_down_4_counter);

	printf("%20s: %10i\n","high_up_1_counter",high_up_1_counter);
	printf("%20s: %10i\n","high_up_2_counter",high_up_2_counter);
	printf("%20s: %10i\n","high_up_3_counter",high_up_3_counter);
	printf("%20s: %10i\n","high_up_4_counter",high_up_4_counter);
	
	printf("%20s: %10i\n","Unidentified",unidentified);*/
	
	return;
}


//----------------------------------------------------------------------------------


void JEventProcessor_BCAL_LED::Finish() {
	// Called before program exit after event processing is finished.

//	int quad_count_up = dHist_quad_occ_up->GetMaximum();
//	if (up_trigger_count > 0) dHist_quad_occ_up->Scale(1/up_trigger_count);
//	int quad_count_down = dHist_quad_occ_down->GetMaximum();
//	if (down_trigger_count > 0) dHist_quad_occ_down->Scale(1/down_trigger_count);
        //cout << "up/ trigger count =" <<  quad_count_up << "/"<< up_trigger_count << ", down = " << quad_count_down<< "/"<< down_trigger_count << endl;

if (simultaneous == 0){
// 	//Write mean pulse peak to output file
	ofstream foutlowdown ; foutlowdown.open("LED_lowbias_downstream.txt");
	ofstream foutlowup; foutlowup.open("LED_lowbias_upstream.txt");
	ofstream fouthighdown; fouthighdown.open("LED_highbias_downstream.txt");
	ofstream fouthighup; fouthighup.open("LED_highbias_upstream.txt");

	for(int k=0 ;k < 768;k += 4)
	{
	double lowdownmean1down = low_down_1->GetBinContent(low_down_1->FindBin(k));
	double lowdownmean1up = low_down_1->GetBinContent(low_down_1->FindBin(768+k));
	double lowdownmean2down = low_down_2->GetBinContent(low_down_2->FindBin(k+1));
	double lowdownmean2up = low_down_2->GetBinContent(low_down_2->FindBin(769+k));
	double lowdownmean3down = low_down_3->GetBinContent(low_down_3->FindBin(k+2));
	double lowdownmean3up = low_down_3->GetBinContent(low_down_3->FindBin(770+k));
	double lowdownmean4down = low_down_4->GetBinContent(low_down_4->FindBin(k+3));
	double lowdownmean4up = low_down_4->GetBinContent(low_down_4->FindBin(771+k));
	
	double lowupmean1down = low_up_1->GetBinContent(low_up_1->FindBin(k));
	double lowupmean1up = low_up_1->GetBinContent(low_up_1->FindBin(768+k));
	double lowupmean2down = low_up_2->GetBinContent(low_up_2->FindBin(k+1));
	double lowupmean2up = low_up_2->GetBinContent(low_up_2->FindBin(769+k));
	double lowupmean3down = low_up_3->GetBinContent(low_up_3->FindBin(k+2));
	double lowupmean3up = low_up_3->GetBinContent(low_up_3->FindBin(770+k));
	double lowupmean4down = low_up_4->GetBinContent(low_up_4->FindBin(k+3));
	double lowupmean4up = low_up_4->GetBinContent(low_up_4->FindBin(771+k));

	double highdownmean1down = high_down_1->GetBinContent(high_down_1->FindBin(k));
	double highdownmean1up = high_down_1->GetBinContent(high_down_1->FindBin(768+k));
	double highdownmean2down = high_down_2->GetBinContent(high_down_2->FindBin(k+1));
	double highdownmean2up = high_down_2->GetBinContent(high_down_2->FindBin(769+k));
	double highdownmean3down = high_down_3->GetBinContent(high_down_3->FindBin(k+2));
	double highdownmean3up = high_down_3->GetBinContent(high_down_3->FindBin(770+k));
	double highdownmean4down = high_down_4->GetBinContent(high_down_4->FindBin(k+3));
	double highdownmean4up = high_down_4->GetBinContent(high_down_4->FindBin(771+k));
	
	double highupmean1down = high_up_1->GetBinContent(high_up_1->FindBin(k));
	double highupmean1up = high_up_1->GetBinContent(high_up_1->FindBin(768+k));
	double highupmean2down = high_up_2->GetBinContent(high_up_2->FindBin(k+1));
	double highupmean2up = high_up_2->GetBinContent(high_up_2->FindBin(769+k));
	double highupmean3down = high_up_3->GetBinContent(high_up_3->FindBin(k+2));
	double highupmean3up = high_up_3->GetBinContent(high_up_3->FindBin(770+k));
	double highupmean4down = high_up_4->GetBinContent(high_up_4->FindBin(k+3));
	double highupmean4up = high_up_4->GetBinContent(high_up_4->FindBin(771+k));
	
	//TString sep = "        ";
	foutlowdown << lowdownmean1down << endl << lowdownmean1up << endl << lowdownmean2down << endl << lowdownmean2up << endl << lowdownmean3down << endl << lowdownmean3up << endl << lowdownmean4down << endl << lowdownmean4up << endl;
	foutlowup << lowupmean1down << endl << lowupmean1up << endl << lowupmean2down << endl << lowupmean2up << endl << lowupmean3down << endl << lowupmean3up << endl << lowupmean4down << endl << lowupmean4up << endl;
	fouthighdown << highdownmean1down << endl << highdownmean1up << endl << highdownmean2down << endl << highdownmean2up << endl << highdownmean3down << endl << highdownmean3up << endl << highdownmean4down << endl << highdownmean4up << endl;
	fouthighup << highupmean1down << endl << highupmean1up << endl << highupmean2down << endl << highupmean2up << endl << highupmean3down << endl << highupmean3up << endl << highupmean4down << endl << highupmean4up << endl;

	}

	foutlowdown.close();
	foutlowup.close();
	fouthighdown.close();
	fouthighup.close();
}

if (simultaneous == 1){
// 	//Write mean pulse peak to output file
	ofstream foutdown ; foutdown.open("LED_downstream.txt");
	ofstream foutup; foutup.open("LED_upstream.txt");

	for(int k=0 ;k < 768;k += 4)
	{
	double downmeandown = down->GetBinContent(down->FindBin(k));
	double downmeanup = down->GetBinContent(down->FindBin(768+k));

   	double upmeandown = up->GetBinContent(up->FindBin(k));
	double upmeanup = up->GetBinContent(up->FindBin(768+k));

	//TString sep = "        ";
	foutdown << downmeandown << endl << downmeanup << endl;
	foutup << upmeandown << endl << upmeanup << endl;
	}

	foutdown.close();
	foutup.close();
}

	delete dTreeInterface; //saves trees to file, closes file	
	delete bcal_vevent;

return;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
