#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include <JANA/JApplication.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/DL1Info.h>

#include <FCAL/DFCALDigiHit.h>
#include <BCAL/DBCALDigiHit.h>

#include <HDDM/DEventSourceHDDM.h>

#include "DL1MCTrigger_factory_DATA.h"
#include "DANA/DEvent.h"


#if HAVE_RCDB
#include "RCDB/Connection.h"
#include "RCDB/ConfigParser.h"
#endif

thread_local DTreeFillData DL1MCTrigger_factory_DATA::dTreeFillData;
bool DL1MCTrigger_factory_DATA::RCDB_LOADED = false;
bool DL1MCTrigger_factory_DATA::PARAMS_LOADED = false;
std::mutex DL1MCTrigger_factory_DATA::params_mutex;
std::mutex DL1MCTrigger_factory_DATA::rcdb_mutex;

vector<DL1MCTrigger_factory_DATA::trigger_conf> DL1MCTrigger_factory_DATA::triggers_enabled;

vector<DL1MCTrigger_factory_DATA::fcal_mod> DL1MCTrigger_factory_DATA::fcal_trig_mask;
vector<DL1MCTrigger_factory_DATA::bcal_mod> DL1MCTrigger_factory_DATA::bcal_trig_mask;
vector<DL1MCTrigger_factory_DATA::ecal_mod> DL1MCTrigger_factory_DATA::ecal_trig_mask;

int    DL1MCTrigger_factory_DATA::FCAL_CELL_THR = 65;
int    DL1MCTrigger_factory_DATA::FCAL_NSA = 10;
int    DL1MCTrigger_factory_DATA::FCAL_NSB = 3;
int    DL1MCTrigger_factory_DATA::FCAL_WINDOW = 10;

int    DL1MCTrigger_factory_DATA::BCAL_CELL_THR = 20;
int    DL1MCTrigger_factory_DATA::BCAL_NSA = 19;
int    DL1MCTrigger_factory_DATA::BCAL_NSB = 3;
int    DL1MCTrigger_factory_DATA::BCAL_WINDOW = 20;

int    DL1MCTrigger_factory_DATA::ECAL_CELL_THR = 35;
int    DL1MCTrigger_factory_DATA::ECAL_NSA = 10;
int    DL1MCTrigger_factory_DATA::ECAL_NSB = 3;
int    DL1MCTrigger_factory_DATA::ECAL_WINDOW = 15;

float  DL1MCTrigger_factory_DATA::FCAL_GAIN = 1.;
float  DL1MCTrigger_factory_DATA::ECAL_GAIN = 0.;

//------------------
// Init
//------------------
void DL1MCTrigger_factory_DATA::Init(void)
{

  debug = 0;
  auto app = GetApplication();

  if(debug){
    hfcal_gains   = new TH1F("fcal_gains", "fcal_gains",  80,  -1., 3.);
    hfcal_gains2  = new TH2F("fcal_gains2","fcal_gains2", 71, -142., 142., 71, -142., 142.);
    hfcal_ped     = new TH1F("fcal_ped", "fcal_ped", 800, 0., 200.);

    hecal_gains   = new TH1F("ecal_gains", "ecal_gains",  80,  -1., 3.);
    hecal_gains2  = new TH2F("ecal_gains2","ecal_gains2", 71, -71., 71., 71, -71., 71.);
    hecal_ped     = new TH1F("ecal_ped", "ecal_ped", 800, 0., 200.);
  }

  BYPASS = 0; // default is to use trigger emulation

  // Default parameters for the main production trigger are taken from the 
  // spring run of 2017: 25 F + B > 45000

  FCAL_ADC_PER_MEV =  3.73;
  //FCAL_CELL_THR    =  65;
  FCAL_EN_SC       =  25;
  //FCAL_NSA         =  10;
  //FCAL_NSB         =  3;
  //FCAL_WINDOW      =  10;

  //BCAL_ADC_PER_MEV =  34.48276; // Not corrected energy 
  BCAL_ADC_PER_MEV =  22.7273;  
  //BCAL_CELL_THR    =  20;
  BCAL_EN_SC       =  1;
  //BCAL_NSA         =  19;
  //BCAL_NSB         =  3;
  //BCAL_WINDOW      =  20;

  ECAL_ADC_PER_MEV =  1.8359;
  
  FCAL_BCAL_EN     =  45000; 

  ST_ADC_PER_MEV   =  1.;
  ST_CELL_THR      =  40;
  ST_NSA           =  10;
  ST_NSB           =  3;
  ST_WINDOW        =  10;
  ST_NHIT          =  1;

  BCAL_OFFSET      =  2;

  SIMU_BASELINE = 0;
  SIMU_GAIN = 0;
  
  OUTPUT_TREE = 0;
  USE_RAW_SAMPLES = 0;
  USE_DIGI = 1;

  simu_baseline_fcal  =  1;
  simu_baseline_bcal  =  1;
  simu_baseline_ecal  =  1;

  simu_gain_fcal  =  0;
  simu_gain_bcal  =  0;
  simu_gain_ecal  =  0;
  
  app->SetDefaultParameter("TRIG:BYPASS", BYPASS,
                              "Bypass trigger by hard coding physics bit");
  app->SetDefaultParameter("TRIG:FCAL_ADC_PER_MEV", FCAL_ADC_PER_MEV,
			      "FCAL energy calibration for the Trigger");
  app->SetDefaultParameter("TRIG:FCAL_EN_SC", FCAL_EN_SC,
			      "FCAL energy threshold");

  app->SetDefaultParameter("TRIG:BCAL_ADC_PER_MEV", BCAL_ADC_PER_MEV,
			      "BCAL energy calibration for the Trigger");
  app->SetDefaultParameter("TRIG:BCAL_EN_SC", BCAL_EN_SC,
			      "BCAL energy threshold");

  app->SetDefaultParameter("TRIG:ST_ADC_PER_MEV", ST_ADC_PER_MEV,
			      "ST energy calibration for the Trigger");
  app->SetDefaultParameter("TRIG:ST_CELL_THR", ST_CELL_THR,
			      "ST energy threshold per cell");
  app->SetDefaultParameter("TRIG:ST_NSA", ST_NSA,
			      "ST NSA");
  app->SetDefaultParameter("TRIG:ST_NSB", ST_NSB,
			      "ST NSB");  
  app->SetDefaultParameter("TRIG:ST_WINDOW", ST_WINDOW,
			      "ST window for merging hits (GTP)");
  app->SetDefaultParameter("TRIG:ST_NHIT", ST_NHIT,
			      "Number of hits in ST");
  
  app->SetDefaultParameter("TRIG:FCAL_BCAL_EN", FCAL_BCAL_EN,
			      "Energy threshold for the FCAL & BCAL trigger");

  app->SetDefaultParameter("TRIG:BCAL_OFFSET", BCAL_OFFSET,
			      "Timing offset between BCAL and FCAL energies at GTP (sampels)");
  app->SetDefaultParameter("TRIG:OUTPUT_TREE", OUTPUT_TREE,
			      "Write out tree with trigger information, for debugging.");
  app->SetDefaultParameter("TRIG:USE_RAW_SAMPLES", USE_RAW_SAMPLES,
			      "Use the measured raw fADC samples instead of the simulated ones (only works for raw mode data).");
  app->SetDefaultParameter("TRIG:USE_DIGI", USE_DIGI,
			      "Use the DigiHits instead of the simulated ones.");
  
  std::lock_guard<std::mutex> lock(params_mutex);
  if(!PARAMS_LOADED) {
	  PARAMS_LOADED = true;

	  app->SetDefaultParameter("TRIG:FCAL_CELL_THR", FCAL_CELL_THR,
					  "FCAL energy threshold per cell");
	  app->SetDefaultParameter("TRIG:FCAL_NSA", FCAL_NSA,
					  "FCAL NSA");
	  app->SetDefaultParameter("TRIG:FCAL_NSB", FCAL_NSB,
					  "FCAL NSB");
	  app->SetDefaultParameter("TRIG:FCAL_WINDOW", FCAL_WINDOW,
					  "FCAL GTP integration window");
	
	  app->SetDefaultParameter("TRIG:BCAL_CELL_THR", BCAL_CELL_THR,
					  "BCAL energy threshold per cell");
	  app->SetDefaultParameter("TRIG:BCAL_NSA", BCAL_NSA,
					  "BCAL NSA");
	  app->SetDefaultParameter("TRIG:BCAL_NSB", BCAL_NSB,
					  "BCAL NSB");
	  app->SetDefaultParameter("TRIG:BCAL_WINDOW", BCAL_WINDOW,
					  "BCAL GTP integration window");

	  app->SetDefaultParameter("TRIG:ECAL_CELL_THR", ECAL_CELL_THR,
					  "ECAL energy threshold per cell");
	  app->SetDefaultParameter("TRIG:ECAL_NSA", ECAL_NSA,
					  "ECAL NSA");
	  app->SetDefaultParameter("TRIG:ECAL_NSB", ECAL_NSB,
					  "ECAL NSB");
	  app->SetDefaultParameter("TRIG:ECAL_WINDOW", ECAL_WINDOW,
					  "ECAL GTP integration window");
  }
  
  if(OUTPUT_TREE) {
  	dTreeInterface = DTreeInterface::Create_DTreeInterface("Trigger_Tree", "tree_trigger.root");
  
    //TTREE BRANCHES
    DTreeBranchRegister locTreeBranchRegister;
    
    locTreeBranchRegister.Register_Single<Int_t>("RunNumber");
    locTreeBranchRegister.Register_Single<ULong64_t>("EventNumber");
    locTreeBranchRegister.Register_Single<Double_t>("FCAL_En");
    locTreeBranchRegister.Register_Single<Double_t>("FCAL_ADC");
    locTreeBranchRegister.Register_Single<Double_t>("FCAL_ADC_En");
    locTreeBranchRegister.Register_Single<Double_t>("FCAL_GTP");
    locTreeBranchRegister.Register_Single<Double_t>("FCAL_GTP_En");
    locTreeBranchRegister.Register_Single<Double_t>("BCAL_En");
    locTreeBranchRegister.Register_Single<Double_t>("BCAL_ADC");
    locTreeBranchRegister.Register_Single<Double_t>("BCAL_ADC_En");
    locTreeBranchRegister.Register_Single<Double_t>("BCAL_GTP");
    locTreeBranchRegister.Register_Single<Double_t>("BCAL_GTP_En");
    locTreeBranchRegister.Register_Single<Double_t>("ECAL_En");
    locTreeBranchRegister.Register_Single<Double_t>("ECAL_ADC");
    locTreeBranchRegister.Register_Single<Double_t>("ECAL_ADC_En");
    locTreeBranchRegister.Register_Single<Double_t>("ECAL_GTP");
    locTreeBranchRegister.Register_Single<Double_t>("ECAL_GTP_En");

    //REGISTER BRANCHES
    dTreeInterface->Create_Branches(locTreeBranchRegister);

  }


  // Allows to switch off gain and baseline fluctuations
  app->SetDefaultParameter("TRIG:SIMU_BASELINE", SIMU_BASELINE,
			      "Enable simulation of pedestal variations");

//   gPARMS->SetDefaultParameter("TRIG:SIMU_GAIN", SIMU_GAIN,
// 			      "Enable simulation of gain variations");


  BCAL_ADC_PER_MEV_CORRECT  =  22.7273;

  pedestal_sigma = 1.2;

  //time_shift = 100;
  time_shift = 0;  // I don't think we need this for real data?

  time_min  =  0;
  time_max  =  (sample - 1)*max_adc_bins;
  
  vector< vector<double > > fcal_gains_temp(DFCALGeometry::kBlocksTall, 
					    vector<double>(DFCALGeometry::kBlocksWide));
  vector< vector<double > > fcal_pedestals_temp(DFCALGeometry::kBlocksTall, 
						vector<double>(DFCALGeometry::kBlocksWide));
  vector< vector<double > > ecal_gains_temp(DECALGeometry::kECALBlocksTall, 
					    vector<double>(DECALGeometry::kECALBlocksWide));
  vector< vector<double > > ecal_pedestals_temp(DECALGeometry::kECALBlocksTall, 
						vector<double>(DECALGeometry::kECALBlocksWide));
  
  fcal_gains      =  fcal_gains_temp;  
  fcal_pedestals  =  fcal_pedestals_temp;
  ecal_gains      =  ecal_gains_temp;
  ecal_pedestals  =  ecal_pedestals_temp;

  if(!SIMU_BASELINE){
    simu_baseline_fcal = 0;
    simu_baseline_bcal = 0;
    simu_baseline_ecal = 0;
  }

  return; // NOERROR;
}


//------------------
// BeginRun
//------------------
void DL1MCTrigger_factory_DATA::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto runnumber = event->GetRunNumber();
  int use_rcdb = 1;

  int status   = 0;

  fcal_trig_mask.clear();
  bcal_trig_mask.clear();
  ecal_trig_mask.clear();
  
  triggers_enabled.clear();

  // Only print messages for one thread whenever run number change

  static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
  static set<int> runs_announced;
  pthread_mutex_lock(&print_mutex);
  bool print_messages = false;

  if(runs_announced.find(runnumber) == runs_announced.end()){
    print_messages = true;
    runs_announced.insert(runnumber);
  }
  
  pthread_mutex_unlock(&print_mutex);
  
  // Don't use RCDB for mc_generic: ideal MC simulation

  string JANA_CALIB_CONTEXT = "";

  if(getenv("JANA_CALIB_CONTEXT") != NULL ){ 
    JANA_CALIB_CONTEXT = getenv("JANA_CALIB_CONTEXT");
    if(JANA_CALIB_CONTEXT.find("mc_generic") != string::npos){
      use_rcdb = 0;
      // Don't simulate baseline fluctuations for mc_generic
      simu_baseline_fcal = 0;
      simu_baseline_bcal = 0;
      simu_baseline_ecal = 0;
      // Don't simulate gain fluctuations for mc_generic
      simu_gain_fcal = 0;
      simu_gain_bcal = 0;
      simu_gain_ecal = 0;
    }
  }

  //  runnumber = 30942;

  if(use_rcdb == 1){
    status = Read_RCDB(event, print_messages);
    if(print_messages) PrintTriggers();
  }

  
  if( (use_rcdb == 0) || (status > 0) || (triggers_enabled.size() == 0)){

    // Simulate FCAL & BCAL main production trigger only
  
    trigger_conf trig_tmp;
    trig_tmp.bit = 0;
    trig_tmp.gtp.fcal      =  FCAL_EN_SC;
    trig_tmp.gtp.bcal      =  BCAL_EN_SC;
    trig_tmp.gtp.en_thr    =  FCAL_BCAL_EN;
    trig_tmp.gtp.fcal_min  =  200;
    triggers_enabled.push_back(trig_tmp);
	
	if(print_messages) 
	    cout << " Do not use RCDB for the trigger simulation. Default (spring 2017) trigger settings are used " << endl;
  }


  // extract the FCAL Geometry
  vector<const DFCALGeometry*> fcalGeomVect;
  event->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1)
    return; //OBJECT_NOT_AVAILABLE;
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);

  // extract the ECAL Geometry
  vector<const DECALGeometry*> ecalGeomVect;
  event->Get( ecalGeomVect );
  if (ecalGeomVect.size() < 1)
    return; //OBJECT_NOT_AVAILABLE;
  const DECALGeometry& ecalGeom = *(ecalGeomVect[0]);
  
  if(print_messages) jout << "In DL1MCTrigger_factory_DATA, loading constants..." << endl;
  
  vector< double > fcal_gains_ch;
  vector< double > fcal_pedestals_ch;
  
  if (DEvent::GetCalib(event, "/FCAL/gains", fcal_gains_ch)){
    jout << "DL1MCTrigger_factory_DATA: Error loading /FCAL/gains !" << endl;
    // Load default values of gains if CCDB table is not found
    for(int ii = 0; ii < DFCALGeometry::kBlocksTall; ii++){
      for(int jj = 0; jj < DFCALGeometry::kBlocksWide; jj++){
	fcal_gains[ii][jj] = 1.;	
      }
    }
  } else {
    LoadFCALConst(fcal_gains, fcal_gains_ch, fcalGeom);

    if(debug){
      for(int ch = 0; ch < (int)fcal_gains_ch.size(); ch++){
	if (fcalGeom.isBlockActive(ch)){
	  int row = fcalGeom.row(ch);
	  int col = fcalGeom.column(ch);
	  hfcal_gains->Fill(fcal_gains[row][col]);
	  DVector2 aaa = fcalGeom.positionOnFace(row,col);
	  hfcal_gains2->Fill(float(aaa.X()), float(aaa.Y()), fcal_gains[row][col]);
	  //cout << aaa.X() << "  " << aaa.Y() << endl;	  
	}	
      }
    }

  }

  if (DEvent::GetCalib(event, "/FCAL/pedestals", fcal_pedestals_ch)){
    jout << "DL1MCTrigger_factory_DATA: Error loading /FCAL/pedestals !" << endl;
    // Load default values of pedestals if CCDB table is not found
    for(int ii = 0; ii < DFCALGeometry::kBlocksTall; ii++){
      for(int jj = 0; jj < DFCALGeometry::kBlocksWide; jj++){
	fcal_pedestals[ii][jj] = 100.;	
      }
    }
  } else {
    LoadFCALConst(fcal_pedestals, fcal_pedestals_ch, fcalGeom);

    if(debug){
      for(int ch = 0; ch < (int)fcal_gains_ch.size(); ch++){
	if(fcalGeom.isBlockActive(ch)){
	  int row = fcalGeom.row(ch);
	  int col = fcalGeom.column(ch);
	  hfcal_ped->Fill(fcal_pedestals[row][col]);
	}
      }	
    }
    
  }

  // ECAL tables
  vector< double > ecal_gains_ch;
  vector< double > ecal_pedestals_ch;
  
  if (DEvent::GetCalib(event, "/ECAL/gains", ecal_gains_ch)){
    jout << "DL1MCTrigger_factory_DATA: Error loading /ECAL/gains !" << endl;
    // Load default values of gains if CCDB table is not found
    for(int ii = 0; ii < DECALGeometry::kECALBlocksTall; ii++){
      for(int jj = 0; jj < DECALGeometry::kECALBlocksWide; jj++){
	ecal_gains[ii][jj] = 1.;	
      }
    }
  } else {
    LoadECALConst(ecal_gains, ecal_gains_ch, ecalGeom);

    if(debug){
      for(int ch = 0; ch < (int)ecal_gains_ch.size(); ch++){
	      //if (ecalGeom.isBlockActive(ch)){
	      int row = ecalGeom.row(ch);
	      int col = ecalGeom.column(ch);
	      hecal_gains->Fill(ecal_gains[row][col]);
	      DVector2 aaa = ecalGeom.positionOnFace(row,col);
	      hecal_gains2->Fill(float(aaa.X()), float(aaa.Y()), ecal_gains[row][col]);
	      //cout << aaa.X() << "  " << aaa.Y() << endl;	  
	      //}	
      }
    }

  }

  if (DEvent::GetCalib(event, "/ECAL/pedestals", ecal_pedestals_ch)){
    jout << "DL1MCTrigger_factory_DATA: Error loading /ECAL/pedestals !" << endl;
    // Load default values of pedestals if CCDB table is not found
    for(int ii = 0; ii < DECALGeometry::kECALBlocksTall; ii++){
      for(int jj = 0; jj < DECALGeometry::kECALBlocksWide; jj++){
	ecal_pedestals[ii][jj] = 100.;	
      }
    }
  } else {
    LoadECALConst(ecal_pedestals, ecal_pedestals_ch, ecalGeom);

    if(debug){
      for(int ch = 0; ch < (int)ecal_gains_ch.size(); ch++){
	      //if(ecalGeom.isBlockActive(ch)){
	      int row = ecalGeom.row(ch);
	      int col = ecalGeom.column(ch);
	      hecal_ped->Fill(ecal_pedestals[row][col]);
	      //}
      }	
    }
    
  }
  
  if(!SIMU_BASELINE){
    simu_baseline_fcal = 0;
    simu_baseline_bcal = 0;
    simu_baseline_ecal = 0;
  }

  if(!SIMU_GAIN){
    simu_gain_fcal = 0;
    simu_gain_bcal = 0;
    simu_gain_ecal = 0;
  }
  
  if(debug){
    for(int ii = 0; ii < 100; ii++){
      cout << " Channel = " << ii <<  " Value = " << 
	fcal_gains_ch[ii] << endl;
    }
  }


//cerr << "end DL1MCTrigger_factory_DATA::brun() ... " << endl;

  return; //NOERROR;
}

//------------------
// Process
//------------------
void DL1MCTrigger_factory_DATA::Process(const std::shared_ptr<const JEvent>& event){
	
	if(BYPASS) {
                DL1MCTrigger *trigger = new DL1MCTrigger;
                trigger->trig_mask = 1;
                Insert(trigger);
		return; //NOERROR;
        }

        int l1_found = 1;  

	int status = 0;

    int32_t runnumber = event->GetRunNumber();
    auto eventnumber = event->GetEventNumber();

//cerr << "in DL1MCTrigger_factory_DATA::evnt() ... " << endl;

	fcal_signal_hits.clear();
	bcal_signal_hits.clear();
	ecal_signal_hits.clear();

	fcal_merged_hits.clear();
	bcal_merged_hits.clear();
	ecal_merged_hits.clear();
	
	memset(fcal_ssp,0,sizeof(fcal_ssp));
	memset(fcal_gtp,0,sizeof(fcal_gtp));

	memset(bcal_ssp,0,sizeof(bcal_ssp));
	memset(bcal_gtp,0,sizeof(bcal_gtp));

	memset(ecal_ssp,0,sizeof(ecal_ssp));
	memset(ecal_gtp,0,sizeof(ecal_gtp));

	vector<const DFCALHit*>  fcal_hits;
	vector<const DBCALHit*>  bcal_hits;
	vector<const DECALHit*>  ecal_hits;

	event->Get(fcal_hits);
	event->Get(bcal_hits);
	event->Get(ecal_hits);

	// Initialize random number generator
	// Read seeds from hddm file
	// Generate seeds according to the event number if they are not stored in hddm
	// The proceedure is consistent with the mcsmear

	UInt_t seed1 = 0;
	UInt_t seed2 = 0;
	UInt_t seed3 = 0;
	
	DRandom2 gDRandom(0); // declared extern in DRandom2.h
	GetSeeds(event, seed1, seed2, seed3);
	
	gDRandom.SetSeeds(seed1, seed2, seed3);
	
	//	cout << endl;
	//	cout << " Event = " << eventnumber << endl;
	//	cout << " Seed 1: " << seed1 << endl;
	//	cout << " Seed 2: " << seed2 << endl;
	//	cout << " Seed 3: " << seed3 << endl;
	//	cout << endl;
	       

	DL1MCTrigger *trigger = new DL1MCTrigger;

	//  FCAL energy sum	
	double fcal_hit_en = 0;
	
	//cout << "FCAL energy sum" << endl;
	//cout << "  num hits = " << fcal_hits.size() << endl;

	for (unsigned int ii = 0; ii < fcal_hits.size(); ii++){
	  const DFCALDigiHit *fcaldigihit = nullptr;
	  fcal_hits[ii]->GetSingle(fcaldigihit);
	  
	  int row  = fcal_hits[ii]->row;
	  int col  = fcal_hits[ii]->column;

	  //cout << "FCAL hit row/column = " << row << "/" << col << endl;
	  
	  // Shift time to simulate pile up hits
	  // don't need this for real data?
// 	  double time = fcal_hits[ii]->t + time_shift;
// 	  if((time < time_min) || (time > time_max)){
// 	    continue;
// 	  }

	  // Check channels masked for trigger
	  int ch_masked = 0;
	  for(unsigned int jj = 0; jj < fcal_trig_mask.size(); jj++){
	    if( (row == fcal_trig_mask[jj].row) && (col == fcal_trig_mask[jj].col)){	      
	      ch_masked = 1;
	      break;
	    } 
	  }
	  
	  if(ch_masked == 0){
	    fcal_hit_en += fcal_hits[ii]->E;
	    
	    fcal_signal fcal_tmp;
	    
	    fcal_tmp.row     = row;
	    fcal_tmp.column  = col;

	    fcal_tmp.energy  = fcal_hits[ii]->E;
	    fcal_tmp.time    = fcal_hits[ii]->t;
	    fcal_tmp.pulse_peak = fcaldigihit->pulse_peak;
	    fcal_tmp.pulse_time = (fcaldigihit->pulse_time >> 6) & 0x1FF; // consider only course time;
	    fcal_tmp.pulse_integral = fcaldigihit->pulse_integral - fcaldigihit->nsamples_integral*TRIG_BASELINE;
	    memset(fcal_tmp.adc_amp,0,sizeof(fcal_tmp.adc_amp));
	    memset(fcal_tmp.adc_en, 0,sizeof(fcal_tmp.adc_en));

	    //double fcal_adc_en  = fcaldigihit->pulse_integral;  // need to pedestal subtract!
	    double fcal_adc_en  = fcal_tmp.energy*FCAL_ADC_PER_MEV*1000;
	    
	    // Account for gain fluctuations 
// 	    if(simu_gain_fcal){
// 	      
// 	      double gain  =  fcal_gains[row][col];	  
// 	      
// 	      fcal_adc_en *= gain;
// 	    }
	    
	    if(USE_RAW_SAMPLES) {
	    	// handle different pulse types
	  		const Df250WindowRawData *rawdata = nullptr;
	  		const Df250PulseData *pulsedata = nullptr;
	  		fcaldigihit->GetSingle(pulsedata);
	  		pulsedata->GetSingle(rawdata);

		    if(rawdata == nullptr) {
		    	jerr << "DL1MCTrigger_factory_DATA: warning! couldn't load raw data ..." << endl;
		    }
// 	  		
// 	  		jerr << "0x" << hex << pulsedata << dec << endl;
// 	  		jerr << "0x" << hex << rawdata << dec << endl;
   	
	    	int numsamples = (sample<rawdata->samples.size()) ? sample : rawdata->samples.size();
	    	for(int i=0; i<numsamples; i++) 
	    		fcal_tmp.adc_en[i] = rawdata->samples[i];
	    	
	    } else {
		    status = SignalPulse(fcal_adc_en, fcal_tmp.time, fcal_tmp.adc_en, 1);
		    status = 0;
		}

	    fcal_signal_hits.push_back(fcal_tmp);
	  }
	  
	}       		


	// Merge FCAL hits
	for(unsigned int ii = 0; ii < fcal_signal_hits.size(); ii++){	  
	  
	  if(fcal_signal_hits[ii].merged == 1) continue;
	  
	  fcal_signal fcal_tmp;
	  fcal_tmp.row     = fcal_signal_hits[ii].row;
	  fcal_tmp.column  = fcal_signal_hits[ii].column;
	  fcal_tmp.energy  = 0.;
	  fcal_tmp.time    = 0.;
	  for(int kk = 0; kk < sample; kk++)	      
	    fcal_tmp.adc_en[kk] = fcal_signal_hits[ii].adc_en[kk];
	  
	  for(unsigned int jj = ii + 1; jj < fcal_signal_hits.size(); jj++){
	    if((fcal_signal_hits[ii].row  == fcal_signal_hits[jj].row) &&
	       (fcal_signal_hits[ii].column == fcal_signal_hits[jj].column)){

	      fcal_signal_hits[jj].merged = 1;
	      
	      for(int kk = 0; kk < sample; kk++)	      
		fcal_tmp.adc_en[kk] += fcal_signal_hits[jj].adc_en[kk];	
	    }
	  }

	  fcal_merged_hits.push_back(fcal_tmp);
	}	
	
	int fcal_hit_adc_en = 0;

	if(USE_RAW_SAMPLES) {
		for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++) {
			int row     = fcal_merged_hits[ii].row;
			int column  = fcal_merged_hits[ii].column;
			double pedestal =  fcal_pedestals[row][column];	    

		    for(int jj = 0; jj < sample; jj++) {
		        fcal_merged_hits[ii].adc_amp[jj] = fcal_merged_hits[ii].adc_en[jj];
				if( (fcal_merged_hits[ii].adc_amp[jj] - pedestal) > 0.)
			  		fcal_hit_adc_en += (fcal_merged_hits[ii].adc_amp[jj] - pedestal);
			}
		}
	} else {
		// Add baseline fluctuations for channels with hits
		if(simu_baseline_fcal){
		  for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++){
			int row     = fcal_merged_hits[ii].row;
			int column  = fcal_merged_hits[ii].column;
			double pedestal =  fcal_pedestals[row][column];	    
			AddBaseline(fcal_merged_hits[ii].adc_en, pedestal, gDRandom);       
		  }
		}
	
		// Digitize		
		for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++){
		  Digitize(fcal_merged_hits[ii].adc_en,fcal_merged_hits[ii].adc_amp);
		  //	  cout << " Digitize " << fcal_merged_hits[ii].adc_en[sample - 3] 
		  //   << "  " <<  fcal_merged_hits[ii].adc_amp[sample - 3] << endl;
		}
		
		for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++)
		  for(int jj = 0; jj < sample; jj++)
			if( (fcal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE) > 0.)
			  fcal_hit_adc_en += (fcal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE);
	}

	if(USE_DIGI && !USE_RAW_SAMPLES)
		status += GTPDigi(fcal_signal_hits, 1);
	else {
		status += FADC_SSP(fcal_merged_hits, 1);
		
		status += GTP(1);
	}
		
	//cerr << "BCAL energy sum" << endl;
	//cerr << "  num hits = " << bcal_hits.size() << endl;

	// BCAL	energy sum
	double bcal_hit_en = 0;
	
	for (unsigned int ii = 0; ii < bcal_hits.size(); ii++){
	  const DBCALDigiHit *bcaldigihit = nullptr;
	  bcal_hits[ii]->GetSingle(bcaldigihit);

	//cout << " ptr 0x" << hex << bcaldigihit << dec << endl;

	  // Shift time to simulate pile up hits
	  // don't need this for real data?
// 	  double time = bcal_hits[ii]->t + time_shift;	 
// 	  if((time < time_min) || (time > time_max)){
// 	    continue;
// 	  }
	  
	  int module = bcal_hits[ii]->module;
	  int layer  = bcal_hits[ii]->layer;
	  int sector = bcal_hits[ii]->sector;
	  int end    = bcal_hits[ii]->end;
	  
	//cerr << "BCAL hit module/layer/sector/end = " << module << "/" << layer << "/" << sector << "/" << end << endl;

	  // Check trigger masks
	  int ch_masked = 0;

	  for(unsigned int jj = 0; jj < bcal_trig_mask.size(); jj++){
	    if( (module == bcal_trig_mask[jj].module) && (layer == bcal_trig_mask[jj].layer) && 
		(sector == bcal_trig_mask[jj].sector) && (end == bcal_trig_mask[jj].end)  ){ 
	      ch_masked = 1;
	      break;
	    } 
	  }
	  

	  if(ch_masked == 0){
	    bcal_hit_en += bcal_hits[ii]->E;
	    
	    bcal_signal bcal_tmp;	    
	    bcal_tmp.module  = module;
	    bcal_tmp.layer   = layer;
  	    bcal_tmp.sector  = sector;
	    bcal_tmp.end     = end;

	    bcal_tmp.energy  = bcal_hits[ii]->E;
	    bcal_tmp.time    = bcal_hits[ii]->t;
	    bcal_tmp.pulse_peak = bcaldigihit->pulse_peak;
	    bcal_tmp.pulse_time = (bcaldigihit->pulse_time >> 6) & 0x1FF; // consider only course time;
	    bcal_tmp.pulse_integral = bcaldigihit->pulse_integral - bcaldigihit->nsamples_integral*TRIG_BASELINE;
	    memset(bcal_tmp.adc_amp,0,sizeof(bcal_tmp.adc_amp));
	    memset(bcal_tmp.adc_en, 0,sizeof(bcal_tmp.adc_en));
	    
	    //double bcal_adc_en  = bcaldigihit->pulse_integral;  // need to pedestal subtract!
	    double bcal_adc_en  = bcal_tmp.energy*BCAL_ADC_PER_MEV*1000;

	    if(USE_RAW_SAMPLES) {
	  		const Df250PulseData *pulsedata = nullptr;
	  		bcaldigihit->GetSingle(pulsedata);
	  		const Df250WindowRawData *rawdata = nullptr;
	  		pulsedata->GetSingle(rawdata);
   	
	    	int numsamples = (sample<rawdata->samples.size()) ? sample : rawdata->samples.size();
	    	for(int i=0; i<numsamples; i++) 
	    		bcal_tmp.adc_en[i] = rawdata->samples[i];
	    	
	    } else {
		    status = SignalPulse(bcal_adc_en, bcal_tmp.time, bcal_tmp.adc_en, 2);
		    status = 0;
 		}
	    
	    bcal_signal_hits.push_back(bcal_tmp);
	  }	  
	}       


	// Merge BCAL hits	
	for(unsigned int ii = 0; ii < bcal_signal_hits.size(); ii++){	  
	  
	  if(bcal_signal_hits[ii].merged == 1) continue;
	  
	  bcal_signal bcal_tmp;
	  bcal_tmp.module  =  bcal_signal_hits[ii].module;
	  bcal_tmp.layer   =  bcal_signal_hits[ii].layer;
	  bcal_tmp.sector  =  bcal_signal_hits[ii].sector;
	  bcal_tmp.end     =  bcal_signal_hits[ii].end;
	  
	  bcal_tmp.energy  = 0.;
	  bcal_tmp.time    = 0.;
	  
	  for(int kk = 0; kk < sample; kk++)	      
	    bcal_tmp.adc_en[kk] = bcal_signal_hits[ii].adc_en[kk];
	  
	  for(unsigned int jj = ii + 1; jj < bcal_signal_hits.size(); jj++){
	    if((bcal_signal_hits[ii].module == bcal_signal_hits[jj].module) &&
	       (bcal_signal_hits[ii].layer  == bcal_signal_hits[jj].layer) &&
	       (bcal_signal_hits[ii].sector == bcal_signal_hits[jj].sector) &&    
	       (bcal_signal_hits[ii].end    == bcal_signal_hits[jj].end)){
	      
	      bcal_signal_hits[jj].merged = 1;
	      
	      for(int kk = 0; kk < sample; kk++)	      
		bcal_tmp.adc_en[kk] += bcal_signal_hits[jj].adc_en[kk];	
	    }
	  }
	  
	  bcal_merged_hits.push_back(bcal_tmp);
	}
	

	int bcal_hit_adc_en = 0;

	if(USE_RAW_SAMPLES) {
		for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++)	  
		  for(int jj = 0; jj < sample; jj++) {
		    bcal_merged_hits[ii].adc_amp[jj] = bcal_merged_hits[ii].adc_en[jj];
			bcal_hit_adc_en += bcal_merged_hits[ii].adc_amp[jj];	 
		  } 
	} else {
		// Add baseline fluctuations for channels with hits
		if(simu_baseline_bcal){
		  for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++){
			// Assume that all BCAL pedestals are 100
			double pedestal = TRIG_BASELINE;	    
			AddBaseline(bcal_merged_hits[ii].adc_en, pedestal, gDRandom);       
		  }
		}
	
		// Digitize		
		for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++)
		  Digitize(bcal_merged_hits[ii].adc_en,bcal_merged_hits[ii].adc_amp);
		
		for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++)	  
		  for(int jj = 0; jj < sample; jj++)
			  if( (bcal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE) > 0.)
				  bcal_hit_adc_en += bcal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE;	  
	}
	
	if(USE_DIGI && !USE_RAW_SAMPLES)
		status += GTPDigi(bcal_signal_hits, 2);
	else {
		status = FADC_SSP(bcal_merged_hits, 2);
		
		status = GTP(2);
	}
		
	// temporary mask until files in RCDB
	int ecal_row_mask_min = 15;
	int ecal_row_mask_max = 24;
	int ecal_col_mask_min = 15;
	int ecal_col_mask_max = 24;
	
	//  ECAL energy sum	
	double ecal_hit_en = 0;

	//cerr << "ECAL energy sum" << endl;
	//cerr << "  num hits = " << ecal_hits.size() << endl;

	for (unsigned int ii = 0; ii < ecal_hits.size(); ii++){
	  const DECALDigiHit *ecaldigihit = nullptr;
	  ecal_hits[ii]->GetSingle(ecaldigihit);
	  
	  int row  = ecal_hits[ii]->row;
	  int col  = ecal_hits[ii]->column;

	  // temporary mask before files in RCDB
	  if( (row >= ecal_row_mask_min) && (row <= ecal_row_mask_max) ){
		  if( (col >= ecal_col_mask_min) && (col <= ecal_col_mask_max) ) continue;
	  }
	  
	  //cerr << "ECAL hit row/column = " << row << "/" << col << endl;
	  
	  // Shift time to simulate pile up hits
	  // don't need this for real data?
// 	  double time = ecal_hits[ii]->t + time_shift;
// 	  if((time < time_min) || (time > time_max)){
// 	    continue;
// 	  }

	  // Check channels masked for trigger
	  int ch_masked = 0;
	  for(unsigned int jj = 0; jj < ecal_trig_mask.size(); jj++){
	    if( (row == ecal_trig_mask[jj].row) && (col == ecal_trig_mask[jj].col)){	      
	      ch_masked = 1;
	      break;
	    } 
	  }
	  
	  if(ch_masked == 0){
	    ecal_hit_en += ecal_hits[ii]->E;
	    
	    ecal_signal ecal_tmp;
	    
	    ecal_tmp.row     = row;
	    ecal_tmp.column  = col;

	    ecal_tmp.energy  = ecal_hits[ii]->E;
	    ecal_tmp.time    = ecal_hits[ii]->t;
	    ecal_tmp.pulse_peak = ecaldigihit->pulse_peak;
	    ecal_tmp.pulse_time = (ecaldigihit->pulse_time >> 6) & 0x1FF; // consider only course time;
	    ecal_tmp.pulse_integral = ecaldigihit->pulse_integral - ecaldigihit->nsamples_integral*TRIG_BASELINE;
	    memset(ecal_tmp.adc_amp,0,sizeof(ecal_tmp.adc_amp));
	    memset(ecal_tmp.adc_en, 0,sizeof(ecal_tmp.adc_en));

	    //double ecal_adc_en  = ecaldigihit->pulse_integral;  // need to pedestal subtract!
	    double ecal_adc_en  = ecal_tmp.energy*ECAL_ADC_PER_MEV*1000;
	    
	    // Account for gain fluctuations 
// 	    if(simu_gain_ecal){
// 	      
// 	      double gain  =  ecal_gains[row][col];	  
// 	      
// 	      ecal_adc_en *= gain;
// 	    }
	    
	    if(USE_RAW_SAMPLES) {
	    	// handle different pulse types
	  		const Df250WindowRawData *rawdata = nullptr;
	  		const Df250PulseData *pulsedata = nullptr;
	  		ecaldigihit->GetSingle(pulsedata);
	  		pulsedata->GetSingle(rawdata);

		    if(rawdata == nullptr) {
		    	jerr << "DL1MCTrigger_factory_DATA: warning! couldn't load raw data ..." << endl;
		    }
// 	  		
// 	  		jerr << "0x" << hex << pulsedata << dec << endl;
// 	  		jerr << "0x" << hex << rawdata << dec << endl;
   	
	    	int numsamples = (sample<rawdata->samples.size()) ? sample : rawdata->samples.size();
	    	for(int i=0; i<numsamples; i++) 
	    		ecal_tmp.adc_en[i] = rawdata->samples[i];
	    	
	    } else {
		    status = SignalPulse(ecal_adc_en, ecal_tmp.time, ecal_tmp.adc_en, 3);
		    status = 0;
		}
	    
	    ecal_signal_hits.push_back(ecal_tmp);
	  }
	  
	}       		


	// Merge ECAL hits
	for(unsigned int ii = 0; ii < ecal_signal_hits.size(); ii++){	  
	  
	  if(ecal_signal_hits[ii].merged == 1) continue;
	  
	  ecal_signal ecal_tmp;
	  ecal_tmp.row     = ecal_signal_hits[ii].row;
	  ecal_tmp.column  = ecal_signal_hits[ii].column;
	  ecal_tmp.energy  = 0.;
	  ecal_tmp.time    = 0.;
	  for(int kk = 0; kk < sample; kk++)	      
	    ecal_tmp.adc_en[kk] = ecal_signal_hits[ii].adc_en[kk];
	  
	  for(unsigned int jj = ii + 1; jj < ecal_signal_hits.size(); jj++){
	    if((ecal_signal_hits[ii].row  == ecal_signal_hits[jj].row) &&
	       (ecal_signal_hits[ii].column == ecal_signal_hits[jj].column)){

	      ecal_signal_hits[jj].merged = 1;
	      
	      for(int kk = 0; kk < sample; kk++)	      
		ecal_tmp.adc_en[kk] += ecal_signal_hits[jj].adc_en[kk];	
	    }
	  }
	  
	  ecal_merged_hits.push_back(ecal_tmp);
	}	
	
	int ecal_hit_adc_en = 0;

	if(USE_RAW_SAMPLES) {
		for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++) {
			int row     = ecal_merged_hits[ii].row;
			int column  = ecal_merged_hits[ii].column;
			// need to add per-module pedestals
			double pedestal = ecal_pedestals[row][column];	    

		    for(int jj = 0; jj < sample; jj++) {
		        ecal_merged_hits[ii].adc_amp[jj] = ecal_merged_hits[ii].adc_en[jj];
				if( (ecal_merged_hits[ii].adc_amp[jj] - pedestal) > 0.)
			  		ecal_hit_adc_en += (ecal_merged_hits[ii].adc_amp[jj] - pedestal);
			}
		}
	} else {
		// Add baseline fluctuations for channels with hits
		if(simu_baseline_ecal){
		  for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++){
			int row     = ecal_merged_hits[ii].row;
			int column  = ecal_merged_hits[ii].column;
			// need to add per-module pedestals
			double pedestal = ecal_pedestals[row][column];	    
			AddBaseline(ecal_merged_hits[ii].adc_en, pedestal, gDRandom);       
		  }
		}
	
		// Digitize		
		for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++){
		  Digitize(ecal_merged_hits[ii].adc_en,ecal_merged_hits[ii].adc_amp);
		  //	  cout << " Digitize " << ecal_merged_hits[ii].adc_en[sample - 3] 
		  //   << "  " <<  ecal_merged_hits[ii].adc_amp[sample - 3] << endl;
		}
		
		for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++)
		  for(int jj = 0; jj < sample; jj++)
			if( (ecal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE) > 0.)
			  ecal_hit_adc_en += (ecal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE);
	}
	
	if(USE_DIGI && !USE_RAW_SAMPLES)
		status += GTPDigi(ecal_signal_hits, 3);
	else {
		status += FADC_SSP(ecal_merged_hits, 3);

		status += GTP(3);
	}
	
	// Search for triggers

	//cerr << "find triggers" << endl;

	l1_found = FindTriggers(trigger);

// 	  int fcal_gtp_max = 0;
// 	  int bcal_gtp_max = 0;
// 	  
// 	  for(unsigned int ii = 0; ii < sample; ii++){
// 	    if(fcal_gtp[ii] > fcal_gtp_max) fcal_gtp_max = fcal_gtp[ii];
// 	    if(bcal_gtp[ii] > bcal_gtp_max) bcal_gtp_max = bcal_gtp[ii];
// 	  }	
// 
// 	cerr << "l1_found = " << l1_found << endl;
// 	cerr << " fcal_hit_en     = " << fcal_hit_en << endl;
// 	cerr << " fcal_hit_adc_en = " << fcal_hit_adc_en << endl;
// 	cerr << " fcal_gtp_max    = " << fcal_gtp_max << endl;
// 	cerr << " bcal_hit_en     = " << bcal_hit_en << endl;
// 	cerr << " bcal_hit_adc_en = " << bcal_hit_adc_en << endl;
// 	cerr << " bcal_gtp_max    = " << bcal_gtp_max << endl;

	
	//if(l1_found){
	  
	  int fcal_gtp_max = 0;
	  int bcal_gtp_max = 0;
	  int ecal_gtp_max = 0;

	  for(unsigned int ii = 0; ii < sample; ii++){
	    if(fcal_gtp[ii] > fcal_gtp_max) fcal_gtp_max = fcal_gtp[ii];
	    if(bcal_gtp[ii] > bcal_gtp_max) bcal_gtp_max = bcal_gtp[ii];
	    if(ecal_gtp[ii] > ecal_gtp_max) ecal_gtp_max = ecal_gtp[ii];
	  }	

	  trigger->fcal_en      =  fcal_hit_en;
	  trigger->fcal_adc     =  fcal_hit_adc_en;
	  trigger->fcal_adc_en  =  fcal_hit_adc_en/FCAL_ADC_PER_MEV/1000.;
	  trigger->fcal_gtp     =  fcal_gtp_max;
	  trigger->fcal_gtp_en  =  fcal_gtp_max/FCAL_ADC_PER_MEV/1000.;
	  
	  trigger->bcal_en      =  bcal_hit_en;
	  trigger->bcal_adc     =  bcal_hit_adc_en;
	  trigger->bcal_adc_en  =  bcal_hit_adc_en/BCAL_ADC_PER_MEV_CORRECT/2./1000.;
	  trigger->bcal_gtp     =  bcal_gtp_max;
	  trigger->bcal_gtp_en  =  bcal_gtp_max/BCAL_ADC_PER_MEV_CORRECT/2./1000.;	  	  

	  trigger->ecal_en      =  ecal_hit_en;
	  trigger->ecal_adc     =  ecal_hit_adc_en;
	  trigger->ecal_adc_en  =  ecal_hit_adc_en/ECAL_ADC_PER_MEV/1000.;
	  trigger->ecal_gtp     =  ecal_gtp_max;
	  trigger->ecal_gtp_en  =  ecal_gtp_max/ECAL_ADC_PER_MEV/1000.;

	  // scale sum according to gains applied in trigger equation
	  trigger->fcal2_en      =  ECAL_GAIN*ecal_hit_en + FCAL_GAIN*fcal_hit_en;
	  trigger->fcal2_adc     =  ECAL_GAIN*ecal_hit_adc_en + FCAL_GAIN*fcal_hit_adc_en;
	  trigger->fcal2_adc_en  =  (ecal_hit_adc_en/ECAL_ADC_PER_MEV + fcal_hit_adc_en/FCAL_ADC_PER_MEV)/1000.;
	  trigger->fcal2_gtp     =  ECAL_GAIN*ecal_gtp_max + FCAL_GAIN*fcal_gtp_max;
	  trigger->fcal2_gtp_en  =  (ecal_gtp_max/ECAL_ADC_PER_MEV + fcal_gtp_max/FCAL_ADC_PER_MEV)/1000.;

	  // inser trigger sums for all events
	  Insert(trigger); 
	  		  
	  if(OUTPUT_TREE) {
	  	dTreeFillData.Fill_Single<ULong64_t>("RunNumber", runnumber);
	  	dTreeFillData.Fill_Single<ULong64_t>("EventNumber", eventnumber);
	  	dTreeFillData.Fill_Single<Double_t>("FCAL_En", trigger->fcal_en);
	    dTreeFillData.Fill_Single<Double_t>("FCAL_ADC", trigger->fcal_adc);
	    dTreeFillData.Fill_Single<Double_t>("FCAL_ADC_En", trigger->fcal_adc_en);
	    dTreeFillData.Fill_Single<Double_t>("FCAL_GTP", trigger->fcal_gtp);
	    dTreeFillData.Fill_Single<Double_t>("FCAL_GTP_En", trigger->fcal_gtp_en);
	  	dTreeFillData.Fill_Single<Double_t>("BCAL_En", trigger->bcal_en);
	    dTreeFillData.Fill_Single<Double_t>("BCAL_ADC", trigger->bcal_adc);
	    dTreeFillData.Fill_Single<Double_t>("BCAL_ADC_En", trigger->bcal_adc_en);
	    dTreeFillData.Fill_Single<Double_t>("BCAL_GTP", trigger->bcal_gtp);
	    dTreeFillData.Fill_Single<Double_t>("BCAL_GTP_En", trigger->bcal_gtp_en);
	    dTreeFillData.Fill_Single<Double_t>("ECAL_En", trigger->ecal_en);
	    dTreeFillData.Fill_Single<Double_t>("ECAL_ADC", trigger->ecal_adc);
	    dTreeFillData.Fill_Single<Double_t>("ECAL_ADC_En", trigger->ecal_adc_en);
	    dTreeFillData.Fill_Single<Double_t>("ECAL_GTP", trigger->ecal_gtp);
	    dTreeFillData.Fill_Single<Double_t>("ECAL_GTP_En", trigger->ecal_gtp_en);
		dTreeInterface->Fill(dTreeFillData);
	  }
	  
	  //} else{
	  //delete trigger;
	  //}
	
	return; //NOERROR;
}

//------------------
// EndRun
//------------------
void DL1MCTrigger_factory_DATA::EndRun(void)
{
}

//------------------
// Finish
//------------------
void DL1MCTrigger_factory_DATA::Finish(void)
{
    if(OUTPUT_TREE) {
  		delete dTreeInterface;
    }

	return; //NOERROR;
}

//*********************
// Read RCDB 
//*********************

int  DL1MCTrigger_factory_DATA::Read_RCDB(const std::shared_ptr<const JEvent>& event, bool print_messages)
{

auto runnumber = event->GetRunNumber();
#if HAVE_RCDB
  {
  // RCDB queries are heavy, so we load the data once for all threads
  std::lock_guard<std::mutex> lock(rcdb_mutex);
  if(RCDB_LOADED) return 0;
  
  RCDB_LOADED = true;

  vector<const DTranslationTable*> ttab;  
  event->Get(ttab);
  
  vector<string> SectionNames = {"TRIGGER", "GLOBAL", "FCAL", "BCAL", "ECAL", "TOF", "ST", "TAGH",
				 "TAGM", "PS", "PSC", "TPOL", "CDC", "FDC"};
  
  string RCDB_CONNECTION;

  if( getenv("RCDB_CONNECTION")!= NULL )
    RCDB_CONNECTION = getenv("RCDB_CONNECTION");
  else
    RCDB_CONNECTION = "mysql://rcdb@hallddb.jlab.org/rcdb";   // default to outward-facing MySQL DB


  rcdb::Connection connection(RCDB_CONNECTION);
  
  auto  rtvsCnd  =  connection.GetCondition(runnumber, "rtvs");
  
  if( !rtvsCnd ) {
    cout<<"'rtvs' condition is not set for run " << runnumber << endl;
    return 2;
  }
  
  
  auto json = rtvsCnd->ToJsonDocument();               // The CODA rtvs is serialized as JSon dictionary.  
  string fileName(json["%(config)"].GetString());      // We need item with name '%(config)'
  
  auto file = connection.GetFile(runnumber, fileName);
  
  if(!file) {                                                         // If there is no such file, null is returned
    cout<<"File with name: "<< fileName
	<<" doesn't exist (not associated) with run: "<< runnumber << endl;
    return 3;
  }
  
  string fileContent = file->GetContent();                               // Get file content
  auto result = rcdb::ConfigParser::Parse(fileContent, SectionNames);    // Parse it!
  
  
  // Pulse parameters for trigger

  auto trig_thr = result.Sections["FCAL"].NameValues["FADC250_TRIG_THR"];
  auto trig_nsb = result.Sections["FCAL"].NameValues["FADC250_TRIG_NSB"];
  auto trig_nsa = result.Sections["FCAL"].NameValues["FADC250_TRIG_NSA"];
  
  if(trig_thr.size() > 0){
    FCAL_CELL_THR  =  stoi(trig_thr);
    if(FCAL_CELL_THR < 0) FCAL_CELL_THR = 0;
  }

  if(trig_nsb.size() > 0)
    FCAL_NSB  =  stoi(trig_nsb);

  if(trig_thr.size() > 0)
    FCAL_NSA   =  stoi(trig_nsa);

  trig_thr = result.Sections["BCAL"].NameValues["FADC250_TRIG_THR"];
  trig_nsb = result.Sections["BCAL"].NameValues["FADC250_TRIG_NSB"];
  trig_nsa = result.Sections["BCAL"].NameValues["FADC250_TRIG_NSA"];
  
  if(trig_thr.size() > 0){
    BCAL_CELL_THR   =  stoi(trig_thr);
    if(BCAL_CELL_THR < 0) BCAL_CELL_THR = 0;
  }

  if(trig_nsb.size() > 0)
    BCAL_NSB  =  stoi(trig_nsb);

  if(trig_thr.size() > 0)
    BCAL_NSA  =  stoi(trig_nsa);

  trig_thr = result.Sections["ECAL"].NameValues["FADC250_TRIG_THR"];
  trig_nsb = result.Sections["ECAL"].NameValues["FADC250_TRIG_NSB"];
  trig_nsa = result.Sections["ECAL"].NameValues["FADC250_TRIG_NSA"];
  
  if(trig_thr.size() > 0){
    ECAL_CELL_THR   =  stoi(trig_thr);
    if(ECAL_CELL_THR < 0) ECAL_CELL_THR = 0;
  }

  if(trig_nsb.size() > 0)
    ECAL_NSB  =  stoi(trig_nsb);

  if(trig_thr.size() > 0)
    ECAL_NSA  =  stoi(trig_nsa);

  // List of enabled GTP equations and triggers
  vector<vector<string>> triggerTypes;

  for(auto row : result.Sections["TRIGGER"].Rows) {    

    if(row[0] == "TRIG_TYPE") {
      
      if(row.size() >= 9){
	triggerTypes.push_back(row);       // The line in config file starts with TRIG_TYPE
      } else {
	cout <<  " Cannot parse TRIG_TYPE. Insufficient number of parameters " <<  row.size() << endl; 
      }      
    }    
   

    // Integration windows for BCAL and FCAL 
    if(row[0] == "TRIG_EQ") {
      if(row.size() >= 5){
	if(stoi(row[4]) == 1){              // Trigger enabled 
	  if(row[1] == "FCAL")
	    FCAL_WINDOW = stoi(row[3]);  
	  if(row[1] == "BCAL_E")
	    BCAL_WINDOW = stoi(row[3]);
	  if(row[1] == "ECAL")
	    ECAL_WINDOW = stoi(row[3]);
	}
      }            
    }

    // ECAL and FCAL gains
    if(row[0] == "TRIG_ECAL_FCAL_GAIN") {
      ECAL_GAIN = stof(row[1]);
      FCAL_GAIN = stof(row[2]);
    }
  }
  
  

  
  for(int ii = 0; ii < 32; ii++){
    
    int trig_found = 0;
    
    trigger_conf trigger_tmp;

    memset(&trigger_tmp,0,sizeof(trigger_tmp));
    
    trigger_tmp.bit = ii;   
    
    for(unsigned int jj = 0; jj < triggerTypes.size(); jj++){
      
      if(triggerTypes[jj].size() < 9) continue;   // Minimum  9 parameters are required in the config file 

      int bit = stoi(triggerTypes[jj][8]);  // Trigger bit
      
      if( bit == ii){      
		
	// FCAL, BCAL triggers 
	if(triggerTypes[jj][1] == "BFCAL"){
	  
	  int fcal = 0; 
	  int bcal = 0; 

	  if(triggerTypes[jj][4].size() > 0) fcal = stoi(triggerTypes[jj][4]); 
	  if(triggerTypes[jj][5].size() > 0) bcal = stoi(triggerTypes[jj][5]); 


	  if( (fcal > 0) && (bcal > 0)){             // FCAL & BCAL 
	    trigger_tmp.type  =  0x3;
	    trigger_tmp.gtp.fcal      =   fcal;
	    trigger_tmp.gtp.bcal      =   bcal;
	    if(triggerTypes[jj][6].size() > 0) trigger_tmp.gtp.en_thr = stoi(triggerTypes[jj][6]);
	    
	    if(triggerTypes[jj].size() > 9){
	      if((triggerTypes[jj].size() >= 10) && (triggerTypes[jj][9].size() > 0))  trigger_tmp.gtp.fcal_min  =   stoi(triggerTypes[jj][9]);
	      if((triggerTypes[jj].size() >= 11) && (triggerTypes[jj][10].size() > 0)) trigger_tmp.gtp.fcal_max  =   stoi(triggerTypes[jj][10]);
	      if((triggerTypes[jj].size() >= 12) && (triggerTypes[jj][11].size() > 0)) trigger_tmp.gtp.bcal_min  =   stoi(triggerTypes[jj][11]);
	      if((triggerTypes[jj].size() >= 13) && (triggerTypes[jj][12].size() > 0)) trigger_tmp.gtp.bcal_max  =   stoi(triggerTypes[jj][12]);
	    }
	    
	    trig_found++;
	  } else if ((fcal > 0) && (bcal == 0)){      // FCAL only
	    trigger_tmp.type  =  0x1;
	    trigger_tmp.gtp.fcal      =   fcal;
	    if(triggerTypes[jj][6].size() > 0)  trigger_tmp.gtp.en_thr      =   stoi(triggerTypes[jj][6]);
	    if(triggerTypes[jj].size() > 9){
	      if(triggerTypes[jj][9].size() > 0) trigger_tmp.gtp.fcal_min   =   stoi(triggerTypes[jj][9]);
	      if(triggerTypes[jj][10].size() > 0) trigger_tmp.gtp.fcal_max  =   stoi(triggerTypes[jj][10]);
	    }
	    trig_found++;
	  } else if ((bcal > 0) && (fcal == 0)){      // BCAL only
	    trigger_tmp.type  =  0x2;
	    trigger_tmp.gtp.bcal      =   bcal;
	    if(triggerTypes[jj][6].size() > 0)  trigger_tmp.gtp.en_thr    =   stoi(triggerTypes[jj][6]);
	    if((triggerTypes[jj].size() >= 12) && (triggerTypes[jj][11].size() > 0))  trigger_tmp.gtp.bcal_min  =   stoi(triggerTypes[jj][11]);
	    if((triggerTypes[jj].size() >= 13) && (triggerTypes[jj][12].size() > 0))  trigger_tmp.gtp.bcal_max  =   stoi(triggerTypes[jj][12]);

	    
	    trig_found++;
	  } else {
	    cout << " Incorrect parameters for BFCAL trigger " << endl;
	  }
	  
	} else if(triggerTypes[jj][1] == "ST"){          //  ST 
	  trigger_tmp.type    =  0x4;
	  if(triggerTypes[jj][7].size() > 0)   trigger_tmp.gtp.st_nhit     =  stoi(triggerTypes[jj][7]);
	  if((triggerTypes[jj].size() >= 14) && (triggerTypes[jj][13].size() > 0))  trigger_tmp.gtp.st_pattern  =  stoi(triggerTypes[jj][13]);

	  trig_found++;
	}   else if(triggerTypes[jj][1] == "PS"){        //  PS
	  trigger_tmp.type    =  0x8;
	  if(triggerTypes[jj][7].size() > 0)  trigger_tmp.gtp.ps_nhit      =  stoi(triggerTypes[jj][7]);
	  if((triggerTypes[jj].size() >= 14) && (triggerTypes[jj][13].size() > 0)) trigger_tmp.gtp.ps_pattern   =  stoi(triggerTypes[jj][13]);
	  trig_found++;
	}  else if(triggerTypes[jj][1] == "TAGH"){       //  TAGH
	  trigger_tmp.type     =  0x10;
	  if(triggerTypes[jj][7].size() > 0)  trigger_tmp.gtp.tof_nhit      =  stoi(triggerTypes[jj][7]);
	  if((triggerTypes[jj].size() >= 14) && (triggerTypes[jj][13].size() > 0)){
	    trigger_tmp.gtp.tagh_pattern   =  stoi(triggerTypes[jj][13],nullptr,0);
	  }

	  trig_found++;
	}  else if(triggerTypes[jj][1] == "TOF"){        //  TOF
	  trigger_tmp.type     =  0x20;
	  if(triggerTypes[jj][7].size() > 0)  trigger_tmp.gtp.ps_nhit      =  stoi(triggerTypes[jj][7]);
	  if((triggerTypes[jj].size() >= 14) && (triggerTypes[jj][13].size() > 0)) trigger_tmp.gtp.tof_pattern   =  stoi(triggerTypes[jj][13]);
	  trig_found++;
	} else {
	  cout << " Incorrect Trigger type " << triggerTypes[jj][1] << endl;
	}
	
	if(trig_found > 0){
	  triggers_enabled.push_back(trigger_tmp);
	}

      }  // Trigger lane is enabled      
    }    // Trigger types
      
  }
  


  // Load FCAL Trigger Masks

  string comDir  = result.Sections["FCAL"].NameValues["FADC250_COM_DIR"];
  string comVer  = result.Sections["FCAL"].NameValues["FADC250_COM_VER"];
  string userDir = result.Sections["FCAL"].NameValues["FADC250_USER_DIR"];
  string userVer = result.Sections["FCAL"].NameValues["FADC250_USER_VER"];
  
  
  for(int crate = 1; crate <= 12; crate++){
    
    std::string s = std::to_string(crate);
    
    string comFileName   =  comDir  +  "/rocfcal" +s + "_fadc250_" + comVer + ".cnf";
    string userFileName  =  userDir + "/rocfcal"  +s + "_" + userVer + ".cnf";
        
    auto userFile = connection.GetFile(runnumber, userFileName);
    
    if(!userFile) {                                                         
      //      cout<<" USER File with name: "<< userFileName
      //	  <<" doesn't exist (not associated) with run: "<< runnumber <<endl;
      continue;      
    }
    
    
    auto userParseResult = rcdb::ConfigParser::ParseWithSlots(userFile->GetContent(), "FADC250_SLOTS");
    
    
    for(unsigned int slot = 3; slot <= 21; slot++){
      
      auto userValues = userParseResult.SectionsBySlotNumber[slot].NameVectors["FADC250_TRG_MASK"];  // Parse it and return
      
      if(userValues.size() > 0){
	for (unsigned int ch = 0; ch < userValues.size(); ++ch) {

	  if(userValues[ch].size() == 0) continue;
	  
	  if(stoi(userValues[ch]) > 0){

	    uint32_t roc_id = 10 + crate;

	    DTranslationTable::csc_t daq_index = {roc_id, slot, ch };
	    
	    DTranslationTable::DChannelInfo channel_info;
	    
	    try {		
	      channel_info = ttab[0]->GetDetectorIndex(daq_index);	      
	    }
	    
	    catch(...){
	      if(print_messages) cout << "Exception: FCAL channel is not in the translation table  " <<  " Crate = " << 10 + crate << "  Slot = " << slot << 
		" Channel = " << ch << endl;
	      continue;
	    }
	    	    
	    fcal_mod tmp;

	    tmp.roc     =  crate;	    
	    tmp.slot    =  slot;
	    tmp.ch      =  ch;
	    
	    tmp.row = channel_info.fcal.row;
	    tmp.col = channel_info.fcal.col;
	    
	    fcal_trig_mask.push_back(tmp);
	  }
	  
	}
	
      }	
      
    }  // Loop over slots
  }    // Loop over crates       
  
  
  

  // Load BCAL Trigger Masks

  comDir  = result.Sections["BCAL"].NameValues["FADC250_COM_DIR"];
  comVer  = result.Sections["BCAL"].NameValues["FADC250_COM_VER"];
  userDir = result.Sections["BCAL"].NameValues["FADC250_USER_DIR"];
  userVer = result.Sections["BCAL"].NameValues["FADC250_USER_VER"];
  

  for(int crate = 1; crate <= 12; crate++){
    
    if( (crate == 3) || (crate == 6) || (crate == 9) || (crate == 12)) continue;

    std::string s = std::to_string(crate);
    
    string comFileName   =  comDir  +  "/rocbcal" +s + "_fadc250_" + comVer + ".cnf";
    string userFileName  =  userDir + "/rocbcal"  +s + "_" + userVer + ".cnf";
    
    auto userFile = connection.GetFile(runnumber, userFileName);
    
    if(!userFile){
      //      cout<<" USER File with name: "<< userFileName
      //	  <<" doesn't exist (not associated) with run: "<< runnumber <<endl;                                               
      continue;
    }
    
    
    auto userParseResult = rcdb::ConfigParser::ParseWithSlots(userFile->GetContent(), "FADC250_SLOTS");
      
    for(unsigned int slot = 3; slot <= 21; slot++){
      
      auto userValues = userParseResult.SectionsBySlotNumber[slot].NameVectors["FADC250_TRG_MASK"];  // Parse it and return
      
      if(userValues.size() > 0){

	for (unsigned int ch = 0; ch < userValues.size(); ++ch) {

	  if(userValues[ch].size() == 0) continue;
	  
	  if(stoi(userValues[ch]) > 0){

	    uint32_t roc_id = 30 + crate;

	    DTranslationTable::csc_t daq_index = {roc_id, slot, ch };
	    
	    DTranslationTable::DChannelInfo channel_info;
	    
	    try {		
	      channel_info = ttab[0]->GetDetectorIndex(daq_index);	      
	    }
	    
	    catch(...){
	      cout << "Exception: BCAL channel is not in the translation table  " <<  " Crate = " << 30 + crate << "  Slot = " << slot << 
		" Channel = " << ch << endl;
	      continue;
	    }
	    
	    
	    bcal_mod tmp;

	    tmp.roc     =  crate;	    
	    tmp.slot    =  slot;
	    tmp.ch      =  ch;

	    tmp.module  =  channel_info.bcal.module;
	    tmp.layer   =  channel_info.bcal.layer;
	    tmp.sector  =  channel_info.bcal.sector;	      
	    tmp.end     =  channel_info.bcal.end;

	    bcal_trig_mask.push_back(tmp);
	  }
	  
	}
	
      }	
      
    }  // Loop over slots
  }    // Loop over crates       
  
  }
  
  return 0;

#else // HAVE_RCDB

  return 10; // RCDB is not available

#endif

}


int  DL1MCTrigger_factory_DATA::SignalPulse(double en, double time, double amp_array[sample], int type){
   

  // Parameterize and digitize pulse shapes. Sum up amplitudes
  // type = 1 - FCAL
  // type = 2 - BCAL
  // type = 3 - ECAL

  float exp_par = 0.358;

  int pulse_length = 20;
  
  if(type == 2) exp_par = 0.0787;
  
  int sample_first = (int)floor(time/time_stamp);
  
  // digitization range
  int ind_min = sample_first + 1;
  int ind_max = ind_min + pulse_length + 1;
    
  if( (ind_min > sample) || (ind_min < 0)){
    //    cout << " SignalPulse() FATAL error: time out of range   "  <<  time <<  "     " << ind_min << endl;
    return 1;
  }
  
  if(ind_max > sample){
    //    cout << " SignalPulse(): ind_max set to maximum" << time <<  "  "  << ind_max << endl;
    ind_max = sample - 1;
  }
  
  for(int i = ind_min; i < ind_max; i++ ){
    double adc_t  =  time_stamp*i - time;
    double amp    =  exp_par*exp_par*exp(-adc_t*exp_par)*adc_t;
    
    //    amp_array[i] += (int)(amp*time_stamp*en + 0.5);
    //    if(amp_array[i] > max_adc_bins){
    //      amp_array[i] = max_adc_bins;
    //    }   

    //if(amp > 0.001) 
    //	    cout<<adc_t<<" "<<amp<<" "<<en<<endl;
    
    amp_array[i] += amp*time_stamp*en;
    
  }
  
  return 0;  
}

int DL1MCTrigger_factory_DATA::GTP(int detector){

  // 1  - FCAL
  // 2  - BCAL
  // 3  - ECAL

  int INT_WINDOW = 20; 

  switch(detector){
  case 1:
    INT_WINDOW =  FCAL_WINDOW;
    break;
  case 2:
    INT_WINDOW =  BCAL_WINDOW;
    break;
  case 3:
    INT_WINDOW =  ECAL_WINDOW;
    break;
  default:
    break;
  }

  int index_min = 0;
  int index_max = 0;

  for(unsigned int samp = 0; samp < sample; samp++){

    index_max = samp;
    index_min = samp - INT_WINDOW;
    
    if(index_min < 0) index_min = 0;

    int energy_sum = 0;

    for(int ii = index_min; ii <= index_max; ii++){
      if(detector == 1)
	energy_sum += fcal_ssp[ii];
      else if(detector == 3)
	energy_sum += ecal_ssp[ii];
      else
	energy_sum += bcal_ssp[ii];
    }

    if(detector == 1)
      fcal_gtp[samp] = energy_sum;
    else if(detector == 3)
      ecal_gtp[samp] = energy_sum;
    else 
      bcal_gtp[samp] = energy_sum;
  }  

  return 0;

}

template <typename T>  int DL1MCTrigger_factory_DATA::GTPDigi(vector<T> digi_hits, int detector){

  // 1  - FCAL
  // 2  - BCAL
  // 3  - ECAL

  int EN_THR =  4096; 
  int INT_WINDOW = 20; 

  switch(detector){
  case 1:
    EN_THR     =  FCAL_CELL_THR;
    INT_WINDOW =  FCAL_WINDOW;
    break;
  case 2:
    EN_THR     =  BCAL_CELL_THR;
    INT_WINDOW =  BCAL_WINDOW;
    break;
  case 3:
    EN_THR     =  ECAL_CELL_THR;
    INT_WINDOW =  ECAL_WINDOW;
    break;
  default:
    break;
  }

  for(unsigned int hit = 0; hit < digi_hits.size(); hit++){
	  
    // require pulse peak above threshold
    if(digi_hits[hit].pulse_peak < EN_THR)
	    continue;
	    
    // (for later) require in trigger timing window
    //int adc_time = digi_hits[hit].pulse_time;
    //if((adc_time < 15) || (adc_time > 50)) continue;

    // insert DigiHit pulse integral as single sample
    if(detector == 1)
      fcal_gtp[10] += digi_hits[hit].pulse_integral;
    else if(detector == 3)
      ecal_gtp[10] += digi_hits[hit].pulse_integral;
    else 
      bcal_gtp[10] += digi_hits[hit].pulse_integral;

  }

  
  return 0;

}

template <typename T>  int DL1MCTrigger_factory_DATA::FADC_SSP(vector<T> merged_hits, int detector){
  
  //  1  - FCAL
  //  2  - BCAL
  //  3  - ECAL

  int EN_THR =  4096; 
  int NSA    =  10;
  int NSB    =  3;
  
  switch(detector){
  case 1:
    EN_THR =  FCAL_CELL_THR;
    NSA    =  FCAL_NSA;
    NSB    =  FCAL_NSB;
    break;
  case 2:
    EN_THR =  BCAL_CELL_THR;
    NSA    =  BCAL_NSA;
    NSB    =  BCAL_NSB;
    break;
  case 3:
    EN_THR =  ECAL_CELL_THR;
    NSA    =  ECAL_NSA;
    NSB    =  ECAL_NSB;
    break;
  default:
    break;
  }
  
  for(unsigned int hit = 0; hit < merged_hits.size(); hit++){

    int index_min  =  -10;
    int index_max  =  -10;
    
    for(int ii = 0; ii < sample; ii++){

      int pulse_found = 0;

      if(merged_hits[hit].adc_amp[ii] >= EN_THR){
	pulse_found = 1;
      } else {
	continue;
      }

      index_min = ii - NSB;

      if(index_max > index_min) index_min = index_max + 1;

      index_max = ii + NSA - 1;

      if(index_min < 0) index_min = 0;

      if(index_max >= sample){
	index_max = sample - 1;
      }

      int extend_nsa = 1;

      // Extend FADC range if needed
      
      while(extend_nsa){
	
	int index_tmp = index_max + 1;
	
	if(index_tmp < sample){
	  if(merged_hits[hit].adc_amp[index_tmp] >= EN_THR){
	    index_max += NSA;
	  } else extend_nsa = 0;
	} else extend_nsa = 0;	
      }
      
      if(index_max >= sample)
	index_max = sample - 1;
          
      for(int kk = index_min; kk <= index_max; kk++){
	if(detector == 1){
	  if((merged_hits[hit].adc_amp[kk] - 100) > 0)
	    fcal_ssp[kk] += (merged_hits[hit].adc_amp[kk] - TRIG_BASELINE);
	}
	else if(detector == 2){
	  if((merged_hits[hit].adc_amp[kk] - 100) > 0)
	    bcal_ssp[kk] += (merged_hits[hit].adc_amp[kk] - TRIG_BASELINE);
	}
	else if(detector == 3){
	  if((merged_hits[hit].adc_amp[kk] - 100) > 0)
	    ecal_ssp[kk] += (merged_hits[hit].adc_amp[kk] - TRIG_BASELINE);
	}
      }
      
      if(pulse_found == 1){
	ii = index_max + 1;
	pulse_found = 0;
      }
      
    }        
    
  }

  return 0;
}

void DL1MCTrigger_factory_DATA::PrintTriggers(){
  
  string detector;
  int nhit = 0;
  unsigned int pattern = 0;

  cout << endl << endl;
  cout << " ------------  Trigger Settings --------------- " << endl;
  cout << endl << endl;

  cout << "----------- FCAL ----------- " << endl << endl;

  cout << "FCAL_CELL_THR  = " <<  setw(10) <<  FCAL_CELL_THR <<  endl;
  cout << "FCAL_NSA       = " <<  setw(10) <<  FCAL_NSA      <<  endl;
  cout << "FCAL_NSB       = " <<  setw(10) <<  FCAL_NSB      <<  endl;
  cout << "FCAL_WINDOW    = " <<  setw(10) <<  FCAL_WINDOW   <<  endl;
  cout << "FCAL_GAIN      = " <<  setw(10) <<  FCAL_GAIN     <<  endl;

  cout << endl;
  
  cout << "----------- BCAL ----------- " << endl << endl;

  cout << "BCAL_CELL_THR  = " <<  setw(10) <<  BCAL_CELL_THR <<  endl;
  cout << "BCAL_NSA       = " <<  setw(10) <<  BCAL_NSA      <<  endl;
  cout << "BCAL_NSB       = " <<  setw(10) <<  BCAL_NSB      <<  endl;
  cout << "BCAL_WINDOW    = " <<  setw(10) <<  BCAL_WINDOW   <<  endl;

   cout << endl;
  
  cout << "----------- ECAL ----------- " << endl << endl;

  cout << "ECAL_CELL_THR  = " <<  setw(10) <<  ECAL_CELL_THR <<  endl;
  cout << "ECAL_NSA       = " <<  setw(10) <<  ECAL_NSA      <<  endl;
  cout << "ECAL_NSB       = " <<  setw(10) <<  ECAL_NSB      <<  endl;
  cout << "ECAL_WINDOW    = " <<  setw(10) <<  ECAL_WINDOW   <<  endl;
  cout << "ECAL_GAIN      = " <<  setw(10) <<  ECAL_GAIN     <<  endl;
  
  cout << endl << endl;


  if(triggers_enabled.size() > 0){
    cout  << "TYPE     "  << "FCAL_E   " << "BCAL_E   " << 
      "EN_THR   " << "NHIT     "  << "LANE     " << "FCAL_EMIN    " << "FCAL_EMAX   "  <<
      "BCAL_EMIN   " << "BCAL_EMAX   " << "PATTERN   " << endl;
  }


  for(unsigned int ii = 0; ii < triggers_enabled.size(); ii++){
    
    switch(triggers_enabled[ii].type){
    case 1: detector =  "BFCAL  ";
      break;
    case 2: detector =  "BFCAL  ";
      break;
    case 3: detector =  "BFCAL  ";
      break;
    case 4: detector =  "ST     ";
      nhit    = triggers_enabled[ii].gtp.st_nhit;
      pattern = triggers_enabled[ii].gtp.st_pattern;
      break;
    case 8: detector = "PS     ";
      nhit    = triggers_enabled[ii].gtp.ps_nhit;
      pattern = triggers_enabled[ii].gtp.ps_pattern;
      break;
    case 16: detector = "TAGH   ";
      nhit    = triggers_enabled[ii].gtp.tagh_nhit;
      pattern = triggers_enabled[ii].gtp.tagh_pattern;
      break;
    case 32: detector = "TOF   ";
      nhit    = triggers_enabled[ii].gtp.tof_nhit;
      pattern = triggers_enabled[ii].gtp.tof_pattern;
      break;
    default: detector = "NONE ";
      cout << " Unknown detector ===== " << triggers_enabled[ii].type << endl;
      break;
    }
    
    cout  << detector  <<  setw(6) <<  
      triggers_enabled[ii].gtp.fcal << setw(9) <<  
      triggers_enabled[ii].gtp.bcal << setw(11) <<  
      triggers_enabled[ii].gtp.en_thr << setw(6) <<  
      nhit << setw(9) << 
      triggers_enabled[ii].bit << setw(12) <<  
      triggers_enabled[ii].gtp.fcal_min << setw(14) <<
      triggers_enabled[ii].gtp.fcal_max << setw(10) <<
      triggers_enabled[ii].gtp.bcal_min << setw(14) <<
      triggers_enabled[ii].gtp.bcal_max << setw(8) <<
      hex << uppercase << "0x" << pattern << nouppercase << dec <<  endl;

  }

  cout << endl << endl;

}



int DL1MCTrigger_factory_DATA::FindTriggers(DL1MCTrigger *trigger){
  
  int trig_found = 0;
  
  // Main production trigger  
  for(unsigned int ii = 0; ii < triggers_enabled.size(); ii++){
    
    if(triggers_enabled[ii].bit == 0){    // Main production trigger found
      
      int gtp_energy  = 0;
      int bcal_energy = 0;
      
      for(unsigned int samp = 0; samp < sample; samp++){
	
	int bcal_samp = samp - BCAL_OFFSET;
	
	if(bcal_samp < 0){
	  bcal_energy = 0;
	} else if(bcal_samp >= sample){ 
	  bcal_energy = 0;
	} else{ 
	  bcal_energy = bcal_gtp[bcal_samp];
	}


	gtp_energy = triggers_enabled[ii].gtp.fcal*(FCAL_GAIN * fcal_gtp[samp] + ECAL_GAIN * ecal_gtp[samp]) +
	  triggers_enabled[ii].gtp.bcal*bcal_energy;
	
	if(gtp_energy >= triggers_enabled[ii].gtp.en_thr){

	  if(triggers_enabled[ii].gtp.fcal_min > 0) {     // FCAL > 0
	    if(fcal_gtp[samp] > triggers_enabled[ii].gtp.fcal_min){
	      trigger->trig_mask   = (trigger->trig_mask | 0x1);
	      trigger->trig_time[0]   = samp - 25;
	      trig_found++;
	    }
	  } else {
	    
	    trigger->trig_mask   = (trigger->trig_mask | 0x1);
	    trigger->trig_time[0]   = samp - 25; 
	    trig_found++;
	  }
	  
	  break;

	}  // Check energy threshold	
      }      
    }     // Trigger Bit 0
  

    if(triggers_enabled[ii].bit == 2){    //  BCAL trigger found
            
      int gtp_energy  = 0;
      int bcal_energy = 0;
      
      for(unsigned int samp = 0; samp < sample; samp++){
	
	int bcal_samp = samp - BCAL_OFFSET;
	
	if(bcal_samp < 0){
	  bcal_energy = 0;
	} else if(bcal_samp >= sample){ 
	  bcal_energy = 0;
	} else{ 
	  bcal_energy = bcal_gtp[bcal_samp];
	}
	      
	gtp_energy = triggers_enabled[ii].gtp.bcal*bcal_energy;
      
	if(gtp_energy >= triggers_enabled[ii].gtp.en_thr){
	  
	  trigger->trig_mask   = (trigger->trig_mask | 0x4);
	  trigger->trig_time[2]   = samp - 25; 
	  trig_found++;

	  break;
	  
	}  // Energy threshold			
      }
    }      // Trigger Bit  3

  }    // Loop over triggers found in the config file
  
  return trig_found;
  
}


// Fill fcal calibration tables similar to FCALHit factory
void DL1MCTrigger_factory_DATA::LoadFCALConst(fcal_constants_t &table, const vector<double> &fcal_const_ch, 
					 const DFCALGeometry  &fcalGeom){
  for (int ch = 0; ch < static_cast<int>(fcal_const_ch.size()); ch++) {
    if (fcalGeom.isBlockActive(ch)){
      int row = fcalGeom.row(ch);
      int col = fcalGeom.column(ch);
      table[row][col] = fcal_const_ch[ch];
    }
  }
}

// Fill ecal calibration tables similar to ECALHit factory
void DL1MCTrigger_factory_DATA::LoadECALConst(ecal_constants_t &table, const vector<double> &ecal_const_ch, 
					 const DECALGeometry  &ecalGeom){
  for (int ch = 0; ch < static_cast<int>(ecal_const_ch.size()); ch++) {
	  //if (ecalGeom.isBlockActive(ch)){
	  int row = ecalGeom.row(ch);
	  int col = ecalGeom.column(ch);
	  table[row][col] = ecal_const_ch[ch];
	  //}
  }
}

void DL1MCTrigger_factory_DATA::Digitize(double adc_amp[sample], int adc_count[sample]){

  for(int samp = 0; samp < sample; samp++ ){
    
    adc_count[samp] += (int)(adc_amp[samp] + TRIG_BASELINE + 0.5);

    if(adc_count[samp] > max_adc_bins)
      adc_count[samp] = max_adc_bins;

  }
}


void DL1MCTrigger_factory_DATA::AddBaseline(double adc_amp[sample], double pedestal, DRandom2 &gDRandom){

  double pedestal_correct = pedestal - TRIG_BASELINE;

  for(int samp = 0; samp < sample; samp++ ){  
    double tmp = gDRandom.Gaus(pedestal_correct, pedestal_sigma);   
    adc_amp[samp] += tmp;
    //    cout << "  " << tmp;
  }
  
  if(debug){
    cout << endl;    
    cout << " Corrected pedestals = " << pedestal_correct << "  " <<  adc_amp[sample - 2] 
	 << "  " << pedestal_sigma << endl; 
  }

}



void DL1MCTrigger_factory_DATA::GetSeeds(const std::shared_ptr<const JEvent>& event, UInt_t &seed1, UInt_t &seed2, UInt_t &seed3){

  // Use seeds similar to mcsmear
  auto eventnumber = event->GetEventNumber();
  
  JEventSource *source = event->GetJEventSource();
  
  DEventSourceHDDM *hddm_source = dynamic_cast<DEventSourceHDDM*>(source);
  
  if (!hddm_source) {

//     cerr << "DL1MCTrigger_factory_DATA:  This program MUST be used with an HDDM file as input!" << endl;
//     cerr << "   Default seeds will be used for the random generator  " << endl;

	// NOTE - this should always happen for EVIO data!
    seed1 = 259921049 + eventnumber;
    seed2 = 442249570 + eventnumber;
    seed3 = 709975946 + eventnumber;
  } else {
  
    hddm_s::HDDM *record = const_cast<hddm_s::HDDM *>(event->GetSingle<hddm_s::HDDM>());
    if (!record){
      seed1 = 259921049 + eventnumber;
      seed2 = 442249570 + eventnumber;
      seed3 = 709975946 + eventnumber;  
    } else {
      
      
      hddm_s::ReactionList::iterator reiter = record->getReactions().begin();
      
      hddm_s::Random my_rand = reiter->getRandom();
	
      // Copy seeds from event record to local variables
      seed1 = my_rand.getSeed1();
      seed2 = my_rand.getSeed2();
      seed3 = my_rand.getSeed3();
      
      // If no seeds are stored in the hddm file, generate them in the same way
      // as in mcsmear      
      if ((seed1 == 0) || (seed2 == 0) || (seed3 == 0)){
	uint64_t eventNo = record->getPhysicsEvent().getEventNo();
	seed1 = 259921049 + eventNo;
	seed2 = 442249570 + eventNo;
	seed3 = 709975946 + eventNo;

      }
      
    }  // Record doesn't exist
  }    // Not an HDDM file
}
