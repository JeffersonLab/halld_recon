// CPP Trigger Factory - Specialized trigger for Charged Pion Polarizability experiment
// Adapted from JEventProcessor_trigtest.cc

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include <JANA/JEvent.h>
#include <DANA/DEvent.h>

#include "DL1MCTrigger_factory_CPP.h"

//------------------
// Init
//------------------
void DL1MCTrigger_factory_CPP::Init()
{
    auto app = GetApplication();

    FCAL_ADC_PER_MEV  = 3.73;
    app->SetDefaultParameter("TRIG:FCAL_ADC_PER_MEV", FCAL_ADC_PER_MEV, "FCAL energy calibration for the CPP Trigger");
    FCAL_CELL_THR = 165;
    app->SetDefaultParameter("TRIG:FCAL_CELL_THR", FCAL_CELL_THR, "FCAL energy threshold per cell");

    FCAL_NSA  = 10;
    app->SetDefaultParameter("TRIG:FCAL_NSA", FCAL_NSA, "FCAL NSA");
    FCAL_NSB  =  3;
    app->SetDefaultParameter("TRIG:FCAL_NSB", FCAL_NSB, "FCAL NSB");
    FCAL_WINDOW =  10;
    app->SetDefaultParameter("TRIG:FCAL_WINDOW", FCAL_WINDOW, "FCAL GTP integration window");

    BCAL_ADC_PER_MEV =  34.48276;
    app->SetDefaultParameter("TRIG:BCAL_ADC_PER_MEV", BCAL_ADC_PER_MEV, "BCAL energy calibration for the CPP Trigger");
    BCAL_CELL_THR =  120;
    app->SetDefaultParameter("TRIG:BCAL_CELL_THR", BCAL_CELL_THR, "BCAL energy threshold per cell");
    BCAL_NSA  = 19;
    app->SetDefaultParameter("TRIG:BCAL_NSA", BCAL_NSA, "BCAL NSA");
    BCAL_NSB  = 3;
    app->SetDefaultParameter("TRIG:BCAL_NSB", BCAL_NSB, "BCAL NSB");
    BCAL_WINDOW = 20;
    app->SetDefaultParameter("TRIG:BCAL_WINDOW", BCAL_WINDOW, "BCAL GTP integration window");

    BCAL_OFFSET = 2;
    app->SetDefaultParameter("TRIG:BCAL_OFFSET", BCAL_OFFSET, "Timing offset between BCAL and FCAL energies at GTP (samples)");

    NSAMPLES_INTEGRAL_FCAL = 16;
    app->SetDefaultParameter("TRIG:NSAMPLES_INTEGRAL_FCAL", NSAMPLES_INTEGRAL_FCAL, "Number of samples to integrate for FCAL normalization");
    NSAMPLES_INTEGRAL_BCAL = 27;
    app->SetDefaultParameter("TRIG:NSAMPLES_INTEGRAL_BCAL", NSAMPLES_INTEGRAL_BCAL, "Number of samples to integrate for BCAL normalization");

    USE_RAW_SAMPLES = false;
    app->SetDefaultParameter("TRIG:USE_RAW_SAMPLES", USE_RAW_SAMPLES, "Use raw waveform samples instead of emulated hits");

    PEDESTAL_SIGMA = 1.2;
    app->SetDefaultParameter("TRIG:PEDESTAL_SIGMA", PEDESTAL_SIGMA, "Pedestal sigma to emulate spread");
}

//------------------
// BeginRun
//------------------
void DL1MCTrigger_factory_CPP::BeginRun(const shared_ptr<const JEvent> &event) {

    fcal_trig_mask.clear();
    bcal_trig_mask.clear();
    triggers_enabled.clear();

    int print_level = 1;
    Read_RCDB(event, print_level);

    vector<const DFCALGeometry*> fcalGeomVect;
    event->Get(fcalGeomVect);
    if(fcalGeomVect.empty())
        throw JException("L1MCTrigger_CPP: DFCALGeometry not found (fcalGeomVect is empty)");

    const DFCALGeometry& fcalGeom = *(fcalGeomVect[0]);
    auto calibration = DEvent::GetJCalibration(event);

    vector< double > fcal_gains_ch;
    if(calibration->Get("/FCAL/gains", fcal_gains_ch))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/FCAL/gains'");
    fcal_gains.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 1.0));
    for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
        fcal_gains[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_gains_ch[ch];

    vector< double > fcal_pedestals_ch;
    if(calibration->Get("/FCAL/pedestals", fcal_pedestals_ch))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/FCAL/pedestals'");

    fcal_pedestals.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, TRIG_BASELINE));
    for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
        fcal_pedestals[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_pedestals_ch[ch];

    // Load timing offsets and ADC offsets
    vector<double> fcal_timing_offsets_ch;
    if(calibration->Get("/FCAL/timing_offsets", fcal_timing_offsets_ch))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/FCAL/timing_offsets'");
    fcal_timing_offsets.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 0.0));
    for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
        fcal_timing_offsets[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_timing_offsets_ch[ch];

    vector<double> fcal_ADC_Offsets_ch;
    if(calibration->Get("/FCAL/ADC_Offsets", fcal_ADC_Offsets_ch))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/FCAL/ADC_Offsets'");
    fcal_ADC_Offsets.resize(DFCALGeometry::kBlocksTall, vector<double>(DFCALGeometry::kBlocksWide, 0.0));
    for(int ch = 0; ch < fcalGeom.numChannels(); ++ch)
        fcal_ADC_Offsets[fcalGeom.row(ch)][fcalGeom.column(ch)] = fcal_ADC_Offsets_ch[ch];

    // Load base time offsets
    map<string,double> fcal_base_time;
    if(calibration->Get("/FCAL/base_time_offset", fcal_base_time))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/FCAL/base_time_offset'");
    fcal_t_base = fcal_base_time["t_base"];

    map<string,double> bcal_base_time;
    if(calibration->Get("/BCAL/base_time_offset", bcal_base_time))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/BCAL/base_time_offset'");
    bcal_t_base = bcal_base_time["t_base"];

    // Load digitization scales
    map<string,double> fcal_digi_scales;
    if(calibration->Get("/FCAL/digi_scales", fcal_digi_scales))
        throw JException("L1MCTrigger_CPP: failed to load calibration path '/FCAL/digi_scales'");
    fcal_a_scale = fcal_digi_scales["a_scale"];
    fcal_t_scale = fcal_digi_scales["t_scale"];
}

//------------------
// Process
//------------------
void DL1MCTrigger_factory_CPP::Process(const shared_ptr<const JEvent> &event) {

    vector<int> fcal_ssp(data_sample,0), fcal_gtp(data_sample,0),
                bcal_ssp(data_sample,0), bcal_gtp(data_sample,0);
    vector<FCAL_SIGNAL> fcal_signal_hits, fcal_merged_hits;
    vector<BCAL_SIGNAL> bcal_signal_hits, bcal_merged_hits;

    vector<const DFCALHit*>  fcal_hits;
    event->Get(fcal_hits);
    vector<const DBCALHit*>  bcal_hits;
    event->Get(bcal_hits);

    // LED event filtering - skip events with too many hits
    if(fcal_hits.size() > 150 || bcal_hits.size() > 500) {
        return;
    }

    //  FCAL Part
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

        if(masked) continue; // skip masked channels

        FCAL_SIGNAL fcal_signal(row,col);

        double gain = fcal_gains[row][col];
        double E  = e / fcal_a_scale / gain;  //  energy in ADC counts

        Emulate_Waveform(E, t, fcal_signal);
        fcal_signal_hits.push_back(fcal_signal);
    }

    // Merge FCAL hits:
    for(size_t i = 0; i < fcal_signal_hits.size(); ++i) {
        auto& base_hit = fcal_signal_hits[i];
        if(base_hit.merged) continue;

        FCAL_SIGNAL merged_hit(base_hit.row, base_hit.column);
        merged_hit.adc = base_hit.adc;

        for(size_t j = i+1; j < fcal_signal_hits.size(); ++j) {
            auto& compare_hit = fcal_signal_hits[j];
            if(compare_hit.row == base_hit.row && compare_hit.column == base_hit.column) {
                compare_hit.merged = true;
                for(size_t k = 0; k < merged_hit.adc.size(); ++k)
                    merged_hit.adc[k] += compare_hit.adc[k];
            }
        }
        fcal_merged_hits.push_back(merged_hit);
    }

    Digitize(fcal_merged_hits);

    FADC_SSP(fcal_merged_hits, fcal_ssp, 1);
    GTP(fcal_ssp, fcal_gtp, FCAL_WINDOW);

    //  BCAL Part
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

        BCAL_SIGNAL bcal_signal(module,layer,sector,end);

        double E  = e * BCAL_ADC_PER_MEV * 1.e3;
        Emulate_Waveform(E, t, bcal_signal);

        bcal_signal_hits.push_back(bcal_signal);
    }

    // Merge BCAL hits
    for(size_t i = 0; i < bcal_signal_hits.size(); ++i) {
        auto& base_hit = bcal_signal_hits[i];
        if(base_hit.merged) continue;

        BCAL_SIGNAL merged_hit(base_hit.module, base_hit.layer, base_hit.sector, base_hit.end);
        merged_hit.adc = base_hit.adc;

        for(size_t j = i+1; j < bcal_signal_hits.size(); ++j) {
            auto& compare_hit = bcal_signal_hits[j];
            if(compare_hit.module == base_hit.module && compare_hit.layer == base_hit.layer &&
               compare_hit.sector == base_hit.sector && compare_hit.end   == base_hit.end) {
                compare_hit.merged = true;
                for(size_t k = 0; k < merged_hit.adc.size(); ++k)
                    merged_hit.adc[k] += compare_hit.adc[k];
            }
        }
        bcal_merged_hits.push_back(merged_hit);
    }

    Digitize(bcal_merged_hits);
    FADC_SSP(bcal_merged_hits, bcal_ssp, 2);
    GTP(bcal_ssp, bcal_gtp, BCAL_WINDOW);

    DL1MCTrigger trigger;
    int ntriggers_found = FindTriggers(trigger, fcal_gtp, bcal_gtp);
    
    if(ntriggers_found > 0) {
        Insert(new DL1MCTrigger(trigger));
    }
}

//------------------
// EndRun
//------------------
void DL1MCTrigger_factory_CPP::EndRun()
{
    // This is called whenever the run number changes
}

//------------------
// Finish
//------------------
void DL1MCTrigger_factory_CPP::Finish()
{
    // Called before program exit after event processing is finished.
}

void DL1MCTrigger_factory_CPP::Read_RCDB(const shared_ptr<const JEvent>& event, int print_level) {

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
                if(print_level>-1) 
                    cout << "TRIG_TYPE row has insufficient columns: " << row.size() << endl;
            }
        }
        if(row[0] == "TRIG_EQ" && row.size() >= 5) {
            if(stoi(row[4]) == 1) { // enabled
                if(row[1] == "FCAL")    { continue; }
                if(row[1] == "BCAL_E")  { continue; }
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
                if(print_level>-1) 
                    cout << "Unsupported trigger type: " << trigType << endl;
                continue;
            }

            trigger_conf trigger_tmp{};
            trigger_tmp.bit = bit;

            int fcal = !trigRow[4].empty() ? stoi(trigRow[4]) : 0;
            int bcal = !trigRow[5].empty() ? stoi(trigRow[5]) : 0;

            if(!fcal && !bcal) {
                if(print_level>-1) 
                    cout << "Both FCAL and BCAL are disabled for trigger " << bit << endl;
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

    // Mask channel processing
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
                    if(userValues[ch] == "1") {
                        if(det == "FCAL") {
                            fcal_mod mask_tmp;
                            mask_tmp.roc = crate;
                            mask_tmp.slot = slot;
                            mask_tmp.ch = ch;
                            // Convert to row/col - simplified version
                            mask_tmp.row = 0; mask_tmp.col = 0; // TODO: Implement proper conversion
                            fcal_trig_mask.push_back(mask_tmp);
                        } else {
                            bcal_mod mask_tmp;
                            mask_tmp.roc = crate;
                            mask_tmp.slot = slot;
                            mask_tmp.ch = ch;
                            // Convert to module/layer/sector/end - simplified version
                            mask_tmp.module = 0; mask_tmp.layer = 0; mask_tmp.sector = 0; mask_tmp.end = 0;
                            bcal_trig_mask.push_back(mask_tmp);
                        }
                    }
                }
            }
        }
    }

    if(print_level>0) {
        cout << "CPP Trigger - NUMBER OF MASKED FCAL CHANNELS = " << fcal_trig_mask.size()
                  << ", BCAL CHANNELS = " << bcal_trig_mask.size() << endl;
        PrintTriggers();
    }
}

//------------------
// Creates a pulse shape array from the given energy and time
//------------------
void DL1MCTrigger_factory_CPP::Emulate_Waveform(double energy, double time, FCAL_SIGNAL& hit) {

    // Parameters for pulse shaping (exponential decay)
    const float decay_constant = 0.358;

    // Length (in data_samples) of the digitized pulse
    const int pulse_window = 20;

    int row = hit.row;
    int col = hit.column;

    // Apply timing offset
    double t_offset = fcal_timing_offsets[row][col] + fcal_t_base;
    double t = time * fcal_t_scale - t_offset;

    // Calculate starting data_sample index based on input time
    int first_data_sample_index = static_cast<int>(floor(t / data_time_bin));
    int start_index = first_data_sample_index + 1;
    int   end_index = start_index + pulse_window;

    // Check if time is out of ADC digitization range
    if(start_index >= int(data_sample)) return;

    if(start_index < 0)  start_index = 0;
    if(end_index >= int(data_sample)) end_index = data_sample - 1;

    // Add shaped pulse to adc
    vector<double> waveform(data_sample, 0.0);
    for(int i = start_index; i <= end_index; ++i) {
        double adc_time     = i * data_time_bin - t;  // Time since signal peak
        double pulse_value  = decay_constant * decay_constant * exp(-adc_time * decay_constant) * adc_time * data_time_bin;
        waveform[i]         = pulse_value;
    }

    // Normalize to NSAMPLES_INTEGRAL_FCAL
    double sum = 0.0;
    for(int i = start_index; i <= min<int>(start_index + NSAMPLES_INTEGRAL_FCAL - 1, end_index); ++i)
        sum += waveform[i];

    if(sum>0.0)
        for(int i = start_index; i <= end_index; ++i)
            hit.adc[i]  += waveform[i] * energy / sum;  // add normalized signal to the existing amplitudes
}

void DL1MCTrigger_factory_CPP::Emulate_Waveform(double energy, double time, BCAL_SIGNAL& hit) {

    // Parameters for pulse shaping (exponential decay)
    const float decay_constant = 0.18;  // BCAL uses a slower pulse

    // Length (in data_samples) of the digitized pulse
    const int pulse_window = 20;

    // Apply timing offset
    double t = time - bcal_t_base;

    // Calculate starting data_sample index based on input time
    int first_data_sample_index = static_cast<int>(floor(t / data_time_bin));
    int start_index = first_data_sample_index + 1;
    int   end_index = start_index + pulse_window;

    // Check if time is out of ADC digitization range
    if(start_index >= int(data_sample)) return;

    if(start_index < 0)  start_index = 0;
    if(end_index >= int(data_sample)) end_index = data_sample - 1;

    // Add shaped pulse to adc
    vector<double> waveform(data_sample, 0.0);
    for(int i = start_index; i <= end_index; ++i) {
        double adc_time     = i * data_time_bin - t;  // Time since signal peak
        double pulse_value  = decay_constant * decay_constant * exp(-adc_time * decay_constant) * adc_time * data_time_bin;
        waveform[i]         = pulse_value;
    }

    // Normalize to NSAMPLES_INTEGRAL_BCAL
    double sum = 0.0;
    for(int i = start_index; i <= min<int>(start_index + NSAMPLES_INTEGRAL_BCAL - 1, end_index); ++i)
        sum += waveform[i];

    if(sum>0.0)
        for(int i = start_index; i <= end_index; ++i)
            hit.adc[i]  += waveform[i] * energy / sum;  // add normalized signal to the existing amplitudes
}

void DL1MCTrigger_factory_CPP::PrintTriggers() {

    cout << "\n\n ------------  CPP Trigger Settings --------------- \n\n";

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

int DL1MCTrigger_factory_CPP::FindTriggers(DL1MCTrigger &trigger, const vector<int>& fcal_gtp, const vector<int>& bcal_gtp) {

    int ntriggers_found = 0;

    // Find max BCAL sample for timing window check
    int max_bcal_samp = 0;
    int max_bcal_energy = 0;
    for(size_t i = 0; i < bcal_gtp.size(); ++i) {
        if(bcal_gtp[i] > max_bcal_energy) {
            max_bcal_energy = bcal_gtp[i];
            max_bcal_samp = i;
        }
    }

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
            } else if(trigger_config.type == 2) {
                // BCAL-only trigger: check timing window
                if(max_bcal_samp < 30 || max_bcal_samp > 70) continue;
            }

            if(gtp_energy<trigger_config.gtp.en_thr)  continue;

            trigger.trig_mask |= (1 << en_bit);

            fill_n(trigger.trig_time, 32, -1);
            trigger.trig_time[trig_time_ind] = static_cast<int>(samp) - 25;

            trigger.fcal_gtp     = fcal_gtp[samp];                           //  FCAL GTP E(cnt)
            trigger.fcal_gtp_en  = fcal_gtp[samp]  / FCAL_ADC_PER_MEV;       //  FCAL GTP E(GeV)
            trigger.bcal_gtp     = bcal_gtp[bcal_samp];                      //  BCAL GTP E(cnt)
            trigger.bcal_gtp_en  = bcal_gtp[bcal_samp] / BCAL_ADC_PER_MEV;   //  BCAL GTP E(GeV)

            ++ntriggers_found;
            break; // Stop after first trigger condition met for this config
        }
    }

    return ntriggers_found;
}

void DL1MCTrigger_factory_CPP::Digitize(vector<FCAL_SIGNAL>& hits) {
    for(auto& hit : hits) {
        double pedestal = fcal_pedestals[hit.row][hit.column];
        double adc_offset = fcal_ADC_Offsets[hit.row][hit.column];
        for(size_t samp = 0; samp < hit.adc.size(); ++samp) {
            hit.adc[samp] += adc_offset + pedestal - TRIG_BASELINE + gRandom->Gaus(0.0, PEDESTAL_SIGMA);
            int adc_int = static_cast<int>(hit.adc[samp] + TRIG_BASELINE + 0.5);
            if(adc_int > max_fadc) adc_int = max_fadc;
            hit.adc[samp] = adc_int;
        }
    }
}

void DL1MCTrigger_factory_CPP::Digitize(vector<BCAL_SIGNAL>& hits) {
    for(auto& hit : hits) {
        for(size_t samp = 0; samp < hit.adc.size(); ++samp) {
            hit.adc[samp] += gRandom->Gaus(0.0, PEDESTAL_SIGMA);
            int adc_int = static_cast<int>(hit.adc[samp] + TRIG_BASELINE + 0.5);
            if(adc_int > max_fadc) adc_int = max_fadc;
            hit.adc[samp] = adc_int;
        }
    }
}

//------------------
//  GTP function computes a running sum over a sliding window on the input vector SSP, storing results in GTP
//------------------
void DL1MCTrigger_factory_CPP::GTP(const vector<int>& ssp, vector<int>& gtp, int window) {

    gtp.assign(data_sample, 0);
    size_t integration_window = size_t(window);

    for (size_t sample_index = 0; sample_index < data_sample; ++sample_index) {
        size_t index_max = sample_index;
        size_t index_min = (sample_index >= integration_window) ? sample_index - integration_window : 0;

        int energy_sum = 0;
        for(size_t i = index_min; i <= index_max; ++i) energy_sum += ssp[i];
        gtp[sample_index] = energy_sum;
    }
}

template <typename T>
void DL1MCTrigger_factory_CPP::FADC_SSP(vector<T>& merged_hits, vector<int>& ssp, int det) {

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
    for(auto& hit : merged_hits) {
        vector<double>& adc = hit.adc;
        int sample_size = adc.size();
        int index_max = -1;
        
        for(int i = 0; i < sample_size; ++i) {
            if(adc[i] > EN_THR) {
                index_max = i;
                break;
            }
        }
        
        if(index_max == -1) continue;
        
        int start_index = max(0, index_max - NSB);
        int end_index = min(sample_size - 1, index_max + NSA);
        
        for(int i = start_index; i <= end_index; ++i) {
            if(adc[i] > TRIG_BASELINE) {
                ssp[i] += static_cast<int>(adc[i]) - TRIG_BASELINE;
            }
        }
    }
}