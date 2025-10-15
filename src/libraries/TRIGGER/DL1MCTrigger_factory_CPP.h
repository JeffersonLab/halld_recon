#ifndef _DL1MCTrigger_factory_CPP_
#define _DL1MCTrigger_factory_CPP_

#include <JANA/JFactoryT.h>
#include "DL1MCTrigger.h"
#include "TTAB/DTranslationTable.h"
#include "FCAL/DFCALGeometry.h"
#include <FCAL/DFCALHit.h>
#include <BCAL/DBCALHit.h>
#include <DRandom2.h>

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
                BCAL_CELL_THR, BCAL_NSA, BCAL_NSB, FCAL_BCAL_EN, BCAL_OFFSET;
        double  FCAL_ADC_PER_MEV, BCAL_ADC_PER_MEV, BCAL_ADC_PER_MEV_CORRECT, PEDESTAL_SIGMA,
                TIME_MIN, TIME_MAX;
        bool    USE_RAW_SAMPLES;

        const int max_fadc = 4096;
        static const int TRIG_BASELINE = 100;

        void Read_RCDB(const shared_ptr<const JEvent>& event, int flag);
        void SignalPulse(double energy, double time, vector<double>& adc_en, int det);
        int FindTriggers(DL1MCTrigger *trigger, const vector<int>& fcal_gtp, const vector<int>& bcal_gtp);

        void GTP(const vector<int>& ssp, vector<int>& gtp, int det);
        void PrintTriggers();
        template <typename T> void FADC_SSP(const vector<T>& merged_hits, vector<int>& ssp, int det);

        struct FCAL_SIGNAL {
            int row, column; 
            vector<int> adc_count;
            bool merged;  
            double energy, time; 
            vector<double> adc_en;
            FCAL_SIGNAL(int row_, int col_, double energy_, double time_) :
                            row(row_), column(col_), adc_count(data_sample, 0), merged(false),
                            energy(energy_), time(time_), adc_en(data_sample, 0.0) {}
        };

        struct BCAL_SIGNAL {
            int module, layer, sector, end; 
            vector<int> adc_count;
            bool merged;  
            double energy, time; 
            vector<double> adc_en;
            BCAL_SIGNAL(int module_, int layer_, int sector_, int end_, double energy_, double time_) :
               module(module_), layer(layer_), sector(sector_), end(end_), adc_count(data_sample, 0), merged(false),
               energy(energy_), time(time_), adc_en(data_sample, 0.0) {}
        };

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

        vector<vector<double>> fcal_gains, fcal_pedestals;

        void Digitize(vector<FCAL_SIGNAL>& hits, int &etot);
        void Digitize(vector<BCAL_SIGNAL>& hits, int &etot);

        vector<int> running_sum(const vector<int>& src, int nsb, int nsa) {
            vector<int> result(src.size(), 0);
            if(src.empty()) return result;
            for(size_t i = 0; i < src.size(); ++i) {
                int start = max<int>(0, static_cast<int>(i) - nsb);
                int end   = min<int>(src.size(), static_cast<int>(i) + nsa + 1);
                result[i] = accumulate(src.begin() + start, src.begin() + end, 0);
            }
            return result;
        }

    private:
        void Init() override;
        void BeginRun(const shared_ptr<const JEvent>& event) override;
        void Process(const shared_ptr<const JEvent>& event) override;
        void EndRun() override;
        void Finish() override;
};

#endif // _DL1MCTrigger_factory_CPP_