// $Id$
//
//    File: JEventProcessor_trigtest.cc
// Created: Mon Aug  4 12:38:45 PM EDT 2025
// Creator: ilarin (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//
//  Simplified version of DL1MCTrigger, currently only for BFCAL trigger emulation, the TOF trigger part will be added later (hopefully)
//

#include "JEventProcessor_trigtest.h"
#include <JANA/JApplication.h>

extern "C"{
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_trigtest());
  }
}

//------------------
// JEventProcessor_trigtest (Constructor)
//------------------
JEventProcessor_trigtest::JEventProcessor_trigtest()
{
  SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_trigtest (Destructor)
//------------------
JEventProcessor_trigtest::~JEventProcessor_trigtest()
{
}

//------------------
// Init
//------------------
void JEventProcessor_trigtest::Init()
{
  auto app = GetApplication();

  FCAL_ADC_PER_MEV  = 3.73;
  app->SetDefaultParameter("TRIG:FCAL_ADC_PER_MEV", FCAL_ADC_PER_MEV, "FCAL energy calibration for the Trigger");
  FCAL_CELL_THR = 165;
  app->SetDefaultParameter("TRIG:FCAL_CELL_THR", FCAL_CELL_THR, "FCAL energy threshold per cell");

  FCAL_NSA  = 10;
  app->SetDefaultParameter("TRIG:FCAL_NSA", FCAL_NSA, "FCAL NSA");
  FCAL_NSB  =  3;
  app->SetDefaultParameter("TRIG:FCAL_NSB", FCAL_NSB, "FCAL NSB");
  FCAL_WINDOW =  10;
  app->SetDefaultParameter("TRIG:FCAL_WINDOW", FCAL_WINDOW, "FCAL GTP integration window");

  BCAL_ADC_PER_MEV =  34.48276;
  app->SetDefaultParameter("TRIG:BCAL_ADC_PER_MEV", BCAL_ADC_PER_MEV, "BCAL energy calibration for the Trigger");
  BCAL_CELL_THR =  120;
  app->SetDefaultParameter("TRIG:BCAL_CELL_THR", BCAL_CELL_THR, "BCAL energy threshold per cell");
  BCAL_NSA  = 19;
  app->SetDefaultParameter("TRIG:BCAL_NSA", BCAL_NSA, "BCAL NSA");
  BCAL_NSB  = 3;
  app->SetDefaultParameter("TRIG:BCAL_NSB", BCAL_NSB, "BCAL NSB");
  BCAL_WINDOW = 20;
  app->SetDefaultParameter("TRIG:BCAL_WINDOW", BCAL_WINDOW, "BCAL GTP integration window");

  FCAL_BCAL_EN  = 45000; 
  app->SetDefaultParameter("TRIG:FCAL_BCAL_EN", FCAL_BCAL_EN, "Energy threshold for the FCAL & BCAL trigger");
  BCAL_OFFSET = 2;
  app->SetDefaultParameter("TRIG:BCAL_OFFSET", BCAL_OFFSET, "Timing offset between BCAL and FCAL energies at GTP (samples)");

  PEDESTAL_SIGMA = 1.2; //  6.5
  app->SetDefaultParameter("TRIG:PEDESTAL_SIGMA", PEDESTAL_SIGMA, "Pedestal sigma to emulate spread");

}

//------------------
// BeginRun
//------------------
void JEventProcessor_trigtest::BeginRun(const shared_ptr<const JEvent> &event) {

  fcal_trig_mask.clear();
  bcal_trig_mask.clear();
  triggers_enabled.clear();

  int print_level = 1;
  Read_RCDB(event, print_level);

  vector<const DFCALGeometry*> fcalGeomVect;
  event->Get(fcalGeomVect);
  if(fcalGeomVect.empty())
    throw JException("L1MCTrigger: DFCALGeometry not found (fcalGeomVect is empty)");

  const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
  auto calibration = DEvent::GetJCalibration(event);

  vector< double > fcal_gains_ch;
  if(calibration->Get("/FCAL/gains", fcal_gains_ch))
    throw JException("L1MCTrigger: failed to load calibration path '/FCAL/gains'");
  fcal_gains.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 1.0));
  for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
    fcal_gains[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_gains_ch[ch];

  vector< double > fcal_pedestals_ch;
  if(calibration->Get("/FCAL/pedestals", fcal_pedestals_ch))
    throw JException("L1MCTrigger: failed to load calibration path '/FCAL/pedestals'");

  fcal_pedestals.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, TRIG_BASELINE));
  for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
    fcal_pedestals[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_pedestals_ch[ch];

}

//------------------
// Process
//------------------
void JEventProcessor_trigtest::Process(const shared_ptr<const JEvent> &event) {


  vector<int> fcal_ssp(data_sample,0), fcal_gtp(data_sample,0),
              bcal_ssp(data_sample,0), bcal_gtp(data_sample,0);
  vector<FCAL_SIGNAL> fcal_signal_hits, fcal_merged_hits;
  vector<BCAL_SIGNAL> bcal_signal_hits, bcal_merged_hits;

  vector<const DFCALHit*>  fcal_hits;
  event->Get(fcal_hits);
  vector<const DBCALHit*>  bcal_hits;
  event->Get(bcal_hits);


//  FCAL Part

  double FCAL_HIT_EN_SUM = 0;
  for(unsigned int i = 0; i < fcal_hits.size(); ++i) {
    int row  = fcal_hits[i]->row;
    int col  = fcal_hits[i]->column;

    double t = fcal_hits[i]->t;
    double e = fcal_hits[i]->E;

    if(row<0 || row>=DFCALGeometry::kBlocksTall || col<0 || col>=DFCALGeometry::kBlocksWide) {
      cout << "Error: row/col out of range " << row << " " << col << endl;
      continue;
    }

    bool masked = any_of(fcal_trig_mask.begin(),fcal_trig_mask.end(), [row, col](const fcal_mod& mask) {
      return mask.row == row && mask.col == col;});

    if(masked) continue; // skip masked channels from the sum

    FCAL_HIT_EN_SUM += e;
    FCAL_SIGNAL fcal_signal(row,col,e,t);

    double E  = e * FCAL_ADC_PER_MEV * 1.e3 / fcal_gains[row][col];  //  energy in counts
    SignalPulse(E, t, fcal_signal.adc_en, 1);

    fcal_signal_hits.push_back(fcal_signal);
  }

// Merge FCAL hits:

  for(size_t i = 0; i < fcal_signal_hits.size(); ++i) {
    auto& base_hit = fcal_signal_hits[i];
    if(base_hit.merged) continue;

    FCAL_SIGNAL merged_hit(base_hit.row, base_hit.column,0.0,0.0);
    merged_hit.adc_en = base_hit.adc_en;

    for(size_t j = i+1; j < fcal_signal_hits.size(); ++j) {
      auto& compare_hit = fcal_signal_hits[j];
      if(compare_hit.row == base_hit.row && compare_hit.column == base_hit.column) {
        compare_hit.merged = true;
        for(size_t k = 0; k < merged_hit.adc_en.size(); ++k)  merged_hit.adc_en[k] += compare_hit.adc_en[k];
      }
    }

    fcal_merged_hits.push_back(merged_hit);
  }

  int fcal_total_en = 0;
  Digitize(fcal_merged_hits, fcal_total_en);

  FADC_SSP(fcal_merged_hits, fcal_ssp, 1);
  GTP(fcal_ssp, fcal_gtp, 1);


//  BCAL Part

  double BCAL_HIT_EN_SUM = 0;
  for(unsigned int i = 0; i < bcal_hits.size(); ++i) {
    double t    = bcal_hits[i]->t;
    double e    = bcal_hits[i]->E;
	  int module  = bcal_hits[i]->module;
	  int layer   = bcal_hits[i]->layer;
	  int sector  = bcal_hits[i]->sector;
	  int end     = bcal_hits[i]->end;

    bool masked = any_of(bcal_trig_mask.begin(),bcal_trig_mask.end(),
      [module, layer, sector, end](const bcal_mod& mask) {
      return mask.module == module && mask.layer == layer && mask.sector == sector && mask.end == end;});

    if(masked) continue; // skip masked channel

    BCAL_HIT_EN_SUM += e;
    BCAL_SIGNAL bcal_signal(module,layer,sector,end,e,t);

    double E  = e * BCAL_ADC_PER_MEV * 1.e3;
    SignalPulse(E, t, bcal_signal.adc_en, 2);

    bcal_signal_hits.push_back(bcal_signal);
  }


// Merge BCAL hits
  for(size_t i = 0; i < bcal_signal_hits.size(); ++i) {
    auto& base_hit = bcal_signal_hits[i];
    if(base_hit.merged) continue;

    BCAL_SIGNAL merged_hit(base_hit.module, base_hit.layer, base_hit.sector, base_hit.end, 0.0,0.0);
    merged_hit.adc_en = base_hit.adc_en;

    for(size_t j = i+1; j < bcal_signal_hits.size(); ++j) {
      auto& compare_hit = bcal_signal_hits[j];
      if(compare_hit.module == base_hit.module && compare_hit.layer == base_hit.layer &&
         compare_hit.sector == base_hit.sector && compare_hit.end   == base_hit.end) {
        compare_hit.merged = true;
        for(size_t k = 0; k < merged_hit.adc_en.size(); ++k)  merged_hit.adc_en[k] += compare_hit.adc_en[k];
      }
    }
    bcal_merged_hits.push_back(merged_hit);
  }

  int bcal_total_en;
  Digitize(bcal_merged_hits, bcal_total_en);
  FADC_SSP(bcal_merged_hits, bcal_ssp, 2);
  GTP(bcal_ssp, bcal_gtp, 2);

	DL1MCTrigger *trigger = new DL1MCTrigger;
  FindTriggers(trigger,fcal_gtp,bcal_gtp);

}

//------------------
// EndRun
//------------------
void JEventProcessor_trigtest::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_trigtest::Finish()
{
    // Called before program exit after event processing is finished.
}


void JEventProcessor_trigtest::Read_RCDB(const shared_ptr<const JEvent>& event, int print_level) {

  vector<const DTranslationTable*> ttab;
  event->Get(ttab);
  int32_t runnumber = event->GetRunNumber();
  vector<string> SectionNames = {
        "TRIGGER", "GLOBAL", "FCAL", "BCAL", "TOF", "ST", "TAGH",
        "TAGM", "PS", "PSC", "TPOL", "CDC", "FDC"};

  const char* env_conn = getenv("RCDB_CONNECTION");
  string RCDB_CONNECTION = env_conn ? env_conn : "mysql://rcdb@hallddb.jlab.org/rcdb";
  rcdb::Connection connection(RCDB_CONNECTION);

  auto rtvsCnd = connection.GetCondition(runnumber, "rtvs");
  if(!rtvsCnd)  throw JException("Read_RCDB: rtvs is not set");

  auto json = rtvsCnd->ToJsonDocument();
  string fileName = json["%(config)"].GetString();
  auto file = connection.GetFile(runnumber, fileName);
  if(!file) throw JException("Read_RCDB: missed json file");

  string fileContent = file->GetContent();
  auto result = rcdb::ConfigParser::Parse(fileContent, SectionNames);

// FCAL
  const auto& FCALSection = result.Sections["FCAL"].NameValues;

  auto it = FCALSection.find("FADC250_TRIG_THR");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_CELL_THR = max(0, stoi(it->second));

  it = FCALSection.find("FADC250_TRIG_NSB");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_NSB = stoi(it->second);

  it = FCALSection.find("FADC250_TRIG_NSA");
  if(it != FCALSection.end() && !it->second.empty())  FCAL_NSA = stoi(it->second);

// BCAL
  const auto& BCALSection = result.Sections["BCAL"].NameValues;

  it = BCALSection.find("FADC250_TRIG_THR");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_CELL_THR = max(0, stoi(it->second));

  it = BCALSection.find("FADC250_TRIG_NSB");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_NSB = stoi(it->second);

  it = BCALSection.find("FADC250_TRIG_NSA");
  if(it != BCALSection.end() && !it->second.empty())  BCAL_NSA = stoi(it->second);

// Trigger types and equations
  vector<vector<string>> triggerTypes;
  for(const auto& row : result.Sections["TRIGGER"].Rows) {
    if(row.empty()) continue;
    if(row[0] == "TRIG_TYPE") {
      if(row.size() >= 9) {
        triggerTypes.push_back(row);
      } else {
        if(print_level>-1) cout << "Read_RCDB: skipping TRIG_TYPE line: not enough parameters N = " << row.size() << endl;
      }
    }
    if(row[0] == "TRIG_EQ" && row.size() >= 5) {
      if(stoi(row[4]) == 1) { // enabled
        if(row[1] == "FCAL")    {FCAL_WINDOW = stoi(row[3]); continue;}
        if(row[1] == "BCAL_E")  {BCAL_WINDOW = stoi(row[3]); continue;}
      }
    }
  }

  triggers_enabled.clear();
  for(int bit = 0; bit<32; ++bit) {
    for(const auto& trigRow : triggerTypes) {
      if(trigRow.size()<9) continue;
      if(stoi(trigRow[8]) != bit) continue; // check if the trigger lane is enabled

      const string& trigType = trigRow[1];
      if(trigType != "BFCAL") {
        if(print_level>-1) cout << "Read_RCDB: skipping Trigger " << trigType << endl;
        continue;
      }

      trigger_conf trigger_tmp{};
      trigger_tmp.bit = bit;

      int fcal = !trigRow[4].empty() ? stoi(trigRow[4]) : 0;
      int bcal = !trigRow[5].empty() ? stoi(trigRow[5]) : 0;

      if(!fcal && !bcal) {
        if(print_level>-1) cout << "Read_RCDB: Incorrect parameters for BFCAL trigger" << endl;
        continue;
      }

      if(fcal)  trigger_tmp.type |= 0x1;
      if(bcal)  trigger_tmp.type |= 0x2;

      trigger_tmp.gtp.fcal = fcal;
      trigger_tmp.gtp.bcal = bcal;

      if(!trigRow[6].empty()) trigger_tmp.gtp.en_thr = stoi(trigRow[6]);
      if(trigRow.size()>9  && !trigRow[9].empty()  && fcal) trigger_tmp.gtp.fcal_min = stoi(trigRow[9]);
      if(trigRow.size()>10 && !trigRow[10].empty() && fcal) trigger_tmp.gtp.fcal_max = stoi(trigRow[10]);
      if(trigRow.size()>11 && !trigRow[11].empty() && bcal) trigger_tmp.gtp.bcal_min = stoi(trigRow[11]);
      if(trigRow.size()>12 && !trigRow[12].empty() && bcal) trigger_tmp.gtp.bcal_max = stoi(trigRow[12]);

      triggers_enabled.push_back(trigger_tmp);
    }
  }


  for(const string det : {"FCAL", "BCAL"}) {

    const auto& section = result.Sections[det].NameValues;

    if(print_level>1) {
      cout << "Keys in " << det << " section:\n";
      for(const auto& entry : section)  cout << "  " << entry.first << " = " << entry.second << endl;
    }

    string userDir;
    it = section.find("FADC250_USER_DIR");
    if (it != section.end()) userDir = it->second;
    else { if(print_level>-1) cout << "\n\nMissing FADC250_USER_DIR in " << det << " section\n" << endl; continue; }

    string userVer;
    it = section.find("FADC250_USER_VER");
    if (it != section.end()) userVer = it->second;
    else { if(print_level>-1) cout << "\n\nMissing FADC250_USER_VER in " << det << " section\n" << endl; continue; }

    for(int crate = 1; crate <= 12; ++crate) {

      if(det == "BCAL" && (crate == 3 || crate == 6 || crate == 9 || crate == 12))  continue; // Skip these crates for BCAL

      string userFileName = userDir + "/" + (det == "FCAL" ? "rocfcal" : "rocbcal") + to_string(crate) + "_" + userVer + ".cnf";

      if(print_level>1)
        cout << "crate " << crate << " userFileName = " << userFileName << endl;

      auto userFile = connection.GetFile(runnumber, userFileName);
      if(!userFile) continue;

      auto userParseResult = rcdb::ConfigParser::ParseWithSlots(userFile->GetContent(), "FADC250_SLOTS");

      for(unsigned int slot = 3; slot<=21; ++slot) {
        auto userValues = userParseResult.SectionsBySlotNumber[slot].NameVectors["FADC250_TRG_MASK"];
        if(userValues.empty()) continue;

        for(unsigned int ch = 0; ch<userValues.size(); ++ch) {
          if(userValues[ch].empty()) continue;
          if(stoi(userValues[ch])<=0) continue;

          uint32_t roc_id = (det == "FCAL" ? 10 : 30) + crate;
          DTranslationTable::csc_t daq_index = {roc_id, slot, ch};
          DTranslationTable::DChannelInfo channel_info;

          try {channel_info = ttab[0]->GetDetectorIndex(daq_index);} catch (...) {
            if(print_level>-1)
              cout << "Exception: " << det << " channel is not in the translation table. "
                   << "Crate = " << crate << " Slot = " << slot << " Channel = " << ch << endl;
            continue;
          }

          if(det == "FCAL") {
            fcal_mod tmp;
            tmp.roc = crate; tmp.slot = slot; tmp.ch = ch;
            tmp.row = channel_info.fcal.row; tmp.col = channel_info.fcal.col;
            fcal_trig_mask.push_back(tmp);
            if(print_level>1)  cout << "MASKED FCAL CHANNEL " << tmp.row << " " << tmp.col << endl;

          } else { // BCAL
            bcal_mod tmp;
            tmp.roc = crate; tmp.slot = slot; tmp.ch = ch;
            tmp.module = channel_info.bcal.module; tmp.layer = channel_info.bcal.layer;
            tmp.sector = channel_info.bcal.sector; tmp.end = channel_info.bcal.end;
            bcal_trig_mask.push_back(tmp);

            if(print_level>1)  cout << "MASKED BCAL CHANNEL "
                                    << "Module = " << tmp.module << ", Layer = " << tmp.layer
                                    << ", Sector = " << tmp.sector << ", End = " << tmp.end << endl;
          }
        }
      }
    }
  }

  if(print_level>0) {
    cout << "NUMBER OF MASKED FCAL CHANNELS = " << fcal_trig_mask.size()
              << ", BCAL CHANNELS = " << bcal_trig_mask.size() << endl;
    PrintTriggers();
  }

}


//------------------
// Creates a pulse shape array from the given energy and time
//------------------
void JEventProcessor_trigtest::SignalPulse(double energy, double time, vector<double>& adc_en, int det) {

// This function generates a digitized signal pulse and adds it to adc_en vector
//
// Parameters:
//   - energy: total energy deposited by the hit
//   - time:   time at which the signal occurs (in ns)
//   - adc_en: array to which the pulse shape will be added
//   - det:  1 = FCAL, 2 = BCAL (controls pulse shape parameters)
//

// Parameters for pulse shaping (exponential decay)
  float decay_constant = (det == 2) ? 0.18 : 0.358;  // BCAL uses a slower pulse

// Length (in data_samples) of the digitized pulse
  const int pulse_window = 20;

// Calculate starting data_sample index based on input time
  int first_data_sample_index = static_cast<int>(floor(time / data_time_bin));
  int start_index = first_data_sample_index + 1;
  int   end_index = start_index + pulse_window;

// Check if time is out of ADC digitization range
  if(start_index >= int(data_sample)) return;

  if(start_index < 0)  start_index = 0;
  if(end_index >= int(data_sample)) end_index = data_sample - 1;

// Add shaped pulse to adc_en

  vector<double> waveform(data_sample, 0.0);
  for(int i = start_index; i <= end_index; ++i) {
    double adc_time     = i * data_time_bin - time;  // Time since signal peak
    double pulse_value  = decay_constant * decay_constant * exp(-adc_time * decay_constant) * adc_time * data_time_bin;
    waveform[i]         = pulse_value;
  }

  double sum = accumulate(waveform.begin(), waveform.end(), 0.0);
  if(sum>0.0)
    for(int i = start_index; i <= end_index; ++i)
      adc_en[i]  += waveform[i] * energy / sum;  // add normilized signal to the existing amplitudes

}

void JEventProcessor_trigtest::PrintTriggers() {

  cout << "\n\n ------------  Trigger Settings --------------- \n\n";

  cout << "----------- FCAL -----------\n\n";
  cout << "FCAL_CELL_THR  = " << setw(10) << FCAL_CELL_THR << endl;
  cout << "FCAL_NSA       = " << setw(10) << FCAL_NSA << endl;
  cout << "FCAL_NSB       = " << setw(10) << FCAL_NSB << endl;
  cout << "FCAL_WINDOW    = " << setw(10) << FCAL_WINDOW << endl;

  cout << "\n----------- BCAL -----------\n\n";
  cout << "BCAL_CELL_THR  = " << setw(10) << BCAL_CELL_THR << endl;
  cout << "BCAL_NSA       = " << setw(10) << BCAL_NSA << endl;
  cout << "BCAL_NSB       = " << setw(10) << BCAL_NSB << endl;
  cout << "BCAL_WINDOW    = " << setw(10) << BCAL_WINDOW << endl;

  cout << "\n\n";

  if (!triggers_enabled.empty()) {
    cout << left
         << setw(10) << "TYPE"
         << setw(10) << "FCAL_E"
         << setw(10) << "BCAL_E"
         << setw(10) << "EN_THR"
         << setw(10) << "NHIT"
         << setw(10) << "LANE"
         << setw(14) << "FCAL_EMIN"
         << setw(14) << "FCAL_EMAX"
         << setw(14) << "BCAL_EMIN"
         << setw(14) << "BCAL_EMAX"
         << "PATTERN"
         << endl;
  }

  for(const auto& trig : triggers_enabled) {
    string detector = "BFCAL";  // All known types are BFCAL
    if (trig.type < 0x1 || trig.type > 0x3) {
      detector = "NONE";
      cout << " Unknown detector ===== " << trig.type << endl;
    }

    cout << left
         << setw(10) << trig.type
         << setw(10) << trig.gtp.fcal
         << setw(10) << trig.gtp.bcal
         << setw(10) << trig.gtp.en_thr
         << setw(10) << "==0"                // Placeholder for NHIT if needed
         << setw(10) << trig.bit
         << setw(14) << trig.gtp.fcal_min
         << setw(14) << trig.gtp.fcal_max
         << setw(14) << trig.gtp.bcal_min
         << setw(14) << trig.gtp.bcal_max
         << "==0x" << hex << uppercase << 0 << dec << nouppercase  // Placeholder for PATTERN
         << endl;
  }

  cout << "\n" << endl;
}

int JEventProcessor_trigtest::FindTriggers(DL1MCTrigger *trigger, const vector<int>& fcal_gtp, const vector<int>& bcal_gtp) {

  int ntriggers_found = 0;

  for(size_t i = 0; i < triggers_enabled.size(); ++i) {
    const auto& trigger_config = triggers_enabled[i];

    if(trigger_config.type != 2 && trigger_config.type != 3) continue; // BFCAL triggers for now

    int en_bit = trigger_config.bit;

    for(size_t samp = 0; samp < data_sample; ++samp) {
      int bcal_samp = static_cast<int>(samp) - BCAL_OFFSET;
      int bcal_energy = (bcal_samp >= 0 && bcal_samp < static_cast<int>(data_sample)) ? bcal_gtp[bcal_samp] : 0;
      int gtp_energy = trigger_config.gtp.bcal * bcal_energy;

      size_t trig_time_ind = 2;         // 2 == BCAL slot
      if(trigger_config.type == 3) {
        if(fcal_gtp[samp] <= trigger_config.gtp.fcal_min) continue;
        gtp_energy += trigger_config.gtp.fcal * fcal_gtp[samp];
        trig_time_ind = 0;              // 0 == FCAL slot
      }

      if(gtp_energy<trigger_config.gtp.en_thr)  continue;

      trigger->trig_mask |= (1 << en_bit);

      fill_n(trigger->trig_time, 32, -1);
      trigger->trig_time[trig_time_ind] = static_cast<int>(samp) - 25;

      trigger->fcal_gtp     = fcal_gtp[samp];                           //  FCAL GTP E(cnt)
      trigger->fcal_gtp_en  = fcal_gtp[samp]  / FCAL_ADC_PER_MEV;       //  FCAL GTP E(GeV)
      trigger->bcal_gtp     = bcal_gtp[bcal_samp];                      //  BCAL GTP E(cnt)
      trigger->bcal_gtp_en  = bcal_gtp[bcal_samp] / BCAL_ADC_PER_MEV;   //  BCAL GTP E(GeV)

      ++ntriggers_found;
      break; // Stop after first trigger condition met for this config
    }
  }

  return ntriggers_found;
}

void JEventProcessor_trigtest::Digitize(vector<FCAL_SIGNAL>& hits, int &etot) {
  etot = 0;
  for(auto& hit : hits) {
    double pedestal = fcal_pedestals[hit.row][hit.column];
    for(size_t samp = 0; samp < hit.adc_en.size(); ++samp) {
        hit.adc_en[samp] += pedestal - TRIG_BASELINE + gRandom->Gaus(0.0, PEDESTAL_SIGMA);
        hit.adc_count[samp] += static_cast<int>(hit.adc_en[samp] + TRIG_BASELINE + 0.5);
        if(hit.adc_count[samp] > max_fadc) hit.adc_count[samp] = max_fadc;
        if(hit.adc_count[samp] > TRIG_BASELINE) etot += hit.adc_count[samp] - TRIG_BASELINE;
    }
  }
}

void JEventProcessor_trigtest::Digitize(vector<BCAL_SIGNAL>& hits, int &etot) {
  etot = 0;
  for(auto& hit : hits) {
    for(size_t samp = 0; samp < hit.adc_en.size(); ++samp) {
        hit.adc_en[samp] += gRandom->Gaus(0.0, PEDESTAL_SIGMA);
        hit.adc_count[samp] += static_cast<int>(hit.adc_en[samp] + TRIG_BASELINE + 0.5);
        if(hit.adc_count[samp] > max_fadc) hit.adc_count[samp] = max_fadc;
        if(hit.adc_count[samp] > TRIG_BASELINE) etot += hit.adc_count[samp] - TRIG_BASELINE;
    }
  }
}

//------------------
//  GTP function computes a running sum over a sliding window of size integration_window on the input vector SSP, storing results in GTP
//------------------
//

void JEventProcessor_trigtest::GTP(const vector<int>& ssp, vector<int>& gtp, int det) {

  gtp.assign(data_sample, 0);
  size_t integration_window;

  switch(det) {
    case 1:
            integration_window  = size_t(FCAL_WINDOW);
            break;
    case 2:
            integration_window  = size_t(BCAL_WINDOW);
            break;
    default:
            throw JException("TP: Unknown detector type");
  }

  for (size_t sample_index = 0; sample_index < data_sample; ++sample_index) {
      size_t index_max = sample_index;
      size_t index_min = (sample_index >= integration_window) ? sample_index - integration_window : 0;

      int energy_sum = 0;
      for(size_t i = index_min; i <= index_max; ++i) energy_sum += ssp[i];
      gtp[sample_index] = energy_sum;
  }

}


template <typename T>
void JEventProcessor_trigtest::FADC_SSP(const vector<T>& merged_hits, vector<int>& ssp, int det) {

  int EN_THR, NSA, NSB;
  switch(det) {
    case 1:
      EN_THR = FCAL_CELL_THR;
      NSA    = FCAL_NSA;
      NSB    = FCAL_NSB;
      break;
    case 2:
      EN_THR = BCAL_CELL_THR;
      NSA    = BCAL_NSA;
      NSB    = BCAL_NSB;
      break;
    default:
      throw JException("FADC_SSP: Unknown detector type");
  }

  ssp.assign(data_sample,0);
  for(const auto& hit : merged_hits) {
    const vector<int>& adc = hit.adc_count;
    int sample_size = adc.size();
    int index_max = -1;
    for(int i = 0; i < sample_size; ++i) {
      if(adc[i]<EN_THR) continue;

      int index_min = i - NSB;
      if(index_min<0) index_min = 0;
      if(index_min<=index_max) index_min = index_max + 1;

      index_max = i + NSA - 1;
      if(index_max>=sample_size) index_max = sample_size - 1;

// Extend NSA window if signal continues above threshold
      while(index_max + 1<sample_size && adc[index_max + 1]>=EN_THR) {
        index_max += NSA;
        if(index_max>=sample_size) {
          index_max = sample_size - 1;
          break;
        }
      }

      for(int j = index_min; j<=index_max; ++j) // Fill SSP buffer
        if(adc[j]>TRIG_BASELINE) ssp[j] += adc[j]-TRIG_BASELINE;

      i = index_max; // Skip processed region
    }
  }
}
