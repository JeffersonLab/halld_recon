// Test version V0 of the L1 trigger simulation, Aug 18, 2017 (asomov)

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include <JANA/JEvent.h>
#include <DANA/DEvent.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/DL1Info.h>
#include <DANA/DStatusBits.h>

#include <HDDM/DEventSourceHDDM.h>



#include "DL1MCTrigger_factory.h"

#if HAVE_RCDB
#include "RCDB/Connection.h"
#include "RCDB/ConfigParser.h"
#endif

static bool print_data_message = true;

bool DL1MCTrigger_factory::RCDB_LOADED = false;
std::mutex DL1MCTrigger_factory::params_mutex;
std::mutex DL1MCTrigger_factory::rcdb_mutex;

vector<DL1MCTrigger_factory::trigger_conf> DL1MCTrigger_factory::triggers_enabled;

vector<DL1MCTrigger_factory::fcal_mod> DL1MCTrigger_factory::fcal_trig_mask;
vector<DL1MCTrigger_factory::ecal_mod> DL1MCTrigger_factory::ecal_trig_mask;
vector<DL1MCTrigger_factory::bcal_mod> DL1MCTrigger_factory::bcal_trig_mask;

int    DL1MCTrigger_factory::FCAL_CELL_THR = 165;
int    DL1MCTrigger_factory::FCAL_NSA = 10;
int    DL1MCTrigger_factory::FCAL_NSB = 3;
int    DL1MCTrigger_factory::FCAL_WINDOW = 10;

int    DL1MCTrigger_factory::ECAL_CELL_THR = 135;
int    DL1MCTrigger_factory::ECAL_NSA = 15;
int    DL1MCTrigger_factory::ECAL_NSB = 3;
int    DL1MCTrigger_factory::ECAL_WINDOW = 15;

int    DL1MCTrigger_factory::BCAL_CELL_THR = 120;
int    DL1MCTrigger_factory::BCAL_NSA = 19;
int    DL1MCTrigger_factory::BCAL_NSB = 3;
int    DL1MCTrigger_factory::BCAL_WINDOW = 20;

//------------------
// Init
//------------------
void DL1MCTrigger_factory::Init()
{
  
  std::lock_guard<std::mutex> lock(params_mutex);
  
  debug = 0;
  
  use_rcdb = 1;
  
  BYPASS = 0; // default is to use trigger emulation
  
  // Default parameters for the main production trigger are taken from the 
  // spring run of 2017: 25 F + B > 45000
  
  FCAL_ADC_PER_MEV =  3.73;
  FCAL_EN_SC       =  35;

  ECAL_ADC_PER_MEV =  1.836;  // CORRECT
  ECAL_EN_SC       =  1;      // CORRECT
  
  
  //  BCAL_ADC_PER_MEV =  34.48276; // Not corrected energy 
  //  BCAL_ADC_PER_MEV =  22.7273;
  BCAL_ADC_PER_MEV =  27.98769;
  BCAL_EN_SC       =  1;

  // Correct calorimeters hit energy (visible energy) according to the readout window size, NSA + NSB 
  FCAL_EN_COR   =  1.4373413;
  BCAL_EN_COR   =  1.2364608;
  //  BCAL_EN_COR   =  1.;
  
  FCAL_BCAL_EN     =  45000; 

  ST_ADC_PER_MEV   =  1.;
  ST_CELL_THR      =  40;
  ST_NSA           =  10;
  ST_NSB           =  3;
  ST_WINDOW        =  10;
  ST_NHIT          =  1;

  ECAL_TRIG_GAIN  =  1;
  FCAL_TRIG_GAIN  =  0.4957;

  ECAL_OFFSET       =  0;
  FCAL_ECAL_OFFSET  =  0;
  BCAL_ECAL_OFFSET  =  2;
			  
  BCAL_OFFSET  =  2;
  
  SC_OFFSET    =  6;

  SIMU_BASELINE  = 0;
  SIMU_GAIN = 0;
  SIMU_BAD_BLOCK = 0;  
  
  VERBOSE = 0;

  simu_baseline_fcal  =  1;
  simu_baseline_ecal  =  1;
  simu_baseline_bcal  =  1;

  simu_gain_fcal  =  1;
  simu_gain_ecal  =  1;
  simu_gain_bcal  =  1;

  // BAD blocks should be excluded by the mcsmear and Hit factories. Use only for testing here
  simu_bad_block_fcal  =  1;
  simu_bad_block_ecal  =  1;
  simu_bad_block_bcal  =  1;
  
  ecal_installed  =  0;
  
  
  auto app = GetApplication();

  //  cout << " ---------------------  PARAMS_LOADED ----------------------- " << PARAMS_LOADED << endl;


  app->SetDefaultParameter("TRIG:BYPASS", BYPASS,
			   "Bypass trigger by hard coding physics bit");
  
  app->SetDefaultParameter("TRIG:VERBOSE", VERBOSE,
			   "Enable more verbose output");
  
  // Allows to switch off gain and baseline fluctuations
  app->SetDefaultParameter("TRIG:SIMU_BASELINE", SIMU_BASELINE,
			   "Enable simulation of pedestal variations");
  
  app->SetDefaultParameter("TRIG:SIMU_GAIN", SIMU_GAIN,
			   "Enable simulation of gain variations");

  app->SetDefaultParameter("TRIG:SIMU_BAD_BLOCK", SIMU_BAD_BLOCK,
			   "Enable simulation of bad modules variations");
  
  app->SetDefaultParameter("TRIG:USE_RCDB", use_rcdb,
			   "Read constants from RCDB");
      
  app->SetDefaultParameter("TRIG:FCAL_CELL_THR", FCAL_CELL_THR,
			   "FCAL energy threshold per cell");
  app->SetDefaultParameter("TRIG:FCAL_NSA", FCAL_NSA,
			   "FCAL NSA");
  app->SetDefaultParameter("TRIG:FCAL_NSB", FCAL_NSB,
			   "FCAL NSB");
  app->SetDefaultParameter("TRIG:FCAL_WINDOW", FCAL_WINDOW,
			   "FCAL GTP integration window");
  
  app->SetDefaultParameter("TRIG:ECAL_CELL_THR", ECAL_CELL_THR,
			   "ECAL energy threshold per cell");
  app->SetDefaultParameter("TRIG:ECAL_NSA", ECAL_NSA,
			   "ECAL NSA");
  app->SetDefaultParameter("TRIG:ECAL_NSB", ECAL_NSB,
			   "ECAL NSB");
  app->SetDefaultParameter("TRIG:ECAL_WINDOW", ECAL_WINDOW,
			   "ECAL GTP integration window"); 
  
  app->SetDefaultParameter("TRIG:BCAL_CELL_THR", BCAL_CELL_THR,
			   "BCAL energy threshold per cell");
  app->SetDefaultParameter("TRIG:BCAL_NSA", BCAL_NSA,
					  "BCAL NSA");
  app->SetDefaultParameter("TRIG:BCAL_NSB", BCAL_NSB,
					  "BCAL NSB");
  app->SetDefaultParameter("TRIG:BCAL_WINDOW", BCAL_WINDOW,
			   "BCAL GTP integration window");
  
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
  
  app->SetDefaultParameter("TRIG:FCAL_ADC_PER_MEV", FCAL_ADC_PER_MEV,
			   "FCAL energy calibration for the Trigger");	  
  app->SetDefaultParameter("TRIG:ECAL_ADC_PER_MEV", ECAL_ADC_PER_MEV,
			   "ECAL energy calibration for the Trigger");	  
  app->SetDefaultParameter("TRIG:BCAL_ADC_PER_MEV", BCAL_ADC_PER_MEV,
			   "BCAL energy calibration for the Trigger");
  app->SetDefaultParameter("TRIG:ST_ADC_PER_MEV", ST_ADC_PER_MEV,
			   "ST energy calibration for the Trigger");
  
  app->SetDefaultParameter("TRIG:FCAL_EN_SC", FCAL_EN_SC,
			   "FCAL energy threshold");
  app->SetDefaultParameter("TRIG:ECAL_EN_SC", ECAL_EN_SC,
			   "ECAL energy threshold");	  
  app->SetDefaultParameter("TRIG:BCAL_EN_SC", BCAL_EN_SC,
			   "BCAL energy threshold");
  
  app->SetDefaultParameter("TRIG:FCAL_BCAL_EN", FCAL_BCAL_EN,
			   "Energy threshold for the FCAL & BCAL trigger");
  
  app->SetDefaultParameter("TRIG:BCAL_OFFSET", BCAL_OFFSET,
			   "Timing offset between BCAL and FCAL energies at GTP (sampels)");
  
  app->SetDefaultParameter("TRIG:ECAL_OFFSET", ECAL_OFFSET,
			   "Timing offset of the ECAL energies at GTP (sampels)");	   
  app->SetDefaultParameter("TRIG:ECAL_OFFSET", FCAL_ECAL_OFFSET,
			   "Timing offset of the FCAL energies at GTP (sampels)");
  app->SetDefaultParameter("TRIG:ECAL_OFFSET", BCAL_ECAL_OFFSET,
			   "Timing offset of the BCAL energies at GTP (sampels)");	  
  app->SetDefaultParameter("TRIG:SC_OFFSET", SC_OFFSET,
			   "Timing offset between SC and FCAL and BCAL energies at GTP (sampels)");	    


  pedestal_sigma = 1.2;

  time_shift = 100;

  time_min  =  0;
  time_max  =  (sample - 1)*max_adc_bins;
  
  vector< vector<double > > fcal_gains_temp(DFCALGeometry::kBlocksTall, 
					    vector<double>(DFCALGeometry::kBlocksWide));
  vector< vector<double > > fcal_pedestals_temp(DFCALGeometry::kBlocksTall, 
						vector<double>(DFCALGeometry::kBlocksWide));
  vector< vector<double > > fcal_bad_blocks_temp(DFCALGeometry::kBlocksTall, 
					    vector<double>(DFCALGeometry::kBlocksWide));
  
  vector< vector<double > > ecal_gains_temp(DECALGeometry::kECALBlocksTall, 
					    vector<double>(DECALGeometry::kECALBlocksWide));
  vector< vector<double > > ecal_pedestals_temp(DECALGeometry::kECALBlocksTall, 
						vector<double>(DECALGeometry::kECALBlocksWide));
  vector< vector<double > > ecal_bad_blocks_temp(DECALGeometry::kECALBlocksTall, 
						vector<double>(DECALGeometry::kECALBlocksWide));
  
  
  
	
  fcal_gains       =  fcal_gains_temp;  
  fcal_pedestals   =  fcal_pedestals_temp;
  fcal_bad_blocks  =  fcal_bad_blocks_temp;
  
  ecal_gains       =  ecal_gains_temp;  
  ecal_pedestals   =  ecal_pedestals_temp;
  ecal_bad_blocks  =  ecal_bad_blocks_temp;
  
  return;
}


//------------------
// BeginRun
//------------------
void DL1MCTrigger_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  if (BYPASS) return;

  auto runnumber = event->GetRunNumber();

  int status   = 0;

  if(!RCDB_LOADED) {
    fcal_trig_mask.clear();
    bcal_trig_mask.clear();

    triggers_enabled.clear();
  }

  bcal_gains.clear();
  bcal_pedestals.clear();
  bcal_bad_blocks.clear();
  
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
    if(print_messages) cout << " ---------DL1MCTrigger (Brun): JANA_CALIB_CONTEXT =" << JANA_CALIB_CONTEXT << endl;
    if ( (JANA_CALIB_CONTEXT.find("mc_generic") != string::npos)
	 || (JANA_CALIB_CONTEXT.find("mc_cpp") != string::npos) ){
      if(print_messages) cout << " ---------DL1MCTrigger (Brun): JANA_CALIB_CONTEXT found mc_generic or mc_cpp" << endl;
      use_rcdb = 0;
      // Don't simulate baseline fluctuations for mc_generic
      simu_baseline_fcal = 0;
      simu_baseline_ecal = 0;
      simu_baseline_bcal = 0;
      // Don't simulate gain fluctuations for mc_generic      
      simu_gain_fcal = 0;
      simu_gain_ecal = 0;
      simu_gain_bcal = 0;
    }
  }
  else {
    if(print_messages) cout << " ---------**** DL1MCTrigger (Brun): JANA_CALIB_CONTEXT = NULL" << endl;
  }
  
  //  runnumber = 30942;
  
  if(use_rcdb == 1){
    status = Read_RCDB(event, runnumber, print_messages);
  }
  


  if(print_messages) jout << "In DL1MCTrigger_factory, loading constants..." << jendl;
  
  auto calibration = DEvent::GetJCalibration(event);
  
  if (calibration->Get("/ECAL/install_status", ecal_installed)){
    jout << "DL1MCTrigger_factory: Error loading /ECAL/install_status !" << jendl;
  }
  
  
  
  if( (use_rcdb == 0) || (status > 0) || (triggers_enabled.size() == 0)){
    
    // Simulate FCAL & BCAL main production trigger only

    if(!RCDB_LOADED) {
      trigger_conf trig_tmp;
      trig_tmp.bit = 0;
      trig_tmp.gtp.fcal      =  FCAL_EN_SC;
      trig_tmp.gtp.bcal      =  BCAL_EN_SC;
      trig_tmp.gtp.en_thr    =  FCAL_BCAL_EN;
      trig_tmp.gtp.fcal_min  =  200;
      trig_tmp.gtp.fcal_max  =  65535;
      trig_tmp.gtp.bcal_min  =  0;
      trig_tmp.gtp.bcal_max  =  65535;
      trig_tmp.type          =  3;
      triggers_enabled.push_back(trig_tmp);
      
      if(ecal_installed == 1) Load_ECAL_mask_default();
    
      if(print_messages){ 
	cout << " Do not use RCDB for the trigger simulation. Default (spring 2025) trigger settings with the ECAL are used." << endl;
	cout << " Use -PTRIG:FCAL_EN_SC and other trigger parameters to modify the settings. The default ECAL masks will always be loaded. " << endl;
      }
      RCDB_LOADED = true;
    }
  }

  
  // extract the FCAL Geometry
  vector<const DFCALGeometry*> fcalGeomVect;
  event->Get( fcalGeomVect );
  if (fcalGeomVect.size() < 1)
    return; // OBJECT_NOT_AVAILABLE;
  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  
  // extract the ECAL Geometry
  vector<const DECALGeometry*> ecalGeomVect;
  event->Get( ecalGeomVect );
  if (ecalGeomVect.size() < 1)
    return; // OBJECT_NOT_AVAILABLE;
  const DECALGeometry& ecalGeom = *(ecalGeomVect[0]);

 
  
  // ---------------------------------------------------------------
  // ---------------- LOAD ECAL PARAMETERS FROM CCDB ---------------
  // ---------------------------------------------------------------
  
  vector< double > ecal_gains_ch;
  vector< double > ecal_pedestals_ch;
  vector< double > ecal_bad_blocks_ch;
  
  if (calibration->Get("/ECAL/gains", ecal_gains_ch)){
    jout << "DL1MCTrigger_factory: Error loading /ECAL/gains !" << jendl;
    // Load default values of gains if CCDB table is not found
    for(int ii = 0; ii < DECALGeometry::kECALBlocksTall; ii++){
      for(int jj = 0; jj < DECALGeometry::kECALBlocksWide; jj++){
	ecal_gains[ii][jj] = 1.;	
      }
    }
  } else {
    LoadECALConst(ecal_gains, ecal_gains_ch, ecalGeom);
    
    if(VERBOSE == 1){
      for(int ch = 0; ch < (int)ecal_gains_ch.size(); ch++){	
	int row = ecalGeom.row(ch);
	int col = ecalGeom.column(ch);	
	// Sanity check for regular ECAL (row,col) ranges
	if(ecalGeom.isBlockActive(row,col) && row<DECALGeometry::kECALBlocksTall
	   && col<DECALGeometry::kECALBlocksWide){
	  DVector2 pos = ecalGeom.positionOnFace(row,col);
	  cout << "  ECAL GAINS:  COL =  "  <<  col << "  ROW = " << row <<  "  X = " << pos.X() <<  "  Y = " <<  pos.Y() <<  "  GAIN = " << ecal_gains[row][col] << endl;
	} 
      }
      cout << endl;      
    }        
  }
  
  if (calibration->Get("/ECAL/pedestals", ecal_pedestals_ch)){
    jout << "DL1MCTrigger_factory: Error loading /ECAL/pedestals !" << jendl;
    // Load default values of pedestals if CCDB table is not found
    for(int ii = 0; ii < DECALGeometry::kECALBlocksTall; ii++){
      for(int jj = 0; jj < DECALGeometry::kECALBlocksWide; jj++){
	ecal_pedestals[ii][jj] = 100.;	
      }
    }
  } else {
    LoadECALConst(ecal_pedestals, ecal_pedestals_ch, ecalGeom);
    
    if(VERBOSE == 1){
      for(int ch = 0; ch < (int)ecal_pedestals_ch.size(); ch++){
	int row = ecalGeom.row(ch);
	int col = ecalGeom.column(ch);
	// Sanity check for regular ECAL (row,col) ranges
	if(ecalGeom.isBlockActive(row,col)&&row<DECALGeometry::kECALBlocksTall
	   && col<DECALGeometry::kECALBlocksWide){
	  cout << "ECAL Pedestals: COL =  " << col << "  ROW =  " << row << "  Pedestal " << ecal_pedestals[row][col] << endl;
	}
      }	
    }
    cout << endl;
  }

  
  if (calibration->Get("/ECAL/bad_block", ecal_bad_blocks_ch)){
    jout << "DL1MCTrigger_factory: Error loading /ECAL/bad_block !" << jendl;
    // Load default values of bad blocks if CCDB table is not found
    for(int ii = 0; ii < DECALGeometry::kECALBlocksTall; ii++){
      for(int jj = 0; jj < DECALGeometry::kECALBlocksWide; jj++){
	ecal_bad_blocks[ii][jj] = 0.;	
      }
    }
  } else {
    LoadECALConst(ecal_bad_blocks, ecal_bad_blocks_ch, ecalGeom);
    
    if(VERBOSE == 1){
      for(int ch = 0; ch < (int)ecal_bad_blocks_ch.size(); ch++){
	int row = ecalGeom.row(ch);
	int col = ecalGeom.column(ch);
	// Sanity check for regular ECAL (row,col) ranges
	if(ecalGeom.isBlockActive(row,col)&&row<DECALGeometry::kECALBlocksTall
	   && col<DECALGeometry::kECALBlocksWide){
	  cout << "ECAL Bad block:  COL =  " << col << "  ROW =  " << row << "  Bad block =  " << ecal_bad_blocks[row][col] << endl;
	}
      }	
    }    
  }


  // ---------------------------------------------------------------
  // ---------------- LOAD ECAL PARAMETERS FROM CCDB ---------------
  // ---------------------------------------------------------------

  
  vector< double > fcal_gains_ch;
  vector< double > fcal_pedestals_ch;
  vector< double > fcal_bad_blocks_ch;
  
  if (calibration->Get("/FCAL/gains", fcal_gains_ch)){
    jout << "DL1MCTrigger_factory: Error loading /FCAL/gains !" << jendl;
    // Load default values of gains if CCDB table is not found
    for(int ii = 0; ii < DFCALGeometry::kBlocksTall; ii++){
      for(int jj = 0; jj < DFCALGeometry::kBlocksWide; jj++){
	fcal_gains[ii][jj] = 1.;	
      }
    }
  } else {
    LoadFCALConst(fcal_gains, fcal_gains_ch, fcalGeom);

    if(VERBOSE == 1){
      for(int ch = 0; ch < (int)fcal_gains_ch.size(); ch++){
	int row = fcalGeom.row(ch);
	int col = fcalGeom.column(ch);	
	// Sanity check for regular FCAL (row,col) ranges 
	if(fcalGeom.isBlockActive(row,col)&&row<DFCALGeometry::kBlocksTall
	   && col<DFCALGeometry::kBlocksWide){
	  DVector2 pos = fcalGeom.positionOnFace(row,col);
	  cout << "  FCAL GAINS:  COL =  "  <<  col << "  ROW = " << row <<  "  X = " << pos.X() <<  "  Y = " <<  pos.Y() <<  "  GAIN = " << fcal_gains[row][col] << endl;	  
	}	
      }
      cout << endl;
    }    
  }

  if (calibration->Get("/FCAL/pedestals", fcal_pedestals_ch)){
    jout << "DL1MCTrigger_factory: Error loading /FCAL/pedestals !" << jendl;
    // Load default values of pedestals if CCDB table is not found
    for(int ii = 0; ii < DFCALGeometry::kBlocksTall; ii++){
      for(int jj = 0; jj < DFCALGeometry::kBlocksWide; jj++){
	fcal_pedestals[ii][jj] = 100.;	
      }
    }
  } else {
    LoadFCALConst(fcal_pedestals, fcal_pedestals_ch, fcalGeom);

    if(VERBOSE == 1){
      for(int ch = 0; ch < (int)fcal_pedestals_ch.size(); ch++){
	int row = fcalGeom.row(ch);
	int col = fcalGeom.column(ch);
	// Sanity check for regular FCAL (row,col) ranges 
	if(fcalGeom.isBlockActive(row,col)&&row<DFCALGeometry::kBlocksTall
	   && col<DFCALGeometry::kBlocksWide){
	  cout << "FCAL Pedestals: COL =  " << col << "  ROW =  " << row << "  Pedestal =  " << fcal_pedestals[row][col] << endl;
	}
      }
      cout << endl;
    }    
  }

  if (calibration->Get("/FCAL/block_quality", fcal_bad_blocks_ch)){
    jout << "DL1MCTrigger_factory: Error loading /FCAL/block_quality !" << jendl;
    // Load default values of bad blocks if CCDB table is not found
    for(int ii = 0; ii < DFCALGeometry::kBlocksTall; ii++){
      for(int jj = 0; jj < DFCALGeometry::kBlocksWide; jj++){
	fcal_bad_blocks[ii][jj] = 0.;	
      }
    }
  } else {
    LoadFCALConst(fcal_bad_blocks, fcal_bad_blocks_ch, fcalGeom);
    
    if(VERBOSE == 1){
      for(int ch = 0; ch < (int)fcal_bad_blocks_ch.size(); ch++){
	int row = fcalGeom.row(ch);
	int col = fcalGeom.column(ch);
	// Sanity check for regular FCAL (row,col) ranges 
	if(fcalGeom.isBlockActive(row,col)&&row<DFCALGeometry::kBlocksTall
	   && col<DFCALGeometry::kBlocksWide){
	  cout << "FCAL Bad block:  " << col << "  " << row << "  " << fcal_bad_blocks[row][col] << endl;
	}
      }
      cout << endl;
    }    
  }

  
  // ---------------------------------------------------------------
  // ---------------- LOAD BCAL PARAMETERS FROM CCDB ---------------
  // --------------------------------------------------------------- 

  
  vector<double> bcal_gains_ch;
  vector<double> bcal_pedestals_ch;
  vector<double> bcal_bad_blocks_ch;
  
  if (calibration->Get("/BCAL/ADC_gains_trig", bcal_gains_ch)){
    jout << "Error loading /BCAL/ADC_gains_trig !" << jendl;
    
    char str[256];
    int channel = 0;
    
    for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
      for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
	for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {
	  if ((channel > BCAL_MAX_CHANNELS) || (channel+1 > BCAL_MAX_CHANNELS)) {  // sanity check
	    sprintf(str, "Too many channels for BCAL table!"
		    " channel=%d (should be %d)", 
		    channel, BCAL_MAX_CHANNELS);
	    cerr << str << endl;
	    throw JException(str);
	  }
	  
	  bcal_gains.push_back( cell_calib_t(BCAL_ADC_PER_MEV,BCAL_ADC_PER_MEV) );	  
	  channel += 2;
	}
      }
    }    
  }
  else {
    LoadBCALConst(bcal_gains, bcal_gains_ch, 1);
    
    if(VERBOSE == 1){
      for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
	for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
	  for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {

	    const int cell =  BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);	 
	    cout << "BCAL gains:  Module = " <<  module <<  " Layer = " << layer << "  Sector = " << sector <<
	      " Gain up:  " << bcal_gains.at(cell).first << " Gain down " << bcal_gains.at(cell).second << endl;	    
	  }
	}
      }
      cout << endl;
    }
  }

  // Determine the average value of the BCAL gains
  BCAL_average_gain();

  if(VERBOSE == 1)
    cout << "  --------  BCAL AVERAGE GAIN -----------   =  " <<  1./BCAL_ADC_PER_MEV << "   BCAL_ADC_PER_MEV = " << BCAL_ADC_PER_MEV  << endl;
  
  
  if (calibration->Get("/BCAL/ADC_pedestals", bcal_pedestals_ch)){
    jout << "Error loading /BCAL/ADC_pedestals !" << jendl;

    char str[256];
    int channel = 0;
    
    for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
      for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
	for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {
	  if ((channel > BCAL_MAX_CHANNELS) || (channel+1 > BCAL_MAX_CHANNELS)) {  // sanity check
	    sprintf(str, "Too many channels for BCAL table!"
		    " channel=%d (should be %d)", 
		    channel, BCAL_MAX_CHANNELS);
	    cerr << str << endl;
	    throw JException(str);
	  }	  
	  bcal_pedestals.push_back( cell_calib_t(100.,100.) );	  
	  channel += 2;
	}
      }
    }    
  } else {
    LoadBCALConst(bcal_pedestals, bcal_pedestals_ch, 0);

    if(VERBOSE == 1){
      for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
	for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
	  for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {
	    
	    const int cell =  BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);	 
	    cout << "BCAL pedestals:  Module = " <<  module <<  " Layer = " << layer << "  Sector = " << sector <<
	      " Pedestal up:  " << bcal_pedestals.at(cell).first << " Pedestal down " << bcal_pedestals.at(cell).second << endl;	    
	  }
	}
      }
      cout << endl;
    }
  }

  
  if (calibration->Get("/BCAL/bad_channels", bcal_bad_blocks_ch)){
    jout << "Error loading /BCAL/bad_channels !" << jendl;

    char str[256];
    int channel = 0;
    
    for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
      for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
	for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {

	  if ((channel > BCAL_MAX_CHANNELS) || (channel+1 > BCAL_MAX_CHANNELS)) {  // sanity check
	    sprintf(str, "Too many channels for BCAL table!"
		    " channel=%d (should be %d)", 
                            channel, BCAL_MAX_CHANNELS);
	    cerr << str << endl;
	    throw JException(str);
	  }
	  
	  bcal_bad_blocks.push_back( cell_calib_t(0.,0.) );	  
	  channel += 2;
	}
      }
    }
    
  }
  else {
    LoadBCALConst(bcal_bad_blocks, bcal_bad_blocks_ch, 0);

    if(VERBOSE == 1){
      for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
	for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
	  for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {
	    
	    const int cell =  BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);	 
	    cout << "BCAL bad blocks:  Module = " <<  module <<  " Layer = " << layer << "  Sector = " << sector <<
	      " Bad block up:  " << bcal_bad_blocks.at(cell).first << " Bad block down " << bcal_bad_blocks.at(cell).second << endl;	    
	  }
	}
      }
      cout << endl;
    }
  }
  

  if(!SIMU_BASELINE){
    simu_baseline_fcal = 0;
    simu_baseline_ecal = 0;
    simu_baseline_bcal = 0;
  }

  if(!SIMU_GAIN){
    simu_gain_fcal = 0;
    simu_gain_ecal = 0;
    simu_gain_bcal = 0;
  }

  if(!SIMU_BAD_BLOCK){
    simu_bad_block_fcal = 0;
    simu_bad_block_ecal = 0;
    simu_bad_block_bcal = 0;
  }

           
  if(print_messages) PrintTriggers();
  
}

//------------------
// Process
//------------------
void DL1MCTrigger_factory::Process(const std::shared_ptr<const JEvent>& event){

	if(BYPASS) {
                DL1MCTrigger *trigger = new DL1MCTrigger;
                trigger->trig_mask = 1;
                Insert(trigger);
		return;
        }
	
        int l1_found = 1;  

	int status = 0;

	fcal_signal_hits.clear();
	ecal_signal_hits.clear();
	bcal_signal_hits.clear();

	fcal_merged_hits.clear();
	ecal_merged_hits.clear();
	bcal_merged_hits.clear();
	
	memset(fcal_ssp,0,sizeof(fcal_ssp));
	memset(fcal_gtp,0,sizeof(fcal_gtp));

	memset(ecal_ssp,0,sizeof(ecal_ssp));	
	memset(ecal_gtp,0,sizeof(ecal_gtp));
	
	memset(bcal_ssp,0,sizeof(bcal_ssp));
	memset(bcal_gtp,0,sizeof(bcal_gtp));


	//	triggers_enabled[0].gtp.fcal   = 20;
	//	triggers_enabled[0].gtp.bcal   = 1;
	//	triggers_enabled[0].gtp.en_thr = 90000;

	
        vector<const DFCALHit*>  fcal_hits;
	vector<const DECALHit*>  ecal_hits;	
	vector<const DBCALHit*>  bcal_hits;
	vector<const DSCHit*>    sc_hits;

	event->Get(fcal_hits);
	event->Get(ecal_hits);		
	event->Get(bcal_hits);
	event->Get(sc_hits);

	
	if(VERBOSE == 3){
	  cout << endl;
	  cout << " Number of ECAL hits = " << ecal_hits.size() << endl;
	  cout << endl;	  
	  cout << " ECAL cell threshold = " << ECAL_CELL_THR << endl;
	  cout << " ECAL NSA            = " << ECAL_NSA << endl;
	  cout << " ECAL NSB            = " << ECAL_NSB << endl;
	  cout << " ECAL_ADC_PER_MEV    = " << ECAL_ADC_PER_MEV << endl;
	  cout << " ECAL_EN_SC          = " << ECAL_EN_SC << endl;
	}
	

	
	DRandom2 gDRandom(0); // declared extern in DRandom2.h


	// This is temporary, to allow this simulation to be run on data
	// to help out with trigger efficiency studies - sdobbs (Aug. 26, 2020)
	if( event->GetSingleStrict<DStatusBits>()->GetStatusBit(kSTATUS_EVIO) ){
		if(print_data_message) {
			jout << "WARNING: Running L1 trigger simulation on EVIO data" << endl; 
			print_data_message = false;
		}
	
		// for data, don't add in baseline shifts, since they already exist
		simu_baseline_fcal = 0;
		simu_baseline_bcal = 0;
	} else {
		// Initialize random number generator
		// Read seeds from hddm file
		// Generate seeds according to the event number if they are not stored in hddm
		// The proceedure is consistent with the mcsmear

		UInt_t seed1 = 0;
		UInt_t seed2 = 0;
		UInt_t seed3 = 0;
	
		GetSeeds(event, event->GetEventNumber(), seed1, seed2, seed3);
	
		gDRandom.SetSeeds(seed1, seed2, seed3);
	}
	
	//	cout << endl;
	//	cout << " Event = " << eventnumber << endl;
	//	cout << " Seed 1: " << seed1 << endl;
	//	cout << " Seed 2: " << seed2 << endl;
	//	cout << " Seed 3: " << seed3 << endl;
	//	cout << endl;
	       

	DL1MCTrigger *trigger = new DL1MCTrigger;




	// ------------------------------------------
	// ----------  ECAL energy sum  -------------
	// ------------------------------------------

	//  ECAL energy sum	
	double ecal_hit_en = 0;
	
	for (unsigned int ii = 0; ii < ecal_hits.size(); ii++){

	  int row  = ecal_hits[ii]->row;
	  int col  = ecal_hits[ii]->column;

	  
	  // Shift time to simulate pile up hits
	  double time = ecal_hits[ii]->t + time_shift;
	  if((time < time_min) || (time > time_max)){
	    continue;
	  }
	  
	  // Check channels masked for trigger
	  int ch_masked = 0;

	  //	  if( (col >= 14 && col <= 25) && (row >= 14 && row <= 25)) ch_masked = 1;	  
	  for(unsigned int jj = 0; jj < ecal_trig_mask.size(); jj++){	   

	    if( (row == ecal_trig_mask[jj].row) && (col == ecal_trig_mask[jj].col)){	      
	      ch_masked = 1;
	      if(VERBOSE == 3)
		cout << " ECAL mask found " << row << "  Col = " << col << endl;
	      break;
	    } 
	  }
	  
	  if(ch_masked == 0){
	    
	    ecal_hit_en += ecal_hits[ii]->E;
	    
	    ecal_signal ecal_tmp;
	    ecal_tmp.merged = 0;
	    
	    ecal_tmp.row     = row;
	    ecal_tmp.column  = col;

	    ecal_tmp.energy  = ecal_hits[ii]->E;
	    ecal_tmp.time    = time;
	    memset(ecal_tmp.adc_amp,0,sizeof(ecal_tmp.adc_amp));
	    memset(ecal_tmp.adc_en, 0,sizeof(ecal_tmp.adc_en));

	    double ecal_adc_en  = ecal_tmp.energy*ECAL_ADC_PER_MEV*1000;

	    // Account for gain fluctuations
	    if(simu_gain_ecal && row<DECALGeometry::kECALBlocksTall
	       && col<DECALGeometry::kECALBlocksWide){
	      
	      double gain  =  ecal_gains[row][col];

	      if(gain > 0.)
		ecal_adc_en /= gain;
	      
	    }
	    
	    status = SignalPulse(ecal_adc_en, ecal_tmp.time, ecal_tmp.adc_en, 3);
	    status = 0;

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
	    if((ecal_signal_hits[ii].row     ==  ecal_signal_hits[jj].row) &&
	       (ecal_signal_hits[ii].column  ==  ecal_signal_hits[jj].column)){

	      ecal_signal_hits[jj].merged = 1;
	      
	      for(int kk = 0; kk < sample; kk++)	      
		ecal_tmp.adc_en[kk] += ecal_signal_hits[jj].adc_en[kk];	
	    }
	  }
	  
	  ecal_merged_hits.push_back(ecal_tmp);
	}	
	
	// Add baseline fluctuations for channels with hits
	if(simu_baseline_ecal){
	  for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++){
	    int row      =  ecal_merged_hits[ii].row;
	    int column   =  ecal_merged_hits[ii].column;
	    double pedestal = 100.0;
	    if (row<DECALGeometry::kECALBlocksTall
		&& column<DECALGeometry::kECALBlocksWide){
	      pedestal = ecal_pedestals[row][column];
	    }
	    AddBaseline(ecal_merged_hits[ii].adc_en, pedestal, gDRandom);       
	  }
	}

	
	// ECAL Digitize		
	for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++){
	  Digitize(ecal_merged_hits[ii].adc_en,ecal_merged_hits[ii].adc_amp);
	}
	
	int ecal_hit_adc_en = 0;
	
	for(unsigned int ii = 0; ii < ecal_merged_hits.size(); ii++)
	  for(int jj = 0; jj < sample; jj++)
	    if( (ecal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE) > 0.)
	      ecal_hit_adc_en += (ecal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE);
	
	
	status += FADC_SSP(ecal_merged_hits, 3);
	
      	status += GTP(3);       

	

	
	//  FCAL energy sum	
	double fcal_hit_en = 0;
	
	for (unsigned int ii = 0; ii < fcal_hits.size(); ii++){
	  int row  = fcal_hits[ii]->row;
	  int col  = fcal_hits[ii]->column;

	  // Shift time to simulate pile up hits
	  double time = fcal_hits[ii]->t + time_shift;
	  if((time < time_min) || (time > time_max)){
	    continue;
	  }

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
	    fcal_tmp.merged = 0;
	    
	    fcal_tmp.row     = row;
	    fcal_tmp.column  = col;

	    fcal_tmp.energy  = fcal_hits[ii]->E;
	    fcal_tmp.energy *= FCAL_EN_COR;

	    
	    fcal_tmp.time    = time;
	    memset(fcal_tmp.adc_amp,0,sizeof(fcal_tmp.adc_amp));
	    memset(fcal_tmp.adc_en, 0,sizeof(fcal_tmp.adc_en));

	    
	    double fcal_adc_en  = fcal_tmp.energy*FCAL_ADC_PER_MEV*1000;
	    
	    // Account for gain fluctuations
	    if(simu_gain_fcal && row<DFCALGeometry::kBlocksTall
	       && col<DFCALGeometry::kBlocksWide){
	      
	      double gain  =  fcal_gains[row][col];	  

	      if(gain > 0.)
		fcal_adc_en /= gain;
	      
	    }
	    
	    status = SignalPulse(fcal_adc_en, fcal_tmp.time, fcal_tmp.adc_en, 1);
	    status = 0;

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
	
	// Add baseline fluctuations for channels with hits
	if(simu_baseline_fcal){
	  for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++){
	    int row     = fcal_merged_hits[ii].row;
	    int column  = fcal_merged_hits[ii].column;
	    double pedestal = 100.0;
	    if (row<DFCALGeometry::kBlocksTall
		&& column<DFCALGeometry::kBlocksWide){
	      pedestal=fcal_pedestals[row][column];
	    }
	    AddBaseline(fcal_merged_hits[ii].adc_en, pedestal, gDRandom);       
	  }
	}
	

	// Digitize		
	for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++){
	  Digitize(fcal_merged_hits[ii].adc_en,fcal_merged_hits[ii].adc_amp);
	  //	  cout << " Digitize " << fcal_merged_hits[ii].adc_en[sample - 3] 
	  //   << "  " <<  fcal_merged_hits[ii].adc_amp[sample - 3] << endl;
	}
	
	
	int fcal_hit_adc_en = 0;

	for(unsigned int ii = 0; ii < fcal_merged_hits.size(); ii++)
	  for(int jj = 0; jj < sample; jj++)
	    if( (fcal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE) > 0.)
	      fcal_hit_adc_en += (fcal_merged_hits[ii].adc_amp[jj] - TRIG_BASELINE);
	
	
	status += FADC_SSP(fcal_merged_hits, 1);

	status += GTP(1);



	// BCAL	energy sum
	double bcal_hit_en = 0;
	
	for (unsigned int ii = 0; ii < bcal_hits.size(); ii++){

	  // Shift time to simulate pile up hits
	  double time = bcal_hits[ii]->t + time_shift;	 
	  if((time < time_min) || (time > time_max)){
	    continue;
	  }
	  
	  int module = bcal_hits[ii]->module;
	  int layer  = bcal_hits[ii]->layer;
	  int sector = bcal_hits[ii]->sector;
	  int end    = bcal_hits[ii]->end;
	  
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
	    bcal_tmp.merged  = 0;
	    bcal_tmp.module  = module;
	    bcal_tmp.layer   = layer;
  	    bcal_tmp.sector  = sector;
	    bcal_tmp.end     = end;	   	    
	    
	    bcal_tmp.energy  = bcal_hits[ii]->E;
	    bcal_tmp.time    = time;
	    memset(bcal_tmp.adc_amp,0,sizeof(bcal_tmp.adc_amp));
	    memset(bcal_tmp.adc_en, 0,sizeof(bcal_tmp.adc_en));

	    bcal_tmp.energy *= BCAL_EN_COR;
	    
	    double bcal_adc_en  = bcal_tmp.energy*BCAL_ADC_PER_MEV*1000;

	    // Account for gain fluctuations
	    
	    if(simu_gain_bcal){
	      double gain = 0;
	      
	      const int cell =  BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);	   	    
	      if (end == DBCALGeometry::kUpstream) {
		// handle the upstream end
		gain = bcal_gains.at(cell).first;
	      } else {
		// handle the downstream end
		gain = bcal_gains.at(cell).second;
	      }

	      
	      //	      cout << " BCAL MODULE = " << module << "  Layer = " << layer << "  Sector = " << sector << "  End = " <<  " CELL = " << cell << "  Gain = " << gain << endl;
	      
	      gain /= BCAL_ADC_PER_MEV;

	      bcal_adc_en *= gain;  // Gains for the BCAL were inverted
	      	      
	    }
	    
	    
	    status = SignalPulse(bcal_adc_en, bcal_tmp.time, bcal_tmp.adc_en, 2);
	    status = 0;
 
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


	// Add baseline fluctuations for channels with hits
	if(simu_baseline_bcal){
	  for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++){

	    // Assume that all BCAL pedestals are 100
	    double pedestal = TRIG_BASELINE;

	    int module  =  bcal_merged_hits[ii].module;
	    int layer   =  bcal_merged_hits[ii].layer;
	    int sector  =  bcal_merged_hits[ii].sector;
	    int end     =  bcal_merged_hits[ii].end;

	    // Use values from the CCDB
	    const int cell =  BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);	   	    
	    if (end == DBCALGeometry::kUpstream) {
	      // handle the upstream end
	      pedestal = bcal_pedestals.at(cell).first;
	    } else {
	      // handle the downstream end
	      pedestal = bcal_pedestals.at(cell).second;
	    }
	    
	    // Some pedestal tables in the CCDB may contain zero values
	    if(pedestal < 10.) pedestal = TRIG_BASELINE;
	    
	    AddBaseline(bcal_merged_hits[ii].adc_en, pedestal, gDRandom);       
	  }
	}


	// Digitize		
	for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++)
	  Digitize(bcal_merged_hits[ii].adc_en,bcal_merged_hits[ii].adc_amp);


	int bcal_hit_adc_en = 0;
	for(unsigned int ii = 0; ii < bcal_merged_hits.size(); ii++)	  
	  for(int jj = 0; jj < sample; jj++)
	    bcal_hit_adc_en += bcal_merged_hits[ii].adc_amp[jj];	  
	
	
	status = FADC_SSP(bcal_merged_hits, 2);

	status = GTP(2);

	// Search for triggers

	if(ecal_installed) l1_found = FindTriggersECAL(trigger);  
	else l1_found = FindTriggers(trigger,sc_hits);


	if(VERBOSE == 5){
	  for(int ii = 0; ii < sample; ii++){
	    cout << " SSP:  ECAL  =  " << ecal_ssp[ii] << " FCAL = " << fcal_ssp[ii] << " BCAL = " << bcal_ssp[ii] <<
	      "  GTP:   ECAL = " << ecal_gtp[ii] << " FCAL = " << fcal_gtp[ii] << " BCAL = " << bcal_gtp[ii] << endl;
	  }	  	  	  
	}
	
	int ecal_ssp_sum1 = 0;
	int fcal_ssp_sum1 = 0;
	int bcal_ssp_sum1 = 0;
	
	for(int ii = 0; ii < sample; ii++){
	  ecal_ssp_sum1 += ecal_ssp[ii];
	  fcal_ssp_sum1 += fcal_ssp[ii];
	  bcal_ssp_sum1 += bcal_ssp[ii];
	}
	
	if(VERBOSE == 2){
	  cout << " MC SSP BEFORE TRIGGER ECAL = " <<  ecal_hit_en << "    " << ecal_ssp_sum1/ECAL_ADC_PER_MEV/1000. << endl;
	  cout << " MC SSP BEFORE TRIGGER FCAL = " <<  fcal_hit_en << "    " << fcal_ssp_sum1/FCAL_ADC_PER_MEV/1000. << endl;
	  cout << " MC SSP BEFORE TRIGGER BCAL = " <<  bcal_hit_en << "    " << bcal_ssp_sum1/BCAL_ADC_PER_MEV/1000. << endl;
	}

	
	
	if(l1_found){
	  
	if(VERBOSE == 2){	  
	  cout << " ======================================== " << endl;
	  cout << " ============ Trigger found ============= " << trigger->trig_mask << endl;
	  cout << " ======================================== " << endl;
	}

	  int ecal_gtp_max = 0;	  
	  int fcal_gtp_max = 0;
	  int bcal_gtp_max = 0;

	    
	  for(unsigned int ii = 0; ii < sample; ii++){
	    if(ecal_gtp[ii] > ecal_gtp_max) ecal_gtp_max = ecal_gtp[ii];
	    if(fcal_gtp[ii] > fcal_gtp_max) fcal_gtp_max = fcal_gtp[ii];
	    if(bcal_gtp[ii] > bcal_gtp_max) bcal_gtp_max = bcal_gtp[ii];
	  }	
	  
	  int trig_sample = trigger->trig_time[0] + 25;
	  ecal_gtp_max = ecal_gtp[trig_sample];
	  fcal_gtp_max = fcal_gtp[trig_sample];
	  bcal_gtp_max = bcal_gtp[trig_sample];
	  
          trigger->ecal_en      =  ecal_hit_en;
          //      trigger->ecal_adc     =  ecal_hit_adc_en;
          //      trigger->ecal_adc_en  =  ecal_hit_adc_en/ECAL_ADC_PER_MEV/1000.;
	  trigger->ecal_adc     =  ecal_ssp_sum1;
	  trigger->ecal_adc_en  =  ecal_ssp_sum1/ECAL_ADC_PER_MEV/1000.;          
          trigger->ecal_gtp     =  ecal_gtp_max;
          trigger->ecal_gtp_en  =  ecal_gtp_max/ECAL_ADC_PER_MEV/1000.;

	  
	  trigger->fcal_en      =  fcal_hit_en;
	  trigger->fcal_adc     =  fcal_ssp_sum1;
	  trigger->fcal_adc_en  =  fcal_ssp_sum1/FCAL_ADC_PER_MEV/1000.;	  
	  //	  trigger->fcal_adc     =  fcal_hit_adc_en;
	  //	  trigger->fcal_adc_en  =  fcal_hit_adc_en/FCAL_ADC_PER_MEV/1000.;
	  trigger->fcal_gtp     =  fcal_gtp_max;
	  trigger->fcal_gtp_en  =  fcal_gtp_max/FCAL_ADC_PER_MEV/1000.;
	  
	  trigger->bcal_en      =  bcal_hit_en;
	  trigger->bcal_adc     =  bcal_ssp_sum1;
	  trigger->bcal_adc_en  =  bcal_ssp_sum1/BCAL_ADC_PER_MEV/1000.;	  
	  //	  trigger->bcal_adc     =  bcal_hit_adc_en;
	  //	  trigger->bcal_adc_en  =  bcal_hit_adc_en/BCAL_ADC_PER_MEV/1000.;
	  trigger->bcal_gtp     =  bcal_gtp_max;
	  trigger->bcal_gtp_en  =  bcal_gtp_max/BCAL_ADC_PER_MEV/1000.;	  	  
	  
	  Insert(trigger);	 
	  
	} else{
	  delete trigger;
	}
	
	return;
}

//------------------
// EndRun
//------------------
void DL1MCTrigger_factory::EndRun()
{
	return;
}

//------------------
// Finish
//------------------
void DL1MCTrigger_factory::Finish()
{
	return;
}

//*********************
// Read RCDB 
//*********************

int  DL1MCTrigger_factory::Read_RCDB(const std::shared_ptr<const JEvent>& event, int32_t runnumber, bool print_messages)
{

#if HAVE_RCDB
  {
  // RCDB queries are heavy, so we load the data once for all threads
  std::lock_guard<std::mutex> lock(rcdb_mutex);
  
  if(RCDB_LOADED) return 0;
  RCDB_LOADED = true;

  vector<const DTranslationTable*> ttab;  
  event->Get(ttab);
  
  vector<string> SectionNames = {"TRIGGER", "GLOBAL", "FCAL", "ECAL", "BCAL", "TOF", "ST", "TAGH",
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

  auto ecal_trig_thr = result.Sections["ECAL"].NameValues["FADC250_TRIG_THR"];
  auto ecal_trig_nsb = result.Sections["ECAL"].NameValues["FADC250_TRIG_NSB"];
  auto ecal_trig_nsa = result.Sections["ECAL"].NameValues["FADC250_TRIG_NSA"];

  if(ecal_trig_thr.size() > 0){
    ECAL_CELL_THR  =  stoi(ecal_trig_thr);
    if(ECAL_CELL_THR < 0) ECAL_CELL_THR = 0;
  }

  if(ecal_trig_nsb.size() > 0)
    ECAL_NSB  =  stoi(ecal_trig_nsb);

  if(ecal_trig_nsa.size() > 0)
    ECAL_NSA   =  stoi(ecal_trig_nsa);


  auto fcal_trig_thr = result.Sections["FCAL"].NameValues["FADC250_TRIG_THR"];
  auto fcal_trig_nsb = result.Sections["FCAL"].NameValues["FADC250_TRIG_NSB"];
  auto fcal_trig_nsa = result.Sections["FCAL"].NameValues["FADC250_TRIG_NSA"];
  
  if(fcal_trig_thr.size() > 0){
    FCAL_CELL_THR  =  stoi(fcal_trig_thr);
    if(FCAL_CELL_THR < 0) FCAL_CELL_THR = 0;
  }

  if(fcal_trig_nsb.size() > 0)
    FCAL_NSB  =  stoi(fcal_trig_nsb);

  if(fcal_trig_nsa.size() > 0)
    FCAL_NSA   =  stoi(fcal_trig_nsa);

  auto bcal_trig_thr = result.Sections["BCAL"].NameValues["FADC250_TRIG_THR"];
  auto bcal_trig_nsb = result.Sections["BCAL"].NameValues["FADC250_TRIG_NSB"];
  auto bcal_trig_nsa = result.Sections["BCAL"].NameValues["FADC250_TRIG_NSA"];
  
  if(bcal_trig_thr.size() > 0){
    BCAL_CELL_THR   =  stoi(bcal_trig_thr);
    if(BCAL_CELL_THR < 0) BCAL_CELL_THR = 0;
  }

  if(bcal_trig_nsb.size() > 0)
    BCAL_NSB  =  stoi(bcal_trig_nsb);

  if(bcal_trig_nsa.size() > 0)
    BCAL_NSA  =  stoi(bcal_trig_nsa);


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

    if(row[0] == "TRIG_ECAL_FCAL_GAIN") {      
      if(row.size() >= 3){
	ECAL_TRIG_GAIN = stof(row[1]);  //   ECAL gain
	FCAL_TRIG_GAIN = stof(row[2]);  //   FCAL gain
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
	  if(row[1] == "ECAL")
	    ECAL_WINDOW = stoi(row[3]);		  
	  if(row[1] == "BCAL_E")
	    BCAL_WINDOW = stoi(row[3]);  
	}
      }            
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

  // Load ECAL Trigger Masks. Temporarily loaded from a function. Will be updated when masks are uploaded to the RCDB
  Load_ECAL_mask();

  for(unsigned int ii = 0; ii < ecal_trig_mask.size(); ii++){
    
    //    cout << " ecal_trig_mask:  II = " << ii <<  " Roc = "  << ecal_trig_mask[ii].roc << "  Slot = " << ecal_trig_mask[ii].slot << " Ch = " << ecal_trig_mask[ii].ch << endl;
    
    uint32_t roc_id     =   ecal_trig_mask[ii].roc;
    unsigned int slot   =   ecal_trig_mask[ii].slot;
    unsigned int ch     =   ecal_trig_mask[ii].ch;
    
    DTranslationTable::csc_t daq_index = {roc_id, slot, ch };
    
    DTranslationTable::DChannelInfo channel_info;
    
    try {		
      channel_info = ttab[0]->GetDetectorIndex(daq_index);	      
    }
    
    catch(...){
      if(VERBOSE && print_messages) cout << "Exception: ECAL channel is not in the translation table  " <<  " Crate = " << roc_id << "  Slot = " << slot << 
				      " Channel = " << ch << endl;
      continue;
    } 

    int idx_col = channel_info.fcal.col;
    int idx_row = channel_info.fcal.row;

    if(idx_col < 0)
      ecal_trig_mask[ii].col = idx_col + 20;
    else if(idx_col > 0) 
      ecal_trig_mask[ii].col = idx_col + 19;
    
    if(idx_row < 0)
      ecal_trig_mask[ii].row = idx_row + 20;
    else if(idx_row > 0) 
      ecal_trig_mask[ii].row = idx_row + 19;
    
  }
  
  if(VERBOSE == 1){
    cout << "NUMBER OF MASKED ECAL CHANNELS = " << ecal_trig_mask.size() << endl;
    for(unsigned int ii = 0; ii < ecal_trig_mask.size(); ii++){    
      cout << " ecal_trig_mask:  II = " << ii <<  "  ROC = "  << ecal_trig_mask[ii].roc << "  Slot = " << ecal_trig_mask[ii].slot << " Ch = " << ecal_trig_mask[ii].ch << 
	"  Column = " << ecal_trig_mask[ii].col <<  "  Row = " << ecal_trig_mask[ii].row << endl;
    }
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
	      if(VERBOSE && print_messages) cout << "Exception: FCAL channel is not in the translation table  " <<  " Crate = " << 10 + crate << "  Slot = " << slot << 
		" Channel = " << ch << endl;
	      continue;
	    }
	    	    
	    fcal_mod tmp;

	    tmp.roc     =  crate;	    
	    tmp.slot    =  slot;
	    tmp.ch      =  ch;
	    
	    tmp.row = channel_info.fcal.row;
	    tmp.col = channel_info.fcal.col;
	    
	    if(VERBOSE == 1)
	    	cout << " MASKED CHANNEL = " << tmp.row << "   " << tmp.col << endl;

	    fcal_trig_mask.push_back(tmp);
	  }
	  
	}
	
      }	
      
    }  // Loop over slots
  }    // Loop over crates       
  
  
  if(VERBOSE == 1) cout << "NUMBER OF MASKED FCAL CHANNELS = " << fcal_trig_mask.size() << endl;
  

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
  }    // HAVE_RCDB 

  
  if(VERBOSE == 1) cout << "NUMBER OF MASKED BCAL CHANNELS = " << bcal_trig_mask.size() << endl;
  
  
  return 0;
  
#else // HAVE_RCDB

  return 10; // RCDB is not available

#endif
  
}


int  DL1MCTrigger_factory::SignalPulse(double en, double time, double amp_array[sample], int type){
   

  // Parameterize and digitize pulse shapes. Sum up amplitudes
  // type = 1 - FCAL
  // type = 2 - BCAL
  // type = 3 - ECAL
  
  //  float exp_par = 0.358;
  //  if(type == 2) exp_par = 0.18;

  double ecal_exp1  =  12.92;
  double ecal_exp2  =  3.173;
  double ecal_norm  =  ecal_exp1 - ecal_exp2;

  double bcal_exp1  =  62.68;
  double bcal_exp2  =  4.16;
  double bcal_norm  =  bcal_exp1 - bcal_exp2;

  double fcal_exp1  =  8.56;
  double fcal_exp2  =  8.043;
  double fcal_exp3  =  215.6;
  double fcal_frac  =  0.001641;
  double fcal_norm  =  fcal_exp1 - fcal_exp2 + fcal_exp3*fcal_frac;
    
  int pulse_length = 125;

  int sample_first = (int)floor(time/time_stamp);
  
  // digitization range
  int ind_min = sample_first + 1;
  int ind_max = ind_min + pulse_length + 1;
    
  if( (ind_min > sample) || (ind_min < 0)){
    return 1;
  }
  
  if(ind_max > sample){
    ind_max = sample - 1;
  }


  double my_int = 0;
   
  for(int i = ind_min; i < ind_max; i++ ){
    double adc_t  =  time_stamp*i - time;
    if(adc_t < 0) adc_t = 0.;

    double amp = 0;
    
    if(type == 3){      
      amp    =  (exp(-adc_t/ecal_exp1) - exp(-adc_t/ecal_exp2))/ecal_norm;
    } else if(type == 2) {      
      amp    =  (exp(-adc_t/bcal_exp1) - exp(-adc_t/bcal_exp2))/bcal_norm;
    } else if(type == 1){
      amp    =  (exp(-adc_t/fcal_exp1) - exp(-adc_t/fcal_exp2) + fcal_frac*exp(-adc_t/fcal_exp3) )/fcal_norm;
    } 
    
    //    double amp    =  exp_par*exp_par*exp(-adc_t*exp_par)*adc_t;
        
    amp_array[i] += amp*time_stamp*en;

    my_int += amp*time_stamp;
    
  }

  if(VERBOSE == 2)
    cout << " ----   CHECK  INTEGRAL -----  Detector type =  "  <<  type <<  "   Integral = " << my_int <<  endl;
  
  return 0;  
}

int DL1MCTrigger_factory::GTP(int detector){

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

    for(int ii = index_min; ii < index_max; ii++){

      if(detector == 1)
	energy_sum += fcal_ssp[ii];
      else if(detector == 2)
	energy_sum += bcal_ssp[ii];
      else if(detector == 3)
	energy_sum += ecal_ssp[ii];         
    }
    
    if(detector == 1)
      fcal_gtp[samp] = energy_sum;
    else if(detector == 2){
      bcal_gtp[samp] = energy_sum;
    } else if(detector == 3){
      ecal_gtp[samp] = energy_sum;
    }   
    
  }  

  return 0;

}


template <typename T>  int DL1MCTrigger_factory::FADC_SSP(vector<T> merged_hits, int detector){
  
  //  1  - FCAL
  //  2  - BCAL
  //  3  - ECAL
  
  int EN_THR =  4096; 
  int NSA    =  10;
  int NSB    =  3;;
  
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

  if(VERBOSE == 2)
    cout << " Inside FADC_SSP: " <<  " Detector =  " << detector << "  NSA =   " << NSA << "  NSB =  " <<  NSB << "  THR =  " << EN_THR << endl;

    
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
	  if(VERBOSE == 2)
	    cout << " SSP FCAL index kk = " << kk <<   "  SSP amplitude =  " <<  fcal_ssp[kk] << endl;
	}
	else if(detector == 2){
	  if((merged_hits[hit].adc_amp[kk] - 100) > 0)
	    bcal_ssp[kk] += (merged_hits[hit].adc_amp[kk] - TRIG_BASELINE);
	  if(VERBOSE == 2)
	    cout << " SSP BCAL index kk = " << kk <<   "  SSP amplitude =  " <<  bcal_ssp[kk] << endl;
	}
	else if(detector == 3){
	  if((merged_hits[hit].adc_amp[kk] - 100) > 0)
	    ecal_ssp[kk] += (merged_hits[hit].adc_amp[kk] - TRIG_BASELINE);
	  if(VERBOSE == 2)
	    cout << " SSP ECAL index kk = " << kk <<   "  SSP amplitude =  " <<  ecal_ssp[kk] << endl;
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

void DL1MCTrigger_factory::PrintTriggers(){
  
  string detector;
  int nhit = 0;
  unsigned int pattern = 0;

  cout << endl << endl;
  cout << " ------------  Trigger Settings --------------- " << endl;
  cout << endl << endl;

  cout << "USE RCDB = "  <<  use_rcdb  <<  endl;

  cout << endl;
  
  cout << "----------- ECAL ----------- " << endl << endl;

  cout << "ECAL_CELL_THR  = " <<  setw(10) <<  ECAL_CELL_THR <<  endl;
  cout << "ECAL_NSA       = " <<  setw(10) <<  ECAL_NSA      <<  endl;
  cout << "ECAL_NSB       = " <<  setw(10) <<  ECAL_NSB      <<  endl;
  cout << "ECAL_WINDOW    = " <<  setw(10) <<  ECAL_WINDOW   <<  endl;
  
  
  cout << "----------- FCAL ----------- " << endl << endl;

  cout << "FCAL_CELL_THR  = " <<  setw(10) <<  FCAL_CELL_THR <<  endl;
  cout << "FCAL_NSA       = " <<  setw(10) <<  FCAL_NSA      <<  endl;
  cout << "FCAL_NSB       = " <<  setw(10) <<  FCAL_NSB      <<  endl;
  cout << "FCAL_WINDOW    = " <<  setw(10) <<  FCAL_WINDOW   <<  endl;

  cout << endl;
  
  cout << "----------- BCAL ----------- " << endl << endl;

  cout << "BCAL_CELL_THR  = " <<  setw(10) <<  BCAL_CELL_THR <<  endl;
  cout << "BCAL_NSA       = " <<  setw(10) <<  BCAL_NSA      <<  endl;
  cout << "BCAL_NSB       = " <<  setw(10) <<  BCAL_NSB      <<  endl;
  cout << "BCAL_WINDOW    = " <<  setw(10) <<  BCAL_WINDOW   <<  endl;
  
  cout << endl;


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

  cout << endl;

  cout << "ECAL_TRIG_GAIN  =  "   <<  ECAL_TRIG_GAIN << "    FCAL_TRIG_GAIN  =  " << FCAL_TRIG_GAIN << endl;

  cout << endl;

  cout << "Number of modules MASKED in DAQ:  " <<  "  ECAL  =  " << ecal_trig_mask.size() << "   FCAL  =  " << fcal_trig_mask.size() << "   BCAL  =  " << bcal_trig_mask.size() << endl;
  
  cout << endl;
 
  cout << "SIMULATION OF GAINS:        " <<  " ECAL = " <<  simu_gain_ecal     << "   FCAL = " << simu_gain_fcal     << "   BCAL = " << simu_gain_bcal << endl;
  cout << "SIMULATION OF BASELINES:    " <<  " ECAL = " <<  simu_baseline_ecal << "   FCAL = " << simu_baseline_fcal << "   BCAL = " << simu_baseline_bcal << endl;

  cout << endl;

  cout << "BCAL_ADC_PER_MEV = " << BCAL_ADC_PER_MEV << endl;
  
  cout << endl;
  
}




// Find Triggers with ECAL

int DL1MCTrigger_factory::FindTriggersECAL(DL1MCTrigger *trigger){
  
  int trig_found = 0;
  
  // Main production trigger  
  for(unsigned int ii = 0; ii < triggers_enabled.size(); ii++){
    
    if(debug)
      cout << "Trigger Type = " << triggers_enabled[ii].type << endl;

    if(triggers_enabled[ii].type == 3){    // FCAL & BCAL trigger

      int en_bit = triggers_enabled[ii].bit;

      
      int gtp_energy  = 0;
      
      int ecal_energy = 0;
      int fcal_energy = 0;
      int bcal_energy = 0;
         
      
      if(debug){	
	cout << "  (ECAL + FCAL) Threshold   = " << triggers_enabled[ii].gtp.fcal <<
	  "   BCAL Threshold = " << triggers_enabled[ii].gtp.bcal <<  " GTP energy = " << gtp_energy <<
	  "   " << triggers_enabled[ii].gtp.en_thr << endl;
	
	cout << "  FCAL_MIN  = " <<  triggers_enabled[ii].gtp.fcal_min << "  FCAL_MAX = " <<   triggers_enabled[ii].gtp.fcal_max <<
	  "  BCAL_MIN  = " <<  triggers_enabled[ii].gtp.bcal_min << "  BCAL_MAX = " <<   triggers_enabled[ii].gtp.bcal_max << endl;
	
	cout << "  ECAL_TRIG_GAIN = " << ECAL_TRIG_GAIN <<  "  FCAL_TRIG_GAIN = " << FCAL_TRIG_GAIN << endl;
      }
	
      
      for(unsigned int samp = 0; samp < sample; samp++){

	//	TEST
	//	FCAL_ECAL_OFFSET =   8;
	//	BCAL_ECAL_OFFSET =   0;
	//	int ecal_samp = samp + 5;
	//	int fcal_samp = samp - FCAL_ECAL_OFFSET + 5;	  
	//	int bcal_samp = samp - BCAL_ECAL_OFFSET + 5;
	
	int ecal_samp = samp;
	int fcal_samp = samp - FCAL_ECAL_OFFSET;	  
	int bcal_samp = samp - BCAL_ECAL_OFFSET;


	if(ecal_samp < 0){
	  ecal_energy = 0;
	} else if(ecal_samp >= sample){ 
	  ecal_energy = 0;
	} else{ 
	  ecal_energy = (int)(ECAL_TRIG_GAIN*ecal_gtp[ecal_samp]);
	}

	if(fcal_samp < 0){
	  fcal_energy = 0;
	} else if(fcal_samp >= sample){ 
	  fcal_energy = 0;
	} else{ 
	  fcal_energy = (int)(FCAL_TRIG_GAIN*fcal_gtp[fcal_samp]);
	}	
			
	if(bcal_samp < 0){
	  bcal_energy = 0;
	} else if(bcal_samp >= sample){ 
	  bcal_energy = 0;
	} else{ 
	  bcal_energy = bcal_gtp[bcal_samp];
	}
	 
	if(debug)
	    cout << " Sample = " << samp <<  "  ECAL = " << ecal_energy << "  FCAL =  " << fcal_energy <<  "  BCAL =   " << bcal_energy << endl; 


	if(bcal_energy > triggers_enabled[ii].gtp.bcal_max)
	  bcal_energy =  triggers_enabled[ii].gtp.bcal_max;
	
	int ecal_fcal_energy = ecal_energy + fcal_energy;

	if(ecal_fcal_energy > triggers_enabled[ii].gtp.fcal_max)
	  ecal_fcal_energy =  triggers_enabled[ii].gtp.fcal_max;
		
	
	gtp_energy = triggers_enabled[ii].gtp.fcal*(ecal_fcal_energy) +
	  triggers_enabled[ii].gtp.bcal*bcal_energy;
	

	if(gtp_energy >= triggers_enabled[ii].gtp.en_thr){
	  
	  if( (ecal_fcal_energy > triggers_enabled[ii].gtp.fcal_min) || (triggers_enabled[ii].gtp.fcal_min == 0)) {  // FCAL > fcal_min	    
	    trigger->trig_mask     =  (trigger->trig_mask | (1 << en_bit) );
	    trigger->trig_time[0]  = samp - 25;
	    trig_found++;
	    break;
	    
	  }  //  Check fcal energy threshold
	}    //  Check global energy threshold
	
	
      }    //  Loop over samples  
    }      //  FCAL & BCAL triggers
    
    
    
    if(triggers_enabled[ii].type == 2){    //  BCAL trigger
      
      int en_bit = triggers_enabled[ii].bit;
      
      int gtp_energy  = 0;
      int bcal_energy = 0;
      
      for(unsigned int samp = 0; samp < sample; samp++){
	
	int bcal_samp = samp - BCAL_ECAL_OFFSET;
	
	if(bcal_samp < 0){
	  bcal_energy = 0;
	} else if(bcal_samp >= sample){ 
	  bcal_energy = 0;
	} else{ 
	  bcal_energy = bcal_gtp[bcal_samp];
	}

	if(bcal_energy > triggers_enabled[ii].gtp.bcal_max)
	  bcal_energy =  triggers_enabled[ii].gtp.bcal_max;
		  
	gtp_energy = triggers_enabled[ii].gtp.bcal*bcal_energy;
	
	if(gtp_energy >= triggers_enabled[ii].gtp.en_thr){
	  
	  trigger->trig_mask   = (trigger->trig_mask | (1 << en_bit));
	  trigger->trig_time[2]   = samp - 25; 
	  trig_found++;

	  break;
	  
	}  // Energy threshold			
      }
    }      // BCAL trigger
    
  }    // Loop over triggers found in the config file
  
  return trig_found;
  
}

int DL1MCTrigger_factory::FindTriggers(DL1MCTrigger *trigger, vector<const DSCHit*> & sc_hits){
  
  int trig_found = 0;
  
  // Main production trigger  
  for(unsigned int ii = 0; ii < triggers_enabled.size(); ii++){
    
    if(debug)
      cout << "Trigger Type = " << triggers_enabled[ii].type << endl;

    if(triggers_enabled[ii].type == 3){    // FCAL & BCAL trigger

      int en_bit = triggers_enabled[ii].bit;

      int fcal_bcal_st = 0;

      for(unsigned int jj = 0; jj < triggers_enabled.size(); jj++){   // Check if other triggers are enabled with the same bit

	int en_bit1 = triggers_enabled[jj].bit;

	if(ii != jj)
	  if( (en_bit == en_bit1) && triggers_enabled[jj].type == 4 ) fcal_bcal_st = 1;
      }


      int gtp_energy  = 0;
      int bcal_energy = 0;
      int fcal_energy = 0;
      
      for(unsigned int samp = 0; samp < sample; samp++){
	
	int bcal_samp = samp - BCAL_OFFSET;
	
	if(bcal_samp < 0){
	  bcal_energy = 0;
	} else if(bcal_samp >= sample){ 
	  bcal_energy = 0;
	} else{ 
	  bcal_energy = bcal_gtp[bcal_samp];
	}

	fcal_energy = fcal_gtp[samp];
	
	
	if(bcal_energy > triggers_enabled[ii].gtp.bcal_max)
	  bcal_energy =  triggers_enabled[ii].gtp.bcal_max;

	if(fcal_energy > triggers_enabled[ii].gtp.fcal_max)
	  fcal_energy =  triggers_enabled[ii].gtp.fcal_max;

	
	gtp_energy = triggers_enabled[ii].gtp.fcal*fcal_energy + 
	  triggers_enabled[ii].gtp.bcal*bcal_energy;
	
	if(debug)
	  cout << " GTP energy = " << gtp_energy << "   " << triggers_enabled[ii].gtp.en_thr << endl;


	if(gtp_energy >= triggers_enabled[ii].gtp.en_thr){

	  if( (fcal_energy > triggers_enabled[ii].gtp.fcal_min) || (triggers_enabled[ii].gtp.fcal_min == 0)){ // FCAL > fcal_min

	    int fcal_bcal_st_found = 0;

	    if(fcal_bcal_st == 1){   // FCAL & BCAL & ST
	      for(unsigned int sc_hit = 0; sc_hit < sc_hits.size(); sc_hit++){

		int sc_time = sc_hits[sc_hit]->t/time_stamp + 0.5 + SC_OFFSET;
		int fcal_bcal_time =  samp - 25;
		
		if(debug){
		  cout << sc_time  << "  "  << " Trigger time  " << fcal_bcal_time <<  endl;
		  cout << " ABS (DT) " << abs(fcal_bcal_time - sc_time) << endl;
		}
		
		if(abs(fcal_bcal_time - sc_time) < 3){    // Coincidence between FCAL, BCAL, and SC found 
		  trigger->trig_mask     =  (trigger->trig_mask | (1 << en_bit) );
		  trigger->trig_time[0]  = samp - 25;
		  trig_found++;
		  fcal_bcal_st_found = 1;
		  break;
		}
	      }   // Loop over SC hits

	      if(fcal_bcal_st_found == 1) break; // Break from loop over samples	      
	      
	    } else  {    // FCAL & BCAL	      
	      trigger->trig_mask     =  (trigger->trig_mask | (1 << en_bit) );
	      trigger->trig_time[0]  = samp - 25;
	      trig_found++;
	      break;
	    }


	  }  //  Check fcal energy threshold
	}    //  Check global energy threshold

	
      }    //  Loop over samples  
    }      //  FCAL & BCAL triggers
  

    if(triggers_enabled[ii].type == 2){    //  BCAL trigger

      int en_bit = triggers_enabled[ii].bit;

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

	if(bcal_energy > triggers_enabled[ii].gtp.bcal_max)
	  bcal_energy =  triggers_enabled[ii].gtp.bcal_max;
	
	gtp_energy = triggers_enabled[ii].gtp.bcal*bcal_energy;
	
	if(gtp_energy >= triggers_enabled[ii].gtp.en_thr){
	  
	  trigger->trig_mask   = (trigger->trig_mask | (1 << en_bit));
	  trigger->trig_time[2]   = samp - 25; 
	  trig_found++;

	  break;
	  
	}  // Energy threshold			
      }
    }      // BCAL trigger

  }    // Loop over triggers found in the config file
  
  return trig_found;
  
}


// Fill fcal calibration tables similar to FCALHit factory
void DL1MCTrigger_factory::LoadFCALConst(fcal_constants_t &table, const vector<double> &fcal_const_ch, 
					 const DFCALGeometry  &fcalGeom){
  for (int ch = 0; ch < fcalGeom.numChannels(); ch++) {
    int row = fcalGeom.row(ch);
    int col = fcalGeom.column(ch);
    table[row][col] = fcal_const_ch[ch];
  }
  	
}

// Fill ecal calibration tables similar to ECALHit factory
void DL1MCTrigger_factory::LoadECALConst(ecal_constants_t &table, const vector<double> &ecal_const_ch, 
					 const DECALGeometry  &ecalGeom){
  for (int ch = 0; ch < ecalGeom.kECALMaxChannels; ch++) {
    int row = ecalGeom.row(ch);
    int col = ecalGeom.column(ch);
    table[row][col] = ecal_const_ch[ch];
  }  	
}

// Fill bcal calibration tables 
void DL1MCTrigger_factory::LoadBCALConst(bcal_constants_t &table, const vector<double> &bcal_const_ch, int type = 0){
   char str[256];
   int channel = 0;

   int db_debug = 0;
   
    // reset the table before filling it
    table.clear();

    for (int module = 1; module <= BCAL_NUM_MODULES; module++) {
        for (int layer = 1; layer <= BCAL_NUM_LAYERS; layer++) {
            for (int sector = 1; sector <= BCAL_NUM_SECTORS; sector++) {
                if ((channel > BCAL_MAX_CHANNELS) || (channel+1 > BCAL_MAX_CHANNELS)) {  // sanity check
                    sprintf(str, "Too many channels for BCAL table!"
                            " channel=%d (should be %d)", 
                            channel, BCAL_MAX_CHANNELS);
                    cerr << str << endl;
                    throw JException(str);
                }

		if(type == 1){
		  
		  double gain1 = BCAL_ADC_PER_MEV;
		  double gain2 = BCAL_ADC_PER_MEV;

		  if(bcal_const_ch[channel] > 0)   gain1 = 1./(bcal_const_ch[channel]*1e3);
		  if(bcal_const_ch[channel+1] > 0) gain2 = 1./(bcal_const_ch[channel+1]*1e3);
		  
		  table.push_back( cell_calib_t(gain1,gain2) );
		  
		  if (db_debug) {
		    cout << " BCAL GAINS:  Module = " << module <<  "  Layer = " <<  layer << "  Sector = " << sector <<
		      "  Gain 1 = " <<  gain1 << "  Gain 2 = " << gain2 << endl;
		  }
		} else 
		  table.push_back( cell_calib_t(bcal_const_ch[channel],bcal_const_ch[channel+1]));		  
		
		
                channel += 2;
            }
        }
    }
    
    // check to make sure that we loaded enough channels
    if (channel != BCAL_MAX_CHANNELS) { 
        sprintf(str, "Not enough channels for BCAL table!"
                " channel=%d (should be %d)", 
                channel, BCAL_MAX_CHANNELS);
        cerr << str << endl;
        throw JException(str);
    }  
}



void DL1MCTrigger_factory::Digitize(double adc_amp[sample], int adc_count[sample]){

  for(int samp = 0; samp < sample; samp++ ){
    
    adc_count[samp] += (int)(adc_amp[samp] + TRIG_BASELINE + 0.5);

    if(adc_count[samp] > max_adc_bins)
      adc_count[samp] = max_adc_bins;

  }
}


void DL1MCTrigger_factory::AddBaseline(double adc_amp[sample], double pedestal, DRandom2 &gDRandom){

  double pedestal_correct = pedestal - TRIG_BASELINE;

  for(int samp = 0; samp < sample; samp++ ){  
    double tmp = gDRandom.Gaus(pedestal_correct, pedestal_sigma);   
    adc_amp[samp] += tmp;
    //    cout << "  " << tmp;
  }
  
  if(VERBOSE == 3){
    cout << endl;    
    cout << " Corrected pedestals = " << pedestal_correct << "  " <<  adc_amp[sample - 2] 
	 << "  " << pedestal_sigma << endl; 
  }

}



void DL1MCTrigger_factory::GetSeeds(const std::shared_ptr<const JEvent>& event, uint64_t eventnumber, UInt_t &seed1, UInt_t &seed2, UInt_t &seed3){

  // Use seeds similar to mcsmear

  JEventSource *source = event->GetJEventSource();
  
  DEventSourceHDDM *hddm_source = dynamic_cast<DEventSourceHDDM*>(source);
  
  if (!hddm_source) {

    cerr << "DL1MCTrigger_factory:  This program MUST be used with an HDDM file as input!" << endl;
    cerr << "   Default seeds will be used for the random generator  " << endl;
    seed1 = 259921049 + eventnumber;
    seed2 = 442249570 + eventnumber;
    seed3 = 709975946 + eventnumber;
  } else {
  
    hddm_s::HDDM *record = const_cast<hddm_s::HDDM*>(event->GetSingleStrict<hddm_s::HDDM>());
    // TODO: NWB: Don't like this const cast

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

void DL1MCTrigger_factory::Load_ECAL_mask(){
  // Load a temporary mask without filling column and row information. This function will be removed once the masks are available in the RCDB.
  // It uses crate, slot, and channel consistent with the info retrieved from the RCDB
  ecal_trig_mask.push_back(ecal_mod{111,3,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,3,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,3,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,3,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,3,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{111,4,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,4,4,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,4,5,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,4,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,4,7,0,0});

  ecal_trig_mask.push_back(ecal_mod{111,5,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,5,7,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,5,8,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,5,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,5,10,0,0});
  
  ecal_trig_mask.push_back(ecal_mod{111,6,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,6,10,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,6,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,6,12,0,0});

  ecal_trig_mask.push_back(ecal_mod{111,7,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,7,12,0,0});
  ecal_trig_mask.push_back(ecal_mod{111,7,13,0,0});

  ecal_trig_mask.push_back(ecal_mod{111,8,12,0,0});

  // ------------------------------

  ecal_trig_mask.push_back(ecal_mod{112,3,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,3,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,3,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,3,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,3,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{112,4,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,4,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,4,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,4,14,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,4,15,0,0});

  ecal_trig_mask.push_back(ecal_mod{112,5,11,0,0});

  ecal_trig_mask.push_back(ecal_mod{112,10,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,10,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,10,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,10,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,10,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{112,13,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,13,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,13,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,13,14,0,0});
  ecal_trig_mask.push_back(ecal_mod{112,13,15,0,0});

  ecal_trig_mask.push_back(ecal_mod{112,14,11,0,0});

  // 

  ecal_trig_mask.push_back(ecal_mod{118,3,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,3,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,3,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,3,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,3,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{118,4,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,4,4,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,4,5,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,4,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,4,7,0,0});

  ecal_trig_mask.push_back(ecal_mod{118,5,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,5,7,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,5,8,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,5,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,5,10,0,0});

  ecal_trig_mask.push_back(ecal_mod{118,6,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,6,10,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,6,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,6,12,0,0});

  ecal_trig_mask.push_back(ecal_mod{118,7,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,7,12,0,0});
  ecal_trig_mask.push_back(ecal_mod{118,7,13,0,0});

  ecal_trig_mask.push_back(ecal_mod{118,8,12,0,0});

  // 

  ecal_trig_mask.push_back(ecal_mod{114,3,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,3,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,3,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,3,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,3,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{114,4,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,4,4,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,4,5,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,4,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,4,7,0,0});

  ecal_trig_mask.push_back(ecal_mod{114,5,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,5,7,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,5,8,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,5,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,5,10,0,0});

  ecal_trig_mask.push_back(ecal_mod{114,6,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,6,10,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,6,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,6,12,0,0});

  ecal_trig_mask.push_back(ecal_mod{114,7,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,7,12,0,0});
  ecal_trig_mask.push_back(ecal_mod{114,7,13,0,0});

  ecal_trig_mask.push_back(ecal_mod{114,8,12,0,0});

  //

  ecal_trig_mask.push_back(ecal_mod{115,3,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,3,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,3,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,3,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,3,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{115,4,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,4,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,4,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,4,14,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,4,15,0,0});

  ecal_trig_mask.push_back(ecal_mod{115,5,11,0,0});

  ecal_trig_mask.push_back(ecal_mod{115,10,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,10,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,10,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,10,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,10,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{115,13,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,13,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,13,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,13,14,0,0});
  ecal_trig_mask.push_back(ecal_mod{115,13,15,0,0});

  ecal_trig_mask.push_back(ecal_mod{115,14,11,0,0});

  //

  ecal_trig_mask.push_back(ecal_mod{116,3,0,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,3,1,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,3,2,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,3,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,3,4,0,0});

  ecal_trig_mask.push_back(ecal_mod{116,4,3,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,4,4,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,4,5,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,4,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,4,7,0,0});

  ecal_trig_mask.push_back(ecal_mod{116,5,6,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,5,7,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,5,8,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,5,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,5,10,0,0});

  ecal_trig_mask.push_back(ecal_mod{116,6,9,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,6,10,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,6,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,6,12,0,0});
  
  ecal_trig_mask.push_back(ecal_mod{116,7,11,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,7,12,0,0});
  ecal_trig_mask.push_back(ecal_mod{116,7,13,0,0});

  ecal_trig_mask.push_back(ecal_mod{116,8,12,0,0});
}

void DL1MCTrigger_factory::Load_ECAL_mask_default(){
  // Default masks for the first run with the ECAL in 2025
  // Intended for use only if USE_RCDB is not used !
  for(int col = 14; col <= 25; col++){
    for(int row = 14; row <= 25; row++){
      // Exclude corners
      if( ((col == 14) && (row == 14)) || ((col == 14) && (row == 25)) ||
	  ((col == 25) && (row == 14)) || ((col == 25) && (row == 25)))
	continue;
      // Don't exclude the beam hole
      ecal_trig_mask.push_back(ecal_mod{111,3,0,col,row});
    }
  }   
}

void DL1MCTrigger_factory::BCAL_average_gain(){
  
  double GAIN_MIN = 20.;
  double GAIN_MAX = 35.;
  
  vector<double> num;
  
  int BCAL_MAX_PAIR = BCAL_MAX_CHANNELS/2;
  
  for (int ii = 0; ii < BCAL_MAX_PAIR; ii++){
    if( (bcal_gains[ii].first >  GAIN_MIN ) && (bcal_gains[ii].first <  GAIN_MAX )) 
      num.push_back(bcal_gains[ii].first);
    if( (bcal_gains[ii].first >  GAIN_MIN ) && (bcal_gains[ii].first <  GAIN_MAX )) 
      num.push_back(bcal_gains[ii].second);
  }
  
  sort(num.begin(),num.end());
  
  double dif[1600];
  memset(dif,0,sizeof(dif));
  
  unsigned int slide_wind = 40;  // Typical size sqrt(BCAL_MAX_CHANNELS)
  
  if(VERBOSE == 1){
    for(unsigned int ii = 0; ii < num.size(); ii++){
      cout << " II = " <<  ii  << "   VAL: " << num[ii] << endl;
    }
  }
  
  for(unsigned int ii = 0; ii < num.size() - slide_wind; ii++){
    dif[ii] = fabs(num[ii+slide_wind] - num[ii]);
  }
  
  double min_dif = 100.;
  double min_ind = 0;
  
  for (unsigned int ii = 0; ii < num.size() - slide_wind; ii++){
    if(dif[ii] <  min_dif){
      min_dif = dif[ii];
      min_ind = ii;
    }
  }
  
  if(VERBOSE == 1)
    cout << " INDEX = " << min_ind << "  NUM " << num[min_ind+slide_wind] << "  " <<  num[min_ind] << endl;
  
  double mean_gain = num[min_ind+slide_wind/2];
  
  if( (mean_gain > GAIN_MIN) && (mean_gain < GAIN_MAX)) BCAL_ADC_PER_MEV = mean_gain; 
  
  
}
