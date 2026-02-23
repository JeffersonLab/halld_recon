#ifndef _DL1MCTrigger_factory_CPP_
#define _DL1MCTrigger_factory_CPP_

#include <JANA/JFactoryT.h>
#include "DL1MCTrigger.h"

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <algorithm>

class DTOFGeometry;

class DL1MCTrigger_factory_CPP : public JFactoryT<DL1MCTrigger> {

public:
    DL1MCTrigger_factory_CPP() {
        SetTag("CPP");
    }
    ~DL1MCTrigger_factory_CPP() override = default;

    static constexpr size_t data_sample = 125;
    const int  data_time_bin = 4;
    const int max_fadc =  4096;

    static const int BCAL_NUM_LAYERS   =  4;
    static const int BCAL_NUM_SECTORS  =  4;

    static const int TRIG_TYPE_TOF = 5;

    const uint32_t FCAL_mask = (1u << (1-1));
    const uint32_t  TOF_mask = (1u << (2-1));
    const uint32_t BCAL_mask = (1u << (3-1));

    int     FCAL_WINDOW, BCAL_WINDOW, TOF_WINDOW,
            FCAL_CELL_THR, FCAL_NSA, FCAL_NSB,
            BCAL_CELL_THR, BCAL_NSA, BCAL_NSB,
             TOF_CELL_THR,  TOF_NSA,  TOF_NSB,
            BCAL_OFFSET,
            FCAL_BASELINE, BCAL_BASELINE, TOF_BASELINE,
            NSAMPLES_INTEGRAL_FCAL, NSAMPLES_INTEGRAL_BCAL, NSAMPLES_INTEGRAL_TOF,
            TOF_READ_NSB, TOF_READ_NSA, TOF_NSAT, TOF_CELL_TRIG_THR,
            TOF_W_WIDTH, TOF_FE_MATCH_WINDOW, TOF_BAR_MATCH_WINDOW,
            TOF_ACCEPT_MIN_SAMP, TOF_ACCEPT_MAX_SAMP,
            FB_ACCEPT_MIN_SAMP, FB_ACCEPT_MAX_SAMP,
            BCAL_ACCEPT_MIN_SAMP, BCAL_ACCEPT_MAX_SAMP,
            BCAL_PEAK_WINDOW_MIN, BCAL_PEAK_WINDOW_MAX,
            TOF_PEAK_WINDOW_MIN, TOF_PEAK_WINDOW_MAX;

    double  FCAL_ADC_PER_MEV, BCAL_ADC_PER_MEV, PEDESTAL_SIGMA;
    bool    USE_RAW_SAMPLES;
    bool tof_lane_enabled;

    int TOF_MAX_CHANNELS; // TOF channels (initizlied by geometry)
    int TOF_NUM_PLANES;   // TOF number of planes (initialized by geometry)
    int TOF_NUM_BARS;     // TOF number of paddles (initialized by geometry)

		const int GetBCALCalibIndex(int module, int layer, int sector) const {
			return BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);
		}

    const int GetBCALCalibIndex(int module, int layer, int sector, int end) const {
      int cell = GetBCALCalibIndex(module, layer, sector);
      return 2*cell + end;  // end = 0 - upstream, = 1 - downstream
    }

    typedef  std::vector< std::vector< std::pair<double,double> > >  tof_digi_constants_t;
    std::vector<double> tof_adc2E; // TOF ADC to energy converstion values (loaded from CCDB) 
    tof_digi_constants_t tof_adc_pedestals, tof_adc_time_offsets;
    const double GetConstant(const tof_digi_constants_t &the_table, const int in_plane, const int in_bar, const int in_end) const;

    struct FCAL_SIGNAL {int row, column;  bool merged;  std::vector<double> adc;
           FCAL_SIGNAL(int row_, int col_) :
                           row(row_), column(col_), merged(false), adc(data_sample, 0.0) {}  };

    struct BCAL_SIGNAL {int module, layer, sector, end; bool merged;  std::vector<double> adc;
           BCAL_SIGNAL(int module_, int layer_, int sector_, int end_) :
                           module(module_), layer(layer_), sector(sector_), end(end_), merged(false), adc(data_sample,0.0) {} };

    struct  TOF_SIGNAL {int plane, bar, end, time;  bool merged;  std::vector<double> adc;
            TOF_SIGNAL(int plane_, int bar_, int end_, int time_) :
                           plane(plane_), bar(bar_), end(end_), time(time_), merged(false), adc(data_sample, 0.0) {}  };

    struct  TOF_FE_COINCIDENCE {int plane, bar, end, time; const TOF_SIGNAL* h1; const TOF_SIGNAL* h2;
            TOF_FE_COINCIDENCE(const TOF_SIGNAL* h1_, const TOF_SIGNAL* h2_) :
                           plane(h1_->plane), bar(h1_->bar), end(h1_->end),
                           time((h1_->time+h2_->time)/2), h1(h1_), h2(h2_) {}  };

    bool exact_equal(const std::vector<double>& a, const std::vector<double>& b) const {
      return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
    }

    void Read_RCDB(const std::shared_ptr<const JEvent>& event, int flag);
    const int GetTOFGroup(int bar, int end);
    uint32_t BuildTOFGroupBits(const std::vector<TOF_FE_COINCIDENCE> & hits, int plane, size_t sample, int dt_max_samples);
    std::vector<TOF_FE_COINCIDENCE> find_coincidences(std::vector<TOF_SIGNAL>& hits, int window);

    void Emulate_Waveform(double energy, double time, FCAL_SIGNAL& hit);
    void Emulate_Waveform(double energy, double time, BCAL_SIGNAL& hit);
    void Emulate_Waveform(double energy, double time,  TOF_SIGNAL& hit);

    void FindTriggers(DL1MCTrigger &trigger, const std::vector<int>& fcal_gtp, const std::vector<int>& bcal_gtp);
    void FillCalibTable(tof_digi_constants_t &table, std::vector<double> &raw_table, const DTOFGeometry &tofGeom);

    void GTP(const std::vector<int>& ssp, std::vector<int>& gtp, int window);
    void PrintTriggers();
    template <typename T> void FADC_SSP(std::vector<T>& merged_hits, std::vector<int>& ssp, int det);

		typedef struct {
		  int  fcal;
		  int  bcal;
		  int  en_thr;  
		  int  fcal_min;
		  int  fcal_max;
		  int  bcal_min;
		  int  bcal_max;
		  int  st_nhit    = 0;
		  int  tagh_nhit  = 0;
		  int  tof_nhit   = 0;
		  int  ps_nhit    = 0;
		  unsigned int st_pattern   = 0;
		  unsigned int tagh_pattern = 0;
		  unsigned int tof_pattern  = 0;
		  unsigned int ps_pattern   = 0;		  
		} gtp_par;

    typedef struct {int status, bit, type; gtp_par gtp;
                    float fcal_en, bcal_en, en_thr, fcal_min_en, fcal_max_en, bcal_min_en, bcal_max_en;} trigger_conf;
    typedef struct {int roc, slot, ch, col, row;} fcal_mod;
    typedef struct {int roc, slot, ch, module, layer, sector, end;} bcal_mod;

    std::vector<trigger_conf> triggers_enabled;
    std::vector<fcal_mod> fcal_trig_mask;
    std::vector<bcal_mod> bcal_trig_mask;

    std::vector<std::vector<double>>  fcal_gains, fcal_pedestals, fcal_timing_offsets, fcal_ADC_Offsets;
    std::vector<double>          bcal_gains, bcal_pedestals;
		double                  fcal_t_base, bcal_t_base, tof_t_base, fcal_a_scale, fcal_t_scale;

    void Digitize(std::vector<FCAL_SIGNAL>& hits);
    void Digitize(std::vector<BCAL_SIGNAL>& hits);
    void Digitize(std::vector<TOF_SIGNAL>& hits);


private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

};

#endif // _DL1MCTrigger_factory_CPP_
