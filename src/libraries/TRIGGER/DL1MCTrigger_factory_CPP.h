#ifndef _DL1MCTrigger_factory_CPP_
#define _DL1MCTrigger_factory_CPP_

#include <JANA/JFactoryT.h>
#include "DL1MCTrigger.h"
#include "TTAB/DTranslationTable.h"
#include "FCAL/DFCALGeometry.h"
#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALDigiHit.h>
#include <BCAL/DBCALHit.h>
#include <BCAL/DBCALDigiHit.h>
#include <DRandom2.h>
#include <DAQ/Df250WindowRawData.h>

#include <numeric>
#include <algorithm>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#include <RCDB/Connection.h>
#pragma GCC diagnostic pop
#include "RCDB/ConfigParser.h"

class DL1MCTrigger_factory_CPP : public JFactoryT<DL1MCTrigger> {
    public:
        DL1MCTrigger_factory_CPP() {
            SetTag("CPP");
        };
        ~DL1MCTrigger_factory_CPP() {};

        static const size_t data_sample = 125;
        const int  data_time_bin = 4;

        int     FCAL_WINDOW, BCAL_WINDOW, FCAL_CELL_THR, FCAL_NSA, FCAL_NSB,
                BCAL_CELL_THR, BCAL_NSA, BCAL_NSB, BCAL_OFFSET,
                NSAMPLES_INTEGRAL_FCAL, NSAMPLES_INTEGRAL_BCAL;
        double  FCAL_ADC_PER_MEV, BCAL_ADC_PER_MEV, PEDESTAL_SIGMA;
        bool    USE_RAW_SAMPLES;

        const int max_fadc = 4096;
        static const int TRIG_BASELINE = 100;

        static const int BCAL_NUM_MODULES  = 48;
        static const int BCAL_NUM_LAYERS   =  4;
        static const int BCAL_NUM_SECTORS  =  4;
        static const int BCAL_NUM_ENDS     =  2;
        static const int BCAL_MAX_CHANNELS =  1536;
        static const int FCAL_MAX_CHANNELS =  2800;

        const int GetBCALCalibIndex(int module, int layer, int sector) const {
            return BCAL_NUM_LAYERS*BCAL_NUM_SECTORS*(module-1) + BCAL_NUM_SECTORS*(layer-1) + (sector-1);
        }

        const int GetBCALCalibIndex(int module, int layer, int sector, int end) const {
            int cell = GetBCALCalibIndex(module, layer, sector);
            return 2*cell + end;  // end = 0 - upstream, = 1 - downstream
        }

        struct FCAL_SIGNAL {int row, column;  bool merged;  vector<double> adc;
               FCAL_SIGNAL(int row_, int col_) :
                               row(row_), column(col_), merged(false), adc(data_sample, 0.0) {}  };

        struct BCAL_SIGNAL {int module, layer, sector, end; bool merged;  vector<double> adc;
               BCAL_SIGNAL(int module_, int layer_, int sector_, int end_) :
                               module(module_), layer(layer_), sector(sector_), end(end_), merged(false), adc(data_sample,0.0) {} };

        void Read_RCDB(const shared_ptr<const JEvent>& event, int flag);
        void Emulate_Waveform(double energy, double time, FCAL_SIGNAL& hit);
        void Emulate_Waveform(double energy, double time, BCAL_SIGNAL& hit);
        int FindTriggers(DL1MCTrigger &trigger, const vector<int>& fcal_gtp, const vector<int>& bcal_gtp);

        void GTP(const vector<int>& ssp, vector<int>& gtp, int window);
        void PrintTriggers();
        template <typename T> void FADC_SSP(vector<T>& merged_hits, vector<int>& ssp, int det);

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

        typedef struct {
            int status, bit, type; 
            gtp_par gtp;
            float fcal_en, bcal_en, en_thr, fcal_min_en, fcal_max_en, bcal_min_en, bcal_max_en;
        } trigger_conf;

        typedef struct {
            int roc, slot, ch, col, row;
        } fcal_mod;

        typedef struct {
            int roc, slot, ch, module, layer, sector, end;
        } bcal_mod;

        vector<trigger_conf> triggers_enabled;
        vector<fcal_mod> fcal_trig_mask;
        vector<bcal_mod> bcal_trig_mask;

        vector<vector<double>>  fcal_gains, fcal_pedestals, fcal_timing_offsets, fcal_ADC_Offsets;
        vector<double>          bcal_gains, bcal_pedestals;
        double                  fcal_t_base, bcal_t_base, fcal_a_scale, fcal_t_scale;

        void Digitize(vector<FCAL_SIGNAL>& hits);
        void Digitize(vector<BCAL_SIGNAL>& hits);

    private:
        void Init() override;
        void BeginRun(const shared_ptr<const JEvent>& event) override;
        void Process(const shared_ptr<const JEvent>& event) override;
        void EndRun() override;
        void Finish() override;
};

#endif // _DL1MCTrigger_factory_CPP_