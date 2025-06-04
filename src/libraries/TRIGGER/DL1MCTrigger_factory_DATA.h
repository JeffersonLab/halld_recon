#ifndef _DL1MCTrigger_factory_DATA_
#define _DL1MCTrigger_factory_DATA_

#include <JANA/JFactoryT.h>

#include "DL1MCTrigger.h"


#include "TTAB/DTranslationTable.h"

#include "FCAL/DFCALGeometry.h"
#include "ECAL/DECALGeometry.h"
#include <FCAL/DFCALHit.h>
#include <BCAL/DBCALHit.h>

#include "ANALYSIS/DTreeInterface.h"

#include <DRandom2.h>

#include <TH1.h>
#include <TH2.h>

typedef  vector< vector<double> >  fcal_constants_t;
typedef  vector< vector<double> >  ecal_constants_t;

class DL1MCTrigger_factory_DATA:public JFactoryT<DL1MCTrigger>{
	public:
		DL1MCTrigger_factory_DATA() {
			SetTag("DATA");
		};
		~DL1MCTrigger_factory_DATA(){};

		static const int  sample        =  125;
		static const int  time_stamp    =  4.;
		static const int  max_adc_bins  =  4096;        /* number of FADC bins */

		static const int FCAL_MAX_CHANNELS = 2800;

		static const int TRIG_BASELINE  = 100;
		//static const int TRIG_BASELINE  = 0;

		fcal_constants_t fcal_gains;
                fcal_constants_t fcal_pedestals;
                ecal_constants_t ecal_gains;
                ecal_constants_t ecal_pedestals;
 
		int fcal_ssp[sample];
		int fcal_gtp[sample];

		int bcal_ssp[sample];
		int bcal_gtp[sample];

	        int ecal_ssp[sample];
		int ecal_gtp[sample];

		typedef struct {
		  int row;
		  int column;
		  
		  double energy;
		  double time;
		  
		  double adc_en[sample];
		  int adc_amp[sample];
		  
		  int merged;
		} fcal_signal;

		typedef struct {
		  int module;
		  int layer;
		  int sector;
		  int end;

			int row;
			int column;

		  double time;      /* Pulse time in ns */
		  double energy;    /* Pulse energy in MeV */

		  double adc_en[sample];
		  int adc_amp[sample];
		  
		  int merged;
		  //int pedestal;
		}  bcal_signal;

	        typedef struct {
		  int row;
		  int column;
		  
		  double energy;
		  double time;
		  
		  double adc_en[sample];
		  int adc_amp[sample];
		  
		  int merged;
		} ecal_signal;
	
		typedef struct {
		  
		  int  fcal;
		  int  bcal;
		  int  ecal;
		  int  en_thr;  
    
		  int  fcal_min;
		  int  fcal_max;
		  int  bcal_min;
		  int  bcal_max;
		  int  ecal_min;
		  int  ecal_max;
			
		  int  st_nhit;
		  int  tagh_nhit;
		  int  tof_nhit;
		  int  ps_nhit;
		  
		  unsigned int st_pattern;
		  unsigned int tagh_pattern;
		  unsigned int tof_pattern;
		  unsigned int ps_pattern;		  
		} gtp_par;
		
		
		typedef struct {
		  
		  int  status;
		  
		  int  bit;
		  int  type;   // Bit pattern: bit 1:  FCAL
		  //              bit 2:  BCAL
		  //              bit 3:  ST
		  //              bit 4:  PS
		  //              bit 5:  TAGH
		  //              bit 6:  TOF
		  		  
		  gtp_par gtp;
		  
		  float  fcal_en;
		  float  bcal_en;
		  float  ecal_en;
		  float  en_thr;
		  
		  float  fcal_min_en;
		  float  fcal_max_en; 
		  float  bcal_min_en;
		  float  bcal_max_en;
		  float  ecal_min_en;
		  float  ecal_max_en;
		} trigger_conf;
				
		typedef struct{
		  int roc;
		  int slot;
		  int ch;

		  int col;
		  int row;
		} fcal_mod;

		typedef struct{
		  int roc;
		  int slot;
		  int ch;

		  int module;
		  int layer; 
		  int sector;
		  int end;
		} bcal_mod;

	        typedef struct{
		  int roc;
		  int slot;
		  int ch;

		  int col;
		  int row;
		} ecal_mod;
	
		static vector<trigger_conf> triggers_enabled;

		static vector<fcal_mod> fcal_trig_mask;
		static vector<bcal_mod> bcal_trig_mask;
	        static vector<ecal_mod> ecal_trig_mask;

		vector<fcal_signal> fcal_signal_hits;
		vector<bcal_signal> bcal_signal_hits;
	        vector<ecal_signal> ecal_signal_hits;
	
		vector<fcal_signal> fcal_merged_hits;
		vector<bcal_signal> bcal_merged_hits;
	        vector<ecal_signal> ecal_merged_hits;
	
		static std::mutex params_mutex;
		static std::mutex rcdb_mutex;
  		static bool RCDB_LOADED;
  		static bool PARAMS_LOADED;

		int    BYPASS;
		float  FCAL_ADC_PER_MEV;
		static int    FCAL_CELL_THR;
		int    FCAL_EN_SC;
		static int    FCAL_NSA;
		static int    FCAL_NSB;
		static int    FCAL_WINDOW;

		float  BCAL_ADC_PER_MEV;
		static int    BCAL_CELL_THR;
		int    BCAL_EN_SC;
		static int    BCAL_NSA;
		static int    BCAL_NSB;
		static int    BCAL_WINDOW;

	        float  ECAL_ADC_PER_MEV;
		static int    ECAL_CELL_THR;
		int    ECAL_EN_SC;
		static int    ECAL_NSA;
		static int    ECAL_NSB;
		static int    ECAL_WINDOW;
	
	        static float FCAL_GAIN;
	        static float ECAL_GAIN;
	
		int    FCAL_BCAL_EN;
		
		float  ST_ADC_PER_MEV;
		int    ST_CELL_THR;
		int    ST_NSA;
		int    ST_NSB;
		int    ST_WINDOW;
		int    ST_NHIT;
		
		int BCAL_OFFSET;
		
		int SIMU_BASELINE;
		int SIMU_GAIN;
		
		int OUTPUT_TREE;
		int USE_RAW_SAMPLES;

		//TREE
		DTreeInterface* dTreeInterface;
		//thread_local: Each thread has its own object: no lock needed
		//important: manages it's own data internally: don't want to call new/delete every event!
		static thread_local DTreeFillData dTreeFillData;

		double time_shift;
		double time_min;
		double time_max;
		
		int simu_baseline_fcal;
		int simu_baseline_bcal;
	        int simu_baseline_ecal;
		double pedestal_sigma;

		int simu_gain_fcal;
		int simu_gain_bcal;
	        int simu_gain_ecal;

		int Read_RCDB(const std::shared_ptr<const JEvent>& event, bool print_messages=true);		
		int SignalPulse(double en, double time, double amp_array[sample], int type);

		void AddBaseline(double adc_amp[sample], double pedestal, DRandom2 &gDRandom);

		void Digitize(double adc_amp[sample], int adc_count[sample]);

		template <typename T>  int FADC_SSP(vector<T> merged_hits, 
						    int detector);
		int GTP(int detector);
		int FindTriggers(DL1MCTrigger *trigger);
		void PrintTriggers();				

		float  BCAL_ADC_PER_MEV_CORRECT;
		
	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;					///< Called after last event of last event source has been processed.
		
		
		void LoadFCALConst( fcal_constants_t &table, 
				    const vector<double> &fcal_const_ch, 
				    const DFCALGeometry  &fcalGeom);
	        void LoadECALConst( ecal_constants_t &table, 
				    const vector<double> &ecal_const_ch, 
				    const DECALGeometry  &ecalGeom);


		void GetSeeds(const std::shared_ptr<const JEvent>& event, UInt_t &seed1, UInt_t &seed2, UInt_t &seed3);

		TH1F *hfcal_gains;
		TH2F *hfcal_gains2;
		TH1F *hfcal_ped;

	        TH1F *hecal_gains;
		TH2F *hecal_gains2;
		TH1F *hecal_ped;

		int debug;
		
};

#endif // _DL1MCTrigger_factory_DATA_

